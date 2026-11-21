#ifndef MINIBOOT_PLATDATA_PL011_SERIAL_H
#define MINIBOOT_PLATDATA_PL011_SERIAL_H

#include <types.h>


/**
 * What a board tells the pl011_serial driver about one UART.
 *
 * base:     address of the UART's registers
 * clk_dev:  name of the clock provider device that feeds the UART
 * clk_id:   the UART's clock within that provider (the provider's
 *           platdata header defines the ids)
 * baudrate: line speed in bits per second
 */
struct pl011_serial_plat {
    uintptr_t base;
    const char *clk_dev;
    u32 clk_id;
    u32 baudrate;
};

#endif /* MINIBOOT_PLATDATA_PL011_SERIAL_H */