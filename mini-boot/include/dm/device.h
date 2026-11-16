#ifndef MINIBOOT_DM_DEVICE_H
#define MINIBOOT_DM_DEVICE_H

#include <dm/driver.h>


/**
 * A board's description of one device it has. Constant, in flash.
 */
struct device_info
{
    const char *name;   /* This device, e.g. "usart1" */
    const char *driver; /* Name of the driver to bind it to */
    const void *plat;   /* Its platform data */
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
 */
struct udevice
{
    const char *name;
    const struct driver *driver;
    const void *plat;
    void *priv;
    bool probed;
};


/**
 * Create a device from 'info', attach it to its driver and add it to the
 * driver's uclass. No hardware is touched.
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
 * Make a bound device ready for use: allocate its private data, zeroed, and
 * call the driver's probe function. Does nothing if the device is already
 * probed.
 *
 * Returns 0, or -ENOMEM if the heap is full, or the error the driver's
 * probe returned. After a failure the device stays unprobed and the next
 * call tries again.
 */
int device_probe(
    struct udevice *dev
);

#endif /* MINIBOOT_DM_DEVICE_H */
