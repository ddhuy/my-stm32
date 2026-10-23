#include <stddef.h>

#include "common/include/memory_map.h"

#include "boot_slot.h"

bool boot_slot_get_info(
    boot_slot_t slot,
    boot_slot_info_t *info)
{
    if (info == NULL)
    {
        return false;
    }

    switch (slot)
    {
    case BOOT_SLOT_A:
        info->base = APP_SLOT_A_BASE;
        info->size = APP_SLOT_SIZE;
        info->sector = APP_SLOT_A_SECTOR;
        return true;

    case BOOT_SLOT_B:
        info->base = APP_SLOT_B_BASE;
        info->size = APP_SLOT_SIZE;
        info->sector = APP_SLOT_B_SECTOR;
        return true;

    default:
        return false;
    }
}
