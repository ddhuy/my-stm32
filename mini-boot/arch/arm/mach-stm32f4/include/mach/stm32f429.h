#ifndef MINIBOOT_MACH_STM32F429_H
#define MINIBOOT_MACH_STM32F429_H

/*
 * SoC facts: the STM32F429 memory map (RM0090, section 2.3).
 *
 * Base addresses only. A peripheral's register layout is private to the
 * code that drives it and lives next to that .c file.
 */

/* Memories */
#define FLASH_BASE      0x08000000
#define CCMRAM_BASE     0x10000000
#define SRAM_BASE       0x20000000
#define FMC_SDRAM1_BASE 0xC0000000  /* FMC SDRAM bank 1 */
#define FMC_SDRAM2_BASE 0xD0000000  /* FMC SDRAM bank 2 */

/* APB1 */
#define SPI2_BASE       0x40003800
#define SPI3_BASE       0x40003C00
#define USART2_BASE     0x40004400
#define USART3_BASE     0x40004800
#define UART4_BASE      0x40004C00
#define UART5_BASE      0x40005000
#define PWR_BASE        0x40007000

/* APB2 */
#define USART1_BASE     0x40011000
#define USART6_BASE     0x40011400
#define SPI1_BASE       0x40013000
#define SPI4_BASE       0x40013400
#define SYSCFG_BASE     0x40013800
#define EXTI_BASE       0x40013C00
#define SPI5_BASE       0x40015000
#define SPI6_BASE       0x40015400
#define LTDC_BASE       0x40016800

/* AHB1 */
#define GPIOA_BASE      0x40020000
#define GPIOB_BASE      0x40020400
#define GPIOC_BASE      0x40020800
#define GPIOD_BASE      0x40020C00
#define GPIOE_BASE      0x40021000
#define GPIOF_BASE      0x40021400
#define GPIOG_BASE      0x40021800
#define GPIOH_BASE      0x40021C00
#define GPIOI_BASE      0x40022000
#define GPIOJ_BASE      0x40022400
#define GPIOK_BASE      0x40022800
#define RCC_BASE        0x40023800
#define FLASH_R_BASE    0x40023C00  /* flash interface registers (ACR, ...) */

/* AHB3 */
#define FMC_BASE        0xA0000000

#endif /* MINIBOOT_MACH_STM32F429_H */
