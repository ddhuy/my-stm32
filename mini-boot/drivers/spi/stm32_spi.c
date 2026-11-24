#include <asm/io.h>
#include <clk.h>
#include <errno.h>
#include <gpio.h>
#include <spi.h>
#include <dm/device.h>
#include <dm/driver.h>
#include <dm/uclass.h>
#include <platdata/stm32_spi.h>


/*
 * STM32 SPI, master only, polled, 8-bit frames. Register layout from
 * RM0090 (SPI), offsets from plat->base.
 */
#define SPI_CR1             0x00
#define SPI_SR              0x08
#define SPI_DR              0x0C

#define SPI_CR1_CPHA        BIT(0)
#define SPI_CR1_CPOL        BIT(1)
#define SPI_CR1_MSTR        BIT(2)
#define SPI_CR1_BR_SHIFT    3
#define SPI_CR1_BR_MAX      7       /* PCLK / 256 */
#define SPI_CR1_SPE         BIT(6)
#define SPI_CR1_SSI         BIT(8)
#define SPI_CR1_SSM         BIT(9)

#define SPI_SR_RXNE         BIT(0)
#define SPI_SR_TXE          BIT(1)
#define SPI_SR_BSY          BIT(7)

/*
 * Software slave management, with the internal NSS held high: the
 * peripheral never sees a mode fault, and chip select is a GPIO.
 */
#define SPI_CR1_MASTER      (SPI_CR1_MSTR | SPI_CR1_SSM | SPI_CR1_SSI)

/* This driver has one chip select line */
#define STM32_SPI_CS_COUNT  1

/* Sent when the caller has nothing to send (dout NULL) */
#define SPI_FILL_BYTE       0x00

/*
 * A byte takes at most 256 * 8 PCLK cycles at the slowest divider; the
 * loop allows far more. A timeout means the peripheral is not running.
 */
#define SPI_WAIT_LOOPS      100000


/**
 * Per-device data: what probe looked up, kept for every transfer.
 */
struct stm32_spi_priv
{
    struct udevice *cs_dev; /* the CS GPIO port */
    u32 clk_hz;             /* PCLK */
};


/**
 * Wait until the status bits 'mask' read 'val'.
 *
 * Returns 0, or -ETIMEDOUT after SPI_WAIT_LOOPS reads.
 */
static int stm32_spi_wait(
    uintptr_t base,
    u32 mask,
    u32 val
)
{
    u32 loops = SPI_WAIT_LOOPS;

    while ((readl(base + SPI_SR) & mask) != val)
        if (--loops == 0)
            return -ETIMEDOUT;

    return SUCCESS;
}


/**
 * Turn the clock on, find the chip select port and drive CS high
 * (deselected) before anything else, then make the peripheral a master.
 * The mode and the speed belong to each slave, so they are set at the
 * start of each transfer, not here.
 *
 * Returns 0; the clock provider's or the GPIO port's error; -EINVAL if
 * the clock rate is 0.
 */
static int stm32_spi_probe(
    struct udevice *dev
)
{
    const struct stm32_spi_plat *plat = dev->plat;
    struct stm32_spi_priv *priv = dev->priv;
    struct clk clk;
    int ret;

    ret = clk_get_by_name(plat->clk_dev, plat->clk_id, &clk);
    if (ret)
        return ret;

    ret = clk_enable(&clk);
    if (ret)
        return ret;

    /* The rate comes from the provider; the divider is computed from it */
    priv->clk_hz = clk_get_rate(&clk);
    if (!priv->clk_hz)
        return -EINVAL;

    /* Probes the port on first use, as clk_get_by_name() probes the RCC */
    ret = uclass_get_device_by_name(UCLASS_GPIO, plat->cs_gpio_dev,
                                    &priv->cs_dev);
    if (ret)
        return ret;

    /* Deselected first: the slave must not see a transfer start */
    ret = gpio_direction_output(priv->cs_dev, plat->cs_pin, 1);
    if (ret)
        return ret;

    /* Master, software NSS; SPE stays clear until a transfer begins */
    writel(SPI_CR1_MASTER, plat->base + SPI_CR1);

    return SUCCESS;
}


/**
 * CR1 for one slave: its mode, and the smallest divider (2^(BR+1)) that
 * brings PCLK to its max_hz or below. The SPI_* mode bits are translated
 * one by one: that they match CR1 here is a coincidence.
 *
 * Returns 0 and the value in '*cr1', or -EINVAL if even PCLK / 256 is
 * faster than the slave allows.
 */
static int stm32_spi_cr1(
    u32 clk_hz,
    const struct spi_slave *slave,
    u32 *cr1
)
{
    u32 br;

    for (br = 0; br <= SPI_CR1_BR_MAX; br++)
        if ((clk_hz >> (br + 1)) <= slave->max_hz)
            break;
    if (br > SPI_CR1_BR_MAX)
        return -EINVAL;

    *cr1 = SPI_CR1_MASTER | (br << SPI_CR1_BR_SHIFT);
    if (slave->mode & SPI_CPOL)
        *cr1 |= SPI_CR1_CPOL;
    if (slave->mode & SPI_CPHA)
        *cr1 |= SPI_CR1_CPHA;

    return SUCCESS;
}


/**
 * Shift 'len' bytes out from 'dout' and in to 'din' (either may be NULL).
 * SPI_XFER_BEGIN sets the bus up for this slave and pulls its CS low;
 * SPI_XFER_END waits for the last bit and raises CS. A register read is
 * two calls: the address with BEGIN, the data with END.
 *
 * Returns 0; -EINVAL for a chip select this bus does not have or a
 * slave too slow for any divider; -ETIMEDOUT if the peripheral stops (CS
 * is then raised).
 */
static int stm32_spi_xfer(
    struct udevice *bus,
    const struct spi_slave *slave,
    const u8 *dout,
    u8 *din,
    size_t len,
    u32 flags
)
{
    const struct stm32_spi_plat *plat = bus->plat;
    struct stm32_spi_priv *priv = bus->priv;
    int ret;

    if (slave->cs >= STM32_SPI_CS_COUNT)
        return -EINVAL;

    if (flags & SPI_XFER_BEGIN) {
        u32 cr1;

        ret = stm32_spi_cr1(priv->clk_hz, slave, &cr1);
        if (ret)
            return ret;

        /* CR1 must not change while SPE is set: write it, then enable */
        writel(cr1, plat->base + SPI_CR1);
        writel(cr1 | SPI_CR1_SPE, plat->base + SPI_CR1);

        ret = gpio_set_value(priv->cs_dev, plat->cs_pin, 0);
        if (ret)
            return ret;
    }

    for (size_t i = 0; i < len; i++) {
        ret = stm32_spi_wait(plat->base, SPI_SR_TXE, SPI_SR_TXE);
        if (ret)
            goto deselect;

        writel(dout ? dout[i] : SPI_FILL_BYTE, plat->base + SPI_DR);

        ret = stm32_spi_wait(plat->base, SPI_SR_RXNE, SPI_SR_RXNE);
        if (ret)
            goto deselect;

        /* Read every byte, wanted or not: an unread one stays in DR */
        u8 in = readl(plat->base + SPI_DR);
        if (din)
            din[i] = in;
    }

    if (!(flags & SPI_XFER_END))
        return SUCCESS;

    /* TXE only means the buffer is empty; BSY clears after the last bit */
    ret = stm32_spi_wait(plat->base, SPI_SR_BSY, 0);

deselect:
    gpio_set_value(priv->cs_dev, plat->cs_pin, 1);
    return ret;
}


/**
 * This driver's implementation of the SPI uclass operations.
 */
static const struct spi_ops stm32_spi_ops = {
    .xfer = &stm32_spi_xfer
};


/**
 * Driver model entry. priv holds the CS port and the clock rate.
 */
DM_DRIVER(stm32_spi) = {
    .name = "stm32_spi",
    .id   = UCLASS_SPI,

    .probe = &stm32_spi_probe,

    .priv_auto = sizeof(struct stm32_spi_priv),

    .ops = &stm32_spi_ops
};
