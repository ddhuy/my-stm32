#ifndef MINIBOOT_BOARD_STM32F429I_DISC1_H
#define MINIBOOT_BOARD_STM32F429I_DISC1_H

/*
 * Board facts: STM32F429I-DISC1. What board.c needs: the device names it
 * looks up and the pins it sets up. The devices themselves (addresses,
 * clocks, crystal, speeds, the gyroscope on SPI5) are described in
 * stm32f429i-disc1.dts. Included by board.c only; drivers never see this
 * file.
 *
 * A device's name is its label in the .dts, so each *_NAME here must
 * match one there.
 */

#include <platdata/stm32f4_rcc.h>

/* Console: USART1 on PA9 (TX) / PA10 (RX),
 * wired to the ST-LINK virtual COM port. The clock, for the banner.
 */
#define CONSOLE_UART_CLK_DEV STM32F4_RCC_NAME
#define CONSOLE_UART_CLK     STM32F4_CLK_USART1
#define CONSOLE_GPIO_NAME    "gpioa"
#define CONSOLE_TX_PIN       9
#define CONSOLE_RX_PIN       10
#define CONSOLE_PIN_AF       7

/* User LEDs, active high */
#define LED_GPIO_NAME       "gpiog"
#define LED_GREEN_PIN       13  /* LD3 */
#define LED_RED_PIN         14  /* LD4 */

/* Gyroscope (L3GD20 or I3G4250D) on SPI5: SCK PF7, MISO PF8, MOSI PF9
 * (AF5); its chip select (PC1) is in the .dts (cs-gpios)
 */
#define GYRO_SPI_GPIO_NAME  "gpiof"
#define GYRO_SPI_SCK_PIN    7
#define GYRO_SPI_MISO_PIN   8
#define GYRO_SPI_MOSI_PIN   9
#define GYRO_SPI_PIN_AF     5
/* SCK and MOSI edges: at the reset speed (low), bit 0 of each byte read
 * at 5.25 MHz was unreliable; "fast" on the STM32 (OSPEEDR 10)
 */
#define GYRO_SPI_PIN_SPEED  GPIO_SPEED_HIGH

/* The gyroscope itself (its node's label) */
#define GYRO_NAME           "gyro"

#endif /* MINIBOOT_BOARD_STM32F429I_DISC1_H */
