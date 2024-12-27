/*
 * interrupts_c.c
 *
 *  Created on: Dec 26, 2024
 *      Author: ddh
 */

#include "interrupts_c.h"


// C-language hardware interrupt method definitions.
void TIM2_IRQHandler()
{
	// Handle a timer 'update' interrupt event.
	if (TIM2->SR & TIM_SR_UIF) {
		TIM2->SR &= ~TIM_SR_UIF;
		GPIOG->ODR ^= GPIO_OTYPER_OT13;
	}
}
