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


/* dev->node of a device that does not come from the device tree */
#define DEV_NODE_NONE   (-1)


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
 * plat:   its platform data; its type is known only to the driver. From
 *         a device_info: the board's, in flash. From the device tree:
 *         plat_auto bytes on the heap, filled by the driver's of_to_plat
 * priv:   the driver's own data, allocated by device_probe(); NULL if the
 *         driver asks for none
 * probed: true once device_probe() has succeeded
 * parent: the device this one is reached through (its bus), set by
 *         device_set_parent() after every device is bound; NULL if none
 * parent_plat: set with the parent; its type is defined by the parent's
 *         uclass, which alone reads it. From the device tree: allocated
 *         (per_child_plat_auto) and filled by the parent's uclass
 *         (child_post_bind) from this device's node
 * orphan: its device_info names a parent that is not (yet) linked. Set by
 *         device_bind(), cleared by device_set_parent(); device_probe()
 *         refuses an orphan, so no driver runs without its parent.
 * uclass_priv: the uclass's own data about this device, allocated by
 *         device_probe() (the uclass driver's per_device_auto bytes);
 *         only the uclass reads it, never the driver. NULL if the uclass
 *         asks for none
 * parent_priv: the run-time half of parent_plat: what the parent's side
 *         keeps about this device, allocated by device_probe() from the
 *         PARENT's uclass driver (per_child_auto) and filled by its
 *         child_pre_probe. Only the parent's uclass and driver read it.
 *         NULL without a parent, or if the parent's uclass asks for none
 * node:   its node in the device tree blob; DEV_NODE_NONE if it was bound
 *         from a device_info
 * driver_data: the data of the match-table entry its node matched (which
 *         chip of a family); 0 for a device from a device_info
 */
struct udevice
{
    const char *name;
    const struct driver *driver;
    void *plat;
    void *priv;
    bool probed;
    struct udevice *parent;
    void *parent_plat;
    bool orphan;
    void *uclass_priv;
    void *parent_priv;
    int node;
    uintptr_t driver_data;
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
 * Create a device from the device tree node 'node', as a child of 'parent'
 * (NULL: top level). Skipped (-ENODEV) unless its status is "okay" (or
 * absent) and one of its compatible strings, tried in order, has a driver:
 * the first string is the most specific. Named after its label in
 * /__symbols__ (dtc -@), or its node name if it has none. Then its plat
 * (plat_auto) and, under a parent, its parent_plat (the parent's uclass's
 * per_child_plat_auto) are allocated, and the hooks run: its uclass's
 * post_bind, then the parent's uclass's child_post_bind. No hardware is
 * touched.
 *
 * Returns 0 and the device in '*devp' (if 'devp' is not NULL); -ENODEV if
 * the node is disabled or no driver matches it; -ENOMEM; -EPFNOSUPPORT
 * if the driver's uclass has no uclass driver; or a hook's error.
 */
int device_bind_node(
    struct udevice *parent,
    int node,
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
 * Make a bound device ready for use: read its node into its plat (the
 * driver's of_to_plat, for a device from the device tree), probe its
 * parent, allocate its private data, its uclass data and its parent data,
 * zeroed, then call its uclass's pre_probe, its parent's uclass's
 * child_pre_probe, the driver's probe and its uclass's post_probe, in that
 * order. Does nothing if the device is already probed.
 *
 * Returns 0, or -ENODEV for an orphan, or the error of_to_plat returned,
 * or the parent's error, or -ENOMEM if the heap is full, or the error a
 * hook or the driver's probe returned. After a failure the device stays
 * unprobed and the next call tries again, from the parent on.
 */
int device_probe(
    struct udevice *dev
);

#endif /* MINIBOOT_DM_DEVICE_H */
