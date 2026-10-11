#ifndef STM32_USART_H
#define STM32_USART_H

#include <stdint.h>

/*
 * Polled driver for the STM32F4 USART/UART peripheral, 8N1.
 * Every call takes the instance base address (USART1_BASE, ...).
 *
 * The driver does not touch RCC or GPIO: the caller (board code) enables
 * the USART clock and routes the pins first. See board_console_init().
 */
void stm32_usart_init(uint32_t base, uint32_t pclk_hz, uint32_t baud);

void stm32_usart_putc(uint32_t base, char c);

/* Sends a NUL-terminated string; "\n" is sent as "\r\n" for terminals. */
void stm32_usart_puts(uint32_t base, const char *s);

/* Waits until the last byte has left the pin (call before reset or jumping to the app). */
void stm32_usart_flush(uint32_t base);

#endif // STM32_USART_H
