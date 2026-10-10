#ifndef STM32F429_SCB_REGS_H
#define STM32F429_SCB_REGS_H

#include "mmio.h"

#define SCB_BASE       0xE000ED00u

#define SCB_VTOR       MMIO32(SCB_BASE + 0x08u)
#define SCB_SHCSR      MMIO32(SCB_BASE + 0x24u)
#define SCB_CFSR       MMIO32(SCB_BASE + 0x28u)
#define SCB_HFSR       MMIO32(SCB_BASE + 0x2Cu)
#define SCB_MMFAR      MMIO32(SCB_BASE + 0x34u)
#define SCB_BFAR       MMIO32(SCB_BASE + 0x38u)

#define SCB_SHCSR_MEMFAULTENA (1u << 16)
#define SCB_SHCSR_BUSFAULTENA (1u << 17)
#define SCB_SHCSR_USGFAULTENA (1u << 18)

#endif