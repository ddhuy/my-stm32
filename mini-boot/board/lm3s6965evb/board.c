#include <board.h>
#include <clock.h>
#include <errno.h>
#include <init.h>
#include <printf.h>
#include <dm/device.h>
#include <platdata/lm3s_sysctl.h>
#include <platdata/pl011_serial.h>


/**
 * Bring up the board. Nothing to do: the system clock is set up by the
 * sysctl device's probe, on first use; GPIO is skipped on this target,
 * and QEMU's UART needs no pin setup. Always returns 0.
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
 * System Control: the clock provider. Its row is here, not in the SoC's
 * devices.c, because its settings are board facts: the crystal and the
 * chosen speed.
 */
static const struct lm3s_sysctl_plat sysctl_plat = {
    .base      = SYSCTL_BASE,
    .xtal_hz   = BOARD_XTAL_HZ,
    .sysclk_hz = BOARD_SYSCLK_HZ,
};

DM_DEVICE(sysctl) = {
    .name   = LM3S_SYSCTL_NAME,
    .driver = "lm3s_sysctl",
    .plat   = &sysctl_plat,
};


/**
 * UART0: the console. QEMU connects it to the terminal (-nographic).
 */
static const struct pl011_serial_plat uart0_plat = {
    .base = CONSOLE_UART_BASE,
    .baudrate = CONSOLE_BAUDRATE,
    .clk_dev = LM3S_SYSCTL_NAME,
    .clk_id = CONSOLE_UART_CLK,
};

DM_DEVICE(uart0) = {
    .name = "uart0",
    .driver = "pl011_serial",
    .plat = &uart0_plat,
};
