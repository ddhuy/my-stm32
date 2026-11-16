#include <console.h>
#include <init.h>
#include <panic.h>
#include <printf.h>
#include <types.h>
#include <dm/root.h>
#include <dm/uclass.h>


/* Crude delay; replaced once there is a timer */
static void delay(u32 count)
{
    for (volatile u32 i = 0; i < count; i++)
        ;
}


int main(void)
{
    board_init();

    int bound = dm_init();

    if (console_init())
        panic("No console!"); /* silent, but the red LED comes on */

    board_print_info();

    printf("DM: %d devices bound\n", bound);

    uclass_print_devices();

    for (;;)
    {
        if (tstc())
            putc(getc());
        
        board_led_toggle();
        delay(500000);
    }
}
