#ifndef MINIBOOT_BOARD_STM32F429I_DISC1_H
#define MINIBOOT_BOARD_STM32F429I_DISC1_H

/*
 * Board facts: STM32F429I-DISC1. What is soldered around the SoC.
 * Included by board.c only; drivers never see this file.
 */

#include <mach/stm32f429.h>
#include <platdata/stm32f4_rcc.h>

/* 8 MHz crystal on OSC_IN / OSC_OUT */
#define HSE_HZ              8000000

/* System clock, from the PLL fed by HSE: the chip's maximum without
 * over-drive. APB2 (the console UART) then runs at 84 MHz.
 */
#define BOARD_SYSCLK_HZ     168000000

/* Console: USART1 on PA9 (TX) / PA10 (RX),
 * wired to the ST-LINK virtual COM port
 */
#define CONSOLE_UART_BASE    USART1_BASE
#define CONSOLE_UART_CLK_DEV STM32F4_RCC_NAME
#define CONSOLE_UART_CLK     STM32F4_CLK_USART1
#define CONSOLE_BAUDRATE     115200
#define CONSOLE_GPIO_NAME    "gpioa"
#define CONSOLE_GPIO_BASE    GPIOA_BASE
#define CONSOLE_GPIO_CLK     STM32F4_CLK_GPIOA
#define CONSOLE_TX_PIN       9
#define CONSOLE_RX_PIN       10
#define CONSOLE_PIN_AF       7

/* User LEDs, active high */
#define LED_GPIO_NAME       "gpiog"
#define LED_GPIO_BASE       GPIOG_BASE
#define LED_GPIO_CLK        STM32F4_CLK_GPIOG
#define LED_GREEN_PIN       13  /* LD3 */
#define LED_RED_PIN         14  /* LD4 */

/* Gyroscope (L3GD20 or I3G4250D) on SPI5: SCK PF7, MISO PF8, MOSI PF9
 * (AF5); chip select PC1, a GPIO driven by the SPI driver
 */
#define GYRO_SPI_NAME       "spi5"
#define GYRO_SPI_BASE       SPI5_BASE
#define GYRO_SPI_CLK        STM32F4_CLK_SPI5
#define GYRO_SPI_GPIO_NAME  "gpiof"
#define GYRO_SPI_GPIO_BASE  GPIOF_BASE
#define GYRO_SPI_GPIO_CLK   STM32F4_CLK_GPIOF
#define GYRO_SPI_SCK_PIN    7
#define GYRO_SPI_MISO_PIN   8
#define GYRO_SPI_MOSI_PIN   9
#define GYRO_SPI_PIN_AF     5
/* SCK and MOSI edges: at the reset speed (low), bit 0 of each byte read
 * at 5.25 MHz was unreliable; "fast" on the STM32 (OSPEEDR 10)
 */
#define GYRO_SPI_PIN_SPEED  GPIO_SPEED_HIGH
#define GYRO_CS_GPIO_NAME   "gpioc"
#define GYRO_CS_GPIO_BASE   GPIOC_BASE
#define GYRO_CS_GPIO_CLK    STM32F4_CLK_GPIOC
#define GYRO_CS_PIN         1

/* The gyroscope itself: chip select 0 of SPI5, at most 10 MHz */
#define GYRO_NAME           "gyro"
#define GYRO_SPI_CS         0
#define GYRO_SPI_MAX_HZ     10000000

#endif /* MINIBOOT_BOARD_STM32F429I_DISC1_H */
