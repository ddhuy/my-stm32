#include <stdint.h>

#include "drivers/led/stm32_led.h"
#include "drivers/tim2/stm32_tim2.h"


volatile uint32_t app_counter = 100;
volatile uint32_t app_status;


int main(void)
{
    app_status = app_counter + 50;

    stm32_led_init();
    stm32_tim2_init();

    /*
     * The bootloader disables interrupts before handing off control.
     * Re-enable them only after the application is configured.
     */
    __asm volatile ("cpsie i" ::: "memory");

    while (1)
    {
        app_counter++;
    }

    return 0;
}