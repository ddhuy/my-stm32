#ifndef MINIBOOT_DM_DRIVER_H
#define MINIBOOT_DM_DRIVER_H

#include <types.h>
#include <dm/uclass-id.h>


struct udevice;


/**
 * One entry of a driver's match table: a compatible string from the device
 * tree, and a value the driver gets back as dev->driver_data when a node
 * matched through this entry (which chip of a family it is). The table
 * ends with an entry whose compatible is NULL.
 */
struct udevice_id
{
    const char *compatible;
    uintptr_t data;
};


/**
 * The code that operates one kind of hardware block. There is one copy per
 * driver, in flash, however many devices use it.
 *
 * name:       what a device_info asks for in its 'driver' field
 * id:         the uclass this driver's devices join
 * of_match:   the compatible strings this driver handles; NULL: none (it
 *             can only be bound from a device_info)
 * of_to_plat: reads the device's node into its plat (plat_auto bytes,
 *             allocated at bind), before probe; only for a device bound
 *             from the device tree. Returns 0 or -Exxx (the probe stops)
 * probe:      sets up the hardware of one device
 * ops:        the uclass's table of operations, implemented by this driver
 * plat_auto:  bytes of platform data to allocate for a device bound from
 *             the device tree (one from a device_info has the board's)
 * priv_auto:  bytes of private data to allocate for each device
 */
struct driver
{
    const char *name;
    enum uclass_id id;
    const struct udevice_id *of_match;

    int (*of_to_plat) (struct udevice *dev);
    int (*probe) (struct udevice *dev);

    const void *ops;

    size_t plat_auto;
    size_t priv_auto;
};


/*
 * Declare a driver: DM_DRIVER(foo) = { .name = "foo", ... };
 *
 * The structure goes into the .dm_drivers section, which the linker script
 * collects into one array. 'used' stops the compiler from dropping an object
 * that no code refers to by name.
 */
#define DM_DRIVER(_name)                        \
    static const struct driver _name##_driver     \
    __attribute__((used, section(".dm_drivers")))


/**
 * Look a driver up by name. Returns NULL if no driver has that name.
 */
const struct driver *driver_find(
    const char *name
);


/**
 * Look a driver up by one compatible string: the first driver whose match
 * table has it. Its entry's data goes to '*datap'.
 *
 * Returns the driver, or NULL if none handles 'compatible' ('*datap' is
 * then left alone).
 */
const struct driver *driver_find_by_compatible(
    const char *compatible,
    uintptr_t *datap
);


/**
 * Print every driver in the image. For debugging.
 */
void driver_print_list(void);

#endif /* MINIBOOT_DM_DRIVER_H */
