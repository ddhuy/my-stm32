/*
 * global.c
 *
 *  Created on: Dec 15, 2024
 *      Author: ddh
 */

#include "main.h"


/****
 * Setup the User Button pins
 */
void init_button(void)
{
	// Enable the GPIOA peripheral in "RCC_AHB1ENR"
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
	// Initialize the GPIOA pins as 'input'.
	GPIOA->MODER &= ~GPIO_MODER_MODER0;
	// PA0 should be set to 'input' mode with pull-up.
	GPIOA->PUPDR &= ~GPIO_PUPDR_PUPD0;
	GPIOA->PUPDR |=  GPIO_PUPDR_PUPD0_1;
}


/****
 * Setup the LED3 pins
 */
void init_led3(void)
{
	// Enable the GPIOG peripheral in "RCC_AHB1ENR"
	RCC->AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
	// PG13 is connected to an LED on the 'STM32F4-DISC' board
	//      It should be set to push-pull low-speed output.
	GPIOG->MODER  &= ~GPIO_MODER_MODER13;
	GPIOG->MODER  |=  GPIO_MODER_MODER13_0;
	GPIOG->OTYPER &= ~GPIO_OTYPER_OT13;
}


/****
 * Setup the EXTI
 */
void init_syscfg(void)
{
	// Enable the SYSCFG peripheral.
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	// Set SYSCFG to connect to the button EXTI line to PA0.
	SYSCFG->EXTICR[SYSCFG_EXTICR1_EXTI0_Pos / 4] &= ~SYSCFG_EXTICR1_EXTI0_Msk;
	SYSCFG->EXTICR[SYSCFG_EXTICR1_EXTI0_Pos / 4] |=  SYSCFG_EXTICR1_EXTI0_PA;
	// Setup the button's EXTI line as an interrupt.
	EXTI->IMR |= EXTI_IMR_IM0;
	// Disable the 'rising edge' trigger (button release).
	EXTI->RTSR &= ~EXTI_RTSR_TR0;
	// Disable the 'falling edge' trigger (button press).
	EXTI->FTSR |=  EXTI_FTSR_TR0;
	// Enable the NVIC interrupt at minimum priority
	NVIC_SetPriority(EXTI0_IRQn, 0);
	NVIC_EnableIRQ(EXTI0_IRQn);
}


/****
 * Main program.
 */
int main(void)
{
	volatile uint8_t temp = 0;
	// setup STM32F4-DISC USR_BUTTON & LED3
	init_button();
	init_led3();
	init_syscfg();
	// main program logic
	while (1) {
		if (temp == 0xFF)
			temp = 0;
		temp++;
	}
}
