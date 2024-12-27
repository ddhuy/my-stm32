/*
 * global.c
 *
 *  Created on: Dec 15, 2024
 *      Author: ddh
 */

#include <stdint.h>


volatile uint8_t led_on;

// (Core system clock speed; initial value depends on the chip.)
volatile uint32_t core_clock_hz;
