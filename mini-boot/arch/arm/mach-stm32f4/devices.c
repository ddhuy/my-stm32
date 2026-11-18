#include <asm/armv7m.h>
#include <dm/device.h>
#include <mach/clock_ids.h>
#include <platdata/armv7m_systick.h>


/*
 * Devices inside the SoC itself: every board built on an STM32F4 has them,
 * so their rows live here rather than in a board.c. mach.mk adds the
 * drivers they need, so no board can forget one.
 */

/**
 * SysTick, the Cortex-M4's own timer, counting the processor clock.
 */
static const struct armv7m_systick_plat systick_plat = {
    .base   = SYSTICK_BASE,
    .clk_id = CLK_HCLK
};


DM_DEVICE(systick) = {
    .name   = "systick",
    .driver = "armv7m_systick",
    .plat   = &systick_plat,
};
