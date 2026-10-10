#ifndef STM32F429_PWR_REGS_H
#define STM32F429_PWR_REGS_H

#include "mmio.h"

#define PWR_BASE       0x40007000u

#define PWR_CR         MMIO32(PWR_BASE + 0x00u)
#define PWR_CSR        MMIO32(PWR_BASE + 0x04u)

#define PWR_CR_VOS_MASK (3u << 14)
#define PWR_CR_VOS_SCALE1 (3u << 14)

#endif