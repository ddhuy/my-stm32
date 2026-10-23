#ifndef STM32F429_FMC_REGS_H
#define STM32F429_FMC_REGS_H

#include "arch/cortex-m4/mmio.h"
#include "soc/stm32f429/periph_base.h"

/* Flexible memory controller (RM0090 section 37). Enable RCC_AHB3ENR_FMCEN. */

/* NOR/PSRAM/SRAM banks, x = 1..4 */
#define FMC_BCR(x)     MMIO32(FMC_BASE + 0x000u + (((x) - 1u) * 8u))
#define FMC_BTR(x)     MMIO32(FMC_BASE + 0x004u + (((x) - 1u) * 8u))
#define FMC_BWTR(x)    MMIO32(FMC_BASE + 0x104u + (((x) - 1u) * 8u))

/* SDRAM. Bank 1 registers hold some fields shared by both banks (SDCLK, RBURST, RPIPE, TRP, TRC). */
#define FMC_SDCR1      MMIO32(FMC_BASE + 0x140u)
#define FMC_SDCR2      MMIO32(FMC_BASE + 0x144u)

#define FMC_SDTR1      MMIO32(FMC_BASE + 0x148u)
#define FMC_SDTR2      MMIO32(FMC_BASE + 0x14Cu)

#define FMC_SDCMR      MMIO32(FMC_BASE + 0x150u)
#define FMC_SDRTR      MMIO32(FMC_BASE + 0x154u)
#define FMC_SDSR       MMIO32(FMC_BASE + 0x158u)

/* SDRAM control register (FMC_SDCRx) */
#define FMC_SDCR_NC(bits)     ((((bits) - 8u) & 3u) << 0)    /* column bits 8..11 */
#define FMC_SDCR_NR(bits)     ((((bits) - 11u) & 3u) << 2)   /* row bits 11..13   */
#define FMC_SDCR_MWID_8       (0u << 4)
#define FMC_SDCR_MWID_16      (1u << 4)
#define FMC_SDCR_MWID_32      (2u << 4)
#define FMC_SDCR_NB_4         (1u << 6)                      /* 0 = 2 internal banks */
#define FMC_SDCR_CAS(c)       (((c) & 3u) << 7)              /* CAS latency 1..3 */
#define FMC_SDCR_WP           (1u << 9)
#define FMC_SDCR_SDCLK_MASK   (3u << 10)
#define FMC_SDCR_SDCLK_HCLK2  (2u << 10)
#define FMC_SDCR_SDCLK_HCLK3  (3u << 10)
#define FMC_SDCR_RBURST       (1u << 12)
#define FMC_SDCR_RPIPE(c)     (((c) & 3u) << 13)

/* SDRAM timing register (FMC_SDTRx), values in SDCLK cycles (1..16) */
#define FMC_SDTR_TMRD(c)      ((((c) - 1u) & 0xFu) << 0)
#define FMC_SDTR_TXSR(c)      ((((c) - 1u) & 0xFu) << 4)
#define FMC_SDTR_TRAS(c)      ((((c) - 1u) & 0xFu) << 8)
#define FMC_SDTR_TRC(c)       ((((c) - 1u) & 0xFu) << 12)
#define FMC_SDTR_TWR(c)       ((((c) - 1u) & 0xFu) << 16)
#define FMC_SDTR_TRP(c)       ((((c) - 1u) & 0xFu) << 20)
#define FMC_SDTR_TRCD(c)      ((((c) - 1u) & 0xFu) << 24)

/* SDRAM command modes (FMC_SDCMR MODE field) */
#define FMC_CMD_NORMAL        0u
#define FMC_CMD_CLK_ENABLE    1u
#define FMC_CMD_PALL          2u
#define FMC_CMD_AUTOREFRESH   3u
#define FMC_CMD_LOAD_MODE     4u
#define FMC_CMD_SELF_REFRESH  5u
#define FMC_CMD_POWER_DOWN    6u

/* Target bank selection */
#define FMC_CMD_CTB1          (1u << 4)
#define FMC_CMD_CTB2          (1u << 3)

#define FMC_SDCMR_NRFS(n)     ((((n) - 1u) & 0xFu) << 5)     /* auto-refresh count 1..16 */
#define FMC_SDCMR_MRD(v)      (((v) & 0x1FFFu) << 9)         /* mode register value */

/* SDRAM refresh timer: COUNT = (refresh period / rows) * f_SDCLK - 20 */
#define FMC_SDRTR_CRE         (1u << 0)
#define FMC_SDRTR_COUNT(n)    (((n) & 0x1FFFu) << 1)
#define FMC_SDRTR_REIE        (1u << 14)

/* SDRAM status */
#define FMC_SDSR_RE           (1u << 0)
#define FMC_SDSR_MODES1_MASK  (3u << 1)
#define FMC_SDSR_MODES2_MASK  (3u << 3)
#define FMC_SDSR_BUSY         (1u << 5)

#endif
