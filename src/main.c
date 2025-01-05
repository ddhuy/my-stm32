/*
 * global.c
 *
 *  Created on: Dec 15, 2024
 *      Author: ddh
 */

#include "main.h"


/**
  * @brief  Configures the System clock source, PLL Multiplier and Divider factors,
  *         AHB/APBx prescalers and Flash settings
  * @Note   This function should be called only once the RCC clock configuration
  *         is reset to the default reset state (done in SystemInit() function).
  * @param  None
  * @retval None
  */
static void SetSysClock(void)
{
/******************************************************************************/
/*            PLL (clocked by HSI) used as System clock source                */
/******************************************************************************/
	__IO uint32_t HSIStatus = 0;

	/* Enable HSE */
	RCC->CR |= ((uint32_t) RCC_CR_HSION);

	/* Wait till HSE is ready and if Time out is reached exit */
	do {
		HSIStatus = RCC->CR & RCC_CR_HSIRDY;
	} while (HSIStatus == 0);

	if ((RCC->CR & RCC_CR_HSIRDY) != RESET) {
		HSIStatus = (uint32_t) 0x01;
	} else {
		HSIStatus = (uint32_t) 0x00;
	}

	if (HSIStatus == (uint32_t) 0x01) {
		/* Select regulator voltage output Scale 1 mode */
		RCC->APB1ENR |= RCC_APB1ENR_PWREN;
		PWR->CR |= PWR_CR_VOS;

		/* HCLK = SYSCLK / 1*/
		RCC->CFGR |= RCC_CFGR_HPRE_DIV1;

		/* PCLK2 = HCLK / 2*/
		RCC->CFGR |= RCC_CFGR_PPRE2_DIV2;

		/* PCLK1 = HCLK / 4*/
		RCC->CFGR |= RCC_CFGR_PPRE1_DIV4;

		/* Configure the main PLL */
		RCC->PLLCFGR = RCC_PLLCFGR_PLLM | RCC_PLLCFGR_PLLN | RCC_PLLCFGR_PLLP
				| RCC_PLLCFGR_PLLSRC_HSI | RCC_PLLCFGR_PLLQ;

		/* Enable the main PLL */
		RCC->CR |= RCC_CR_PLLON;

		/* Wait till the main PLL is ready */
		while ((RCC->CR & RCC_CR_PLLRDY) == 0) {
		}

		/* Enable the Over-drive to extend the clock frequency to 180 Mhz */
		PWR->CR |= PWR_CR_ODEN;
		while ((PWR->CSR & PWR_CSR_ODRDY) == 0) {
		}
		PWR->CR |= PWR_CR_ODSWEN;
		while ((PWR->CSR & PWR_CSR_ODSWRDY) == 0) {
		}

		/* Configure Flash prefetch, Instruction cache, Data cache and wait state */
		FLASH->ACR = FLASH_ACR_PRFTEN | FLASH_ACR_ICEN |FLASH_ACR_DCEN |FLASH_ACR_LATENCY_5WS;

		/* Select the main PLL as system clock source */
		RCC->CFGR &= (uint32_t) ((uint32_t) ~(RCC_CFGR_SW));
		RCC->CFGR |= RCC_CFGR_SW_PLL;

		/* Wait till the main PLL is used as system clock source */
		while ((RCC->CFGR & (uint32_t) RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) {
		}

	} else { /* If HSE fails to start-up, the application will have wrong clock
	 configuration. User can add here some code to deal with this error */
	}
}


/**
  * @brief  Setup the micro-controller system
  *         Initialize the Embedded Flash Interface, the PLL and update the
  *         SystemFrequency variable.
  * @param  None
  * @retval None
  */
void SystemInit(void)
{
    /* Reset the RCC clock configuration to the default reset state ----------*/
	/* Set HSION bit */
	RCC->CR |= (uint32_t) 0x00000001;

	/* Reset CFGR register */
	RCC->CFGR = 0x00000000;

	/* Reset HSEON, CSSON and PLLON bits */
	RCC->CR &= (uint32_t) 0xFEF6FFFF;

	/* Reset PLLCFGR register */
	RCC->PLLCFGR = 0x24003010;

	/* Reset HSEBYP bit */
	RCC->CR &= (uint32_t) 0xFFFBFFFF;

	/* Disable all interrupts */
	RCC->CIR = 0x00000000;

	/* Configure the System clock source, PLL Multiplier and Divider factors,
	 AHB/APBx pre-scalers and Flash settings ---------------------------------*/
	SetSysClock();
}


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
	/* Reset CFGR register */
	RCC->CFGR = 0x00000000;
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
	while ((RCC->CFGR & RCC_CFGR_SWS) != RCC_CFGR_SWS_PLL) {};
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
