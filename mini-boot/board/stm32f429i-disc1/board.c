#include <clock.h>
#include <gpio.h>
#include <init.h>

#include "board.h"

/**
 * 
 */
static void led_init(void)
{
    clock_enable(LED_GPIO_CLK);
    gpio_direction_output(LED_GPIO_BASE, LED_GREEN_PIN, 0);
    gpio_direction_output(LED_GPIO_BASE, LED_RED_PIN, 0);
}

/**
 * 
 */
void board_init(void)
{
    led_init();
}

/**
 * 
 */
void board_led_toggle(void)
{
    gpio_set_value(LED_GPIO_BASE, LED_GREEN_PIN,
                   !gpio_get_value(LED_GPIO_BASE, LED_GREEN_PIN));
}
