#ifndef MINIBOOT_DT_BINDINGS_CLOCK_STM32F4_RCC_H
#define MINIBOOT_DT_BINDINGS_CLOCK_STM32F4_RCC_H

/*
 * Clock IDs of the stm32f4_rcc clock provider, shared by C (through
 * platdata/stm32f4_rcc.h) and the device tree (clocks = <&rcc ID>), so
 * defines only: the .dts is run through the C preprocessor, and anything
 * else here would end up in it.
 *
 * An ID is which bus the peripheral hangs on and its bit in that bus's
 * enable register (RM0090, RCC_AHB1ENR ... RCC_APB2ENR). Prefixed with
 * STM32F4_ because a board may include more than one provider's header.
 *
 * STM32F4_CLK_BUS_CORE is not an RCC bus. Its IDs name clocks that cannot
 * be switched off; the enable operation accepts them and does nothing,
 * get_rate reports them.
 *
 * STM32F4_CLK_ID() is in parentheses as a whole: dtc evaluates an
 * expression in a cell only inside parentheses.
 */
#define STM32F4_CLK_BUS_AHB1      0
#define STM32F4_CLK_BUS_AHB2      1
#define STM32F4_CLK_BUS_AHB3      2
#define STM32F4_CLK_BUS_APB1      4
#define STM32F4_CLK_BUS_APB2      5
#define STM32F4_CLK_BUS_CORE      7

#define STM32F4_CLK_ID(bus, bit)  ((bus) * 32 + (bit))

#define STM32F4_CLK_GPIOA         STM32F4_CLK_ID(STM32F4_CLK_BUS_AHB1, 0)
#define STM32F4_CLK_GPIOB         STM32F4_CLK_ID(STM32F4_CLK_BUS_AHB1, 1)
#define STM32F4_CLK_GPIOC         STM32F4_CLK_ID(STM32F4_CLK_BUS_AHB1, 2)
#define STM32F4_CLK_GPIOD         STM32F4_CLK_ID(STM32F4_CLK_BUS_AHB1, 3)
#define STM32F4_CLK_GPIOE         STM32F4_CLK_ID(STM32F4_CLK_BUS_AHB1, 4)
#define STM32F4_CLK_GPIOF         STM32F4_CLK_ID(STM32F4_CLK_BUS_AHB1, 5)
#define STM32F4_CLK_GPIOG         STM32F4_CLK_ID(STM32F4_CLK_BUS_AHB1, 6)
#define STM32F4_CLK_USART1        STM32F4_CLK_ID(STM32F4_CLK_BUS_APB2, 4)
#define STM32F4_CLK_SPI5          STM32F4_CLK_ID(STM32F4_CLK_BUS_APB2, 20)
#define STM32F4_CLK_HCLK          STM32F4_CLK_ID(STM32F4_CLK_BUS_CORE, 0)

#endif /* MINIBOOT_DT_BINDINGS_CLOCK_STM32F4_RCC_H */
