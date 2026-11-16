#include <errno.h>
#include <serial.h>
#include <dm/device.h>


/*
 * The serial uclass: the generic half of every UART driver. Each function
 * finds the driver's table of operations through the device and calls the
 * matching entry, so callers never know which chip they are talking to.
 */

/**
 * A driver without an ops table cannot send.
 */
int serial_putc(
    struct udevice *dev,
    char ch
)
{
    const struct serial_ops *ops = dev->driver->ops;

    if (!ops)
        return -ENOSYS;

    return ops->putc(dev, ch);
}


/**
 * A driver without an ops table cannot receive.
 */
int serial_getc(
    struct udevice *dev
)
{
    const struct serial_ops *ops = dev->driver->ops;

    if (!ops)
        return -ENOSYS;
    
    return ops->getc(dev);
}


/**
 * The answer is a yes or no, so there is no room for an error code: a
 * driver without an ops table never has a character waiting.
 */
int serial_tstc(
    struct udevice *dev
)
{
    const struct serial_ops *ops = dev->driver->ops;

    if (!ops)
        return 0;

    return ops->tstc(dev);
}