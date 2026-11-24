#ifndef MINIBOOT_DM_ROOT_H
#define MINIBOOT_DM_ROOT_H


/**
 * Bind every enabled node of the board's device tree that has a driver:
 * the root's children, and a bus's children in its post_bind. A node that
 * fails to bind is reported and skipped; the reports are lost if the
 * console is not up yet. No hardware is touched. Needs the heap.
 *
 * Returns the number of devices bound.
 */
int dm_init(void);


struct udevice;


/**
 * Bind the child nodes of the device's node as its children: the
 * post_bind of a bus's uclass (.post_bind = dm_scan_fdt_dev). Children
 * that fail are reported and skipped, so this always returns 0.
 */
int dm_scan_fdt_dev(
    struct udevice *dev
);


#endif /* MINIBOOT_DM_ROOT_H */
