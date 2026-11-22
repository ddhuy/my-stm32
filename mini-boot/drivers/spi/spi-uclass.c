#include <errno.h>
#include <spi.h>
#include <dm/device.h>


/*
 * The SPI uclass: the generic half of every SPI bus driver. A slave's
 * driver talks to its own device; this file finds the bus (the slave's
 * parent) and the slave's settings (its parent_plat), and calls the bus
 * driver's operation.
 */

/**
 * The core probed the bus when it probed the slave, so it is ready. The
 * parent's uclass is checked because the core accepts any parent: a slave
 * row naming a GPIO port would otherwise call GPIO operations as SPI ones,
 * and the bus would read parent_plat as the wrong type.
 */
int spi_xfer(
    struct udevice *slave,
    const u8 *dout,
    u8 *din,
    size_t len,
    u32 flags
)
{
    struct udevice *bus = slave->parent;

    if (!bus)
        return -ENODEV;

    if (bus->driver->id != UCLASS_SPI || !slave->parent_plat)
        return -EINVAL;

    const struct spi_ops *ops = bus->driver->ops;

    if (!ops || !ops->xfer)
        return -ENOSYS;

    return ops->xfer(bus, slave->parent_plat, dout, din, len, flags);
}
