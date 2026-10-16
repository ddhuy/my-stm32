#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdbool.h>
#include <stdint.h>


/* Terminal output: LF is expanded to CRLF. Pass strings with LF newlines. */
void console_putc(char c);
void console_puts(const char *s);
void console_puthex(uint32_t value);
/* Raw byte input; no echo or newline processing. */
char console_getc(void);
bool console_try_getc(char *out);
/* Blocking ASCII line editor with echo, Backspace/Delete, and CR/LF/CRLF.
 * Size includes the NUL terminator. Invalid arguments return false immediately.
 * Overflow drains through end-of-line, NUL-terminates the prefix, returns false.
 * Consecutive calls share CRLF state; do not interleave raw input reads.
 */
bool console_gets(char *buf, uint32_t buf_size);
void console_flush(void);

#endif // CONSOLE_H