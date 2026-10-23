
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#include "boot_metadata.h"
#include "image_loader.h"
#include "common/include/memory_map.h"
#include "drivers/flash/stm32_flash.h"
#include "lib/crc/crc32.h" /* Adjust to your existing header path. */

#define META_MAGIC       0x424D4554u
#define META_COMMITTED   0x434F4D54u
#define META_ERASED      0xFFFFFFFFu

typedef struct
{
    uint32_t magic;
    uint32_t slot;
    uint32_t crc;
    uint32_t commit;
} boot_meta_record_t;

_Static_assert(sizeof(boot_meta_record_t) == 16u,
               "Unexpected metadata record size");

#define META_RECORD_COUNT \
    (BOOT_META_FLASH_SIZE / sizeof(boot_meta_record_t))

static const boot_meta_record_t *meta_record(size_t index)
{
    return (const boot_meta_record_t *)(uintptr_t)(
        BOOT_META_FLASH_BASE +
        index * sizeof(boot_meta_record_t));
}

static bool meta_is_erased(const boot_meta_record_t *record)
{
    return record->magic == META_ERASED &&
           record->slot == META_ERASED &&
           record->crc == META_ERASED &&
           record->commit == META_ERASED;
}

static uint32_t meta_crc(const boot_meta_record_t *record)
{
    /* CRC32 covers the magic and slot words. */
    return crc32_compute(
        (const uint8_t *)record,
        offsetof(boot_meta_record_t, crc));
}

static bool meta_is_valid(const boot_meta_record_t *record)
{
    if (record->commit != META_COMMITTED ||
        record->magic != META_MAGIC)
    {
        return false;
    }

    if (record->slot != (uint32_t)BOOT_SLOT_A &&
        record->slot != (uint32_t)BOOT_SLOT_B)
    {
        return false;
    }

    return meta_crc(record) == record->crc;
}

bool boot_meta_read(boot_slot_t *slot)
{
    if (slot == NULL)
    {
        return false;
    }

    bool found = false;

    /*
     * Records are appended in physical order.
     * The last valid record wins.
     *
     * Scan all entries to tolerate torn writes.
     */
    for (size_t i = 0u; i < META_RECORD_COUNT; ++i)
    {
        const boot_meta_record_t *record = meta_record(i);

        if (meta_is_valid(record))
        {
            *slot = (boot_slot_t)record->slot;
            found = true;
        }
    }

    return found;
}

static bool meta_find_append_index(size_t *index)
{
    size_t last_used = 0u;
    bool any_used = false;

    for (size_t i = 0u; i < META_RECORD_COUNT; ++i)
    {
        if (!meta_is_erased(meta_record(i)))
        {
            last_used = i;
            any_used = true;
        }
    }

    size_t next = any_used ? last_used + 1u : 0u;

    if (next >= META_RECORD_COUNT)
    {
        return false;
    }

    *index = next;
    return true;
}

boot_meta_status_t boot_meta_select(boot_slot_t slot)
{
    if (slot != BOOT_SLOT_A && slot != BOOT_SLOT_B)
    {
        return BOOT_META_ERR_ARGUMENT;
    }

    /*
     * Never persist a selection pointing to an
     * image that fails integrity validation.
     */
    if (image_validate_slot(slot) != IMAGE_OK)
    {
        return BOOT_META_ERR_ARGUMENT;
    }

    size_t index;

    if (!meta_find_append_index(&index))
    {
        return BOOT_META_ERR_FULL;
    }

    boot_meta_record_t record = {
        .magic = META_MAGIC,
        .slot = (uint32_t)slot,
        .crc = 0u,
        .commit = META_COMMITTED
    };

    record.crc = meta_crc(&record);

    const uint32_t address =
        BOOT_META_FLASH_BASE +
        (uint32_t)(index * sizeof(record));

    /*
     * Stage 1: write the first three words.
     * The commit word remains erased.
     */
    stm32_flash_status_t status = stm32_flash_program(
        address,
        (const uint8_t *)&record,
        offsetof(boot_meta_record_t, commit));

    if (status != STM32_FLASH_OK)
    {
        return BOOT_META_ERR_FLASH;
    }

    /*
     * Verify all data before writing the commit word.
     */
    const boot_meta_record_t *stored = meta_record(index);

    if (stored->magic != record.magic ||
        stored->slot != record.slot ||
        stored->crc != record.crc ||
        stored->commit != META_ERASED)
    {
        return BOOT_META_ERR_VERIFY;
    }

    /*
     * Stage 2: commit last.
     */
    status = stm32_flash_program(
        address + offsetof(boot_meta_record_t, commit),
        (const uint8_t *)&record.commit,
        sizeof(record.commit));

    if (status != STM32_FLASH_OK)
    {
        return BOOT_META_ERR_FLASH;
    }

    if (!meta_is_valid(stored))
    {
        return BOOT_META_ERR_VERIFY;
    }

    return BOOT_META_OK;
}

bool boot_select_slot(boot_slot_t *slot)
{
    if (slot == NULL)
    {
        return false;
    }

    boot_slot_t preferred = BOOT_SLOT_A;

    (void)boot_meta_read(&preferred);

    if (image_validate_slot(preferred) == IMAGE_OK)
    {
        *slot = preferred;
        return true;
    }

    boot_slot_t alternative =
        (preferred == BOOT_SLOT_A)
            ? BOOT_SLOT_B
            : BOOT_SLOT_A;

    if (image_validate_slot(alternative) == IMAGE_OK)
    {
        *slot = alternative;
        return true;
    }

    return false;
}
