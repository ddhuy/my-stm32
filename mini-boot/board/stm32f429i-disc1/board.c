#include <board.h>
#include <clock.h>
#include <gpio.h>
#include <init.h>
#include <printf.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <platdata/stm32_gpio.h>
#include <platdata/stm32_serial.h>


/*
 * The GPIO port that carries the LEDs; NULL until led_init() has found it.
 */
static struct udevice *led_port;


/**
 * Route the console pins to the UART. The UART itself is started by its
 * driver when console_init() probes it. Getting the port probes it, which
 * turns its clock on.
 */
static int console_pins_setup(void)
{
    struct udevice *port;
    int ret;

    ret = uclass_get_device_by_name(UCLASS_GPIO, CONSOLE_GPIO_NAME, &port);
    if (ret)
        return ret;

    ret = gpio_set_alternative(port, CONSOLE_TX_PIN, CONSOLE_PIN_AF);
    if (ret)
        return ret;

    return gpio_set_alternative(port, CONSOLE_RX_PIN, CONSOLE_PIN_AF);
}


/**
 * Both user LEDs as outputs, switched off. The port is remembered as soon
 * as it is found, so board_panic() can use it even if a later step fails.
 */
static int led_init(void)
{
    struct udevice *port;
    int ret;

    ret = uclass_get_device_by_name(UCLASS_GPIO, LED_GPIO_NAME, &port);
    if (ret)
        return ret;

    led_port = port;

    ret = gpio_direction_output(led_port, LED_GREEN_PIN, 0);
    if (ret)
        return ret;

    return gpio_direction_output(led_port, LED_RED_PIN, 0);
}

/**
 * See init.h. The LEDs come first, so the red one can report a failure of
 * the console pins. Nothing can print yet: the console starts later, in
 * console_init().
 */
int board_init(void)
{
    int ret = led_init();

    if (ret)
        return ret;

    return console_pins_setup();
}

/**
 * Heartbeat on the green LED. Does nothing if the LED port was not found.
 */
void board_led_toggle(void)
{
    if (!led_port)
        return;

    gpio_set_value(led_port, LED_GREEN_PIN,
                   !gpio_get_value(led_port, LED_GREEN_PIN));
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
    printf(    "| - Console clock: %u Hz. |\n",
        clock_get_rate(CONSOLE_UART_CLK));
    printf(    "+-------------------------------+\n");
}

/**
 * Red LED on, and it stays on: panic() never returns. If the LED port was
 * never found there is no way left to signal anything.
 */
void board_panic(void)
{
    if (!led_port)
        return;

    gpio_set_value(led_port, LED_RED_PIN, 1);
}


/*
 * The devices on this board, for the driver model. dm_init() binds each
 * DM_DEVICE entry to the driver it names.
 */

/**
 * GPIO port G: the two user LEDs.
 */
static const struct stm32_gpio_plat gpiog_plat = {
    .base = LED_GPIO_BASE,
    .clk_id = LED_GPIO_CLK
};

DM_DEVICE(gpiog) = {
    .name = LED_GPIO_NAME,
    .driver = "stm32_gpio",
    .plat = &gpiog_plat,
};


/**
 * GPIO port A: the console pins, PA9 and PA10.
 */
static const struct stm32_gpio_plat gpioa_plat = {
    .base = CONSOLE_GPIO_BASE,
    .clk_id = CONSOLE_GPIO_CLK
};

DM_DEVICE(gpioa) = {
    .name = CONSOLE_GPIO_NAME,
    .driver = "stm32_gpio",
    .plat = &gpioa_plat,
};


/**
 * USART1: the console, through the ST-LINK virtual COM port.
 */
static const struct stm32_serial_plat usart1_plat = {
    .base = CONSOLE_UART_BASE,
    .baudrate = CONSOLE_BAUDRATE,
    .clk_id = CONSOLE_UART_CLK,
};

DM_DEVICE(usart1) = {
    .name = "usart1",
    .driver = "stm32_serial",
    .plat = &usart1_plat,
};
