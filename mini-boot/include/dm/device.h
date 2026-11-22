#ifndef MINIBOOT_DM_DEVICE_H
#define MINIBOOT_DM_DEVICE_H

#include <dm/driver.h>


/**
 * A board's description of one device it has. Constant, in flash.
 */
struct device_info
{
    const char *name;        /* This device, e.g. "usart1" */
    const char *driver;      /* Name of the driver to bind it to */
    const void *plat;        /* Its platform data */
    const char *parent;      /* Name of its parent device; NULL: none */
    const void *parent_plat; /* What the parent's uclass needs about this
                                device (an SPI slave: spi_slave_plat) */
};


/*
 * Declare a device: DM_DEVICE(usart1) = { .name = "usart1", ... };
 *
 * Collected into the .dm_devices section the same way DM_DRIVER collects
 * drivers. dm_init() binds every entry.
 */
#define DM_DEVICE(_name)                        \
    static const struct device_info _name##_info  \
    __attribute__((used, section(".dm_devices")))


/**
 * One device at run time, created by device_bind() from a device_info.
 *
 * name:   copied from the device_info
 * driver: the driver that operates this device
 * plat:   the board's platform data; its type is known only to the driver
 * priv:   the driver's own data, allocated by device_probe(); NULL if the
 *         driver asks for none
 * probed: true once device_probe() has succeeded
 * parent: the device this one is reached through (its bus), set by
 *         device_set_parent() after every device is bound; NULL if none
 * parent_plat: set with the parent; its type is defined by the parent's
 *         uclass, which alone reads it
 * orphan: its device_info names a parent that is not (yet) linked. Set by
 *         device_bind(), cleared by device_set_parent(); device_probe()
 *         refuses an orphan, so no driver runs without its parent.
 */
struct udevice
{
    const char *name;
    const struct driver *driver;
    const void *plat;
    void *priv;
    bool probed;
    struct udevice *parent;
    const void *parent_plat;
    bool orphan;
};


/**
 * Create a device from 'info', attach it to its driver and add it to the
 * driver's uclass. No hardware is touched. A device whose 'info' names a
 * parent starts as an orphan, until device_set_parent() links it.
 *
 * Returns 0 and stores the new device in '*devp', or -ENOENT if the driver
 * does not exist, -ENOMEM if the heap or the uclass is full. '*devp' is left
 * alone on failure.
 */
int device_bind(
    const struct device_info *info,
    struct udevice **devp
);


/**
 * Make 'parent' the parent of 'dev', with 'parent_plat' (what the
 * parent's uclass needs about 'dev'), and clear the orphan flag. The one
 * place a parent is linked: dm_init() calls it once every device is
 * bound, so the order of the rows does not matter.
 *
 * Returns 0, or -EINVAL if the link would make a cycle ('dev' would be
 * its own ancestor, or its own parent): device_probe() would then
 * recurse until the stack overflows. Nothing changes on failure.
 */
int device_set_parent(
    struct udevice *dev,
    struct udevice *parent,
    const void *parent_plat
);


/**
 * Make a bound device ready for use: probe its parent first, then
 * allocate its private data, zeroed, and call the driver's probe
 * function. Does nothing if the device is already probed.
 *
 * Returns 0, or -ENODEV for an orphan, or the parent's error, or -ENOMEM
 * if the heap is full, or the error the driver's probe returned. After a
 * failure the device stays unprobed and the next call tries again.
 */
int device_probe(
    struct udevice *dev
);

#endif /* MINIBOOT_DM_DEVICE_H */
