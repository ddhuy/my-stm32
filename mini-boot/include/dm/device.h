#ifndef MINIBOOT_DM_DEVICE_H
#define MINIBOOT_DM_DEVICE_H

#include <dm/driver.h>


/**
 * One device at run time, created by device_bind_node() from a node of
 * the board's device tree.
 *
 * name:   its label in the .dts ("gpioa"), or its node name without one
 * driver: the driver that operates this device
 * plat:   its platform data, plat_auto bytes on the heap, filled by the
 *         driver's of_to_plat; its type is known only to the driver. NULL
 *         if the driver asks for none
 * priv:   the driver's own data, allocated by device_probe(); NULL if the
 *         driver asks for none
 * probed: true once device_probe() has succeeded
 * parent: the device this one is reached through (its bus): the device of
 *         the node it is nested in; NULL at the top level
 * parent_plat: what the parent's uclass needs about this device (an SPI
 *         slave: struct spi_slave_plat), allocated at bind from the
 *         parent's uclass driver (per_child_plat_auto) and filled by its
 *         child_post_bind from this device's node; only that uclass reads
 *         it. NULL at the top level, or if the parent's uclass asks for
 *         none
 * uclass_priv: the uclass's own data about this device, allocated by
 *         device_probe() (the uclass driver's per_device_auto bytes);
 *         only the uclass reads it, never the driver. NULL if the uclass
 *         asks for none
 * parent_priv: the run-time half of parent_plat: what the parent's side
 *         keeps about this device, allocated by device_probe() from the
 *         PARENT's uclass driver (per_child_auto) and filled by its
 *         child_pre_probe. Only the parent's uclass and driver read it.
 *         NULL without a parent, or if the parent's uclass asks for none
 * node:   its node in the device tree blob (an offset, see fdt.h)
 * driver_data: the data of the match-table entry its node matched (which
 *         chip of a family)
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
    void *uclass_priv;
    void *parent_priv;
    int node;
    uintptr_t driver_data;
};


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
 * Make a bound device ready for use: read its node into its plat (the
 * driver's of_to_plat), probe its parent, allocate its private data, its
 * uclass data and its parent data, zeroed, then call its uclass's
 * pre_probe, its parent's uclass's child_pre_probe, the driver's probe
 * and its uclass's post_probe, in that order. Does nothing if the device
 * is already probed.
 *
 * Returns 0, or the error of_to_plat returned, or the parent's error, or
 * -ENOMEM if the heap is full, or the error a hook or the driver's probe
 * returned. After a failure the device stays unprobed and the next call
 * tries again, from the parent on.
 */
int device_probe(
    struct udevice *dev
);

#endif /* MINIBOOT_DM_DEVICE_H */
