#ifndef MINIBOOT_PLATDATA_STM32_SERIAL_H
#define MINIBOOT_PLATDATA_STM32_SERIAL_H

#include <types.h>


/**
 * What a board tells the stm32_serial driver about one UART.
 *
 * base:     address of the UART's registers
 * clk_id:   the UART's clock, as understood by clock_enable() and
 *           clock_get_rate()
 * baudrate: line speed in bits per second
 */
struct stm32_serial_plat {
    uintptr_t base;
    u32 clk_id;
    u32 baudrate;
};

#endif /* MINIBOOT_PLATDATA_STM32_SERIAL_H */
