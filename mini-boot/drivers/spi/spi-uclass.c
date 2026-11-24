#include <errno.h>
#include <spi.h>
#include <dm/device.h>
#include <dm/read.h>
#include <dm/root.h>
#include <dm/uclass.h>


/*
 * The SPI uclass: the generic half of every SPI bus driver. A slave's
 * driver talks to its own device; this file finds the bus (the slave's
 * parent) and the slave's settings (its parent_priv), and calls the bus
 * driver's operation.
 */


/**
 * At the bind of a slave from the device tree, under a bus of this uclass:
 * its node's SPI properties into its parent_plat (struct spi_slave_plat,
 * allocated by the core, per_child_plat_auto), which
 * spi_child_pre_probe() then checks and copies. 'dev' is the slave.
 *
 * Returns 0, or -EINVAL if the node has no reg (the chip select) or no
 * spi-max-frequency.
 */
static int spi_child_post_bind(
    struct udevice *dev
)
{
    struct spi_slave_plat *plat = dev->parent_plat;

    if (dev_read_u32(dev, "reg", &plat->cs))
        return -EINVAL;

    if (dev_read_u32(dev, "spi-max-frequency", &plat->max_hz))
        return -EINVAL;

    plat->mode = 0;
    if (dev_read_bool(dev, "spi-cpol"))
        plat->mode |= SPI_CPOL;
    if (dev_read_bool(dev, "spi-cpha"))
        plat->mode |= SPI_CPHA;

    return SUCCESS;
}


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

    /* A bus binds its slaves, the child nodes of its own node */
    .post_bind       = dm_scan_fdt_dev,
    .child_post_bind = spi_child_post_bind,
    .child_pre_probe = spi_child_pre_probe,

    .per_child_plat_auto = sizeof(struct spi_slave_plat),
    .per_child_auto      = sizeof(struct spi_slave),
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
