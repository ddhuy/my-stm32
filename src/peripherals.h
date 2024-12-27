/*
 * peripherals.h
 *
 *  Created on: Dec 26, 2024
 *      Author: ddh
 */

#ifndef _VVC_PERIPHERALS_H_
#define _VVC_PERIPHERALS_H_

#include "global.h"


/* Timer Peripherals */
void stop_timer(TIM_TypeDef *TIMx);
void start_timer(TIM_TypeDef *TIMx, uint16_t ms);


#endif /* _VVC_PERIPHERALS_H_ */
