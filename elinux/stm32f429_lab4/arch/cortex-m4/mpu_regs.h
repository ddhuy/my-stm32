#ifndef CORTEX_M4_MPU_REGS_H
#define CORTEX_M4_MPU_REGS_H

#include "mmio.h"
#include "core_base.h"

/* Cortex-M4 memory protection unit, 8 regions (PM0214 section 4.2) */

#define MPU_TYPE       MMIO32(MPU_BASE + 0x00u)
#define MPU_CTRL       MMIO32(MPU_BASE + 0x04u)
#define MPU_RNR        MMIO32(MPU_BASE + 0x08u)
#define MPU_RBAR       MMIO32(MPU_BASE + 0x0Cu)
#define MPU_RASR       MMIO32(MPU_BASE + 0x10u)

#define MPU_CTRL_ENABLE       (1u << 0)
#define MPU_CTRL_HFNMIENA     (1u << 1)
#define MPU_CTRL_PRIVDEFENA   (1u << 2)

/* MPU_RBAR: base must be aligned to the region size */
#define MPU_RBAR_REGION(n)    (((n) & 0xFu) << 0)
#define MPU_RBAR_VALID        (1u << 4)

/* MPU_RASR */
#define MPU_RASR_ENABLE       (1u << 0)
/* Region size = 2^(SIZE+1) bytes; pass log2 of the size (5..32) */
#define MPU_RASR_SIZE_LOG2(l) ((((l) - 1u) & 0x1Fu) << 1)
#define MPU_RASR_SIZE_32B     MPU_RASR_SIZE_LOG2(5u)
#define MPU_RASR_SIZE_1KB     MPU_RASR_SIZE_LOG2(10u)
#define MPU_RASR_SIZE_16KB    MPU_RASR_SIZE_LOG2(14u)
#define MPU_RASR_SIZE_64KB    MPU_RASR_SIZE_LOG2(16u)
#define MPU_RASR_SIZE_128KB   MPU_RASR_SIZE_LOG2(17u)
#define MPU_RASR_SIZE_256KB   MPU_RASR_SIZE_LOG2(18u)
#define MPU_RASR_SIZE_512KB   MPU_RASR_SIZE_LOG2(19u)
#define MPU_RASR_SIZE_1MB     MPU_RASR_SIZE_LOG2(20u)
#define MPU_RASR_SIZE_2MB     MPU_RASR_SIZE_LOG2(21u)
#define MPU_RASR_SIZE_8MB     (22u << 1)
#define MPU_RASR_SIZE_4GB     MPU_RASR_SIZE_LOG2(32u)
#define MPU_RASR_SRD(mask)    (((mask) & 0xFFu) << 8)   /* subregion disable */
#define MPU_RASR_B            (1u << 16)
#define MPU_RASR_C            (1u << 17)
#define MPU_RASR_S            (1u << 18)
#define MPU_RASR_TEX(t)       (((t) & 7u) << 19)
#define MPU_RASR_TEX_NORMAL   (1u << 19)                /* TEX=001, C=B=0: normal, non-cacheable */
#define MPU_RASR_AP_NONE      (0u << 24)
#define MPU_RASR_AP_PRIV_RW   (1u << 24)
#define MPU_RASR_AP_PRIV_RW_USER_RO (2u << 24)
#define MPU_RASR_AP_FULL      (3u << 24)
#define MPU_RASR_AP_PRIV_RO   (5u << 24)
#define MPU_RASR_AP_RO        (6u << 24)
#define MPU_RASR_XN           (1u << 28)

#endif
