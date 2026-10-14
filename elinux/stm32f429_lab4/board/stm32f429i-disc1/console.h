#ifndef CONSOLE_H
#define CONSOLE_H

#include <stdbool.h>
#include <stdint.h>


void console_putc(char c);
void console_puts(const char *s);
void console_puthex(uint32_t value);
char console_getc();
bool console_gets(char *buf, uint32_t buf_size);
void console_flush();

#endif // CONSOLE_H