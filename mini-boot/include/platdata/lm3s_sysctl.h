#ifndef MINIBOOT_PLATDATA_LM3S_SYSCTL_H
#define MINIBOOT_PLATDATA_LM3S_SYSCTL_H

#include <types.h>
#include <dt-bindings/clock/lm3s-sysctl.h>


/*
 * The lm3s_sysctl clock provider's contract with the board that declares
 * it and with every consumer that names it.
 */

/* The provider's device name (its label in the device tree) */
#define LM3S_SYSCTL_NAME  "sysctl"

/* The clock ids (LM3S_CLK_*) are in dt-bindings/clock/lm3s-sysctl.h */


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
