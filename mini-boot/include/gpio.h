#ifndef MINIBOOT_GPIO_H
#define MINIBOOT_GPIO_H


#include <types.h>


struct udevice;


/**
 * How fast a pin's output edges are, slowest first. A slow edge means less
 * noise and current; a fast signal (an SPI clock at several MHz) needs a
 * fast edge, or the receiver samples it while it is still changing. A
 * driver maps these to its own steps (on the STM32: OSPEEDR low, medium,
 * fast, high).
 */
enum gpio_speed
{
    GPIO_SPEED_LOW,
    GPIO_SPEED_MEDIUM,
    GPIO_SPEED_HIGH,
    GPIO_SPEED_VERY_HIGH,
};


/**
 * The operations every GPIO driver implements. One device is one port; a
 * pin is a number within its port, starting at 0.
 *
 * direction_output: make the pin a push-pull output driving 'value'
 * set_alternative:  hand the pin to an on-chip peripheral
 * set_speed:        set how fast the pin's output edges are
 * set_value:        drive an output pin to 0 or 1
 * get_value:        return the level on the pin, 0 or 1
 *
 * Each returns a negative error code on failure, such as -EINVAL for a pin
 * the port does not have.
 */
struct gpio_ops {
    int (*direction_output) (struct udevice *dev, unsigned int pin, int value);
    int (*set_alternative) (struct udevice *dev, unsigned int pin,
                            unsigned int af);
    int (*set_speed) (struct udevice *dev, unsigned int pin,
                      enum gpio_speed speed);
    int (*set_value) (struct udevice *dev, unsigned int pin, int value);
    int (*get_value) (struct udevice *dev, unsigned int pin);
};


/*
 * Generic GPIO API. 'dev' is a probed device of UCLASS_GPIO, as returned by
 * uclass_get_device_by_name(); probing is what turns the port's clock on.
 *
 * Every function returns -ENOSYS if the driver does not provide the
 * operation, or the error from the driver.
 */


/**
 * Make the pin a push-pull output driving 'value'. Returns 0 or -Exxx.
 */
int gpio_direction_output(
    struct udevice *dev,
    unsigned int pin,
    int value
);

/**
 * Hand the pin to a peripheral; 'af' is the alternate function number.
 * Returns 0 or -Exxx.
 */
int gpio_set_alternative(
    struct udevice *dev,
    unsigned int pin,
    unsigned int af
);

/**
 * Set how fast the pin's output edges are. Has no effect on an input; set
 * it before the pin starts driving a fast signal. Returns 0 or -Exxx
 * (-EINVAL for a speed the driver does not know).
 */
int gpio_set_speed(
    struct udevice *dev,
    unsigned int pin,
    enum gpio_speed speed
);

/**
 * Set level on the pin: 0 or 1. Returns 0 or -Exxx.
 */
int gpio_set_value(
    struct udevice *dev,
    unsigned int pin,
    int value
);

/**
 * Get level of the pin: 0 or 1, or -Exxx. A caller that cares must test for
 * a negative result before using it as a level.
 */
int gpio_get_value(
    struct udevice *dev,
    unsigned int pin
);



/**
 * For a consumer's of_to_plat: entry 'index' of the GPIO list 'list_name'
 * of its node (cs-gpios = <&gpioc 1 GPIO_ACTIVE_LOW>), as the name of the
 * port's device, the pin, and the flags (dt-bindings/gpio/gpio.h). The
 * port must have #gpio-cells = <2>. The port is looked up, not probed.
 *
 * Returns 0; -ENODEV if the consumer has no node or the phandle points to
 * a node without a device; -EINVAL if the port's #gpio-cells is not 2; or
 * the error from reading the list.
 */
int gpio_read_by_name(
    const struct udevice *dev,
    const char *list_name,
    int index,
    const char **port,
    u32 *pin,
    u32 *flags
);

#endif /* MINIBOOT_GPIO_H */
