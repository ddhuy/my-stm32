#ifndef STM32F429_PWR_REGS_H
#define STM32F429_PWR_REGS_H

#include "arch/cortex-m4/mmio.h"
#include "soc/stm32f429/periph_base.h"

/* Power controller (RM0090 section 5.4). Enable RCC_APB1ENR_PWREN first. */

#define PWR_CR         MMIO32(PWR_BASE + 0x00u)
#define PWR_CSR        MMIO32(PWR_BASE + 0x04u)

/* PWR_CR */
#define PWR_CR_LPDS         (1u << 0)
#define PWR_CR_PDDS         (1u << 1)
#define PWR_CR_CWUF         (1u << 2)
#define PWR_CR_CSBF         (1u << 3)
#define PWR_CR_PVDE         (1u << 4)
#define PWR_CR_PLS_MASK     (7u << 5)
#define PWR_CR_DBP          (1u << 8)   /* unlock RTC / backup domain */
#define PWR_CR_FPDS         (1u << 9)
#define PWR_CR_LPUDS        (1u << 10)
#define PWR_CR_MRUDS        (1u << 11)
#define PWR_CR_ADCDC1       (1u << 13)
#define PWR_CR_VOS_MASK     (3u << 14)
#define PWR_CR_VOS_SCALE3   (1u << 14)  /* HCLK <= 120 MHz           */
#define PWR_CR_VOS_SCALE2   (2u << 14)  /* HCLK <= 144 (168 w/ OD)   */
#define PWR_CR_VOS_SCALE1   (3u << 14)  /* HCLK <= 168 (180 w/ OD)   */
#define PWR_CR_ODEN         (1u << 16)  /* over-drive, for 180 MHz   */
#define PWR_CR_ODSWEN       (1u << 17)
#define PWR_CR_UDEN_MASK    (3u << 18)

/* PWR_CSR */
#define PWR_CSR_WUF         (1u << 0)
#define PWR_CSR_SBF         (1u << 1)
#define PWR_CSR_PVDO        (1u << 2)
#define PWR_CSR_BRR         (1u << 3)
#define PWR_CSR_EWUP        (1u << 8)
#define PWR_CSR_BRE         (1u << 9)
#define PWR_CSR_VOSRDY      (1u << 14)
#define PWR_CSR_ODRDY       (1u << 16)
#define PWR_CSR_ODSWRDY     (1u << 17)
#define PWR_CSR_UDRDY_MASK  (3u << 18)

#endif
