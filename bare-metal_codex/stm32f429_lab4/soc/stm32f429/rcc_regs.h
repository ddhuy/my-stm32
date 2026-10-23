#ifndef STM32F429_RCC_REGS_H
#define STM32F429_RCC_REGS_H

#include "arch/cortex-m4/mmio.h"
#include "soc/stm32f429/periph_base.h"

/* Reset and clock control (RM0090 section 6.3) */

#define HSI_HZ          16000000u    /* internal RC oscillator */

#define RCC_CR          MMIO32(RCC_BASE + 0x00u)
#define RCC_PLLCFGR     MMIO32(RCC_BASE + 0x04u)
#define RCC_CFGR        MMIO32(RCC_BASE + 0x08u)
#define RCC_CIR         MMIO32(RCC_BASE + 0x0Cu)
#define RCC_AHB1RSTR    MMIO32(RCC_BASE + 0x10u)
#define RCC_AHB2RSTR    MMIO32(RCC_BASE + 0x14u)
#define RCC_AHB3RSTR    MMIO32(RCC_BASE + 0x18u)
#define RCC_APB1RSTR    MMIO32(RCC_BASE + 0x20u)
#define RCC_APB2RSTR    MMIO32(RCC_BASE + 0x24u)
#define RCC_AHB1ENR     MMIO32(RCC_BASE + 0x30u)
#define RCC_AHB2ENR     MMIO32(RCC_BASE + 0x34u)
#define RCC_AHB3ENR     MMIO32(RCC_BASE + 0x38u)
#define RCC_APB1ENR     MMIO32(RCC_BASE + 0x40u)
#define RCC_APB2ENR     MMIO32(RCC_BASE + 0x44u)
#define RCC_AHB1LPENR   MMIO32(RCC_BASE + 0x50u)
#define RCC_AHB2LPENR   MMIO32(RCC_BASE + 0x54u)
#define RCC_AHB3LPENR   MMIO32(RCC_BASE + 0x58u)
#define RCC_APB1LPENR   MMIO32(RCC_BASE + 0x60u)
#define RCC_APB2LPENR   MMIO32(RCC_BASE + 0x64u)
#define RCC_BDCR        MMIO32(RCC_BASE + 0x70u)
#define RCC_CSR         MMIO32(RCC_BASE + 0x74u)
#define RCC_SSCGR       MMIO32(RCC_BASE + 0x80u)
#define RCC_PLLI2SCFGR  MMIO32(RCC_BASE + 0x84u)
#define RCC_PLLSAICFGR  MMIO32(RCC_BASE + 0x88u)
#define RCC_DCKCFGR     MMIO32(RCC_BASE + 0x8Cu)

/* RCC_CR */
#define RCC_CR_HSION       (1u << 0)
#define RCC_CR_HSIRDY      (1u << 1)
#define RCC_CR_HSITRIM_MASK (0x1Fu << 3)
#define RCC_CR_HSICAL_MASK (0xFFu << 8)
#define RCC_CR_HSEON       (1u << 16)
#define RCC_CR_HSERDY      (1u << 17)
#define RCC_CR_HSEBYP      (1u << 18)
#define RCC_CR_CSSON       (1u << 19)
#define RCC_CR_PLLON       (1u << 24)
#define RCC_CR_PLLRDY      (1u << 25)
#define RCC_CR_PLLI2SON    (1u << 26)
#define RCC_CR_PLLI2SRDY   (1u << 27)
#define RCC_CR_PLLSAION    (1u << 28)
#define RCC_CR_PLLSAIRDY   (1u << 29)

/*
 * RCC_PLLCFGR
 *   f_VCO_in  = f_src / PLLM        (must be 1..2 MHz, 2 MHz recommended)
 *   f_VCO     = f_VCO_in * PLLN     (100..432 MHz)
 *   SYSCLK    = f_VCO / PLLP        (PLLP = 2, 4, 6 or 8; max 180 MHz)
 *   USB/SDIO  = f_VCO / PLLQ        (needs exactly 48 MHz for USB)
 */
#define RCC_PLLCFGR_PLLM(m)     (((m) & 0x3Fu) << 0)    /* 2..63    */
#define RCC_PLLCFGR_PLLM_MASK   (0x3Fu << 0)
#define RCC_PLLCFGR_PLLN(n)     (((n) & 0x1FFu) << 6)   /* 50..432  */
#define RCC_PLLCFGR_PLLN_MASK   (0x1FFu << 6)
#define RCC_PLLCFGR_PLLP(p)     (((((p) / 2u) - 1u) & 3u) << 16) /* p = 2,4,6,8 */
#define RCC_PLLCFGR_PLLP_MASK   (3u << 16)
#define RCC_PLLCFGR_PLLSRC_HSE  (1u << 22)              /* 0 = HSI  */
#define RCC_PLLCFGR_PLLQ(q)     (((q) & 0xFu) << 24)    /* 2..15    */
#define RCC_PLLCFGR_PLLQ_MASK   (0xFu << 24)

/* RCC_CFGR */
#define RCC_CFGR_SW_MASK   (3u << 0)
#define RCC_CFGR_SW_HSI    (0u << 0)
#define RCC_CFGR_SW_HSE    (1u << 0)
#define RCC_CFGR_SW_PLL    (2u << 0)
#define RCC_CFGR_SWS_MASK  (3u << 2)
#define RCC_CFGR_SWS_HSI   (0u << 2)
#define RCC_CFGR_SWS_HSE   (1u << 2)
#define RCC_CFGR_SWS_PLL   (2u << 2)

/* AHB prescaler: HCLK = SYSCLK / div */
#define RCC_CFGR_HPRE_MASK    (0xFu << 4)
#define RCC_CFGR_HPRE_DIV1    (0x0u << 4)
#define RCC_CFGR_HPRE_DIV2    (0x8u << 4)
#define RCC_CFGR_HPRE_DIV4    (0x9u << 4)
#define RCC_CFGR_HPRE_DIV8    (0xAu << 4)
#define RCC_CFGR_HPRE_DIV16   (0xBu << 4)
#define RCC_CFGR_HPRE_DIV64   (0xCu << 4)
#define RCC_CFGR_HPRE_DIV128  (0xDu << 4)
#define RCC_CFGR_HPRE_DIV256  (0xEu << 4)
#define RCC_CFGR_HPRE_DIV512  (0xFu << 4)

/* APB1 prescaler (PCLK1 max 45 MHz) */
#define RCC_CFGR_PPRE1_MASK   (7u << 10)
#define RCC_CFGR_PPRE1_DIV1   (0u << 10)
#define RCC_CFGR_PPRE1_DIV2   (4u << 10)
#define RCC_CFGR_PPRE1_DIV4   (5u << 10)
#define RCC_CFGR_PPRE1_DIV8   (6u << 10)
#define RCC_CFGR_PPRE1_DIV16  (7u << 10)

/* APB2 prescaler (PCLK2 max 90 MHz) */
#define RCC_CFGR_PPRE2_MASK   (7u << 13)
#define RCC_CFGR_PPRE2_DIV1   (0u << 13)
#define RCC_CFGR_PPRE2_DIV2   (4u << 13)
#define RCC_CFGR_PPRE2_DIV4   (5u << 13)
#define RCC_CFGR_PPRE2_DIV8   (6u << 13)
#define RCC_CFGR_PPRE2_DIV16  (7u << 13)

/* Prescaler field from a divisor: RCC_CFGR_HPRE(1), RCC_CFGR_PPRE1(4), ... */
#define RCC_CFGR_HPRE_BITS(d)  ((d) == 1u ? 0x0u : (d) == 2u ? 0x8u : (d) == 4u ? 0x9u :    \
                                (d) == 8u ? 0xAu : (d) == 16u ? 0xBu : (d) == 64u ? 0xCu : \
                                (d) == 128u ? 0xDu : (d) == 256u ? 0xEu : 0xFu)
#define RCC_CFGR_PPRE_BITS(d)  ((d) == 1u ? 0u : (d) == 2u ? 4u : (d) == 4u ? 5u :          \
                                (d) == 8u ? 6u : 7u)
#define RCC_CFGR_HPRE(d)       (RCC_CFGR_HPRE_BITS(d) << 4)
#define RCC_CFGR_PPRE1(d)      (RCC_CFGR_PPRE_BITS(d) << 10)
#define RCC_CFGR_PPRE2(d)      (RCC_CFGR_PPRE_BITS(d) << 13)

#define RCC_CFGR_RTCPRE_MASK  (0x1Fu << 16)
#define RCC_CFGR_MCO1_MASK    (3u << 21)   /* 0 HSI, 1 LSE, 2 HSE, 3 PLL    */
#define RCC_CFGR_I2SSRC       (1u << 23)
#define RCC_CFGR_MCO1PRE_MASK (7u << 24)
#define RCC_CFGR_MCO2PRE_MASK (7u << 27)
#define RCC_CFGR_MCO2_MASK    (3u << 30)   /* 0 SYSCLK, 1 PLLI2S, 2 HSE, 3 PLL */

/* RCC_CIR: ready interrupt flags (RO), enables, and clears (WO) */
#define RCC_CIR_LSIRDYF    (1u << 0)
#define RCC_CIR_LSERDYF    (1u << 1)
#define RCC_CIR_HSIRDYF    (1u << 2)
#define RCC_CIR_HSERDYF    (1u << 3)
#define RCC_CIR_PLLRDYF    (1u << 4)
#define RCC_CIR_CSSF       (1u << 7)
#define RCC_CIR_CSSC       (1u << 23)

/*
 * Peripheral clock enables.
 * The matching *RSTR (reset) and *LPENR (clock in sleep mode) registers
 * use the same bit positions, so these masks work for them too.
 */

/* RCC_AHB1ENR */
#define RCC_AHB1ENR_GPIOAEN     (1u << 0)
#define RCC_AHB1ENR_GPIOBEN     (1u << 1)
#define RCC_AHB1ENR_GPIOCEN     (1u << 2)
#define RCC_AHB1ENR_GPIODEN     (1u << 3)
#define RCC_AHB1ENR_GPIOEEN     (1u << 4)
#define RCC_AHB1ENR_GPIOFEN     (1u << 5)
#define RCC_AHB1ENR_GPIOGEN     (1u << 6)
#define RCC_AHB1ENR_GPIOHEN     (1u << 7)
#define RCC_AHB1ENR_GPIOIEN     (1u << 8)
#define RCC_AHB1ENR_GPIOJEN     (1u << 9)
#define RCC_AHB1ENR_GPIOKEN     (1u << 10)
#define RCC_AHB1ENR_CRCEN       (1u << 12)
#define RCC_AHB1ENR_BKPSRAMEN   (1u << 18)
#define RCC_AHB1ENR_CCMDATARAMEN (1u << 20)
#define RCC_AHB1ENR_DMA1EN      (1u << 21)
#define RCC_AHB1ENR_DMA2EN      (1u << 22)
#define RCC_AHB1ENR_DMA2DEN     (1u << 23)
#define RCC_AHB1ENR_ETHMACEN    (1u << 25)
#define RCC_AHB1ENR_OTGHSEN     (1u << 29)
#define RCC_AHB1ENR_OTGHSULPIEN (1u << 30)

/* GPIO port n (0 = A ... 10 = K) */
#define RCC_AHB1ENR_GPIOEN(n)   (1u << (n))

/* RCC_AHB2ENR */
#define RCC_AHB2ENR_DCMIEN      (1u << 0)
#define RCC_AHB2ENR_CRYPEN      (1u << 4)
#define RCC_AHB2ENR_HASHEN      (1u << 5)
#define RCC_AHB2ENR_RNGEN       (1u << 6)
#define RCC_AHB2ENR_OTGFSEN     (1u << 7)

/* RCC_AHB3ENR */
#define RCC_AHB3ENR_FMCEN       (1u << 0)

/* RCC_APB1ENR */
#define RCC_APB1ENR_TIM2EN      (1u << 0)
#define RCC_APB1ENR_TIM3EN      (1u << 1)
#define RCC_APB1ENR_TIM4EN      (1u << 2)
#define RCC_APB1ENR_TIM5EN      (1u << 3)
#define RCC_APB1ENR_TIM6EN      (1u << 4)
#define RCC_APB1ENR_TIM7EN      (1u << 5)
#define RCC_APB1ENR_TIM12EN     (1u << 6)
#define RCC_APB1ENR_TIM13EN     (1u << 7)
#define RCC_APB1ENR_TIM14EN     (1u << 8)
#define RCC_APB1ENR_WWDGEN      (1u << 11)
#define RCC_APB1ENR_SPI2EN      (1u << 14)
#define RCC_APB1ENR_SPI3EN      (1u << 15)
#define RCC_APB1ENR_USART2EN    (1u << 17)
#define RCC_APB1ENR_USART3EN    (1u << 18)
#define RCC_APB1ENR_UART4EN     (1u << 19)
#define RCC_APB1ENR_UART5EN     (1u << 20)
#define RCC_APB1ENR_I2C1EN      (1u << 21)
#define RCC_APB1ENR_I2C2EN      (1u << 22)
#define RCC_APB1ENR_I2C3EN      (1u << 23)
#define RCC_APB1ENR_CAN1EN      (1u << 25)
#define RCC_APB1ENR_CAN2EN      (1u << 26)
#define RCC_APB1ENR_PWREN       (1u << 28)
#define RCC_APB1ENR_DACEN       (1u << 29)
#define RCC_APB1ENR_UART7EN     (1u << 30)
#define RCC_APB1ENR_UART8EN     (1u << 31)

/* RCC_APB2ENR */
#define RCC_APB2ENR_TIM1EN      (1u << 0)
#define RCC_APB2ENR_TIM8EN      (1u << 1)
#define RCC_APB2ENR_USART1EN    (1u << 4)
#define RCC_APB2ENR_USART6EN    (1u << 5)
#define RCC_APB2ENR_ADC1EN      (1u << 8)
#define RCC_APB2ENR_ADC2EN      (1u << 9)
#define RCC_APB2ENR_ADC3EN      (1u << 10)
#define RCC_APB2ENR_SDIOEN      (1u << 11)
#define RCC_APB2ENR_SPI1EN      (1u << 12)
#define RCC_APB2ENR_SPI4EN      (1u << 13)
#define RCC_APB2ENR_SYSCFGEN    (1u << 14)
#define RCC_APB2ENR_TIM9EN      (1u << 16)
#define RCC_APB2ENR_TIM10EN     (1u << 17)
#define RCC_APB2ENR_TIM11EN     (1u << 18)
#define RCC_APB2ENR_SPI5EN      (1u << 20)
#define RCC_APB2ENR_SPI6EN      (1u << 21)
#define RCC_APB2ENR_SAI1EN      (1u << 22)
#define RCC_APB2ENR_LTDCEN      (1u << 26)

/* RCC_BDCR (write-protected: set PWR_CR_DBP first) */
#define RCC_BDCR_LSEON        (1u << 0)
#define RCC_BDCR_LSERDY       (1u << 1)
#define RCC_BDCR_LSEBYP       (1u << 2)
#define RCC_BDCR_RTCSEL_MASK  (3u << 8)
#define RCC_BDCR_RTCSEL_LSE   (1u << 8)
#define RCC_BDCR_RTCSEL_LSI   (2u << 8)
#define RCC_BDCR_RTCSEL_HSE   (3u << 8)
#define RCC_BDCR_RTCEN        (1u << 15)
#define RCC_BDCR_BDRST        (1u << 16)

/* RCC_CSR: LSI and reset-cause flags (clear them all with RMVF) */
#define RCC_CSR_LSION         (1u << 0)
#define RCC_CSR_LSIRDY        (1u << 1)
#define RCC_CSR_RMVF          (1u << 24)
#define RCC_CSR_BORRSTF       (1u << 25)
#define RCC_CSR_PINRSTF       (1u << 26)
#define RCC_CSR_PORRSTF       (1u << 27)
#define RCC_CSR_SFTRSTF       (1u << 28)
#define RCC_CSR_IWDGRSTF      (1u << 29)
#define RCC_CSR_WWDGRSTF      (1u << 30)
#define RCC_CSR_LPWRRSTF      (1u << 31)

/*
 * RCC_PLLSAICFGR: PLLSAI feeds the LTDC pixel clock.
 *   f_VCO  = (f_src / PLLM) * PLLSAIN
 *   LCD_CLK = f_VCO / PLLSAIR / PLLSAIDIVR (see RCC_DCKCFGR)
 */
#define RCC_PLLSAICFGR_PLLSAIN(n)   (((n) & 0x1FFu) << 6)   /* 50..432 */
#define RCC_PLLSAICFGR_PLLSAIN_MASK (0x1FFu << 6)
#define RCC_PLLSAICFGR_PLLSAIQ(q)   (((q) & 0xFu) << 24)    /* 2..15   */
#define RCC_PLLSAICFGR_PLLSAIQ_MASK (0xFu << 24)
#define RCC_PLLSAICFGR_PLLSAIR(r)   (((r) & 7u) << 28)      /* 2..7    */
#define RCC_PLLSAICFGR_PLLSAIR_MASK (7u << 28)

/* RCC_DCKCFGR */
#define RCC_DCKCFGR_PLLSAIDIVR_MASK  (3u << 16)
#define RCC_DCKCFGR_PLLSAIDIVR_DIV2  (0u << 16)
#define RCC_DCKCFGR_PLLSAIDIVR_DIV4  (1u << 16)
#define RCC_DCKCFGR_PLLSAIDIVR_DIV8  (2u << 16)
#define RCC_DCKCFGR_PLLSAIDIVR_DIV16 (3u << 16)
#define RCC_DCKCFGR_TIMPRE           (1u << 24)

#endif
