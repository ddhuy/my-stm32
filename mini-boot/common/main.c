#include <console.h>
#include <init.h>
#include <panic.h>
#include <printf.h>
#include <types.h>
#include <dm/root.h>
#include <dm/uclass.h>

#include <timer.h>


/* Crude delay; replaced once there is a timer */
static void delay(u32 count)
{
    for (volatile u32 i = 0; i < count; i++)
        ;
}


int main(void)
{
    int bound = dm_init();

    /* Both are silent: the console is not up. The red LED comes on if the
     * board got as far as finding its LED port. */
    if (board_init())
        panic("Board init failed!");

    if (console_init())
        panic("No console!");

    board_print_info();

    printf("DM: %d devices bound\n", bound);

    uclass_print_devices();

    struct udevice *tmr;
    int ret = uclass_get_device_by_name(UCLASS_TIMER, "systick", &tmr);
    if (ret)
        printf("Error %d. Timer\n", ret);
    else
        printf("Timer: %d  priv %p  ms %u\n",
            ret, tmr->priv, timer_get_ms(tmr));

    for (;;)
    {
        if (tstc())
            putc(getc());
        
        board_led_toggle();
        delay(500000);

        if (!ret)
            printf("Timer: %u\n", timer_get_ms(tmr));
    }
}
