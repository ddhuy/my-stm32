#ifndef MINIBOOT_GPIO_H
#define MINIBOOT_GPIO_H


#include <types.h>


struct udevice;


/**
 * The operations every GPIO driver implements. One device is one port; a
 * pin is a number within its port, starting at 0.
 *
 * direction_output: make the pin a push-pull output driving 'value'
 * set_alternative:  hand the pin to an on-chip peripheral
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


#endif /* MINIBOOT_GPIO_H */
