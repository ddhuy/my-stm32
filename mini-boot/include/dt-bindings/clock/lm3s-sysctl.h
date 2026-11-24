#ifndef MINIBOOT_DT_BINDINGS_CLOCK_LM3S_SYSCTL_H
#define MINIBOOT_DT_BINDINGS_CLOCK_LM3S_SYSCTL_H

/*
 * Clock ids of the lm3s_sysctl clock provider, shared by C (through
 * platdata/lm3s_sysctl.h) and the device tree (clocks = <&sysctl ID>);
 * defines only, as every dt-bindings header.
 *
 * An id is a gate register (RCGC0..2) and a bit within it. Prefixed with
 * LM3S_ because a board may include more than one provider's header.
 * LM3S_CLK_ID() is in parentheses as a whole, for dtc.
 */
#define LM3S_CLK_RCGC0       0
#define LM3S_CLK_RCGC1       1
#define LM3S_CLK_RCGC2       2
#define LM3S_CLK_BUS_CORE    7        /* not a gate register */

#define LM3S_CLK_ID(reg, bit)  ((reg) * 32 + (bit))

#define LM3S_CLK_UART0       LM3S_CLK_ID(LM3S_CLK_RCGC1, 0)
#define LM3S_CLK_SYSCLK      LM3S_CLK_ID(LM3S_CLK_BUS_CORE, 0)

#endif /* MINIBOOT_DT_BINDINGS_CLOCK_LM3S_SYSCTL_H */
