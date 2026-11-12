#include <board.h>
#include <console.h>
#include <clock.h>
#include <gpio.h>
#include <init.h>
#include <serial.h>


/**
 * 
 */
static void console_setup(void)
{
    /* Setup GPIOA clock & PA9 & PA10 as alternative function */
    clock_enable(CONSOLE_GPIO_CLK);
    gpio_set_alternative(CONSOLE_GPIO_BASE, CONSOLE_TX_PIN, CONSOLE_PIN_AF);
    gpio_set_alternative(CONSOLE_GPIO_BASE, CONSOLE_RX_PIN, CONSOLE_PIN_AF);

    /* Setup UART clock & baudrate */
    clock_enable(CONSOLE_UART_CLK);
    serial_init(CONSOLE_UART_BASE, clock_get_rate(CONSOLE_UART_CLK), CONSOLE_BAUDRATE);

    /* Inititiate the console */
    console_init(CONSOLE_UART_BASE);
}


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
    console_setup();
}

/**
 * 
 */
void board_led_toggle(void)
{
    gpio_set_value(LED_GPIO_BASE, LED_GREEN_PIN,
                   !gpio_get_value(LED_GPIO_BASE, LED_GREEN_PIN));
}
