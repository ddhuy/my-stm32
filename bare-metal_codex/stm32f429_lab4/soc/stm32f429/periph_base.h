#ifndef STM32F429_PERIPH_BASE_H
#define STM32F429_PERIPH_BASE_H

/*
 * STM32F429ZI memory map (RM0090 section 2.3, table 1).
 * Each *_regs.h header includes this file for its base address.
 * Safe to include from linker scripts (see arch/const.h).
 */

#include "arch/cortex-m4/const.h"

/* ---- Memories ---------------------------------------------------------- */
#define FLASH_BASE          _AC(0x08000000, u)   /* 2 MB, two 1 MB banks          */
#define FLASH_SIZE          (_AC(2048, u) * 1024)
#define CCMRAM_BASE         _AC(0x10000000, u)   /* 64 KB, CPU (D-bus) only, no DMA */
#define CCMRAM_SIZE         (_AC(64, u) * 1024)
#define SYSTEM_MEMORY_BASE  _AC(0x1FFF0000, u)   /* ROM bootloader                */
#define OTP_BASE            _AC(0x1FFF7800, u)
#define OPTION_BYTES_BASE   _AC(0x1FFFC000, u)
#define SRAM1_BASE          _AC(0x20000000, u)   /* 112 KB                         */
#define SRAM2_BASE          _AC(0x2001C000, u)   /* 16 KB                          */
#define SRAM3_BASE          _AC(0x20020000, u)   /* 64 KB                          */
#define SRAM_BASE           SRAM1_BASE    /* SRAM1..3 are contiguous: 192 KB */
#define SRAM_SIZE           (_AC(192, u) * 1024)
#define BKPSRAM_BASE        _AC(0x40024000, u)   /* 4 KB backup SRAM               */
#define SDRAM_BANK1_BASE    _AC(0xC0000000, u)   /* FMC SDRAM bank 1 (SDNE0)       */
#define SDRAM_BANK2_BASE    _AC(0xD0000000, u)   /* FMC SDRAM bank 2 (SDNE1)       */

/* Device identification (RM0090 section 39) */
#define UID_BASE            _AC(0x1FFF7A10, u)   /* 96-bit unique ID               */
#define FLASHSIZE_BASE      _AC(0x1FFF7A22, u)   /* 16-bit flash size in KB        */

/* ---- APB1 -------------------------------------------------------------- */
#define APB1PERIPH_BASE     _AC(0x40000000, u)
#define TIM2_BASE           _AC(0x40000000, u)
#define TIM3_BASE           _AC(0x40000400, u)
#define TIM4_BASE           _AC(0x40000800, u)
#define TIM5_BASE           _AC(0x40000C00, u)
#define TIM6_BASE           _AC(0x40001000, u)
#define TIM7_BASE           _AC(0x40001400, u)
#define TIM12_BASE          _AC(0x40001800, u)
#define TIM13_BASE          _AC(0x40001C00, u)
#define TIM14_BASE          _AC(0x40002000, u)
#define RTC_BASE            _AC(0x40002800, u)
#define WWDG_BASE           _AC(0x40002C00, u)
#define IWDG_BASE           _AC(0x40003000, u)
#define I2S2EXT_BASE        _AC(0x40003400, u)
#define SPI2_BASE           _AC(0x40003800, u)
#define SPI3_BASE           _AC(0x40003C00, u)
#define I2S3EXT_BASE        _AC(0x40004000, u)
#define USART2_BASE         _AC(0x40004400, u)
#define USART3_BASE         _AC(0x40004800, u)
#define UART4_BASE          _AC(0x40004C00, u)
#define UART5_BASE          _AC(0x40005000, u)
#define I2C1_BASE           _AC(0x40005400, u)
#define I2C2_BASE           _AC(0x40005800, u)
#define I2C3_BASE           _AC(0x40005C00, u)
#define CAN1_BASE           _AC(0x40006400, u)
#define CAN2_BASE           _AC(0x40006800, u)
#define PWR_BASE            _AC(0x40007000, u)
#define DAC_BASE            _AC(0x40007400, u)
#define UART7_BASE          _AC(0x40007800, u)
#define UART8_BASE          _AC(0x40007C00, u)

/* ---- APB2 -------------------------------------------------------------- */
#define APB2PERIPH_BASE     _AC(0x40010000, u)
#define TIM1_BASE           _AC(0x40010000, u)
#define TIM8_BASE           _AC(0x40010400, u)
#define USART1_BASE         _AC(0x40011000, u)
#define USART6_BASE         _AC(0x40011400, u)
#define ADC1_BASE           _AC(0x40012000, u)
#define ADC2_BASE           _AC(0x40012100, u)
#define ADC3_BASE           _AC(0x40012200, u)
#define ADC_COMMON_BASE     _AC(0x40012300, u)
#define SDIO_BASE           _AC(0x40012C00, u)
#define SPI1_BASE           _AC(0x40013000, u)
#define SPI4_BASE           _AC(0x40013400, u)
#define SYSCFG_BASE         _AC(0x40013800, u)
#define EXTI_BASE           _AC(0x40013C00, u)
#define TIM9_BASE           _AC(0x40014000, u)
#define TIM10_BASE          _AC(0x40014400, u)
#define TIM11_BASE          _AC(0x40014800, u)
#define SPI5_BASE           _AC(0x40015000, u)
#define SPI6_BASE           _AC(0x40015400, u)
#define SAI1_BASE           _AC(0x40015800, u)
#define LTDC_BASE           _AC(0x40016800, u)

/* ---- AHB1 -------------------------------------------------------------- */
#define AHB1PERIPH_BASE     _AC(0x40020000, u)
#define GPIOA_BASE          _AC(0x40020000, u)
#define GPIOB_BASE          _AC(0x40020400, u)
#define GPIOC_BASE          _AC(0x40020800, u)
#define GPIOD_BASE          _AC(0x40020C00, u)
#define GPIOE_BASE          _AC(0x40021000, u)
#define GPIOF_BASE          _AC(0x40021400, u)
#define GPIOG_BASE          _AC(0x40021800, u)
#define GPIOH_BASE          _AC(0x40021C00, u)
#define GPIOI_BASE          _AC(0x40022000, u)
#define GPIOJ_BASE          _AC(0x40022400, u)
#define GPIOK_BASE          _AC(0x40022800, u)
#define CRC_BASE            _AC(0x40023000, u)
#define RCC_BASE            _AC(0x40023800, u)
#define FLASH_REG_BASE      _AC(0x40023C00, u)   /* flash interface registers      */
#define DMA1_BASE           _AC(0x40026000, u)
#define DMA2_BASE           _AC(0x40026400, u)
#define ETH_BASE            _AC(0x40028000, u)
#define DMA2D_BASE          _AC(0x4002B000, u)
#define USB_OTG_HS_BASE     _AC(0x40040000, u)

/* ---- AHB2 -------------------------------------------------------------- */
#define AHB2PERIPH_BASE     _AC(0x50000000, u)
#define USB_OTG_FS_BASE     _AC(0x50000000, u)
#define DCMI_BASE           _AC(0x50050000, u)
#define CRYP_BASE           _AC(0x50060000, u)
#define HASH_BASE           _AC(0x50060400, u)
#define RNG_BASE            _AC(0x50060800, u)

/* ---- AHB3 -------------------------------------------------------------- */
#define FMC_BASE            _AC(0xA0000000, u)   /* FMC control registers          */

/* ---- ST debug block (the Cortex-M4 core addresses are in arch/core_base.h) */
#define DBGMCU_BASE         _AC(0xE0042000, u)

#endif
