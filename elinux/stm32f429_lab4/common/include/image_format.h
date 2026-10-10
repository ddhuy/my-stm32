#ifndef IMAGE_FORMAT_H
#define IMAGE_FORMAT_H

#include <assert.h>
#include <stdint.h>

#define APP_IMAGE_MAGIC     0x00070013u /* Magic number for application image */
#define APP_IMAGE_LOAD_ADDR 0xD0000000u


typedef struct {
    uint32_t magic;        /* Magic number for the application image */
    uint32_t image_size;   /* Size of the application image */
    uint32_t load_address; /* Load address of the application image */
    uint32_t crc;          /* CRC32 of the application image */
} app_image_header_t;


static_assert(sizeof(app_image_header_t) == 16,
              "Application image header must be 16 bytes");

#endif