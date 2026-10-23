#ifndef IMAGE_LOADER_H
#define IMAGE_LOADER_H

#include <stdbool.h>
#include <stddef.h>

#include "common/include/image_format.h"

#include "boot_slot.h"


typedef enum {
    IMAGE_OK = 0,
    IMAGE_ERR_HEADER,
    IMAGE_ERR_MAGIC,
    IMAGE_ERR_LOAD_ADDRESS,
    IMAGE_ERR_SIZE,
    IMAGE_ERR_FLASH_BOUNDS,
    IMAGE_ERR_CRC,
    IMAGE_ERR_TRUNCATED
} image_status_t;


image_status_t image_header_validate(const app_image_header_t *header);
/* Validate a packaged image without modifying it. buffer must provide
 * received_size readable bytes; no alignment is required. Trailing transport
 * padding is allowed and excluded from the payload CRC.
 */
image_status_t image_validate_buffer(const uint8_t *buffer, size_t received_size);

/* Validate bounds, copy to SDRAM, then verify the payload CRC before handoff. */
image_status_t image_load(boot_slot_t slot);

/*
 * Validate the packaged application stored in Flash.
 * Does not copy the payload into SDRAM.
 */
image_status_t image_validate_flash(boot_slot_t slot);

/* Validate a packaged image stored in a given slot. */
image_status_t image_validate_slot(boot_slot_t slot);

/* Copy a selected slot's payload to SDRAM and verify CRC32. */
image_status_t image_load_slot(boot_slot_t slot);

#endif
