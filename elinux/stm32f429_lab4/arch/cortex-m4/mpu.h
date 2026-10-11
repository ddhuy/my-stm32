#ifndef CORTEX_M4_MPU_H
#define CORTEX_M4_MPU_H

#include <stdint.h>

#include "barrier.h"
#include "mpu_regs.h"

/* Turn the MPU off before changing regions. */
static inline void mpu_disable(void)
{
    MPU_CTRL = 0;
    dsb();
    isb();
}

/* base must be aligned to the region size encoded in rasr. */
static inline void mpu_set_region(uint32_t region, uint32_t base, uint32_t rasr)
{
    MPU_RNR = region;
    MPU_RBAR = base;
    MPU_RASR = rasr;
}

/* flags: e.g. MPU_CTRL_PRIVDEFENA to keep the default map for uncovered addresses. */
static inline void mpu_enable(uint32_t flags)
{
    MPU_CTRL = flags | MPU_CTRL_ENABLE;
    dsb();
    isb();
}

#endif
