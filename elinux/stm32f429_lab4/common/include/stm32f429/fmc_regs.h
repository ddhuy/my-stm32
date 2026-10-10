#ifndef STM32F429_FMC_REGS_H
#define STM32F429_FMC_REGS_H

#include "mmio.h"

#define FMC_BASE       0xA0000000u

#define FMC_SDCR1      MMIO32(FMC_BASE + 0x140u)
#define FMC_SDCR2      MMIO32(FMC_BASE + 0x144u)

#define FMC_SDTR1      MMIO32(FMC_BASE + 0x148u)
#define FMC_SDTR2      MMIO32(FMC_BASE + 0x14Cu)

#define FMC_SDCMR      MMIO32(FMC_BASE + 0x150u)
#define FMC_SDRTR      MMIO32(FMC_BASE + 0x154u)
#define FMC_SDSR       MMIO32(FMC_BASE + 0x158u)

/* SDRAM control register */
#define FMC_SDCR_SDCLK_HCLK2  (2u << 10)
#define FMC_SDCR_RBURST       (1u << 12)

/* SDRAM command modes */
#define FMC_CMD_CLK_ENABLE    1u
#define FMC_CMD_PALL          2u
#define FMC_CMD_AUTOREFRESH   3u
#define FMC_CMD_LOAD_MODE     4u
#define FMC_CMD_SELF_REFRESH  5u
#define FMC_CMD_POWER_DOWN    6u

/* Target bank selection */
#define FMC_CMD_CTB1          (1u << 4)
#define FMC_CMD_CTB2          (1u << 3)

/* SDRAM status */
#define FMC_SDSR_BUSY         (1u << 5)

#endif