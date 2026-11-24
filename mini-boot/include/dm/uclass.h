#ifndef MINIBOOT_DM_UCLASS_H
#define MINIBOOT_DM_UCLASS_H

#include <dm/device.h>
#include <dm/uclass-id.h>

/* Most devices one uclass can hold */
#define UCLASS_MAX_DEVICES 8


/**
 * What one uclass is: the code and data shared by every device of one kind,
 * whatever its driver. One per uclass, constant, in flash; the core finds it
 * by id when the uclass gets its first device.
 *
 * name:       for printing only
 * id:         the key drivers name in their .id
 * post_bind:  called by the bind of each device of this uclass, once it is
 *             in the uclass (a bus binds its children from the device tree
 *             here); NULL: none. Returns 0 or -Exxx (the bind fails, the
 *             device stays in the uclass: the heap cannot free)
 * child_post_bind: called by the bind of each CHILD of a device in this
 *             uclass, after the child's post_bind; fills the child's
 *             parent_plat from its node. NULL: none. Returns 0 or -Exxx
 * pre_probe:  called by device_probe() before the driver's probe, so the
 *             uclass can refuse a driver before the hardware is touched;
 *             NULL: none. Returns 0 or -Exxx (the probe stops)
 * post_probe: called after the driver's probe succeeded, for what every
 *             device of the uclass needs once its hardware works; NULL:
 *             none. Returns 0 or -Exxx (the device stays unprobed)
 * per_device_auto: bytes of uclass data to allocate for each device
 *             (dev->uclass_priv); 0: none
 * child_pre_probe: called for each CHILD of a device in this uclass ('dev'
 *             is the child), after the child's own uclass pre_probe and
 *             before the child's driver probe; fills the child's
 *             parent_priv. NULL: none. Returns 0 or -Exxx (the child's
 *             probe stops)
 * per_child_auto: bytes to allocate for each child of a device in this
 *             uclass (child->parent_priv); 0: none
 * per_child_plat_auto: bytes allocated at bind for each child bound from
 *             the device tree under a device in this uclass
 *             (child->parent_plat); 0: none
 *
 * A hook that is set is linked on every board, even one with no device in
 * the uclass: the row is kept by the linker and points at it.
 */
struct uclass_driver
{
    const char *name;
    enum uclass_id id;
    int (*post_bind) (struct udevice *dev);
    int (*child_post_bind) (struct udevice *dev);
    int (*pre_probe) (struct udevice *dev);
    int (*post_probe) (struct udevice *dev);
    size_t per_device_auto;
    int (*child_pre_probe) (struct udevice *dev);
    size_t per_child_auto;
    size_t per_child_plat_auto;
};


/*
 * Declare a uclass driver, in the uclass's own file:
 * UCLASS_DRIVER(gyro) = { .name = "gyro", .id = UCLASS_GYRO };
 *
 * Collected into the .dm_uclasses section the same way DM_DRIVER collects
 * drivers. The suffix keeps gyro_uclass apart from a driver's object.
 */
#define UCLASS_DRIVER(_name)                         \
    static const struct uclass_driver _name##_uclass \
    __attribute__((used, section(".dm_uclasses")))


/**
 * Look a uclass driver up by uclass id. Returns NULL if no uclass driver
 * has that id. With two for the same id, the first in the section wins.
 */
const struct uclass_driver *uclass_driver_find(
    enum uclass_id id
);


/**
 * Add a bound device to the uclass of its driver. The uclass's first device
 * links the uclass to its uclass driver.
 *
 * Returns 0, or -EINVAL if the driver's uclass ID is not valid,
 * -EPFNOSUPPORT if no uclass driver has that ID, -ENOMEM if the uclass is
 * full.
 */
int uclass_add_device(
    struct udevice *dev
);


/**
 * Get the device at position 'index' in a uclass, counting from 0 in the
 * order the devices were bound. The device is not probed.
 *
 * Returns 0 and stores the device in '*devp', or -EINVAL if 'id' is not
 * valid, -ENODEV if the uclass has no device at that index.
 */
int uclass_find_device(
    enum uclass_id id,
    int index,
    struct udevice **devp
);


/**
 * Print every uclass (by its uclass driver's name, "(none)" without one)
 * and the devices in it, with each device's parent ("-" for none). For
 * debugging.
 */
void uclass_print_devices(void);


/**
 * Like uclass_find_device(), and also probe the device, so what comes back
 * is ready for use. This is the call drivers' users make.
 *
 * Returns 0 and stores the device in '*devp', or the error from the lookup
 * or from the probe. '*devp' is left alone on failure.
 */
int uclass_get_device(
    enum uclass_id id,
    int index,
    struct udevice **devp
);


/**
 * Get the device called 'name' in a uclass. The device is not probed. Use
 * this, not an index, when a uclass can hold more than one device: the
 * order in which devices are bound is not guaranteed.
 *
 * Returns 0 and stores the device in '*devp', or -EINVAL if 'id' is not
 * valid, -ENODEV if the uclass has no device of that name.
 */
int uclass_find_device_by_name(
    enum uclass_id id,
    const char *name,
    struct udevice **devp
);


/**
 * Like uclass_find_device_by_name(), and also probe the device.
 *
 * Returns 0 and stores the device in '*devp', or the error from the lookup
 * or from the probe. '*devp' is left alone on failure.
 */
int uclass_get_device_by_name(
    enum uclass_id id,
    const char *name,
    struct udevice **devp
);


/**
 * Look a device up by its device tree node, in every uclass, without
 * probing it (an of_to_plat turns a phandle into a device this way).
 *
 * Returns 0 and the device in '*devp', or -ENODEV if no device was bound
 * from 'node' (a node without a driver, such as a fixed-clock, has none).
 */
int device_find_by_node(
    int node,
    struct udevice **devp
);


/**
 * The number of devices bound, in every uclass.
 */
int uclass_device_count(void);


/**
 * The uclass driver of a bound device: how the core reaches the hooks and
 * sizes of the device's uclass. Never NULL for a bound device, because a
 * device whose uclass has no uclass driver fails to bind.
 */
const struct uclass_driver *device_get_uclass_driver(
    const struct udevice *dev
);

#endif /* MINIBOOT_DM_UCLASS_H */
