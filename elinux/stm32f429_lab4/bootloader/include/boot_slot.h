#ifndef BOOT_SLOT_H
#define BOOT_SLOT_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    BOOT_SLOT_A = 0,
    BOOT_SLOT_B = 1
} boot_slot_t;

typedef struct
{
    uint32_t base;
    uint32_t size;
    uint32_t sector;
} boot_slot_info_t;

/*
 * Return the memory layout for a firmware slot.
 *
 * Returns false for an invalid slot identifier.
 */
bool boot_slot_get_info(
    boot_slot_t slot,
    boot_slot_info_t *info);

#endif
