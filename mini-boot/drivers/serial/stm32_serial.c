#include <asm/io.h>
#include <clk.h>
#include <errno.h>
#include <serial.h>
#include <dm/device.h>
#include <dm/driver.h>
#include <platdata/stm32_serial.h>


/* STM32 USART register layout */
#define USART_SR     0x00
#define USART_DR     0x04
#define USART_BRR    0x08
#define USART_CR1    0x0C

#define USART_SR_RXNE    BIT(5) /* Receive data register not empty */
#define USART_SR_TXE     BIT(7) /* Transmit data register empty */

#define USART_CR1_RE     BIT(2)  /* Receiver enable */
#define USART_CR1_TE     BIT(3)  /* Transmitter enable */
#define USART_CR1_UE     BIT(13) /* USART enable */


/**
 * Turn on the UART's clock, program the baud divider, then enable the UART
 * with its transmitter and receiver. Word length, parity and stop bits stay
 * at their reset values, which give 8N1. A baud rate of 0 is rejected
 * because the divider calculation would divide by zero.
 */
static int stm32_serial_probe(
    struct udevice *dev
)
{
    const struct stm32_serial_plat *plat = dev->plat;
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

    /* With 16x oversampling (the reset default) BRR is simply the clock
     * divider, clk_rate / baudrate, rounded to the nearest integer.
     */
    writel((clk_rate + baudrate / 2) / baudrate, base + USART_BRR);
    writel(USART_CR1_UE | USART_CR1_TE | USART_CR1_RE, base + USART_CR1);

    return SUCCESS;
}


/**
 * Wait until the transmit data register is empty, then write the character
 * to it.
 */
static int stm32_serial_putc(
    struct udevice *dev,
    char ch
)
{
    const struct stm32_serial_plat *plat = dev->plat;

    while ( !(readl(plat->base + USART_SR) & USART_SR_TXE) )
        ;
    writel((u8)ch, plat->base + USART_DR);

    return SUCCESS;
}


/**
 * RXNE is set while a received byte is waiting in DR.
 */
static int stm32_serial_tstc(
    struct udevice *dev
)
{
    const struct stm32_serial_plat *plat = dev->plat;
    
    return (readl(plat->base + USART_SR) & USART_SR_RXNE) != 0;
}


/**
 * Wait for a received byte and read it. Reading DR also clears RXNE.
 */
static int stm32_serial_getc(
    struct udevice *dev
)
{
    const struct stm32_serial_plat *plat = dev->plat;
    
    while ( !stm32_serial_tstc(dev) )
        ;
    return (u8)readl(plat->base + USART_DR);
}


/**
 * This driver's implementation of the serial uclass operations.
 */
static const struct serial_ops stm32_serial_ops = {
    .putc = &stm32_serial_putc,
    .getc = &stm32_serial_getc,
    .tstc = &stm32_serial_tstc
};


/**
 * Driver model entry. No private data: everything the driver needs is in
 * the device's platform data.
 */
DM_DRIVER(stm32_serial) = {
    .name = "stm32_serial",
    .id   = UCLASS_SERIAL,

    .probe = &stm32_serial_probe,

    .ops = &stm32_serial_ops,
};
