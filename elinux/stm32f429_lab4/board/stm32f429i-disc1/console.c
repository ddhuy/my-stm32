#include "board/stm32f429i-disc1/board.h"
#include "drivers/serial/stm32_usart.h"

#include "console.h"


void console_putc(char c)
{
    if (c == '\n')
        stm32_usart_putc(CONSOLE_USART, '\r');
    stm32_usart_putc(CONSOLE_USART, c);
}

void console_puts(const char *s)
{
    while (*s)
        console_putc(*s++);
}

void console_puthex(uint32_t value)
{
    static const char digits[] = "0123456789ABCDEF";
    char buf[11] = "0x";

    for (unsigned i = 0; i < 8u; ++i)
        buf[2u + i] = digits[(value >> (28u - 4u * i)) & 0xFu];
    buf[10] = '\0';

    console_puts(buf);
}

char console_getc(void)
{
    return stm32_usart_getc(CONSOLE_USART);
}

bool console_gets(char *buf, uint32_t buf_size)
{
    /* One terminal: preserve CRLF state between consecutive line reads. */
    static bool skip_lf;
    uint32_t len = 0u;
    bool overflow = false;

    if (!buf || buf_size == 0u)
        return false;

    for (;;)
    {
        unsigned char c = (unsigned char)console_getc();

        if (skip_lf)
        {
            skip_lf = false;
            if (c == '\n')
                continue;
        }

        if (c == '\r' || c == '\n')
        {
            skip_lf = (c == '\r');
            console_putc('\n');
            buf[len] = '\0';
            return !overflow;
        }

        if (overflow)
            continue; /* Discard the entire remainder of an overlong line. */

        /* Handle terminal Backspace and Delete. */
        if (c == '\b' || c == 0x7Fu)
        {
            if (len > 0u)
            {
                --len;
                console_puts("\b \b");
            }
            continue;
        }

        if (c < 0x20u || c > 0x7Eu)
            continue;   /* ignore non-printable characters */

        if (len < buf_size - 1u)
        {
            buf[len++] = c;
            console_putc((char)c);
        }
        else
        {
            overflow = true;
        }
    }
}

bool console_try_getc(char *out)
{
    return stm32_usart_try_getc(CONSOLE_USART, out);
}

void console_flush(void)
{
    stm32_usart_flush(CONSOLE_USART);
}
