#include <init.h>
#include <types.h>


/* Crude delay; replaced once there is a timer */
static void delay(u32 count)
{
    for (volatile u32 i = 0; i < count; i++)
        ;
}


int main(void)
{
    board_init();

    for (;;) {
        board_led_toggle();
        delay(500000);
    }
}
