
#include <stdint.h>

#include "board/stm32f429i-disc1/console.h"
#include "boot_autoboot.h"
#include "boot_time.h"

#define AUTOBOOT_TIMEOUT_MS 5000u

bool boot_autoboot_wait(void)
{
    uint32_t elapsed_ms = 0u;
    uint32_t seconds_remaining = 5u;

    char c;

    console_puts("\nPress any key to stop autoboot.\n");
    console_puts("Autoboot in 5 seconds...\n");

    /* Discard an old SysTick COUNTFLAG before starting. */
    (void)boot_time_tick();

    while (elapsed_ms < AUTOBOOT_TIMEOUT_MS)
    {
        /* Give UART input priority over the timeout. */
        if (console_try_getc(&c))
        {
            console_puts("\nAutoboot interrupted.\n");
            return false;
        }

        if (boot_time_tick())
        {
            elapsed_ms++;

            uint32_t remaining =
                (AUTOBOOT_TIMEOUT_MS - elapsed_ms + 999u) / 1000u;

            if (remaining < seconds_remaining)
            {
                seconds_remaining = remaining;

                if (remaining != 0u)
                {
                    console_puts(".");
                }
            }
        }
    }

    console_puts("\nAutoboot timeout expired.\n");
    return true;
}
