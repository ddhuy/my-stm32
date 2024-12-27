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
	/**
	 * STEPS FOLLOWED
	 * 1. Enable the SYSCFGEN bit in RCC register.
	 * 2. Configure the EXTI configuration Register in the SYSCFG.
	 * 3. Disable the EXTI using Interrupt Mask Register (IMR).
	 * 4. Configure the Rising Edge / Falling Edge Trigger.
	 * 5. Set the Interrupt Priority.
	 * 6. Enable the interrupt.
	 */
	// Enable the SYSCFG peripheral.
	RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
	// Set SYSCFG to connect to the button EXTI line to PA0.
	SYSCFG->EXTICR[SYSCFG_EXTICR1_EXTI0_Pos / 4] &= ~SYSCFG_EXTICR1_EXTI0_Msk;
	SYSCFG->EXTICR[SYSCFG_EXTICR1_EXTI0_Pos / 4] |=  SYSCFG_EXTICR1_EXTI0_PA;
	// Setup the button's EXTI line as an interrupt.
	EXTI->IMR |= EXTI_IMR_IM0;
	// Disable the 'rising edge' trigger (button release).
	EXTI->RTSR &= ~EXTI_RTSR_TR0;
	// Enable the 'falling edge' trigger (button press).
	EXTI->FTSR |=  EXTI_FTSR_TR0;
	// Enable the NVIC interrupt at minimum priority
	NVIC_SetPriority(EXTI0_IRQn, 0);
	NVIC_EnableIRQ(EXTI0_IRQn);
}


/****
 * Initial clock setup.
 */
void init_sysclock(void)
{
	// Reset the Flash 'Access Control Register', and
	// then set 1 wait-state and enable the pre-fetch buffer.
	FLASH->ACR &= ~(FLASH_ACR_LATENCY | FLASH_ACR_PRFTEN);
	FLASH->ACR |= (FLASH_ACR_LATENCY | FLASH_ACR_PRFTEN);
	// Configure the PLL to (HSI / 2) * 12 = 48MHz.
	// Use a PLLMUL of 0xA for *12, and keep PLLSRC at 0
	// to use (HSI / PREDIV) as the core source. HSI = 8MHz.
//	RCC->CFGR &= ~(RCC_CFGR_PLLMUL | RCC_PLLCFGR_PLLSRC);
//	RCC->CFGR |= (RCC_CFGR_PLLSRC_HSI_DIV2 | RCC_CFGR_PLLMUL12);
	// Turn the PLL on and wait for it to be ready.
	RCC->CR |= (RCC_CR_PLLON);
	while (!(RCC->CR & RCC_CR_PLLRDY)) {};
	// Select the PLL as the system clock source.
	RCC->CFGR &= ~(RCC_CFGR_SW);
	RCC->CFGR |= (RCC_CFGR_SW_PLL);
	while (!(RCC->CFGR & RCC_CFGR_SWS_PLL)) {};
	// Set the global clock speed variable.
	core_clock_hz = 48000000;
}


/****
 * Main program.
 */
int main(void)
{
	// setup STM32F4-DISC USR_BUTTON & LED3
	init_sysclock();
	init_syscfg();
	init_button();
	init_led3();

	// Enable the TIM2 clock.
	RCC->APB1ENR |= RCC_APB1ENR_TIM12EN;
	// Enable the NVIC interrupt for TIM2.
	// (Timer peripheral initialized and used elsewhere)
	NVIC_SetPriority(TIM2_IRQn, 0x01);
	NVIC_EnableIRQ(TIM2_IRQn);

	// main program logic
	while (1) {
//		if (led_on)
//			GPIOG->ODR |= GPIO_OTYPER_OT13;
//		else
//			GPIOG->ODR &= ~GPIO_OTYPER_OT13;
	}
}
