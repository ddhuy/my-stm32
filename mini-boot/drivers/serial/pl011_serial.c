#include <asm/io.h>
#include <clk.h>
#include <errno.h>
#include <serial.h>
#include <dm/device.h>
#include <dm/driver.h>
#include <platdata/pl011_serial.h>
#include <dm/read.h>


/*
 * ARM PrimeCell UART (PL011) register layout, from the PL011 Technical
 * Reference Manual (ARM DDI 0183). Offsets from plat->base.
 */
#define PL011_DR        0x000
#define PL011_FR        0x018
#define PL011_IBRD      0x024
#define PL011_FBRD      0x028
#define PL011_LCR_H     0x02C
#define PL011_CR        0x030

#define PL011_FR_RXFE   BIT(4)  /* Receive FIFO empty */
#define PL011_FR_TXFF   BIT(5)  /* Transmit FIFO full */

#define PL011_LCR_H_FEN         BIT(4)  /* FIFOs enable */
#define PL011_LCR_H_WLEN_SHIFT  5
#define PL011_LCR_H_WLEN_8      (0x3 << PL011_LCR_H_WLEN_SHIFT)

#define PL011_CR_UARTEN BIT(0)  /* UART enable */
#define PL011_CR_TXE    BIT(8)  /* Transmit enable */
#define PL011_CR_RXE    BIT(9)  /* Receive enable */

/*
 * Baud divisor = clk / (16 * baud) = IBRD + FBRD / 64. It is computed once
 * in 64ths: the low 6 bits are FBRD, the rest is IBRD.
 */
#define PL011_FBRD_BITS 6
#define PL011_FBRD_MASK ((1 << PL011_FBRD_BITS) - 1)


/**
 * Get the UART's clock from its provider (which probes the provider on
 * first use), turn it on, then program the UART in the order the TRM
 * requires: disable, write the divisor, write LCR_H (the write that
 * latches IBRD and FBRD), enable. 8 data bits, no parity, 1 stop bit,
 * FIFOs on. A baud rate or a clock rate of 0 is rejected; an error from
 * the clock provider is passed on.
 */
static int pl011_serial_probe(
    struct udevice *dev
)
{
    const struct pl011_serial_plat *plat = dev->plat;
    struct clk clk;
    int ret;

    if (plat->baudrate == 0)
        return -EINVAL;

    ret = clk_get_by_name(plat->clk_dev, plat->clk_id, &clk);
    if (ret)
        return ret;

    ret = clk_enable(&clk);
    if (ret)
        return ret;

    uintptr_t base = plat->base;
    u32 clk_rate = clk_get_rate(&clk);
    u32 baudrate = plat->baudrate;

    if (clk_rate == 0)
        return -EINVAL;

    /*
     * 64 * clk / (16 * baud) = 4 * clk / baud, rounded to the nearest. 4 *
     * clk fits in a u32 up to about 1 GHz of UART clock.
     */
    u32 div64 = (4 * clk_rate + baudrate / 2) / baudrate;

    /* Settings must not change while the UART is enabled */
    writel(0, base + PL011_CR);

    writel(div64 >> PL011_FBRD_BITS, base + PL011_IBRD);
    writel(div64 & PL011_FBRD_MASK, base + PL011_FBRD);
    writel(PL011_LCR_H_WLEN_8 | PL011_LCR_H_FEN, base + PL011_LCR_H);

    writel(PL011_CR_UARTEN | PL011_CR_TXE | PL011_CR_RXE, base + PL011_CR);

    return SUCCESS;
}


/**
 * Wait while the transmit FIFO is full, then write the character to it.
 * The opposite test from the STM32 (TXE: wait until empty).
 */
static int pl011_serial_putc(
    struct udevice *dev,
    char ch
)
{
    const struct pl011_serial_plat *plat = dev->plat;

    while (readl(plat->base + PL011_FR) & PL011_FR_TXFF)
        ;
    writel((u8)ch, plat->base + PL011_DR);

    return SUCCESS;
}


/**
 * A byte is waiting when the receive FIFO is not empty (RXFE clear).
 */
static int pl011_serial_tstc(
    struct udevice *dev
)
{
    const struct pl011_serial_plat *plat = dev->plat;

    return !(readl(plat->base + PL011_FR) & PL011_FR_RXFE);
}


/**
 * Wait for a received byte and read it. Bits 11:8 of DR are error flags,
 * so only the low byte is returned.
 */
static int pl011_serial_getc(
    struct udevice *dev
)
{
    const struct pl011_serial_plat *plat = dev->plat;

    while (!pl011_serial_tstc(dev))
        ;
    return (u8)readl(plat->base + PL011_DR);
}


/**
 * This driver's implementation of the serial uclass operations.
 */
static const struct serial_ops pl011_serial_ops = {
    .putc = &pl011_serial_putc,
    .getc = &pl011_serial_getc,
    .tstc = &pl011_serial_tstc
};


/**
 * The UART's node into its plat: base, clock and current-speed (the baud
 * rate; required, probe refuses 0).
 *
 * Returns 0 or the error from reading the node.
 */
static int pl011_serial_of_to_plat(
    struct udevice *dev
)
{
    struct pl011_serial_plat *plat = dev->plat;
    int ret = dev_read_addr_index(dev, 0, &plat->base);

    if (ret)
        return ret;

    ret = clk_read_by_index(dev, 0, &plat->clk_dev, &plat->clk_id);
    if (ret)
        return ret;

    return dev_read_u32(dev, "current-speed", &plat->baudrate);
}


/**
 * The compatible strings this driver handles (Linux's for the PL011).
 */
static const struct udevice_id pl011_serial_ids[] = {
    { .compatible = "arm,pl011" },
    { }
};

/**
 * Driver model entry. No private data: everything the driver needs is in
 * the device's platform data.
 */
DM_DRIVER(pl011_serial) = {
    .name = "pl011_serial",
    .id   = UCLASS_SERIAL,
    .of_match = pl011_serial_ids,

    .of_to_plat = pl011_serial_of_to_plat,
    .plat_auto  = sizeof(struct pl011_serial_plat),

    .probe = &pl011_serial_probe,

    .ops = &pl011_serial_ops,
};
