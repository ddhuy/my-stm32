
#ifndef BOOT_METADATA_H
#define BOOT_METADATA_H

#include <stdbool.h>
#include "boot_slot.h"

typedef enum
{
    BOOT_META_OK = 0,
    BOOT_META_ERR_ARGUMENT,
    BOOT_META_ERR_FULL,
    BOOT_META_ERR_FLASH,
    BOOT_META_ERR_VERIFY
} boot_meta_status_t;

/*
 * Read the most recent committed selection.
 *
 * Returns false if no valid record exists.
 */
bool boot_meta_read(boot_slot_t *slot);

/*
 * Append a new slot selection.
 *
 * Does not erase the metadata sector.
 * The requested slot must contain a valid image.
 */
boot_meta_status_t boot_meta_select(boot_slot_t slot);

/*
 * Resolve the slot to boot.
 *
 * Prefer persistent selection, otherwise Slot A.
 * Fall back to the other slot if image validation fails.
 */
bool boot_select_slot(boot_slot_t *slot);

#endif
