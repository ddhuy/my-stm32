#ifndef MINIBOOT_CLOCK_IDS_H
#define MINIBOOT_CLOCK_IDS_H


/**
 * Clock IDs: which bus the peripheral hangs on, and its bit in that bus's
 * enable register (RM0090, RCC_AHB1ENR ... RCC_APB2ENR).
 *
 * CLK_BUS_CORE is not an RCC bus. Its IDs name clocks that cannot be
 * switched off; clock_enable() ignores them, clock_get_rate() reports them.
 */
#define CLK_BUS_AHB1        0
#define CLK_BUS_AHB2        1
#define CLK_BUS_AHB3        2
#define CLK_BUS_APB1        4
#define CLK_BUS_APB2        5
#define CLK_BUS_CORE        7

#define CLK_ID(bus, bit)    ((bus) * 32 + (bit))

#define CLK_GPIOA           CLK_ID(CLK_BUS_AHB1, 0)
#define CLK_GPIOG           CLK_ID(CLK_BUS_AHB1, 6)
#define CLK_USART1          CLK_ID(CLK_BUS_APB2, 4)
#define CLK_HCLK            CLK_ID(CLK_BUS_CORE, 0)


#endif /* MINIBOOT_CLOCK_IDS_H */
