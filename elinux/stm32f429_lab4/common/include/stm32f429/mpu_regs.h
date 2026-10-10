#ifndef STM32F429_MPU_REGS_H
#define STM32F429_MPU_REGS_H

#include "mmio.h"

#define MPU_BASE       0xE000ED90u

#define MPU_TYPE       MMIO32(MPU_BASE + 0x00u)
#define MPU_CTRL       MMIO32(MPU_BASE + 0x04u)
#define MPU_RNR        MMIO32(MPU_BASE + 0x08u)
#define MPU_RBAR       MMIO32(MPU_BASE + 0x0Cu)
#define MPU_RASR       MMIO32(MPU_BASE + 0x10u)

#define MPU_CTRL_ENABLE       (1u << 0)
#define MPU_CTRL_PRIVDEFENA   (1u << 2)

#define MPU_RASR_ENABLE       (1u << 0)
#define MPU_RASR_SIZE_8MB     (22u << 1)
#define MPU_RASR_TEX_NORMAL   (1u << 19)
#define MPU_RASR_AP_FULL      (3u << 24)
#define MPU_RASR_XN           (1u << 28)

#endif