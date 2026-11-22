#include <errno.h>
#include <gyro.h>
#include <dm/device.h>


/*
 * The gyroscope uclass: the generic half of every gyroscope driver. The
 * caller gets the rate in mdps and never learns how the sensor is
 * reached (on this board: SPI, as a child of its bus).
 */

/**
 * Both the table and the entry are checked, as in the other uclasses.
 */
int gyro_read(
    struct udevice *dev,
    struct gyro_data *data
)
{
    const struct gyro_ops *ops = dev->driver->ops;

    if (!ops || !ops->read)
        return -ENOSYS;

    return ops->read(dev, data);
}
