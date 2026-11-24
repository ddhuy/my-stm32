#include <asm/io.h>
#include <clk.h>
#include <errno.h>
#include <gpio.h>
#include <dm/device.h>
#include <dm/driver.h>
#include <platdata/stm32_gpio.h>
#include <dm/read.h>


/* Every STM32 GPIO port has pins 0-15 */
#define GPIO_PINS_PER_PORT      16


/* STM32 GPIO port register layout */
#define GPIO_MODER      0x00
#define GPIO_OSPEEDR    0x08
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
 * OSPEEDR: a 2-bit output speed field per pin, laid out like MODER. 00 at
 * reset (low) on most pins: fine for a LED or a UART, too slow for SPI at
 * several MHz (item 4: bit 0 of every byte read from the gyroscope was
 * unreliable at 5.25 MHz).
 */
#define GPIO_OSPEEDR_SHIFT(pin) ((pin) * GPIO_MODER_BITS_PER_PIN)
#define GPIO_OSPEEDR_MASK(pin)  (0x3UL << GPIO_OSPEEDR_SHIFT(pin))
#define GPIO_OSPEED_LOW         0x0UL
#define GPIO_OSPEED_MEDIUM      0x1UL
#define GPIO_OSPEED_FAST        0x2UL
#define GPIO_OSPEED_HIGH        0x3UL
#define GPIO_OSPEEDR_SPEED(pin, s)  ((s) << GPIO_OSPEEDR_SHIFT(pin))

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
    struct clk clk;
    int ret;

    ret = clk_get_by_name(plat->clk_dev, plat->clk_id, &clk);
    if (ret)
        return ret;

    ret = clk_enable(&clk);
    if (ret)
        return ret;

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
 * The generic speeds map one to one onto the four OSPEEDR steps. Mapped
 * one by one, not by value: the enum's order is the uclass's business.
 */
static int stm32_gpio_set_speed(
    struct udevice *dev,
    unsigned int pin,
    enum gpio_speed speed
)
{
    const struct stm32_gpio_plat *plat = dev->plat;
    u32 field;

    if (pin >= GPIO_PINS_PER_PORT)
        return -EINVAL;

    switch (speed) {
    case GPIO_SPEED_LOW:
        field = GPIO_OSPEED_LOW;
        break;
    case GPIO_SPEED_MEDIUM:
        field = GPIO_OSPEED_MEDIUM;
        break;
    case GPIO_SPEED_HIGH:
        field = GPIO_OSPEED_FAST;
        break;
    case GPIO_SPEED_VERY_HIGH:
        field = GPIO_OSPEED_HIGH;
        break;
    default:
        return -EINVAL;
    }

    clrsetbits(plat->base + GPIO_OSPEEDR,
               GPIO_OSPEEDR_MASK(pin),
               GPIO_OSPEEDR_SPEED(pin, field));

    return SUCCESS;
}


/**
 * This driver's implementation of the GPIO uclass operations.
 */
static const struct gpio_ops stm32_gpio_ops = {
    .direction_output = stm32_gpio_direction_output,
    .set_alternative  = stm32_gpio_set_alternative,
    .set_speed        = stm32_gpio_set_speed,
    .set_value        = stm32_gpio_set_value,
    .get_value        = stm32_gpio_get_value,
};


/**
 * The port's node into its plat: the base from reg, the clock from
 * clocks = <&rcc ID> (the provider's device name and the id, as a C row
 * would give them).
 *
 * Returns 0 or the error from reading the node.
 */
static int stm32_gpio_of_to_plat(
    struct udevice *dev
)
{
    struct stm32_gpio_plat *plat = dev->plat;
    int ret = dev_read_addr_index(dev, 0, &plat->base);

    if (ret)
        return ret;

    return clk_read_by_index(dev, 0, &plat->clk_dev, &plat->clk_id);
}


/**
 * The compatible strings this driver handles.
 */
static const struct udevice_id stm32_gpio_ids[] = {
    { .compatible = "st,stm32f429-gpio" },
    { }
};

/**
 * Driver model entry. No private data: everything the driver needs is in
 * the device's platform data.
 */
DM_DRIVER(stm32_gpio) = {
    .name = "stm32_gpio",
    .id   = UCLASS_GPIO,
    .of_match = stm32_gpio_ids,

    .of_to_plat = stm32_gpio_of_to_plat,
    .plat_auto  = sizeof(struct stm32_gpio_plat),

    .probe = stm32_gpio_probe,

    .ops = &stm32_gpio_ops,
};
