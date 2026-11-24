#include <board.h>
#include <clk.h>
#include <errno.h>
#include <init.h>
#include <printf.h>
#include <platdata/lm3s_sysctl.h>


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
 * Print the boot banner with the board's name and facts. The console clock
 * is looked up by the names in board.h, which must agree with the uart0
 * node's clocks in the .dts; a failed lookup prints 0, since the banner
 * only informs.
 */
void board_print_info(void)
{
    struct clk clk;
    u32 rate = 0;

    if (!clk_get_by_name(CONSOLE_UART_CLK_DEV, CONSOLE_UART_CLK, &clk))
        rate = clk_get_rate(&clk);

    printf("\n""+---------------------------------+\n");
    printf(    "| MINI-BOOT                       |\n");
    printf(    "| - Board: LM3S6965EVB (QEMU).    |\n");
    printf(    "| - Console clock: %u Hz.   |\n", rate);
    printf(    "+---------------------------------+\n");
}


/**
 * Signal a fatal error to the user. Called by panic(). Empty: there is no
 * LED; the "PANIC:" line on the console is the only signal.
 */
void board_panic(void)
{}
