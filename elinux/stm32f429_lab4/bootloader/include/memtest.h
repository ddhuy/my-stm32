#ifndef MEMTEST_H
#define MEMTEST_H

#include <stdbool.h>
#include <stdint.h>

/*
 * Destructive RAM test for external memory (overwrites the whole region).
 * Each test targets a different hardware fault:
 *
 *   data bus    walking 1s / 0s at one address: open, shorted or stuck data lines
 *   address bus power-of-two offsets: open, shorted or stuck address lines
 *   byte lanes  8- and 16-bit writes: broken byte enables (FMC NBL0/NBL1)
 *   device      unique value in every word, then inverted: bad cells, aliasing
 */

typedef struct
{
    const char *test;      /* name of the failing test, NULL if all passed */
    uint32_t address;      /* failing address                              */
    uint32_t expected;
    uint32_t actual;
} memtest_result_t;

/* base must be word aligned; size must be a power of two. Returns true if all tests pass. */
bool memtest_run(uint32_t base, uint32_t size, memtest_result_t *result);

#endif // MEMTEST_H
