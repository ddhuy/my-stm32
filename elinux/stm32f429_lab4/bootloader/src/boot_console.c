#include <stdint.h>
#include <stdbool.h>

#include "board/stm32f429i-disc1/console.h"

#include "boot_console.h"


#define CONSOLE_LINE_SIZE  32u


static bool str_equals(const char *a, const char *b)
{
    while (*a && *b) {
        if (*a != *b)
            return false;
        ++a;
        ++b;
    }
    return *a == *b;
}


boot_console_action_t boot_console_run(void)
{
    char line[CONSOLE_LINE_SIZE];

    console_puts("STM32F429 Bootloader\n");
    console_puts("Type 'help' for commands.\n");

    for (;;)
    {
        console_puts("#> ");

        if (!console_gets(line, sizeof(line))) {
            console_puts("[ERR] bootloader: input line too long\n");
            continue;
        }

        if (str_equals(line, "help"))
        {
            console_puts("STM32F429 Bootloader Usage:\n");
            console_puts("  help - show this help\n");
            console_puts("  info - show application information\n");
            console_puts("  boot - boot the application image\n");
            continue;
        }
        else if (str_equals(line, "info"))
        {
            console_puts("MCU: STM32F429ZIT6U\n");
            console_puts("Application: external SDRAM\n");
            console_puts("Transfer: Flash -> SDRAM\n");
        }
        else if (str_equals(line, "boot"))
        {
            console_puts("Boot requested\n");
            return BOOT_CONSOLE_BOOT;
        }
        else
        {
            console_puts("[ERR] bootloader: unknown command: ");
            console_puts(line);
            console_puts("\n");
        }
    }
}