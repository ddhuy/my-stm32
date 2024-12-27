/*
 * peripherals.c
 *
 *  Created on: Dec 26, 2024
 *      Author: ddh
 */

#include "peripherals.h"


/* Timer peripherals */
/*
 * 'Start Basic Timer'
 * This method starts a new 'basic' timer, which triggers
 * an 'UPDATE' interrupt every time that it triggers.
 * It should trigger every N milliseconds.
 */
void start_timer(TIM_TypeDef *TIMx, uint16_t ms)
{
	// Start by making sure the timer's 'counter' is off.
	TIMx->CR1 &= ~(TIM_CR1_CEN);
	// Next, reset the peripheral. (This is where a HAL can help)
	if (TIMx == TIM2) {
		RCC->APB1RSTR |= (RCC_APB1RSTR_TIM2RST);
		RCC->APB1RSTR &= ~(RCC_APB1RSTR_TIM2RST);
	}
	// Set the timer pre-scaler/auto-reload timing registers.
	TIMx->PSC = core_clock_hz / 1000;
	TIMx->ARR = ms;
	// Send an update event to reset the timer an apply settings.
	TIMx->EGR = TIM_EGR_UG;
	// Enable the hardware interrupt.
	TIMx->DIER = TIM_DIER_UIE;
	// Enable the timer.
	TIMx->CR1 = TIM_CR1_CEN;
}


/*
 * Stop a running timer on a given Timer peripheral.
 */
void stop_timer(TIM_TypeDef *TIMx)
{
	// Turn off the timer's 'counter'.
	TIMx->CR1 &= ~(TIM_CR1_CEN);
	// Clear the 'pending update interrupt' flag.
	TIMx->SR &= ~(TIM_SR_UIF);
}
