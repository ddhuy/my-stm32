#ifndef MINIBOOT_PLATDATA_STM32_SPI_H
#define MINIBOOT_PLATDATA_STM32_SPI_H

#include <types.h>


/**
 * What a board tells the stm32_spi driver about one SPI controller.
 *
 * base:        address of the SPI registers
 * clk_dev:     name of the clock provider device
 * clk_id:      the controller's clock within that provider
 * cs_gpio_dev: GPIO port of chip select 0, the only one this driver has
 * cs_pin:      its pin; active low, driven by software
 */
struct stm32_spi_plat {
    uintptr_t base;
    const char *clk_dev;
    u32 clk_id;
    const char *cs_gpio_dev; /* GPIO port of chip select 0 */
    u32 cs_pin;              /* its pin; active low */
};

#endif /* MINIBOOT_PLATDATA_STM32_SPI_H */
