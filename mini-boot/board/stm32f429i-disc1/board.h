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

/* Console: USART1 on PA9 (TX) / PA10 (RX),
 * wired to the ST-LINK virtual COM port
 */
#define CONSOLE_UART_BASE   USART1_BASE
#define CONSOLE_UART_CLK    STM32F4_CLK_USART1
#define CONSOLE_BAUDRATE    115200
#define CONSOLE_GPIO_NAME   "gpioa"
#define CONSOLE_GPIO_BASE   GPIOA_BASE
#define CONSOLE_GPIO_CLK    STM32F4_CLK_GPIOA
#define CONSOLE_TX_PIN      9
#define CONSOLE_RX_PIN      10
#define CONSOLE_PIN_AF      7

/* User LEDs, active high */
#define LED_GPIO_NAME       "gpiog"
#define LED_GPIO_BASE       GPIOG_BASE
#define LED_GPIO_CLK        STM32F4_CLK_GPIOG
#define LED_GREEN_PIN       13  /* LD3 */
#define LED_RED_PIN         14  /* LD4 */

#endif /* MINIBOOT_BOARD_STM32F429I_DISC1_H */
