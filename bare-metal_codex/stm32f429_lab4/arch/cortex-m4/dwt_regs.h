#ifndef CORTEX_M4_DWT_REGS_H
#define CORTEX_M4_DWT_REGS_H

#include "mmio.h"
#include "core_base.h"

/*
 * Data watchpoint and trace unit: mainly used for its free-running
 * CPU cycle counter (handy for busy-wait delays and timing code).
 *
 *     DEMCR      |= DEMCR_TRCENA;
 *     DWT_CYCCNT  = 0;
 *     DWT_CTRL   |= DWT_CTRL_CYCCNTENA;
 */

#define DEMCR          MMIO32(SCS_BASE + 0xDFCu)
#define DEMCR_TRCENA   (1u << 24)

#define DWT_CTRL       MMIO32(DWT_BASE + 0x00u)
#define DWT_CYCCNT     MMIO32(DWT_BASE + 0x04u)

#define DWT_CTRL_CYCCNTENA (1u << 0)

#endif
