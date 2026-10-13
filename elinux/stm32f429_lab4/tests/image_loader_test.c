#define _GNU_SOURCE
#include <assert.h>
#include <stdio.h>
#include <string.h>
#include <sys/mman.h>

#include "crc32.h"
#include "image_loader.h"
#include "memory_map.h"

/* Linux host test: reserve the MCU address ranges without replacing mappings. */
int main(int argc, char **argv)
{
    assert(argc == 2);
    const size_t flash_size = APP_FLASH_END - APP_FLASH_BASE;
    void *flash = mmap((void *)(uintptr_t)APP_FLASH_BASE, flash_size,
                      PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    void *sdram = mmap((void *)(uintptr_t)APP_SDRAM_BASE, APP_SDRAM_SIZE,
                      PROT_READ | PROT_WRITE,
                      MAP_PRIVATE | MAP_ANONYMOUS | MAP_FIXED_NOREPLACE, -1, 0);
    assert(flash == (void *)(uintptr_t)APP_FLASH_BASE);
    assert(sdram == (void *)(uintptr_t)APP_SDRAM_BASE);

    FILE *file = fopen(argv[1], "rb");
    assert(file != NULL);
    size_t size = fread(flash, 1, flash_size, file);
    assert(!ferror(file));
    fclose(file);

    app_image_header_t *header = flash;
    uint8_t *payload = (uint8_t *)(header + 1);
    assert(size == sizeof(*header) + header->image_size);
    assert(crc32_compute("123456789", 9) == 0xCBF43926u);
    assert(crc32_compute(NULL, 0) == 0u);
    assert(image_load());
    assert(memcmp(sdram, payload, header->image_size) == 0);

    payload[header->image_size - 1] ^= 1u;
    assert(!image_load());
    payload[header->image_size - 1] ^= 1u;
    header->crc ^= 1u;
    assert(!image_load());
    header->crc ^= 1u;
    assert(image_load());

    const uint32_t good_crc = header->crc;
    header->crc = 0u; /* Legacy reserved value must not disable validation. */
    assert(!image_load());
    header->crc = good_crc;
    payload[header->image_size] ^= 1u; /* Trailing padding is not covered. */
    assert(image_load());

    header->image_size = UINT32_MAX;
    assert(!image_load()); /* Bounds are checked before copying or CRC reads. */
    header->image_size = 0;
    assert(!image_load());
    assert(munmap(flash, flash_size) == 0);
    assert(munmap(sdram, APP_SDRAM_SIZE) == 0);
    return 0;
}
