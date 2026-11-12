#include <console.h>
#include <serial.h>


static uintptr_t console_base;


/**
 * 
 */
void console_init(
    uintptr_t uart_base
)
{
    console_base = uart_base;
}


/**
 * 
 */
void putc(
    char ch
)
{
    if (!console_base)
        return;

    if (ch == '\n')
        serial_putc(console_base, '\r');

    serial_putc(console_base, ch);
}


/**
 * 
 */
void puts(
    const char *s
)
{
    while ( *s )
        putc(*s++);
}


/**
 * 
 */
int getc(void)
{
    if (!console_base)
        return -1;

    return serial_getc(console_base);
}


/**
 * 
 */
int tstc(void)
{
    if (!console_base)
        return 0;
    
    return serial_tstc(console_base);
}
