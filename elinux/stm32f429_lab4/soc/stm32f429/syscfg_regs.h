#ifndef STM32F429_SYSCFG_REGS_H
#define STM32F429_SYSCFG_REGS_H

#include "arch/cortex-m4/mmio.h"
#include "soc/stm32f429/periph_base.h"


/* System configuration controller (RM0090 section 9.3). Enable RCC_APB2ENR_SYSCFGEN. */

#define SYSCFG_MEMRMP      MMIO32(SYSCFG_BASE + 0x00u)
#define SYSCFG_PMC         MMIO32(SYSCFG_BASE + 0x04u)
#define SYSCFG_EXTICR1     MMIO32(SYSCFG_BASE + 0x08u)   /* EXTI0..3   */
#define SYSCFG_EXTICR2     MMIO32(SYSCFG_BASE + 0x0Cu)   /* EXTI4..7   */
#define SYSCFG_EXTICR3     MMIO32(SYSCFG_BASE + 0x10u)   /* EXTI8..11  */
#define SYSCFG_EXTICR4     MMIO32(SYSCFG_BASE + 0x14u)   /* EXTI12..15 */
#define SYSCFG_CMPCR       MMIO32(SYSCFG_BASE + 0x20u)

/* SYSCFG_MEMRMP */
#define SYSCFG_MEMRMP_MEM_MODE_MASK   (7u << 0)
#define SYSCFG_MEMRMP_MEM_MODE_FLASH  (0u << 0)
#define SYSCFG_MEMRMP_MEM_MODE_SYSTEM (1u << 0)
#define SYSCFG_MEMRMP_MEM_MODE_FMC    (2u << 0)
#define SYSCFG_MEMRMP_MEM_MODE_SRAM   (3u << 0)
#define SYSCFG_MEMRMP_MEM_MODE_SDRAM  (4u << 0)
#define SYSCFG_MEMRMP_FB_MODE         (1u << 8)
#define SYSCFG_MEMRMP_SWP_FMC_MASK    (3u << 10)

/* SYSCFG_PMC */
#define SYSCFG_PMC_ADC1DC2            (1u << 16)
#define SYSCFG_PMC_MII_RMII_SEL       (1u << 23)

/*
 * EXTI line -> GPIO port routing. Each EXTICR holds four 4-bit fields.
 * Port codes are GPIO_PORT_A (0) ... GPIO_PORT_K (10).
 *
 *     SYSCFG_EXTICR_FOR(0) = (SYSCFG_EXTICR_FOR(0) & ~SYSCFG_EXTICR_MASK(0))
 *                          | SYSCFG_EXTICR_VAL(0, GPIO_PORT_A);
 */
#define SYSCFG_EXTICR_FOR(line)      MMIO32(SYSCFG_BASE + 0x08u + (((line) >> 2) << 2))
#define SYSCFG_EXTICR_SHIFT(line)    (((line) & 3u) * 4u)
#define SYSCFG_EXTICR_MASK(line)     (0xFu << SYSCFG_EXTICR_SHIFT(line))
#define SYSCFG_EXTICR_VAL(line, port) (((port) & 0xFu) << SYSCFG_EXTICR_SHIFT(line))

/* SYSCFG_CMPCR (I/O compensation cell, recommended above 50 MHz I/O speed) */
#define SYSCFG_CMPCR_CMP_PD           (1u << 0)
#define SYSCFG_CMPCR_READY            (1u << 8)

#endif
