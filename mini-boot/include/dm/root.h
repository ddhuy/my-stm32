#ifndef MINIBOOT_DM_ROOT_H
#define MINIBOOT_DM_ROOT_H


/**
 * Bind every device the board declared with DM_DEVICE, then link each
 * child to the parent its row names (two passes, so the order of the rows
 * does not matter). A device that fails to bind is skipped; a child whose
 * parent is missing or refused stays bound as an orphan, which cannot be
 * probed. The reports are lost if the console is not up yet. Needs the
 * heap.
 *
 * Returns the number of devices bound, orphans included.
 */
int dm_init(void);


#endif /* MINIBOOT_DM_ROOT_H */
