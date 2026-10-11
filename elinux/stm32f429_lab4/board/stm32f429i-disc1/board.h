#ifndef BOARD_H
#define BOARD_H

#include "arch/cortex-m4/const.h"
#include "soc/stm32f429/periph_base.h"

/*
 * STM32F429I-DISC1 board wiring (UM1670).
 *
 * Code includes "board.h"; the Makefile's BOARD variable picks which
 * board/<name>/include directory is on the include path.
 * Each signal is a port base + pin number, for use with gpio_regs.h:
 *
 *     GPIO_BSRR(LED_GREEN_PORT) = GPIO_BSRR_SET(LED_GREEN_PIN);
 *
 * Also included by linker scripts (via memory_map.h): C-only parts are
 * inside #ifndef __ASSEMBLER__ at the bottom.
 */

/* Clocks (the 16 MHz HSI is part of the chip: HSI_HZ in soc/rcc_regs.h) */
#define BOARD_HSE_HZ            8000000u     /* 8 MHz crystal (X2) */

/* Bus clocks after board_clock_init(); board.c programs these divisors */
#define CLOCK_SYSCLK_HZ         168000000u
#define BOARD_AHB_DIV           1u
#define BOARD_APB1_DIV          4u
#define BOARD_APB2_DIV          2u
#define CLOCK_HCLK_HZ           (CLOCK_SYSCLK_HZ / BOARD_AHB_DIV) /* core, FMC, DMA, GPIO */
#define CLOCK_PCLK1_HZ          (CLOCK_HCLK_HZ / BOARD_APB1_DIV)  /* 42 MHz (max 45)      */
#define CLOCK_PCLK2_HZ          (CLOCK_HCLK_HZ / BOARD_APB2_DIV)  /* 84 MHz (max 90)      */

/* User LEDs, active high */
#define LED_GREEN_PORT          GPIOG_BASE   /* LD3 */
#define LED_GREEN_PIN           13u
#define LED_RED_PORT            GPIOG_BASE   /* LD4 */
#define LED_RED_PIN             14u

/* User button B1, active high (external pull-down); EXTI line 0 */
#define BUTTON_PORT             GPIOA_BASE
#define BUTTON_PIN              0u

/* ST-LINK virtual COM port: USART1, AF7. Used as the console. */
#define VCP_USART               USART1_BASE
#define CONSOLE_USART           VCP_USART
#define VCP_TX_PORT             GPIOA_BASE
#define VCP_TX_PIN              9u
#define VCP_RX_PORT             GPIOA_BASE
#define VCP_RX_PIN              10u

/* 8 MB SDRAM (IS42S16400J: 4 banks x 4096 rows x 256 columns x 16 bit) on FMC bank 2 */
#define SDRAM_BASE              SDRAM_BANK2_BASE
#define SDRAM_SIZE              (_AC(8, u) * 1024 * 1024)

/* SPI5 (AF5): shared by the L3GD20 gyroscope and the ILI9341 LCD controller */
#define BOARD_SPI               SPI5_BASE
#define SPI5_SCK_PORT           GPIOF_BASE
#define SPI5_SCK_PIN            7u
#define SPI5_MISO_PORT          GPIOF_BASE
#define SPI5_MISO_PIN           8u
#define SPI5_MOSI_PORT          GPIOF_BASE
#define SPI5_MOSI_PIN           9u

/* L3GD20 gyroscope */
#define GYRO_CS_PORT            GPIOC_BASE   /* active low */
#define GYRO_CS_PIN             1u
#define GYRO_INT1_PORT          GPIOA_BASE
#define GYRO_INT1_PIN           1u
#define GYRO_INT2_PORT          GPIOA_BASE
#define GYRO_INT2_PIN           2u

/* ILI9341 LCD (240 x 320) control pins; pixel data goes through LTDC */
#define LCD_WIDTH               240u
#define LCD_HEIGHT              320u
#define LCD_CS_PORT             GPIOC_BASE   /* CSX, active low  */
#define LCD_CS_PIN              2u
#define LCD_DCX_PORT            GPIOD_BASE   /* WRX/DCX: 0 = command, 1 = data */
#define LCD_DCX_PIN             13u

/* STMPE811 touch controller on I2C3 (AF4) */
#define TOUCH_I2C               I2C3_BASE
#define TOUCH_I2C_ADDR          0x41u        /* 7-bit */
#define TOUCH_SCL_PORT          GPIOA_BASE
#define TOUCH_SCL_PIN           8u
#define TOUCH_SDA_PORT          GPIOC_BASE
#define TOUCH_SDA_PIN           9u
#define TOUCH_INT_PORT          GPIOA_BASE
#define TOUCH_INT_PIN           15u

#ifndef __ASSEMBLER__

#include <stdbool.h>
#include <stdint.h>

/*
 * Board setup, implemented in board/<name>/. Call in this order:
 *   board_clock_init()     PLL and bus clocks (CLOCK_*_HZ above)
 *   board_console_init()   pclk_hz = CLOCK_PCLK2_HZ (USART1 is on APB2)
 *   board_sdram_init()     needs board_clock_init() first: refresh rate depends on HCLK
 */
bool board_clock_init(void);
void board_console_init(uint32_t pclk_hz, uint32_t baud);
bool board_sdram_init(void);

#endif /* __ASSEMBLER__ */

#endif
