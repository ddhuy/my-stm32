#ifndef MINIBOOT_CONSOLE_H
#define MINIBOOT_CONSOLE_H

#include <types.h>


/**
 * Select the UART that carries the console. It must already be set up with
 * serial_init().
 */
void console_init(
    uintptr_t uart_base
);


/**
 * Write one character. A newline is sent as CR+LF. Output is dropped until
 * console_init() has been called.
 */
void putc(
    char ch
);


/**
 * Write a string. Unlike the C library puts(), no newline is appended.
 */
void puts(
    const char *s
);


/**
 * Wait for a character and return it, or -1 if there is no console yet.
 */
int getc(void);


/**
 * Non-zero if a received character is waiting. Never blocks.
 */
int tstc(void);


#endif /* MINIBOOT_CONSOLE_H */