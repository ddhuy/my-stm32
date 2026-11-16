#ifndef MINIBOOT_DM_DRIVER_H
#define MINIBOOT_DM_DRIVER_H

#include <types.h>
#include <dm/uclass-id.h>


struct udevice;


/**
 * The code that operates one kind of hardware block. There is one copy per
 * driver, in flash, however many devices use it.
 *
 * name:      what a device_info asks for in its 'driver' field
 * id:        the uclass this driver's devices join
 * probe:     sets up the hardware of one device
 * ops:       the uclass's table of operations, implemented by this driver
 * priv_auto: bytes of private data to allocate for each device
 */
struct driver
{
    const char *name;
    enum uclass_id id;

    int (*probe) (struct udevice *dev);

    const void *ops;

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
 * Print every driver in the image. For debugging.
 */
void driver_print_list(void);

#endif /* MINIBOOT_DM_DRIVER_H */
