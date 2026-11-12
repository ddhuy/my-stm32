#include <asm/io.h>
#include <serial.h>


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
 * 
 */
void serial_init(
    uintptr_t base,
    u32 clk_rate,
    u32 baudrate
)
{
    /* With 16x oversampling (the reset default) BRR is simply the clock
     * divider, clk_rate / baudrate, rounded to the nearest integer.
     */
    writel((clk_rate + baudrate / 2) / baudrate, base + USART_BRR);
    writel(USART_CR1_UE | USART_CR1_TE | USART_CR1_RE, base + USART_CR1);
}


/**
 * 
 */
void serial_putc(
    uintptr_t base,
    char ch
)
{
    while ( !(readl(base + USART_SR) & USART_SR_TXE) )
        ;
    writel((u8)ch, base + USART_DR);
}


/**
 * 
 */
int serial_getc(
    uintptr_t base
)
{
    while ( !serial_tstc(base) )
        ;
    return (u8)readl(base + USART_DR);
}


/**
 * 
 */
int serial_tstc(
    uintptr_t base
)
{
    return (readl(base + USART_SR) & USART_SR_RXNE) != 0;
}
