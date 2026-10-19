#include <stdbool.h>
#include <stdint.h>

#include "board/stm32f429i-disc1/console.h"

#include "boot_xmodem_io.h"
#include "boot_time.h"


/*
 * Receive one UART byte, waiting at most approximately
 * timeout_ms milliseconds.
 *
 * This callback owns SysTick COUNTFLAG while active.
 */
static bool boot_uart_read_byte(
    void *context,
    uint8_t *byte,
    uint32_t timeout_ms)
{
    (void)context;

    if (byte == NULL)
    {
        return false;
    }

    uint32_t elapsed_ms = 0u;

    /*
     * Discard a previous SysTick COUNTFLAG.
     * A new timeout interval begins here.
     */
    (void)boot_time_tick();

    for (;;)
    {
        char c;

        if (console_try_getc(&c))
        {
            *byte = (uint8_t)(unsigned char)c;
            return true;
        }

        if (boot_time_tick())
        {
            elapsed_ms++;

            if (elapsed_ms >= timeout_ms)
            {
                return false;
            }
        }
    }
}

/*
 * Transmit one byte through the board console UART.
 */
static void boot_uart_write_byte(
    void *context,
    uint8_t byte)
{
    (void)context;

    console_putc((char)byte);
}

/*
 * Construct an XMODEM transport interface.
 */
xmodem_io_t boot_xmodem_io_create(void)
{
    xmodem_io_t io = {
        .read_byte = boot_uart_read_byte,
        .write_byte = boot_uart_write_byte,
        .context = NULL
    };

    return io;
}