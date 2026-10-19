#include <stdint.h>
#include "image_loader.h"
#include "crc32.h"
#include "memory_map.h"


image_status_t image_header_validate(const app_image_header_t *header) {
    if (!header)
        return IMAGE_ERR_HEADER;

    if (header->magic != APP_IMAGE_MAGIC)
        return IMAGE_ERR_MAGIC;

    if (header->load_address != APP_SDRAM_BASE)
        return IMAGE_ERR_LOAD_ADDRESS;

    if (header->image_size == 0 ||
        header->image_size > APP_SDRAM_SIZE)
        return IMAGE_ERR_SIZE;

    const uint32_t flash_payload_start =
        APP_FLASH_BASE + sizeof(app_image_header_t);

    const uint32_t flash_available = APP_FLASH_END - flash_payload_start;

    if (header->image_size > flash_available)
        return IMAGE_ERR_FLASH_BOUNDS;

    return IMAGE_OK;
}


image_status_t image_validate_buffer(const uint8_t *buffer, size_t received_size)
{
    if (buffer == NULL)
        return IMAGE_ERR_HEADER;

    if (received_size < sizeof(app_image_header_t))
        return IMAGE_ERR_TRUNCATED;

    /* Copy into aligned storage; the image and STM32 header are little-endian. */
    app_image_header_t header;
    uint8_t *header_bytes = (uint8_t *)&header;
    for (size_t i = 0; i < sizeof(header); ++i)
        header_bytes[i] = buffer[i];

    image_status_t status = image_header_validate(&header);
    if (status != IMAGE_OK)
        return status;

    /* Subtract only after checking the header fits, avoiding size overflow. */
    if (header.image_size > received_size - sizeof(header))
        return IMAGE_ERR_TRUNCATED;

    const uint8_t *payload = buffer + sizeof(header);
    if (crc32_compute(payload, header.image_size) != header.crc)
        return IMAGE_ERR_CRC;

    return IMAGE_OK;
}


image_status_t image_load(void) {
    const app_image_header_t *header =
        (const app_image_header_t *)APP_FLASH_BASE;

    const image_status_t status = image_header_validate(header);
    if (status != IMAGE_OK)
        return status;

    const uint8_t *src =
        (const uint8_t *)(APP_FLASH_BASE + sizeof(app_image_header_t));
    
    uint8_t *dst = (uint8_t *)(uintptr_t) header->load_address;

    for (uint32_t i = 0; i < header->image_size; ++i)
        dst[i] = src[i];

    /* Check the bytes that will execute, catching flash and copy corruption. */
    if (crc32_compute(dst, header->image_size) != header->crc)
        return IMAGE_ERR_CRC;

    return IMAGE_OK;
}
