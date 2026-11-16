#include <asm/io.h>
#include <clock.h>
#include <errno.h>
#include <gpio.h>
#include <dm/device.h>
#include <dm/driver.h>
#include <platdata/stm32_gpio.h>


/* Every STM32 GPIO port has pins 0-15 */
#define GPIO_PINS_PER_PORT      16


/* STM32 GPIO port register layout */
#define GPIO_MODER      0x00
#define GPIO_IDR        0x10
#define GPIO_BSRR       0x18
#define GPIO_AFRL       0x20
#define GPIO_AFRH       0x24

/*
 * MODER holds a 2-bit mode field for each of the 16 pins:
 * pin 0 uses bits 1:0, pin 1 bits 3:2, ... pin 15 bits 31:30.
 */
#define GPIO_MODER_BITS_PER_PIN 2
#define GPIO_MODER_SHIFT(pin)   ((pin) * GPIO_MODER_BITS_PER_PIN)
#define GPIO_MODER_MASK(pin)    (0x3UL << GPIO_MODER_SHIFT(pin))

#define GPIO_MODE_INPUT         0x0UL
#define GPIO_MODE_OUTPUT        0x1UL
#define GPIO_MODE_ALTERNATE     0x2UL
#define GPIO_MODE_ANALOG        0x3UL
#define GPIO_MODER_MODE(pin, mode)  ((mode) << GPIO_MODER_SHIFT(pin))

/*
 * BSRR is write-only: writing 1 to bit n (0-15) sets pin n,
 * writing 1 to bit n + 16 resets it. Zeros are ignored.
 */
#define GPIO_BSRR_RESET_SHIFT   16
#define GPIO_BSRR_SET(pin)      BIT(pin)
#define GPIO_BSRR_RESET(pin)    BIT((pin) + GPIO_BSRR_RESET_SHIFT)

/*
 * The alternate function number is a 4-bit field per pin, so 16 pins need
 * two registers: AFRL holds pins 0-7, AFRH pins 8-15.
 */
#define GPIO_AFR_BITS_PER_PIN   4
#define GPIO_AFR_PINS_PER_REG   8
#define GPIO_AFR(pin)           ((pin) < GPIO_AFR_PINS_PER_REG ? \
                                 GPIO_AFRL : GPIO_AFRH)
#define GPIO_AFR_SHIFT(pin)     (((pin) % GPIO_AFR_PINS_PER_REG) * \
                                 GPIO_AFR_BITS_PER_PIN)
#define GPIO_AFR_MASK(pin)      (0xFUL << GPIO_AFR_SHIFT(pin))
#define GPIO_AFR_AF(pin, af)    ((u32)(af) << GPIO_AFR_SHIFT(pin))

/**
 * A port needs only its clock: its registers are ignored until the clock
 * runs. The pins keep their reset state until somebody configures them.
 */
static int stm32_gpio_probe(
    struct udevice *dev
)
{
    const struct stm32_gpio_plat *plat = dev->plat;

    clock_enable(plat->clk_id);

    return SUCCESS;
}


/**
 * Drive the pin high or low.
 */
static int stm32_gpio_set_value(
    struct udevice *dev,
    unsigned int pin,
    int value
)
{
    const struct stm32_gpio_plat *plat = dev->plat;

    if (pin >= GPIO_PINS_PER_PORT)
        return -EINVAL;

    /* BSRR changes one pin without a read-modify-write of ODR */
    writel(value ? GPIO_BSRR_SET(pin) : GPIO_BSRR_RESET(pin),
           plat->base + GPIO_BSRR);

    return SUCCESS;
}


/**
 * Read the level from the input data register. This works for an output pin
 * too, which is how a caller can toggle one.
 */
static int stm32_gpio_get_value(
    struct udevice *dev,
    unsigned int pin
)
{
    const struct stm32_gpio_plat *plat = dev->plat;

    if (pin >= GPIO_PINS_PER_PORT)
        return -EINVAL;

    return (readl(plat->base + GPIO_IDR) >> pin) & 1;
}


/**
 * Switch the pin to output mode with a defined starting level.
 */
static int stm32_gpio_direction_output(
    struct udevice *dev,
    unsigned int pin,
    int value
)
{
    const struct stm32_gpio_plat *plat = dev->plat;

    /* set the level first so the pin does not glitch when it turns output;
     * this call also rejects a pin the port does not have */
    int ret = stm32_gpio_set_value(dev, pin, value);

    if (ret)
        return ret;

    clrsetbits(plat->base + GPIO_MODER,
               GPIO_MODER_MASK(pin),
               GPIO_MODER_MODE(pin, GPIO_MODE_OUTPUT));

    return SUCCESS;
}


/**
 * Connect the pin to an on-chip peripheral (UART, SPI, ...) instead of
 * driving it as a plain GPIO.
 */
static int stm32_gpio_set_alternative(
    struct udevice *dev,
    unsigned int pin,
    unsigned int af
)
{
    const struct stm32_gpio_plat *plat = dev->plat;

    if (pin >= GPIO_PINS_PER_PORT)
        return -EINVAL;

    /* choose the function first, then switch the pin over to it */
    clrsetbits(plat->base + GPIO_AFR(pin),
               GPIO_AFR_MASK(pin),
               GPIO_AFR_AF(pin, af));
    clrsetbits(plat->base + GPIO_MODER,
               GPIO_MODER_MASK(pin),
               GPIO_MODER_MODE(pin, GPIO_MODE_ALTERNATE));

    return SUCCESS;
}


/**
 * This driver's implementation of the GPIO uclass operations.
 */
static const struct gpio_ops stm32_gpio_ops = {
    .direction_output = stm32_gpio_direction_output,
    .set_alternative  = stm32_gpio_set_alternative,
    .set_value        = stm32_gpio_set_value,
    .get_value        = stm32_gpio_get_value,
};


/**
 * Driver model entry. No private data: everything the driver needs is in
 * the device's platform data.
 */
DM_DRIVER(stm32_gpio) = {
    .name = "stm32_gpio",
    .id   = UCLASS_GPIO,

    .probe = stm32_gpio_probe,

    .ops = &stm32_gpio_ops,
};
