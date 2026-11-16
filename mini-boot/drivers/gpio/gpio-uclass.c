#include <errno.h>
#include <gpio.h>
#include <dm/device.h>


/*
 * The GPIO uclass: the generic half of every GPIO driver. Each function
 * finds the driver's table of operations through the device and calls the
 * matching entry. Both the table and the entry are checked, so a driver may
 * leave out an operation it cannot do.
 */


/**
 * See gpio.h.
 */
int gpio_direction_output(
    struct udevice *dev,
    unsigned int pin,
    int value
)
{
    const struct gpio_ops *ops = dev->driver->ops;

    if (!ops || !ops->direction_output)
        return -ENOSYS;

    return ops->direction_output(dev, pin, value);
}


/**
 * See gpio.h.
 */
int gpio_set_alternative(
    struct udevice *dev,
    unsigned int pin,
    unsigned int af
)
{
    const struct gpio_ops *ops = dev->driver->ops;

    if (!ops || !ops->set_alternative)
        return -ENOSYS;

    return ops->set_alternative(dev, pin, af);
}


/**
 * See gpio.h.
 */
int gpio_set_value(
    struct udevice *dev,
    unsigned int pin,
    int value
)
{
    const struct gpio_ops *ops = dev->driver->ops;

    if (!ops || !ops->set_value)
        return -ENOSYS;

    return ops->set_value(dev, pin, value);
}


/**
 * See gpio.h.
 */
int gpio_get_value(
    struct udevice *dev,
    unsigned int pin
)
{
    const struct gpio_ops *ops = dev->driver->ops;

    if (!ops || !ops->get_value)
        return -ENOSYS;

    return ops->get_value(dev, pin);
}
