#ifndef STM32F429_IRQ_H
#define STM32F429_IRQ_H

/*
 * STM32F429 interrupt configuration for the Cortex-M4 NVIC.
 * IRQ n is vector table entry n + 16 (after the 16 system exceptions).
 */

/* ST implements 4 priority bits: 0 = highest .. 15 = lowest */
#define NVIC_PRIO_BITS 4u

#include "arch/cortex-m4/nvic_regs.h"

/* STM32F42x/43x interrupt numbers (RM0090 table 62) */
#define IRQ_WWDG                 0u
#define IRQ_PVD                  1u
#define IRQ_TAMP_STAMP           2u
#define IRQ_RTC_WKUP             3u
#define IRQ_FLASH                4u
#define IRQ_RCC                  5u
#define IRQ_EXTI0                6u
#define IRQ_EXTI1                7u
#define IRQ_EXTI2                8u
#define IRQ_EXTI3                9u
#define IRQ_EXTI4                10u
#define IRQ_DMA1_STREAM0         11u
#define IRQ_DMA1_STREAM1         12u
#define IRQ_DMA1_STREAM2         13u
#define IRQ_DMA1_STREAM3         14u
#define IRQ_DMA1_STREAM4         15u
#define IRQ_DMA1_STREAM5         16u
#define IRQ_DMA1_STREAM6         17u
#define IRQ_ADC                  18u
#define IRQ_CAN1_TX              19u
#define IRQ_CAN1_RX0             20u
#define IRQ_CAN1_RX1             21u
#define IRQ_CAN1_SCE             22u
#define IRQ_EXTI9_5              23u
#define IRQ_TIM1_BRK_TIM9        24u
#define IRQ_TIM1_UP_TIM10        25u
#define IRQ_TIM1_TRG_COM_TIM11   26u
#define IRQ_TIM1_CC              27u
#define IRQ_TIM2                 28u
#define IRQ_TIM3                 29u
#define IRQ_TIM4                 30u
#define IRQ_I2C1_EV              31u
#define IRQ_I2C1_ER              32u
#define IRQ_I2C2_EV              33u
#define IRQ_I2C2_ER              34u
#define IRQ_SPI1                 35u
#define IRQ_SPI2                 36u
#define IRQ_USART1               37u
#define IRQ_USART2               38u
#define IRQ_USART3               39u
#define IRQ_EXTI15_10            40u
#define IRQ_RTC_ALARM            41u
#define IRQ_OTG_FS_WKUP          42u
#define IRQ_TIM8_BRK_TIM12       43u
#define IRQ_TIM8_UP_TIM13        44u
#define IRQ_TIM8_TRG_COM_TIM14   45u
#define IRQ_TIM8_CC              46u
#define IRQ_DMA1_STREAM7         47u
#define IRQ_FMC                  48u
#define IRQ_SDIO                 49u
#define IRQ_TIM5                 50u
#define IRQ_SPI3                 51u
#define IRQ_UART4                52u
#define IRQ_UART5                53u
#define IRQ_TIM6_DAC             54u
#define IRQ_TIM7                 55u
#define IRQ_DMA2_STREAM0         56u
#define IRQ_DMA2_STREAM1         57u
#define IRQ_DMA2_STREAM2         58u
#define IRQ_DMA2_STREAM3         59u
#define IRQ_DMA2_STREAM4         60u
#define IRQ_ETH                  61u
#define IRQ_ETH_WKUP             62u
#define IRQ_CAN2_TX              63u
#define IRQ_CAN2_RX0             64u
#define IRQ_CAN2_RX1             65u
#define IRQ_CAN2_SCE             66u
#define IRQ_OTG_FS               67u
#define IRQ_DMA2_STREAM5         68u
#define IRQ_DMA2_STREAM6         69u
#define IRQ_DMA2_STREAM7         70u
#define IRQ_USART6               71u
#define IRQ_I2C3_EV              72u
#define IRQ_I2C3_ER              73u
#define IRQ_OTG_HS_EP1_OUT       74u
#define IRQ_OTG_HS_EP1_IN        75u
#define IRQ_OTG_HS_WKUP          76u
#define IRQ_OTG_HS               77u
#define IRQ_DCMI                 78u
/* 79 is CRYP on STM32F43x only; reserved on the F429 */
#define IRQ_HASH_RNG             80u   /* RNG only on the F429 */
#define IRQ_FPU                  81u
#define IRQ_UART7                82u
#define IRQ_UART8                83u
#define IRQ_SPI4                 84u
#define IRQ_SPI5                 85u
#define IRQ_SPI6                 86u
#define IRQ_SAI1                 87u
#define IRQ_LTDC                 88u
#define IRQ_LTDC_ER              89u
#define IRQ_DMA2D                90u

#define IRQ_COUNT                91u

#endif
