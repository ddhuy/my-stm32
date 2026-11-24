#ifndef MINIBOOT_PLATDATA_STM32F4_RCC_H
#define MINIBOOT_PLATDATA_STM32F4_RCC_H

#include <types.h>
#include <dt-bindings/clock/stm32f4-rcc.h>


/*
 * The stm32f4_rcc clock provider's contract with the board that declares
 * it and with every consumer that names it.
 */

/* The provider's device name: the board's row and every consumer use it */
#define STM32F4_RCC_NAME "rcc"

/* The clock IDs (STM32F4_CLK_*) are in dt-bindings/clock/stm32f4-rcc.h */


/**
 * What a board tells the stm32f4_rcc driver about its RCC.
 *
 * base:       address of the RCC registers
 * flash_base: address of the flash interface registers (FLASH_ACR): the
 *             wait states must follow the clock, so the driver sets them
 * pwr_base:   address of the PWR registers (PWR_CR): the voltage scale
 *             must follow the clock, so the driver sets it
 * hse_hz:     crystal on OSC_IN / OSC_OUT; a whole multiple of 2 MHz
 * sysclk_hz:  system clock to run from the PLL; a whole number of MHz,
 *             at most 168 MHz
 */
struct stm32f4_rcc_plat
{
    uintptr_t base;
    uintptr_t flash_base;
    uintptr_t pwr_base;
    u32 hse_hz;
    u32 sysclk_hz;
};


#endif /* MINIBOOT_PLATDATA_STM32F4_RCC_H */
