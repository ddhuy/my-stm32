#include <errno.h>
#include <gpio.h>
#include <dm/device.h>
#include <dm/read.h>
#include <dm/uclass.h>


/*
 * The GPIO uclass: the generic half of every GPIO driver. Each function
 * finds the driver's table of operations through the device and calls the
 * matching entry. Both the table and the entry are checked, so a driver may
 * leave out an operation it cannot do.
 */

UCLASS_DRIVER(gpio) = {
    .name = "gpio",
    .id   = UCLASS_GPIO,
};


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
 * A driver whose pins have one fixed speed need not provide it.
 */
int gpio_set_speed(
    struct udevice *dev,
    unsigned int pin,
    enum gpio_speed speed
)
{
    const struct gpio_ops *ops = dev->driver->ops;

    if (!ops || !ops->set_speed)
        return -ENOSYS;

    return ops->set_speed(dev, pin, speed);
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


/**
 * The same shape as clk_read_by_index(): phandle, node, device, name.
 */
int gpio_read_by_name(
    const struct udevice *dev,
    const char *list_name,
    int index,
    const char **port,
    u32 *pin,
    u32 *flags
)
{
    /* Pin and flags: the only layout this uclass knows */
    enum { GPIO_CELLS = 2 };
    struct fdt_phandle_args args;
    struct udevice *port_dev;

    int ret = dev_read_phandle_with_args(dev, list_name, "#gpio-cells",
                                         index, &args);
    if (ret)
        return ret;

    if (args.count != GPIO_CELLS)
        return -EINVAL;

    ret = device_find_by_node(args.node, &port_dev);
    if (ret)
        return ret;

    *port = port_dev->name;
    *pin = args.args[0];
    *flags = args.args[1];

    return SUCCESS;
}
