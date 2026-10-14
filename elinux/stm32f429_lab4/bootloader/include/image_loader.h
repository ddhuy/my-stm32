#ifndef IMAGE_LOADER_H
#define IMAGE_LOADER_H

#include <stdbool.h>
#include "image_format.h"


typedef enum {
    IMAGE_OK = 0,
    IMAGE_ERR_HEADER,
    IMAGE_ERR_MAGIC,
    IMAGE_ERR_LOAD_ADDRESS,
    IMAGE_ERR_SIZE,
    IMAGE_ERR_FLASH_BOUNDS,
    IMAGE_ERR_CRC
} image_status_t;


image_status_t image_header_validate(const app_image_header_t *header);
/* Validate bounds, copy to SDRAM, then verify the payload CRC before handoff. */
image_status_t image_load(void);

#endif
