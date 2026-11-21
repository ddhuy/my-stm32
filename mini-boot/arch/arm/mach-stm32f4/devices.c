#include <asm/armv7m.h>
#include <dm/device.h>
#include <platdata/armv7m_systick.h>
#include <platdata/stm32f4_rcc.h>


/*
 * Devices inside the SoC itself: every board built on an STM32F4 has them,
 * so their rows live here rather than in a board.c. mach.mk adds the
 * drivers they need, so no board can forget one.
 */

/**
 * SysTick, the Cortex-M4's own timer, counting the processor clock. Its
 * clock comes from the RCC, whose row is in each board's board.c (its
 * settings are board facts): every board on this SoC must declare a
 * provider named STM32F4_RCC_NAME. Without one, the SysTick probe fails
 * with -ENODEV and main() stops at "PANIC: No Timer!".
 */
static const struct armv7m_systick_plat systick_plat = {
    .base    = SYSTICK_BASE,
    .clk_dev = STM32F4_RCC_NAME,
    .clk_id  = STM32F4_CLK_HCLK
};


DM_DEVICE(systick) = {
    .name   = "systick",
    .driver = "armv7m_systick",
    .plat   = &systick_plat,
};
