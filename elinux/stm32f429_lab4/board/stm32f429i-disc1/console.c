#include "board/stm32f429i-disc1/board.h"
#include "drivers/serial/stm32_usart.h"

#include "console.h"


void console_putc(char c)
{
    stm32_usart_putc(CONSOLE_USART, c);
}

void console_puts(const char *s)
{
    stm32_usart_puts(CONSOLE_USART, s);
}

void console_puthex(uint32_t value)
{
    static const char digits[] = "0123456789ABCDEF";
    char buf[11] = "0x";

    for (unsigned i = 0; i < 8u; ++i)
        buf[2u + i] = digits[(value >> (28u - 4u * i)) & 0xFu];
    buf[10] = '\0';

    stm32_usart_puts(CONSOLE_USART, buf);
}

char console_getc()
{
    return stm32_usart_getc(CONSOLE_USART);
}

bool console_gets(char *buf, uint32_t buf_size)
{
    return stm32_usart_gets(CONSOLE_USART, buf, buf_size);
}

void console_flush()
{
    stm32_usart_flush(CONSOLE_USART);
}