#include <stdint.h>
#include "image_loader.h"


#define SDRAM_SIZE_BYTES    (8u * 1024u * 1024u)
#define APP_FLASH_BASE_ADDR  0x0800'2000u
#define FLASH_END            0x0820'0000u


bool image_header_validate(const app_image_header_t *header) {
    if (!header) {
        return false;
    }

    if (header->magic != APP_IMAGE_MAGIC) {
        return false;
    }

    if (header->load_address != APP_IMAGE_LOAD_ADDR) {
        return false;
    }

    if (header->image_size == 0 || header->image_size > SDRAM_SIZE_BYTES) {
        return false;
    }

    const uint32_t flash_payload_start =
        APP_FLASH_BASE_ADDR + sizeof(app_image_header_t);

    const uint32_t flash_available = FLASH_END - flash_payload_start;

    if (header->image_size > flash_available) {
        return false;
    }

    return true;
}

bool image_load(void) {
    const app_image_header_t *header =
        (const app_image_header_t *)APP_FLASH_BASE_ADDR;

    if (!image_header_validate(header))
        return false;

    const uint8_t *src =
        (const uint8_t *)(APP_FLASH_BASE_ADDR + sizeof(app_image_header_t));
    
    uint8_t *dst = (uint8_t *)(uintptr_t) header->load_address;

    for (uint32_t i = 0; i < header->image_size; ++i)
        dst[i] = src[i];

    return true;
}
