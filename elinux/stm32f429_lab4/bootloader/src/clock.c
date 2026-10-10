#include <stdint.h>

#include "stm32f429/rcc_regs.h"
#include "stm32f429/pwr_regs.h"
#include "stm32f429/flash_regs.h"

#include "clock.h"


bool clock_init(void)
{
    RCC_APB1ENR |= RCC_APB1ENR_PWREN;
    (void)RCC_APB1ENR;

    PWR_CR = (PWR_CR & ~PWR_CR_VOS_MASK) | PWR_CR_VOS_SCALE1;
    FLASH_ACR = (1u << 10) | (1u << 9) | (1u << 8) | 5u;
    while ((FLASH_ACR & 7u) != 5u)
    {
    }

    RCC_CFGR = (RCC_CFGR & ~((15u << 4) | (7u << 10) | (7u << 13))) |
               (5u << 10) | (4u << 13);
    RCC_CR &= ~(1u << 24);
    while (RCC_CR & (1u << 25))
    {
    }

    RCC_PLLCFGR = (8u << 0) | (168u << 6) | (7u << 24); /* HSI PLL source */
    RCC_CR |= (1u << 24);
    while (!(RCC_CR & (1u << 25)))
    {
    }

    RCC_CFGR = (RCC_CFGR & ~3u) | 2u;
    while (((RCC_CFGR >> 2) & 3u) != 2u)
    {
    }

    return true;
}