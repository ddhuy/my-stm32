#include <errno.h>
#include <spi.h>
#include <dm/device.h>
#include <dm/uclass.h>


/*
 * The SPI uclass: the generic half of every SPI bus driver. A slave's
 * driver talks to its own device; this file finds the bus (the slave's
 * parent) and the slave's settings (its parent_priv), and calls the bus
 * driver's operation.
 */


/**
 * Before a slave's driver probes: check what the board says about the
 * slave (its parent_plat) and copy it into the slave's run-time settings
 * (its parent_priv, struct spi_slave). 'dev' is the slave. Only the
 * generic checks: how many chip selects a bus has is its driver's fact.
 * From here on nothing reads the board's plat.
 *
 * Returns 0, or -EINVAL if the row has no spi_slave_plat, a mode bit other
 * than CPOL / CPHA, or a max_hz of 0.
 */
static int spi_child_pre_probe(
    struct udevice *dev
)
{
    const struct spi_slave_plat *plat = dev->parent_plat;
    struct spi_slave *slave = dev->parent_priv;

    if (!plat)
        return -EINVAL;

    if (plat->mode & ~SPI_MODE_MASK)
        return -EINVAL;

    if (!plat->max_hz)
        return -EINVAL;

    slave->cs = plat->cs;
    slave->mode = plat->mode;
    slave->max_hz = plat->max_hz;

    return SUCCESS;
}


UCLASS_DRIVER(spi) = {
    .name = "spi",
    .id   = UCLASS_SPI,

    .child_pre_probe = spi_child_pre_probe,
    .per_child_auto  = sizeof(struct spi_slave),
};


/**
 * The core probed the bus when it probed the slave, so it is ready. The
 * parent's uclass is checked because the core accepts any parent: a slave
 * row naming a GPIO port would otherwise call GPIO operations as SPI ones.
 * That check comes first: such a slave never ran spi_child_pre_probe()
 * (the hook belongs to the parent's uclass), so its parent_priv is NULL.
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

    if (bus->driver->id != UCLASS_SPI || !slave->parent_priv)
        return -EINVAL;

    const struct spi_ops *ops = bus->driver->ops;

    if (!ops || !ops->xfer)
        return -ENOSYS;

    return ops->xfer(bus, slave->parent_priv, dout, din, len, flags);
}
