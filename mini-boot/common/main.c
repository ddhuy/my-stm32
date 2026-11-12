#include <init.h>
#include <console.h>
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

    puts("\n""+-------------------------------+\n");
    puts(    "| MINI-BOOT                     |\n");
    puts(    "| - Board: STM32F429I-DISC1.    |\n");
    puts(    "| - Console clock: 16000000 Hz. |\n");
    puts(    "+-------------------------------+\n");

    for (;;)
    {
        if (tstc())
            putc(getc());

        board_led_toggle();
        delay(500000);
    }
}
