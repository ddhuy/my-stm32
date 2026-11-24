#include <errno.h>
#include <serial.h>
#include <dm/device.h>
#include <dm/uclass.h>


/*
 * The serial uclass: the generic half of every UART driver. Each function
 * finds the driver's table of operations through the device and calls the
 * matching entry, so callers never know which chip they are talking to.
 * Both the table and the entry are checked, so a driver may leave out an
 * operation it cannot do (a transmit-only UART, for example).
 */

UCLASS_DRIVER(serial) = {
    .name = "serial",
    .id   = UCLASS_SERIAL,
};


/**
 * A driver that does not provide the operation cannot send.
 */
int serial_putc(
    struct udevice *dev,
    char ch
)
{
    const struct serial_ops *ops = dev->driver->ops;

    if (!ops || !ops->putc)
        return -ENOSYS;

    return ops->putc(dev, ch);
}


/**
 * A driver that does not provide the operation cannot receive.
 */
int serial_getc(
    struct udevice *dev
)
{
    const struct serial_ops *ops = dev->driver->ops;

    if (!ops || !ops->getc)
        return -ENOSYS;

    return ops->getc(dev);
}


/**
 * The answer is a yes or no, so there is no room for an error code: a
 * driver that does not provide the operation never has a character waiting.
 */
int serial_tstc(
    struct udevice *dev
)
{
    const struct serial_ops *ops = dev->driver->ops;

    if (!ops || !ops->tstc)
        return 0;

    return ops->tstc(dev);
}