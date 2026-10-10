#ifndef STM32F429_FLASH_REGS_H
#define STM32F429_FLASH_REGS_H

#include "mmio.h"

#define FLASH_REG_BASE  0x40023C00u

#define FLASH_ACR       MMIO32(FLASH_REG_BASE + 0x00u)

#define FLASH_ACR_LATENCY_MASK (0xFu << 0)
#define FLASH_ACR_LATENCY_5WS  (5u << 0)

#define FLASH_ACR_PRFTEN       (1u << 8)
#define FLASH_ACR_ICEN         (1u << 9)
#define FLASH_ACR_DCEN         (1u << 10)

#endif