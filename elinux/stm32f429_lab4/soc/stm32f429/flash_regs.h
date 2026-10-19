#ifndef STM32F429_FLASH_REGS_H
#define STM32F429_FLASH_REGS_H

#include "arch/cortex-m4/mmio.h"
#include "soc/stm32f429/periph_base.h"

/* Embedded flash interface (RM0090 section 3.9) */

#define FLASH_ACR       MMIO32(FLASH_REG_BASE + 0x00u)
#define FLASH_KEYR      MMIO32(FLASH_REG_BASE + 0x04u)
#define FLASH_OPTKEYR   MMIO32(FLASH_REG_BASE + 0x08u)
#define FLASH_SR        MMIO32(FLASH_REG_BASE + 0x0Cu)
#define FLASH_CR        MMIO32(FLASH_REG_BASE + 0x10u)
#define FLASH_OPTCR     MMIO32(FLASH_REG_BASE + 0x14u)
#define FLASH_OPTCR1    MMIO32(FLASH_REG_BASE + 0x18u)

/*
 * FLASH_ACR
 * Wait states at 2.7..3.6 V: 0 WS up to 30 MHz, +1 WS per 30 MHz,
 * so 168 MHz needs 5 WS and 180 MHz needs 5 WS.
 */
#define FLASH_ACR_LATENCY_MASK (0xFu << 0)
#define FLASH_ACR_LATENCY(ws)  (((ws) & 0xFu) << 0)
#define FLASH_ACR_LATENCY_0WS  (0u << 0)
#define FLASH_ACR_LATENCY_1WS  (1u << 0)
#define FLASH_ACR_LATENCY_2WS  (2u << 0)
#define FLASH_ACR_LATENCY_3WS  (3u << 0)
#define FLASH_ACR_LATENCY_4WS  (4u << 0)
#define FLASH_ACR_LATENCY_5WS  (5u << 0)
#define FLASH_ACR_LATENCY_6WS  (6u << 0)
#define FLASH_ACR_LATENCY_7WS  (7u << 0)

#define FLASH_ACR_PRFTEN       (1u << 8)
#define FLASH_ACR_ICEN         (1u << 9)
#define FLASH_ACR_DCEN         (1u << 10)
#define FLASH_ACR_ICRST        (1u << 11)
#define FLASH_ACR_DCRST        (1u << 12)

/* Unlock sequences: write KEY1 then KEY2 */
#define FLASH_KEY1             0x45670123u
#define FLASH_KEY2             0xCDEF89ABu
#define FLASH_OPTKEY1          0x08192A3Bu
#define FLASH_OPTKEY2          0x4C5D6E7Fu

/* FLASH_SR (error flags are cleared by writing 1) */
#define FLASH_SR_EOP           (1u << 0)
#define FLASH_SR_OPERR         (1u << 1)
#define FLASH_SR_WRPERR        (1u << 4)
#define FLASH_SR_PGAERR        (1u << 5)
#define FLASH_SR_PGPERR        (1u << 6)
#define FLASH_SR_PGSERR        (1u << 7)
#define FLASH_SR_RDERR         (1u << 8)
#define FLASH_SR_BSY           (1u << 16)
#define FLASH_SR_ERRORS        (FLASH_SR_OPERR | FLASH_SR_WRPERR | \
                                FLASH_SR_PGAERR | FLASH_SR_PGPERR | \
                                FLASH_SR_PGSERR | FLASH_SR_RDERR)

/* FLASH_CR */
#define FLASH_CR_PG            (1u << 0)
#define FLASH_CR_SER           (1u << 1)
#define FLASH_CR_MER           (1u << 2)    /* mass erase bank 1 */
#define FLASH_CR_SNB_SHIFT     3u
#define FLASH_CR_SNB_MASK      (0x1Fu << FLASH_CR_SNB_SHIFT)
#define FLASH_CR_SNB(snb)      (((snb) & 0x1Fu) << FLASH_CR_SNB_SHIFT)
#define FLASH_CR_PSIZE_SHIFT   8u
#define FLASH_CR_PSIZE_MASK    (3u << FLASH_CR_PSIZE_SHIFT)
#define FLASH_CR_PSIZE_X8      (0u << FLASH_CR_PSIZE_SHIFT)
#define FLASH_CR_PSIZE_X16     (1u << FLASH_CR_PSIZE_SHIFT)
#define FLASH_CR_PSIZE_X32     (2u << FLASH_CR_PSIZE_SHIFT)    /* use at 2.7..3.6 V */
#define FLASH_CR_PSIZE_X64     (3u << FLASH_CR_PSIZE_SHIFT)
#define FLASH_CR_MER1          (1u << 15)   /* mass erase bank 2 (RM0090 name;
                                               ST's CMSIS calls it MER2) */
#define FLASH_CR_STRT          (1u << 16)
#define FLASH_CR_EOPIE         (1u << 24)
#define FLASH_CR_ERRIE         (1u << 25)
#define FLASH_CR_LOCK          (1u << 31)

/*
 * Sector number -> SNB field value.
 * Bank 1 sectors 0..11 use SNB 0..11; bank 2 sectors 12..23 use SNB 16..27.
 *   Sectors 0-3 / 12-15: 16 KB, 4 / 16: 64 KB, 5-11 / 17-23: 128 KB.
 */
#define FLASH_SECTOR_TO_SNB(s) ((s) < 12u ? (s) : ((s) - 12u + 16u))

/* FLASH_OPTCR */
#define FLASH_OPTCR_OPTLOCK    (1u << 0)
#define FLASH_OPTCR_OPTSTRT    (1u << 1)
#define FLASH_OPTCR_BOR_LEV_MASK (3u << 2)
#define FLASH_OPTCR_BFB2       (1u << 4)
#define FLASH_OPTCR_WDG_SW     (1u << 5)
#define FLASH_OPTCR_nRST_STOP  (1u << 6)
#define FLASH_OPTCR_nRST_STDBY (1u << 7)
#define FLASH_OPTCR_RDP_MASK   (0xFFu << 8)
#define FLASH_OPTCR_nWRP_MASK  (0xFFFu << 16)
#define FLASH_OPTCR_DB1M       (1u << 30)
#define FLASH_OPTCR_SPRMOD     (1u << 31)


#define FLASH_SR_ERROR_MASK \
    (FLASH_SR_OPERR  | FLASH_SR_WRPERR | \
     FLASH_SR_PGAERR | FLASH_SR_PGPERR | \
     FLASH_SR_PGSERR)

#define FLASH_SR_CLEAR_MASK \
    (FLASH_SR_EOP | FLASH_SR_ERROR_MASK)

#endif
