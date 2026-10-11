#include <stdint.h>

#include "soc/stm32f429/usart_regs.h"

#include "drivers/serial/stm32_usart.h"


void stm32_usart_init(uint32_t base, uint32_t pclk_hz, uint32_t baud)
{
    USART_CR1(base) = 0;  /* disable while configuring */
    USART_CR2(base) = USART_CR2_STOP_1BIT;
    USART_CR3(base) = 0;
    USART_BRR(base) = USART_BRR_VALUE(pclk_hz, baud);
    USART_CR1(base) = USART_CR1_UE | USART_CR1_TE | USART_CR1_RE;
}

void stm32_usart_putc(uint32_t base, char c)
{
    while (!(USART_SR(base) & USART_SR_TXE))
    {
    }

    USART_DR(base) = (uint8_t)c;
}

void stm32_usart_puts(uint32_t base, const char *s)
{
    while (*s)
    {
        if (*s == '\n')
            stm32_usart_putc(base, '\r');

        stm32_usart_putc(base, *s++);
    }
}

void stm32_usart_flush(uint32_t base)
{
    while (!(USART_SR(base) & USART_SR_TC))
    {
    }
}
