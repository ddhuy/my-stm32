/*
 * global.c
 *
 *  Created on: Dec 15, 2024
 *      Author: ddh
 */

#ifndef _VVC_MAIN_H_
#define _VVC_MAIN_H_


#include <stdint.h>
#ifdef VVC_F4
	#include "stm32f429i_discovery.h"
#elif VVC_F0
	#include "stm32f031x6.h"
#elif  VVC_L0
	#include "stm32l031xx.h"
#endif


#endif // _VVC_MAIN_H_
