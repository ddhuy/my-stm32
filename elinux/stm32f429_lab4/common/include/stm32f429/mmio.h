#ifndef STM32F429_MMIO_H
#define STM32F429_MMIO_H

#include <stdint.h>

#define MMIO32(addr) (*(volatile uint32_t *)(uintptr_t)(addr))

#endif