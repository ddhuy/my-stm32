#ifndef MINIBOOT_SERIAL_H
#define MINIBOOT_SERIAL_H

#include <types.h>


/**
 * 
 */
void serial_init(
    uintptr_t base,
    u32 clk_rate,
    u32 baudrate
);


/**
 * 
 */
void serial_putc(
    uintptr_t base,
    char ch
);


/**
 * 
 */
int serial_getc(
    uintptr_t base
);


/**
 * 
 */
int serial_tstc(
    uintptr_t base
);

#endif /* MINIBOOT_SERIAL_H */