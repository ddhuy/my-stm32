#include <init.h>
#include <console.h>
#include <printf.h>
#include <string.h>
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

    board_print_info();

    printf("strlen=%u strcmp=%d %d strncmp=%d\n",
        (u32)strlen("hello"), strcmp("abc", "abc"),
        strcmp("abc", "abd") < 0, strncmp("abc", "abd", 2));
    printf("printf: %d %u %x [%08x] [%5d] [%05d] %p\n",
        -42, 42U, 0xbeefU, 0xbeefU, -7, -7, (void *)main);

    for (;;)
    {
        if (tstc())
            putc(getc());

        board_led_toggle();
        delay(500000);
    }
}
