#ifndef MINIBOOT_DM_ROOT_H
#define MINIBOOT_DM_ROOT_H


/**
 * Bind every device the board declared with DM_DEVICE. A device that fails
 * is skipped; the report is lost if the console is not up yet. Needs the
 * heap.
 *
 * Returns the number of devices bound.
 */
int dm_init(void);


#endif /* MINIBOOT_DM_ROOT_H */
