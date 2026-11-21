#ifndef MINIBOOT_PLATDATA_STM32_GPIO_H
#define MINIBOOT_PLATDATA_STM32_GPIO_H

#include <types.h>


/**
 * What a board tells the stm32_gpio driver about one port.
 *
 * base:    address of the port's registers
 * clk_dev: name of the clock provider device that feeds the port
 * clk_id:  the port's clock within that provider (the provider's
 *          platdata header defines the ids)
 */
struct stm32_gpio_plat {
    uintptr_t base;
    const char *clk_dev;
    u32 clk_id;
};


#endif /* MINIBOOT_PLATDATA_STM32_GPIO_H */
