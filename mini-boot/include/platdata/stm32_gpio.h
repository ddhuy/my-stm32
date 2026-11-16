#ifndef MINIBOOT_PLATDATA_STM32_GPIO_H
#define MINIBOOT_PLATDATA_STM32_GPIO_H

#include <types.h>


/**
 * What a board tells the stm32_gpio driver about one port.
 *
 * base:   address of the port's registers
 * clk_id: the port's clock, as understood by clock_enable()
 */
struct stm32_gpio_plat {
    uintptr_t base;
    u32 clk_id;
};


#endif /* MINIBOOT_PLATDATA_STM32_GPIO_H */
