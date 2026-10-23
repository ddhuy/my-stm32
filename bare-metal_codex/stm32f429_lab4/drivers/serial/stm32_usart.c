#include <stdint.h>

#include "soc/stm32f429/usart_regs.h"

#include "stm32_usart.h"


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

void stm32_usart_flush(uint32_t base)
{
    while (!(USART_SR(base) & USART_SR_TC))
    {
    }
}


char stm32_usart_getc(uint32_t base)
{
    char c;

    while (!stm32_usart_try_getc(base, &c))
    {
        /* Wait until a valid character arrives. */
    }

    return c;
}

bool stm32_usart_try_getc(uint32_t base, char *out)
{
    if (!out)
        return false;

    uint32_t sr = USART_SR(base);
    if (sr & (USART_SR_ORE | USART_SR_FE | USART_SR_NF | USART_SR_PE))
    {
        /* Clear the error flags by reading SR and DR */
        (void)USART_DR(base);
        return false;
    }

    if (sr & USART_SR_RXNE)
    {
        *out = (char)(uint8_t)(USART_DR(base) & 0xFFu);
        return true;
    }

    return false;
}
