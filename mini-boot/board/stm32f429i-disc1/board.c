#include <board.h>
#include <console.h>
#include <clock.h>
#include <gpio.h>
#include <init.h>
#include <printf.h>
#include <serial.h>


/**
 * Route the console pins to the UART, start the UART, and hand it to the
 * console layer. The order matters: each clock before the registers it
 * feeds, and serial_init() before console_init().
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
 * Both user LEDs as outputs, switched off.
 */
static void led_init(void)
{
    clock_enable(LED_GPIO_CLK);
    gpio_direction_output(LED_GPIO_BASE, LED_GREEN_PIN, 0);
    gpio_direction_output(LED_GPIO_BASE, LED_RED_PIN, 0);
}

/**
 * See init.h. The console comes last, so everything after this call can
 * print.
 */
void board_init(void)
{
    led_init();
    console_setup();
}

/**
 * Heartbeat on the green LED.
 */
void board_led_toggle(void)
{
    gpio_set_value(LED_GPIO_BASE, LED_GREEN_PIN,
                   !gpio_get_value(LED_GPIO_BASE, LED_GREEN_PIN));
}

/**
 * Boot banner. The board prints it because only the board knows which clock
 * feeds the console.
 */
void board_print_info(void)
{
    printf("\n""+-------------------------------+\n");
    printf(    "| MINI-BOOT                     |\n");
    printf(    "| - Board: STM32F429I-DISC1.    |\n");
    printf(    "| - Console clock: %u Hz. |\n", clock_get_rate(CONSOLE_UART_CLK));
    printf(    "+-------------------------------+\n");
}

/**
 * Red LED on, and it stays on: panic() never returns.
 */
void board_panic(void)
{
    gpio_set_value(LED_GPIO_BASE, LED_RED_PIN, 1);
}