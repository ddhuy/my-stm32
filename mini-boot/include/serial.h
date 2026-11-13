#ifndef MINIBOOT_SERIAL_H
#define MINIBOOT_SERIAL_H

#include <types.h>

/*
 * Generic serial API. A UART is identified by its base address for now;
 * with the driver model this becomes a struct udevice pointer.
 */


/**
 * Set up the UART for 8 data bits, no parity, 1 stop bit, polled. 'clk_rate'
 * is the frequency of the UART's bus clock in Hz.
 */
void serial_init(
    uintptr_t base,
    u32 clk_rate,
    u32 baudrate
);


/**
 * Send one character. Blocks until the transmitter accepts it.
 */
void serial_putc(
    uintptr_t base,
    char ch
);


/**
 * Wait for a character and return it (0-255).
 */
int serial_getc(
    uintptr_t base
);


/**
 * Non-zero if a received character is waiting. Never blocks.
 */
int serial_tstc(
    uintptr_t base
);

#endif /* MINIBOOT_SERIAL_H */