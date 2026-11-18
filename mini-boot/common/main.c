#include <console.h>
#include <init.h>
#include <panic.h>
#include <printf.h>
#include <time.h>
#include <types.h>
#include <dm/root.h>
#include <dm/uclass.h>


/* Heartbeat: the green LED changes state this often */
#define BLINK_MS 500


/**
 * Bring the system up in dependency order, then run the main loop. Each
 * init step needs the ones before it: the board needs the driver model,
 * the console needs the board's pins, and the timer goes after the console
 * so that a failure can be reported.
 */
int main(void)
{
    int bound = dm_init();

    /* Both are silent: the console is not up. The red LED comes on if the
     * board got as far as finding its LED port. */
    if (board_init())
        panic("Board init failed!");

    if (console_init())
        panic("No console!");

    /* get_timer() and mdelay() work only from here on */
    if (timer_init())
        panic("No Timer!");

    board_print_info();

    printf("DM: %d devices bound\n", bound);

    uclass_print_devices();

    /* Boot-time self-check of the timer: expect 1000 (1001 at most) */
    u32 t = get_timer(0);
    mdelay(1000);
    printf("mdelay(1000): %u ms\n", get_timer(t));

    /*
     * Start the blink schedule now, not at 0: the counter has been running
     * since probe, and starting behind would make the first toggles burst.
     */
    u32 last = get_timer(0);

    /*
     * Nothing in the loop blocks, so the echo is polled continuously and a
     * pasted line comes back whole (USART1 holds only one received byte).
     */
    for (;;)
    {
        if (tstc())
            putc(getc());

        /*
         * Compare the elapsed time, never 'now >= last + BLINK_MS': that sum
         * wraps near 2^32 ms and the LED bursts, then stops for a minute or
         * more (seen in step 9). 'last +=' keeps the toggles on a 500 ms grid
         * instead of drifting with the loop's lateness.
         */
        if (get_timer(last) >= BLINK_MS)
        {
            board_led_toggle();
            last += BLINK_MS;
        }
    }
}

