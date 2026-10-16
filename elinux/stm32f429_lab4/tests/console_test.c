#include <assert.h>
#include <stddef.h>
#include <string.h>
#include "board/stm32f429i-disc1/console.h"
#include "drivers/serial/stm32_usart.h"

static const char *input;
static char output[256];
static size_t used;

char stm32_usart_getc(uint32_t base)
{
    (void)base;
    assert(input && *input); /* Fail instead of blocking on insufficient input. */
    return *input++;
}
bool stm32_usart_try_getc(uint32_t base, char *out)
{
    if (!out || !input || !*input) return false;
    *out = stm32_usart_getc(base);
    return true;
}
void stm32_usart_putc(uint32_t base, char c)
{
    (void)base;
    assert(used + 1 < sizeof output);
    output[used++] = c;
    output[used] = '\0';
}
void stm32_usart_flush(uint32_t base) { (void)base; }

int main(void)
{
    char buf[8];
    input = "help\r\ninfo\n";
    assert(console_gets(buf, sizeof buf) && !strcmp(buf, "help"));
    assert(console_gets(buf, sizeof buf) && !strcmp(buf, "info"));
    assert(!strcmp(output, "help\r\ninfo\r\n"));
    used = 0;
    input = "ab\bC\177d\n";
    assert(console_gets(buf, sizeof buf) && !strcmp(buf, "ad"));
    assert(!strcmp(output, "ab\b \bC\b \bd\r\n"));
    input = "x\ry\n";
    assert(console_gets(buf, sizeof buf) && !strcmp(buf, "x"));
    assert(console_gets(buf, sizeof buf) && !strcmp(buf, "y"));
    input = "\n";
    assert(console_gets(buf, sizeof buf) && !strcmp(buf, ""));
    input = "1234567\n";
    assert(console_gets(buf, sizeof buf) && !strcmp(buf, "1234567"));
    input = "12345678\bX\r\nok\n";
    assert(!console_gets(buf, sizeof buf) && !strcmp(buf, "1234567"));
    assert(console_gets(buf, sizeof buf) && !strcmp(buf, "ok"));
    input = "\t\001\200z\n";
    assert(console_gets(buf, sizeof buf) && !strcmp(buf, "z"));
    input = "\n";
    assert(!console_gets(NULL, 8));
    buf[0] = 'X';
    assert(!console_gets(buf, 0) && buf[0] == 'X');
    assert(console_gets(buf, 1) && buf[0] == '\0');
    input = "a\n";
    assert(!console_gets(buf, 1) && buf[0] == '\0');
    used = 0;
    console_puts("hello\n");
    console_puthex(0x1234ABCDu);
    assert(!strcmp(output, "hello\r\n0x1234ABCD"));
    return 0;
}
