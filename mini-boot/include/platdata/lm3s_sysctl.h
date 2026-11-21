#ifndef MINIBOOT_PLATDATA_LM3S_SYSCTL_H
#define MINIBOOT_PLATDATA_LM3S_SYSCTL_H

#include <types.h>


/*
 * The lm3s_sysctl clock provider's contract with the board that declares
 * it and with every consumer that names it.
 */

/* The provider's device name: the board's row and every consumer use it */
#define LM3S_SYSCTL_NAME  "sysctl"

/*
 * Clock ids: gate register (RCGC0..2) and bit within it. Prefixed with
 * LM3S_ because a board may include more than one provider's header.
 */
#define LM3S_CLK_RCGC0       0
#define LM3S_CLK_RCGC1       1
#define LM3S_CLK_RCGC2       2
#define LM3S_CLK_BUS_CORE    7        /* not a gate register */

#define LM3S_CLK_ID(reg, bit)  ((reg) * 32 + (bit))

#define LM3S_CLK_UART0       LM3S_CLK_ID(LM3S_CLK_RCGC1, 0)
#define LM3S_CLK_SYSCLK      LM3S_CLK_ID(LM3S_CLK_BUS_CORE, 0)


/**
 * What a board tells the lm3s_sysctl driver about its System Control.
 *
 * base:      address of the System Control registers
 * xtal_hz:   crystal on the main oscillator (must be in the RCC XTAL table)
 * sysclk_hz: system clock to run from the PLL: 200 MHz / (4..16)
 */
struct lm3s_sysctl_plat
{
    uintptr_t base;
    u32 xtal_hz;
    u32 sysclk_hz;
};


#endif /* MINIBOOT_PLATDATA_LM3S_SYSCTL_H */
