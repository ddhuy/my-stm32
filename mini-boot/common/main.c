#include <init.h>
#include <printf.h>
#include <types.h>
#include <dm/driver.h>
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

    board_print_info();

    driver_print_list();

    printf("find STM32 GPIO: %p\n",
        (void *) driver_find("stm32_gpio")
    );
    printf("find STM32 SERIAL: %p\n",
        (void *) driver_find("stm32_serial")
    );
    printf("find STM32 SPI: %p\n",
        (void *) driver_find("stm32_spi")
    );

    printf("dm: %d devices bound\n", dm_init());
    uclass_print_device();

    for (;;)
    {
        board_led_toggle();
        delay(500000);
    }
}
