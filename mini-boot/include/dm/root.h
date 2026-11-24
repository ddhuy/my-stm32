#ifndef MINIBOOT_DM_ROOT_H
#define MINIBOOT_DM_ROOT_H


/**
 * Bind every device: first every enabled node of the device tree that has
 * a driver (the root's children; a bus binds its own children in its
 * post_bind), then every DM_DEVICE row, then link each row's child to the
 * parent it names (two passes, so the order of the rows does not matter).
 * A device that fails to bind is skipped; a row whose parent is missing or
 * refused stays bound as an orphan, which cannot be probed. The reports
 * are lost if the console is not up yet. Needs the heap.
 *
 * Returns the number of devices bound, orphans included.
 */
int dm_init(void);


struct udevice;


/**
 * Bind the child nodes of the device's node as its children: the
 * post_bind of a bus's uclass (.post_bind = dm_scan_fdt_dev). Does nothing
 * for a device without a node. Children that fail are reported and
 * skipped, so this always returns 0.
 */
int dm_scan_fdt_dev(
    struct udevice *dev
);


#endif /* MINIBOOT_DM_ROOT_H */
