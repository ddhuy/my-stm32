#ifndef MINIBOOT_PLATDATA_STM32F4_RCC_H
#define MINIBOOT_PLATDATA_STM32F4_RCC_H

#include <types.h>


/*
 * The stm32f4_rcc clock provider's contract with the board that declares
 * it and with every consumer that names it.
 */

/* The provider's device name: the board's row and every consumer use it */
#define STM32F4_RCC_NAME "rcc"

/**
 * Clock IDs: which bus the peripheral hangs on, and its bit in that bus's
 * enable register (RM0090, RCC_AHB1ENR ... RCC_APB2ENR). Prefixed with
 * STM32F4_ because a board may include more than one provider's header.
 *
 * STM32F4_CLK_BUS_CORE is not an RCC bus. Its IDs name clocks that cannot
 * be switched off; the enable operation accepts them and does nothing,
 * get_rate reports them.
 */
#define STM32F4_CLK_BUS_AHB1      0
#define STM32F4_CLK_BUS_AHB2      1
#define STM32F4_CLK_BUS_AHB3      2
#define STM32F4_CLK_BUS_APB1      4
#define STM32F4_CLK_BUS_APB2      5
#define STM32F4_CLK_BUS_CORE      7

#define STM32F4_CLK_ID(bus, bit)  ((bus) * 32 + (bit))

#define STM32F4_CLK_GPIOA         STM32F4_CLK_ID(STM32F4_CLK_BUS_AHB1, 0)
#define STM32F4_CLK_GPIOG         STM32F4_CLK_ID(STM32F4_CLK_BUS_AHB1, 6)
#define STM32F4_CLK_USART1        STM32F4_CLK_ID(STM32F4_CLK_BUS_APB2, 4)
#define STM32F4_CLK_HCLK          STM32F4_CLK_ID(STM32F4_CLK_BUS_CORE, 0)


/**
 * What a board tells the stm32f4_rcc driver about its RCC.
 *
 * base: address of the RCC registers
 *
 * Item 3 Part D adds the crystal (HSE) and the system clock to run.
 */
struct stm32f4_rcc_plat
{
    uintptr_t base;
};


#endif /* MINIBOOT_PLATDATA_STM32F4_RCC_H */
