#ifndef CORTEX_M4_SYSTICK_REGS_H
#define CORTEX_M4_SYSTICK_REGS_H

#include "mmio.h"
#include "core_base.h"

/*
 * SysTick 24-bit down-counter (PM0214 section 4.5).
 * For a tick every N core clocks: LOAD = N - 1, VAL = 0,
 * CTRL = CLKSOURCE | TICKINT | ENABLE.
 */

#define SYSTICK_CTRL     MMIO32(SYSTICK_BASE + 0x00u)
#define SYSTICK_LOAD     MMIO32(SYSTICK_BASE + 0x04u)
#define SYSTICK_VAL      MMIO32(SYSTICK_BASE + 0x08u)
#define SYSTICK_CALIB    MMIO32(SYSTICK_BASE + 0x0Cu)

#define SYSTICK_CTRL_ENABLE     (1u << 0)
#define SYSTICK_CTRL_TICKINT    (1u << 1)
#define SYSTICK_CTRL_CLKSOURCE  (1u << 2)    /* 1 = HCLK, 0 = HCLK / 8 */
#define SYSTICK_CTRL_COUNTFLAG  (1u << 16)

#define SYSTICK_LOAD_MAX        0x00FFFFFFu

#endif
