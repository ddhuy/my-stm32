#ifndef STM32F429_TIM_REGS_H
#define STM32F429_TIM_REGS_H

#include "arch/cortex-m4/mmio.h"
#include "soc/stm32f429/periph_base.h"

/*
 * Timers (RM0090 sections 17-20). One shared layout; registers a given timer
 * does not have read as zero.
 *   TIM1, TIM8         advanced, 16-bit, 4 ch + complementary outputs
 *   TIM2, TIM5         general purpose, 32-bit, 4 ch
 *   TIM3, TIM4         general purpose, 16-bit, 4 ch
 *   TIM9, TIM12        2 ch;  TIM10/11/13/14  1 ch
 *   TIM6, TIM7         basic (no channels), drive the DAC
 *
 * Timer clock: if the APB prescaler is 1 the timer runs at PCLK, otherwise
 * at 2 * PCLK. Update rate = f_tim / ((PSC + 1) * (ARR + 1)).
 */

#define TIM_CR1(base)    MMIO32((base) + 0x00u)
#define TIM_CR2(base)    MMIO32((base) + 0x04u)
#define TIM_SMCR(base)   MMIO32((base) + 0x08u)
#define TIM_DIER(base)   MMIO32((base) + 0x0Cu)
#define TIM_SR(base)     MMIO32((base) + 0x10u)
#define TIM_EGR(base)    MMIO32((base) + 0x14u)
#define TIM_CCMR1(base)  MMIO32((base) + 0x18u)
#define TIM_CCMR2(base)  MMIO32((base) + 0x1Cu)
#define TIM_CCER(base)   MMIO32((base) + 0x20u)
#define TIM_CNT(base)    MMIO32((base) + 0x24u)
#define TIM_PSC(base)    MMIO32((base) + 0x28u)
#define TIM_ARR(base)    MMIO32((base) + 0x2Cu)
#define TIM_RCR(base)    MMIO32((base) + 0x30u)
#define TIM_CCR1(base)   MMIO32((base) + 0x34u)
#define TIM_CCR2(base)   MMIO32((base) + 0x38u)
#define TIM_CCR3(base)   MMIO32((base) + 0x3Cu)
#define TIM_CCR4(base)   MMIO32((base) + 0x40u)
#define TIM_CCR(base, ch) MMIO32((base) + 0x30u + ((ch) * 4u))   /* ch = 1..4 */
#define TIM_BDTR(base)   MMIO32((base) + 0x44u)
#define TIM_DCR(base)    MMIO32((base) + 0x48u)
#define TIM_DMAR(base)   MMIO32((base) + 0x4Cu)
#define TIM_OR(base)     MMIO32((base) + 0x50u)   /* TIM2, TIM5, TIM11 only */

/* TIM_CR1 */
#define TIM_CR1_CEN        (1u << 0)
#define TIM_CR1_UDIS       (1u << 1)
#define TIM_CR1_URS        (1u << 2)
#define TIM_CR1_OPM        (1u << 3)
#define TIM_CR1_DIR        (1u << 4)   /* 1 = down-counter */
#define TIM_CR1_CMS_MASK   (3u << 5)
#define TIM_CR1_ARPE       (1u << 7)
#define TIM_CR1_CKD_MASK   (3u << 8)

/* TIM_CR2 */
#define TIM_CR2_CCDS       (1u << 3)
#define TIM_CR2_MMS_MASK   (7u << 4)
#define TIM_CR2_MMS_RESET  (0u << 4)
#define TIM_CR2_MMS_ENABLE (1u << 4)
#define TIM_CR2_MMS_UPDATE (2u << 4)   /* TRGO on update, e.g. to trigger ADC/DAC */
#define TIM_CR2_TI1S       (1u << 7)

/* TIM_SMCR */
#define TIM_SMCR_SMS_MASK  (7u << 0)
#define TIM_SMCR_TS_MASK   (7u << 4)
#define TIM_SMCR_MSM       (1u << 7)
#define TIM_SMCR_ETF_MASK  (0xFu << 8)
#define TIM_SMCR_ETPS_MASK (3u << 12)
#define TIM_SMCR_ECE       (1u << 14)
#define TIM_SMCR_ETP       (1u << 15)

/* TIM_DIER */
#define TIM_DIER_UIE       (1u << 0)
#define TIM_DIER_CC1IE     (1u << 1)
#define TIM_DIER_CC2IE     (1u << 2)
#define TIM_DIER_CC3IE     (1u << 3)
#define TIM_DIER_CC4IE     (1u << 4)
#define TIM_DIER_COMIE     (1u << 5)
#define TIM_DIER_TIE       (1u << 6)
#define TIM_DIER_BIE       (1u << 7)
#define TIM_DIER_UDE       (1u << 8)
#define TIM_DIER_CC1DE     (1u << 9)
#define TIM_DIER_CC2DE     (1u << 10)
#define TIM_DIER_CC3DE     (1u << 11)
#define TIM_DIER_CC4DE     (1u << 12)
#define TIM_DIER_COMDE     (1u << 13)
#define TIM_DIER_TDE       (1u << 14)

/* TIM_SR (clear a flag by writing 0 to it, 1 to the others) */
#define TIM_SR_UIF         (1u << 0)
#define TIM_SR_CC1IF       (1u << 1)
#define TIM_SR_CC2IF       (1u << 2)
#define TIM_SR_CC3IF       (1u << 3)
#define TIM_SR_CC4IF       (1u << 4)
#define TIM_SR_COMIF       (1u << 5)
#define TIM_SR_TIF         (1u << 6)
#define TIM_SR_BIF         (1u << 7)
#define TIM_SR_CC1OF       (1u << 9)
#define TIM_SR_CC2OF       (1u << 10)
#define TIM_SR_CC3OF       (1u << 11)
#define TIM_SR_CC4OF       (1u << 12)

/* TIM_EGR */
#define TIM_EGR_UG         (1u << 0)   /* reload PSC/ARR immediately */
#define TIM_EGR_CC1G       (1u << 1)
#define TIM_EGR_CC2G       (1u << 2)
#define TIM_EGR_CC3G       (1u << 3)
#define TIM_EGR_CC4G       (1u << 4)
#define TIM_EGR_COMG       (1u << 5)
#define TIM_EGR_TG         (1u << 6)
#define TIM_EGR_BG         (1u << 7)

/*
 * TIM_CCMR1 (channels 1, 2) / TIM_CCMR2 (channels 3, 4), output-compare mode.
 * Each register holds two 8-bit channel fields: low byte = ch1/ch3,
 * high byte = ch2/ch4. Use the _LO / _HI variants, or the n-based helpers.
 */
#define TIM_CCS_OUTPUT     0u
#define TIM_CCS_INPUT_TI1  1u   /* ICx mapped on its own TIx */
#define TIM_CCS_INPUT_TI2  2u
#define TIM_CCS_INPUT_TRC  3u

#define TIM_OCM_FROZEN     0u
#define TIM_OCM_ACTIVE     1u
#define TIM_OCM_INACTIVE   2u
#define TIM_OCM_TOGGLE     3u
#define TIM_OCM_FORCE_LOW  4u
#define TIM_OCM_FORCE_HIGH 5u
#define TIM_OCM_PWM1       6u
#define TIM_OCM_PWM2       7u

#define TIM_CCMR_CCS(ch, v)  (((v) & 3u) << (((((ch) - 1u) & 1u)) * 8u))
#define TIM_CCMR_OCFE(ch)    (1u << (2u + (((ch) - 1u) & 1u) * 8u))
#define TIM_CCMR_OCPE(ch)    (1u << (3u + (((ch) - 1u) & 1u) * 8u))
#define TIM_CCMR_OCM(ch, m)  (((m) & 7u) << (4u + (((ch) - 1u) & 1u) * 8u))
#define TIM_CCMR_OCCE(ch)    (1u << (7u + (((ch) - 1u) & 1u) * 8u))
#define TIM_CCMR_CH_MASK(ch) (0xFFu << ((((ch) - 1u) & 1u) * 8u))
/* Input capture: prescaler and filter */
#define TIM_CCMR_ICPSC(ch, v) (((v) & 3u) << (2u + (((ch) - 1u) & 1u) * 8u))
#define TIM_CCMR_ICF(ch, v)   (((v) & 0xFu) << (4u + (((ch) - 1u) & 1u) * 8u))
/* Which CCMR register holds channel ch (1..4) */
#define TIM_CCMR(base, ch)   MMIO32((base) + 0x18u + ((((ch) - 1u) >> 1) << 2))

/* TIM_CCER: 4 bits per channel */
#define TIM_CCER_CCE(ch)   (1u << (((ch) - 1u) * 4u))
#define TIM_CCER_CCP(ch)   (1u << (((ch) - 1u) * 4u + 1u))
#define TIM_CCER_CCNE(ch)  (1u << (((ch) - 1u) * 4u + 2u))   /* TIM1/8 ch1..3 */
#define TIM_CCER_CCNP(ch)  (1u << (((ch) - 1u) * 4u + 3u))

/* TIM_BDTR (TIM1/TIM8): MOE must be set for outputs to drive the pins */
#define TIM_BDTR_DTG_MASK  (0xFFu << 0)
#define TIM_BDTR_LOCK_MASK (3u << 8)
#define TIM_BDTR_OSSI      (1u << 10)
#define TIM_BDTR_OSSR      (1u << 11)
#define TIM_BDTR_BKE       (1u << 12)
#define TIM_BDTR_BKP       (1u << 13)
#define TIM_BDTR_AOE       (1u << 14)
#define TIM_BDTR_MOE       (1u << 15)

/* TIM2 register definitions */
#define TIM2_CR1   TIM_CR1(TIM2_BASE)
#define TIM2_DIER  TIM_DIER(TIM2_BASE)
#define TIM2_SR    TIM_SR(TIM2_BASE)
#define TIM2_EGR   TIM_EGR(TIM2_BASE)
#define TIM2_CNT   TIM_CNT(TIM2_BASE)
#define TIM2_PSC   TIM_PSC(TIM2_BASE)
#define TIM2_ARR   TIM_ARR(TIM2_BASE)

#endif
