#ifndef MINIBOOT_DM_ROOT_H
#define MINIBOOT_DM_ROOT_H


/**
 * Bind every device the board declared with DM_DEVICE. A device that fails
 * is reported on the console and skipped. Needs the console and the heap.
 *
 * Returns the number of devices bound.
 */
int dm_init(void);


#endif /* MINIBOOT_DM_ROOT_H */
