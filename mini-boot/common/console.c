#include <console.h>
#include <serial.h>


/*
 * Base address of the console UART; 0 until console_init() is called.
 * The driver model will replace this with a lookup of a serial device.
 */
static uintptr_t console_base;


/**
 * Remember which UART to use.
 */
void console_init(
    uintptr_t uart_base
)
{
    console_base = uart_base;
}


/**
 * Terminals need CR+LF, so a carriage return goes out before each newline.
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
 * Goes through putc(), so the newline translation lives in one place.
 */
void puts(
    const char *s
)
{
    while ( *s )
        putc(*s++);
}


/**
 * Blocks until a character arrives.
 */
int getc(void)
{
    if (!console_base)
        return -1;

    return serial_getc(console_base);
}


/**
 * Polls the UART for a received character.
 */
int tstc(void)
{
    if (!console_base)
        return 0;
    
    return serial_tstc(console_base);
}
