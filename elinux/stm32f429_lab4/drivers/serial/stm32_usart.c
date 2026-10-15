#include <stddef.h>
#include <stdint.h>

#include "soc/stm32f429/usart_regs.h"
#include "board/stm32f429i-disc1/board.h"

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

bool stm32_usart_gets(uint32_t base, char *buf, uint32_t buf_size)
{
    uint32_t len = 0u;
    bool overflow = false;

    if (!buf || buf_size == 0u)
        return false;

    for (;;)
    {
        char c = stm32_usart_getc(base);

        if (c == '\r' || c == '\n')
        {
            stm32_usart_puts(base, "\n");
            buf[len] = '\0';
            return !overflow;
        }

        /* Handle terminal Backspace and Delete. */
        if (c == '\b' || c == 0x7Fu)
        {
            if (len > 0u)
            {
                --len;
                stm32_usart_puts(base, "\b \b");
            }
            continue;
        }

        if (c < 0x20u || c > 0x7Eu)
            continue;   /* ignore non-printable characters */

        if (!overflow && (len + 1u < buf_size))
        {
            buf[len++] = c;
            stm32_usart_putc(base, c);
        }
        else
        {
            overflow = true;
        }
    }
}