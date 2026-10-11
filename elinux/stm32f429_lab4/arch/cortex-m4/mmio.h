#ifndef CORTEX_M4_MMIO_H
#define CORTEX_M4_MMIO_H

#include <stdint.h>

/*
 * Raw memory-mapped register access. Every register macro in this directory
 * expands to one of these, so it can be read and written like a variable:
 *
 *     RCC_AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
 */
#define MMIO32(addr) (*(volatile uint32_t *)(uintptr_t)(addr))
#define MMIO16(addr) (*(volatile uint16_t *)(uintptr_t)(addr))
#define MMIO8(addr)  (*(volatile uint8_t  *)(uintptr_t)(addr))

#define BIT(n)       (1u << (n))

#endif
