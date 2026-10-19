#ifndef CRC32_H
#define CRC32_H

#include <stddef.h>
#include <stdint.h>

/* CRC-32/ISO-HDLC; data must contain size readable bytes. */
uint32_t crc32_compute(const void *data, size_t size);

#endif
