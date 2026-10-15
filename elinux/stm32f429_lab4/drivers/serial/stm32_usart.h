#ifndef STM32_USART_H
#define STM32_USART_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Polled driver for the STM32F4 USART/UART peripheral, 8N1.
 * Every call takes the instance base address (USART1_BASE, ...).
 *
 * The driver does not touch RCC or GPIO: the caller (board code) enables
 * the USART clock and routes the pins first. See board_console_init().
 */
void stm32_usart_init(uint32_t base, uint32_t pclk_hz, uint32_t baud);

/* Sends a single character. */
void stm32_usart_putc(uint32_t base, char c);

/* Sends a NUL-terminated string; "\n" is sent as "\r\n" for terminals. */
void stm32_usart_puts(uint32_t base, const char *s);

/* Waits until the last byte has left the pin (call before reset or jumping to the app). */
void stm32_usart_flush(uint32_t base);

/* Block until a valid character is received. */
char stm32_usart_getc(uint32_t base);

/*
 * Read one line of input.
 *
 * Returns false if the line was too long to fit in the buffer.
 * Excess input is discarded until a newline is received.
 */
bool stm32_usart_gets(uint32_t base, char *buf, uint32_t buf_size);

/* Return immediately.
 * true:  a byte was received and written to *out.
 * false: no valid byte is available.
 * out must point to writable memory.
 */
bool stm32_usart_try_getc(uint32_t base, char *out);

#endif // STM32_USART_H
