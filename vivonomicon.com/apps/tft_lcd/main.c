/*
 * main.c
 *
 *  Created on: Jan 4, 2025
 *      Author: ddh
 */
#include "main.h"


/****
 * @brief  Simple delay method, with instructions not to optimize.
 *         It doesn't accurately delay a precise # of cycles,
 *         it's just a rough scale.
 *
 * @param  None
 *
 * @retval None
 */
__attribute__((optimize("O0")))
void DelayCycles(uint32_t cyc)
{
	for (uint32_t i = 0; i < cyc; ++i)
		asm("NOP");
}


/****
 * @brief  Setup the micro-controller system
 *         Initialize the Embedded Flash Interface, the PLL and update the
 *         SystemFrequency variable.
 *
 * @param  None
 *
 * @retval None
 */
void SystemInit(void)
{
#ifdef VVC_F4
    RCC->AHB1ENR   |= RCC_AHB1ENR_GPIOAEN;
    RCC->AHB1ENR   |= RCC_AHB1ENR_GPIOBEN;
#elif  VVC_F0
    RCC->AHBENR   |= RCC_AHBENR_GPIOAEN;
    RCC->AHBENR   |= RCC_AHBENR_GPIOBEN;
#elif  VVC_L0
    RCC->IOPENR   |= RCC_IOPENR_IOPAEN;
    RCC->IOPENR   |= RCC_IOPENR_IOPBEN;
#endif
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN;
}


/****
 * @brief  Main program.
 *
 * @param  None
 *
 * @retval None
 */
int main(void)
{
	return 0;
}
