/*
 * global.c
 *
 *  Created on: Dec 15, 2024
 *      Author: ddh
 */

#ifndef _VVC_GLOBAL_H_
#define _VVC_GLOBAL_H_

#include <stdint.h>
#ifdef VVC_F4
	#include "stm32f429i_discovery.h"
#elif VVC_F0
	#include "stm32f031x6.h"
#elif  VVC_L0
	#include "stm32l031xx.h"
#endif


extern volatile uint8_t led_on;

// (Core system clock speed; initial value depends on the chip.)
extern volatile uint32_t core_clock_hz;


#endif // _VVC_GLOBAL_H_
