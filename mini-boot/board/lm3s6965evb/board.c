#include <board.h>
#include <clock.h>
#include <errno.h>
#include <init.h>
#include <printf.h>
#include <dm/device.h>
#include <platdata/pl011_serial.h>


/**
 * Bring up the board. Nothing to do: GPIO is skipped on this target, and
 * QEMU's UART needs no pin setup. Always returns 0.
 */
int board_init(void)
{
    return SUCCESS;
}


/**
 * Toggle the heartbeat LED. Empty: QEMU shows no LED and GPIO is skipped.
 */
void board_led_toggle(void)
{}


/**
 * Print the boot banner with the board's name and facts.
 */
void board_print_info(void)
{
    printf("\n""+---------------------------------+\n");
    printf(    "| MINI-BOOT                       |\n");
    printf(    "| - Board: LM3S6965EVB (QEMU).    |\n");
    printf(    "| - Console clock: %u Hz.   |\n",
        clock_get_rate(CONSOLE_UART_CLK));
    printf(    "+---------------------------------+\n");
}


/**
 * Signal a fatal error to the user. Called by panic(). Empty: there is no
 * LED; the "PANIC:" line on the console is the only signal.
 */
void board_panic(void)
{}


/*
 * The devices on this board, for the driver model. dm_init() binds each
 * DM_DEVICE entry to the driver it names.
 */

/**
 * UART0: the console. QEMU connects it to the terminal (-nographic).
 */
static const struct pl011_serial_plat uart0_plat = {
    .base = CONSOLE_UART_BASE,
    .baudrate = CONSOLE_BAUDRATE,
    .clk_id = CONSOLE_UART_CLK,
};

DM_DEVICE(uart0) = {
    .name = "uart0",
    .driver = "pl011_serial",
    .plat = &uart0_plat,
};
