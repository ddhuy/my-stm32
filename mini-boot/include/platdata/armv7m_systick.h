#ifndef MINIBOOT_PLATDATA_ARMV7M_SYSTICK_H
#define MINIBOOT_PLATDATA_ARMV7M_SYSTICK_H

#include <types.h>


/**
 * What a SoC tells the armv7m_systick driver about its SysTick.
 *
 * base:    address of the SysTick registers (SYSTICK_BASE)
 * clk_dev: name of the clock provider device that feeds the CPU
 * clk_id:  the processor clock within that provider. The driver always
 *          counts the processor clock (CTRL.CLKSOURCE = 1), never the
 *          optional external reference, so this must name the clock that
 *          feeds the CPU.
 */
struct armv7m_systick_plat
{
    uintptr_t base;
    const char *clk_dev;
    u32 clk_id;
};

#endif /* MINIBOOT_PLATDATA_ARMV7M_SYSTICK_H */
