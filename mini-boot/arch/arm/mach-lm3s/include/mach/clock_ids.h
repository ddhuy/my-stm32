#ifndef MINIBOOT_MACH_LM3S_CLOCK_IDS_H
#define MINIBOOT_MACH_LM3S_CLOCK_IDS_H

#define CLK_RCGC0       0
#define CLK_RCGC1       1
#define CLK_RCGC2       2
#define CLK_BUS_CORE    7        /* not a gate register */

#define CLK_ID(reg, bit)  ((reg) * 32 + (bit))

#define CLK_UART0       CLK_ID(CLK_RCGC1, 0)
#define CLK_SYSCLK      CLK_ID(CLK_BUS_CORE, 0)

#endif /* MINIBOOT_MACH_LM3S_CLOCK_IDS_H */
