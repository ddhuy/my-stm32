#include <console.h>
#include <fdt.h>
#include <gyro.h>
#include <init.h>
#include <panic.h>
#include <printf.h>
#include <systime.h>
#include <types.h>
#include <dm/read.h>
#include <dm/root.h>
#include <dm/uclass.h>


/* Heartbeat: the green LED changes state this often */
#define BLINK_MS 500

/* One gyroscope line this often */
#define GYRO_PRINT_MS 1000


/**
 * What the blob says, read through the reader's API: the model, the root's
 * #address-cells and the number of nodes under the root. fdt_print() would
 * print the whole tree (about 120 lines on the STM32): call it here when
 * debugging the .dts.
 */
static void dt_print_info(void)
{
    const void *fdt = dm_fdt();

    if (!fdt)
    {
        printf("DT: no valid blob\n");
        return;
    }

    const char *model = fdt_getprop(fdt, FDT_ROOT, "model", NULL);
    u32 addr_cells = 0;
    int nodes = 0;

    fdt_read_u32(fdt, FDT_ROOT, "#address-cells", &addr_cells);

    for (int node = fdt_first_child(fdt, FDT_ROOT);
         node >= 0;
         node = fdt_next_sibling(fdt, node))
        nodes++;

    printf("DT: \"%s\", #address-cells %u, %d nodes under /\n",
           model ? model : "?", addr_cells, nodes);
}


/**
 * Bring the system up in dependency order, then run the main loop. Each
 * init step needs the ones before it: the board needs the driver model,
 * the console needs the board's pins and clock (whatever board_init() sets
 * up), and the timer goes after the console
 * so that a failure can be reported.
 */
int main(void)
{
    int bound = dm_init();

    /* Both are silent: the console is not up. board_panic() is the only
     * signal (the red LED on the STM32 board, if its LED port was found). */
    if (board_init())
        panic("Board init failed!");

    if (console_init())
        panic("No console!");

    /* systime_elapsed_ms() and systime_mdelay() work only from here on */
    if (systime_init())
        panic("No Timer!");

    board_print_info();

    printf("DM: %d devices bound\n", bound);

    uclass_print_devices();

    dt_print_info();

    /* Boot-time self-check of the timer: expect 1000 (1001 at most) */
    u32 t = systime_elapsed_ms(0);
    systime_mdelay(1000);
    printf("systime_mdelay(1000): %u ms\n", systime_elapsed_ms(t));

    /*
     * Start the blink schedule now, not at 0: the counter has been running
     * since probe, and starting behind would make the first toggles burst.
     */
    u32 last = systime_elapsed_ms(0);

    /*
     * Gyroscope device 0, if the board has one (the LM3S has none): its
     * absence is not an error, so it is only reported once, here.
     */
    struct udevice *gyro;

    if (uclass_get_device(UCLASS_GYRO, 0, &gyro))
        gyro = NULL;

    u32 last_gyro = last;
    bool gyro_failed = false;

    /*
     * Nothing in the loop blocks, so the echo is polled continuously and a
     * pasted line comes back whole (the STM32 USART holds only one received
     * byte).
     */
    for (;;)
    {
        /*
         * Enter sends '\r', which alone only returns the cursor to column
         * 0. Echo it as '\n', which putc() sends as "\r\n". Done here, on
         * input, not in putc(): output may use a bare '\r' on purpose.
         */
        if (tstc())
        {
            char ch = getc();

            putc(ch == '\r' ? '\n' : ch);
        }

        /*
         * Compare the elapsed time, never 'now >= last + BLINK_MS': that sum
         * wraps near 2^32 ms and the LED bursts, then stops for a minute or
         * more (seen in step 9). 'last +=' keeps the toggles on a 500 ms grid
         * instead of drifting with the loop's lateness.
         */
        if (systime_elapsed_ms(last) >= BLINK_MS)
        {
            board_led_toggle();
            last += BLINK_MS;
        }

        /*
         * Same schedule form as the blink. Printing a line blocks for
         * about 3 ms; a character pasted meanwhile can be lost (the USART
         * holds one byte). A read error is printed once, not every second.
         */
        if (gyro && systime_elapsed_ms(last_gyro) >= GYRO_PRINT_MS)
        {
            struct gyro_data rate;
            int ret = gyro_read(gyro, &rate);

            if (!ret)
                printf("gyro: x=%d y=%d z=%d mdps\n", rate.x, rate.y, rate.z);
            else if (!gyro_failed)
                printf("gyro: read error %d\n", ret);

            gyro_failed = (ret != 0);
            last_gyro += GYRO_PRINT_MS;
        }
    }
}
