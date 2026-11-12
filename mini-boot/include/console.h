#ifndef MINIBOOT_CONSOLE_H
#define MINIBOOT_CONSOLE_H

#include <types.h>


/**
 * 
 */
void console_init(
    uintptr_t uart_base
);


/**
 * 
 */
void putc(
    char ch
);


/**
 * 
 */
void puts(
    const char *s
);


/**
 * 
 */
int getc(void);


/**
 * 
 */
int tstc(void);


#endif /* MINIBOOT_CONSOLE_H */