#include <stdbool.h>
#include <stdint.h>

#include "memtest.h"


/*
 * Memory accessors. Every access goes through these so that it is a real
 * bus cycle (volatile), and so a host-side unit test can replace them with
 * a faulty memory model by defining MEMTEST_ACCESSORS.
 */
#ifndef MEMTEST_ACCESSORS
static inline void wr32(uint32_t a, uint32_t v) { *(volatile uint32_t *)(uintptr_t)a = v; }
static inline void wr16(uint32_t a, uint16_t v) { *(volatile uint16_t *)(uintptr_t)a = v; }
static inline void wr8(uint32_t a, uint8_t v)   { *(volatile uint8_t *)(uintptr_t)a = v; }
static inline uint32_t rd32(uint32_t a)         { return *(volatile uint32_t *)(uintptr_t)a; }
#endif


static bool fail(memtest_result_t *r, const char *test, uint32_t address,
                 uint32_t expected, uint32_t actual)
{
    r->test = test;
    r->address = address;
    r->expected = expected;
    r->actual = actual;
    return false;
}

static bool check(memtest_result_t *r, const char *test, uint32_t address, uint32_t expected)
{
    uint32_t actual = rd32(address);

    if (actual != expected)
        return fail(r, test, address, expected, actual);

    return true;
}

/*
 * Walking 1s and 0s through all 32 bits at one address. The FMC splits each
 * 32-bit access into two 16-bit transfers, so this exercises D0..D15 twice.
 * Writing the inverse to a second address in between drives every data line
 * the other way, so a disconnected line cannot "remember" the value through
 * bus capacitance and pass by accident.
 */
static bool test_data_bus(uint32_t base, memtest_result_t *r)
{
    for (uint32_t pattern = 1u; pattern != 0u; pattern <<= 1)
    {
        wr32(base, pattern);
        wr32(base + 4u, ~pattern);
        if (!check(r, "data bus (walking 1)", base, pattern))
            return false;

        wr32(base, ~pattern);
        wr32(base + 4u, pattern);
        if (!check(r, "data bus (walking 0)", base, ~pattern))
            return false;
    }

    return true;
}

/*
 * Write to base + every power-of-two word offset, so each address line is
 * toggled on its own. A stuck or shorted line makes two of these addresses
 * the same cell, which shows up as one location changing another.
 */
static bool test_address_bus(uint32_t base, uint32_t size, memtest_result_t *r)
{
    const uint32_t pattern = 0xAAAAAAAAu;
    const uint32_t antipattern = 0x55555555u;
    const uint32_t mask = (size / 4u) - 1u;   /* word offsets */
    uint32_t offset;
    uint32_t test;

    for (offset = 1u; (offset & mask) != 0u; offset <<= 1)
        wr32(base + offset * 4u, pattern);

    /* Writing offset 0 must not change the others (a stuck line aliases them with it) */
    wr32(base, antipattern);
    for (offset = 1u; (offset & mask) != 0u; offset <<= 1)
    {
        if (!check(r, "address bus (aliases base address)", base + offset * 4u, pattern))
            return false;
    }
    wr32(base, pattern);

    /* Each write must only change its own cell (stuck or shorted lines alias two offsets) */
    for (test = 1u; (test & mask) != 0u; test <<= 1)
    {
        wr32(base + test * 4u, antipattern);

        if (!check(r, "address bus (aliases base address)", base, pattern))
            return false;

        for (offset = 1u; (offset & mask) != 0u; offset <<= 1)
        {
            if (offset != test &&
                !check(r, "address bus (two offsets alias)", base + offset * 4u, pattern))
                return false;
        }

        wr32(base + test * 4u, pattern);
    }

    return true;
}

/*
 * Byte and halfword writes must touch only their own bytes. On a 16-bit
 * SDRAM this checks the byte enables (FMC NBL0/NBL1), which 32-bit
 * accesses never exercise.
 */
static bool test_byte_lanes(uint32_t base, memtest_result_t *r)
{
    wr32(base, 0u);
    wr8(base + 0u, 0x11u);
    wr8(base + 1u, 0x22u);
    wr8(base + 2u, 0x33u);
    wr8(base + 3u, 0x44u);
    if (!check(r, "byte lanes (8-bit)", base, 0x44332211u))
        return false;

    wr32(base, 0u);
    wr16(base + 0u, 0xAABBu);
    wr16(base + 2u, 0xCCDDu);
    if (!check(r, "byte lanes (16-bit)", base, 0xCCDDAABBu))
        return false;

    return true;
}

/*
 * Fill every word with a value unique to it (its index + 1), verify, then
 * repeat with the inverse so every bit of every cell holds both 0 and 1.
 * Unique values also catch aliasing that the address bus test can miss.
 */
static bool test_device(uint32_t base, uint32_t size, memtest_result_t *r)
{
    const uint32_t words = size / 4u;
    uint32_t i;

    for (i = 0u; i < words; ++i)
        wr32(base + i * 4u, i + 1u);

    for (i = 0u; i < words; ++i)
    {
        if (!check(r, "device (pattern)", base + i * 4u, i + 1u))
            return false;

        wr32(base + i * 4u, ~(i + 1u));
    }

    for (i = 0u; i < words; ++i)
    {
        if (!check(r, "device (inverted)", base + i * 4u, ~(i + 1u)))
            return false;
    }

    return true;
}

bool memtest_run(uint32_t base, uint32_t size, memtest_result_t *result)
{
    result->test = 0;
    result->address = 0u;
    result->expected = 0u;
    result->actual = 0u;

    if ((base & 3u) != 0u || size < 8u || (size & (size - 1u)) != 0u)
        return fail(result, "invalid arguments", base, 0u, size);

    return test_data_bus(base, result) &&
           test_address_bus(base, size, result) &&
           test_byte_lanes(base, result) &&
           test_device(base, size, result);
}
