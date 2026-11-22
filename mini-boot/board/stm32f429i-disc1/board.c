#include <board.h>
#include <clk.h>
#include <gpio.h>
#include <init.h>
#include <printf.h>
#include <spi.h>
#include <dm/device.h>
#include <dm/uclass.h>
#include <platdata/stm32_gpio.h>
#include <platdata/stm32_serial.h>
#include <platdata/stm32_spi.h>
#include <platdata/stm32f4_rcc.h>


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
 * SCK, MISO and MOSI of the gyroscope's SPI bus, handed to SPI5. The two
 * outputs get fast edges first, so the first clock is already sharp;
 * MISO is an input, which a speed does not affect. Chip select is not
 * here: it is a plain GPIO, which the SPI driver owns.
 */
static int spi_pins_setup(void)
{
    struct udevice *port;
    int ret;

    ret = uclass_get_device_by_name(UCLASS_GPIO, GYRO_SPI_GPIO_NAME, &port);
    if (ret)
        return ret;

    ret = gpio_set_speed(port, GYRO_SPI_SCK_PIN, GYRO_SPI_PIN_SPEED);
    if (ret)
        return ret;

    ret = gpio_set_speed(port, GYRO_SPI_MOSI_PIN, GYRO_SPI_PIN_SPEED);
    if (ret)
        return ret;

    ret = gpio_set_alternative(port, GYRO_SPI_SCK_PIN, GYRO_SPI_PIN_AF);
    if (ret)
        return ret;

    ret = gpio_set_alternative(port, GYRO_SPI_MISO_PIN, GYRO_SPI_PIN_AF);
    if (ret)
        return ret;

    return gpio_set_alternative(port, GYRO_SPI_MOSI_PIN, GYRO_SPI_PIN_AF);
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

    ret = console_pins_setup();
    if (ret)
        return ret;

    return spi_pins_setup();
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
 * feeds the console. The clock is looked up with the same two names as the
 * usart1 row (board.h); a failed lookup prints 0, since the banner only
 * informs (and the console working means the provider is there).
 */
void board_print_info(void)
{
    struct clk clk;
    u32 rate = 0;

    if (!clk_get_by_name(CONSOLE_UART_CLK_DEV, CONSOLE_UART_CLK, &clk))
        rate = clk_get_rate(&clk);

    printf("\n""+-------------------------------+\n");
    printf(    "| MINI-BOOT                     |\n");
    printf(    "| - Board: STM32F429I-DISC1.    |\n");
    printf(    "| - Console clock: %u Hz. |\n", rate);
    printf(    "+-------------------------------+\n");

    /*
     * The gyroscope's first use: probing it probes its bus first (the
     * core does that), then checks the chip. Outside the box, and not in
     * board_init(): a missing gyroscope must not stop the boot.
     */
    struct udevice *gyro;
    int ret = uclass_get_device_by_name(UCLASS_GYRO, GYRO_NAME, &gyro);

    if (ret)
        printf("Gyro: error %d\n", ret);
    else
        printf("Gyro: ready\n");
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
    .clk_dev = STM32F4_RCC_NAME,
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
    .clk_dev = STM32F4_RCC_NAME,
    .clk_id = CONSOLE_GPIO_CLK
};

DM_DEVICE(gpioa) = {
    .name = CONSOLE_GPIO_NAME,
    .driver = "stm32_gpio",
    .plat = &gpioa_plat,
};


/**
 * GPIO port C: the gyroscope's chip select (PC1).
 */
static const struct stm32_gpio_plat gpioc_plat = {
    .base = GYRO_CS_GPIO_BASE,
    .clk_dev = STM32F4_RCC_NAME,
    .clk_id = GYRO_CS_GPIO_CLK
};

DM_DEVICE(gpioc) = {
    .name = GYRO_CS_GPIO_NAME,
    .driver = "stm32_gpio",
    .plat = &gpioc_plat,
};


/**
 * GPIO port F: the gyroscope's SPI pins (PF7-PF9).
 */
static const struct stm32_gpio_plat gpiof_plat = {
    .base = GYRO_SPI_GPIO_BASE,
    .clk_dev = STM32F4_RCC_NAME,
    .clk_id = GYRO_SPI_GPIO_CLK
};

DM_DEVICE(gpiof) = {
    .name = GYRO_SPI_GPIO_NAME,
    .driver = "stm32_gpio",
    .plat = &gpiof_plat,
};


/**
 * USART1: the console, through the ST-LINK virtual COM port.
 */
static const struct stm32_serial_plat usart1_plat = {
    .base = CONSOLE_UART_BASE,
    .baudrate = CONSOLE_BAUDRATE,
    .clk_dev = CONSOLE_UART_CLK_DEV,
    .clk_id = CONSOLE_UART_CLK,
};

DM_DEVICE(usart1) = {
    .name = "usart1",
    .driver = "stm32_serial",
    .plat = &usart1_plat,
};


/**
 * RCC: the clock provider every other device here names in its clk_dev.
 * Its row is here, not in the SoC's devices.c, because its settings (the
 * crystal and the chosen speed) are board facts.
 */
static const struct stm32f4_rcc_plat rcc_plat = {
    .base       = RCC_BASE,
    .flash_base = FLASH_R_BASE,
    .pwr_base   = PWR_BASE,
    .hse_hz     = HSE_HZ,
    .sysclk_hz  = BOARD_SYSCLK_HZ
};

DM_DEVICE(rcc) = {
    .name = STM32F4_RCC_NAME,
    .driver = "stm32f4_rcc",
    .plat = &rcc_plat,
};


/**
 * The gyroscope: a child of SPI5. It has no platform data of its own;
 * gyro_slave tells the bus how to reach it (chip select, mode 3, speed),
 * and only the SPI uclass reads it.
 */
static const struct spi_slave_plat gyro_slave = {
    .cs = GYRO_SPI_CS,
    .mode = SPI_MODE_3,
    .max_hz = GYRO_SPI_MAX_HZ,
};

DM_DEVICE(gyro) = {
    .name = GYRO_NAME,
    .driver = "l3gd20",
    .parent = GYRO_SPI_NAME,
    .parent_plat = &gyro_slave,
};


/**
 * SPI5: the bus the gyroscope is on, with its chip select line.
 */
static const struct stm32_spi_plat spi5_plat = {
    .base = GYRO_SPI_BASE,
    .clk_dev = STM32F4_RCC_NAME,
    .clk_id = GYRO_SPI_CLK,
    .cs_gpio_dev = GYRO_CS_GPIO_NAME,
    .cs_pin = GYRO_CS_PIN,
};

DM_DEVICE(spi5) = {
    .name = GYRO_SPI_NAME,
    .driver = "stm32_spi",
    .plat = &spi5_plat,
};
