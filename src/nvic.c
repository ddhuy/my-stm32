/*
 * global.c
 *
 *  Created on: Dec 15, 2024
 *      Author: ddh
 */

#include "nvic.h"


void EXTI0_1_IRQ_handler(void)
{
    // Check that EXTI1 (or 0?) is the line that triggered
    if (EXTI->PR & EXTI_PR_PR0) {
    	// If it was, write 1 to clear the interrupt flag.
    	EXTI->PR |= EXTI_PR_PR0;
    	// If you are using a button, toggle the LED state:
    	GPIOG->ODR ^= GPIO_OTYPER_OT13;
    }
}
