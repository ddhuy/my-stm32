#ifndef STM32F429_RCC_REGS_H
#define STM32F429_RCC_REGS_H

#include "mmio.h"

#define RCC_BASE        0x40023800u

#define RCC_CR          MMIO32(RCC_BASE + 0x00u)
#define RCC_PLLCFGR     MMIO32(RCC_BASE + 0x04u)
#define RCC_CFGR        MMIO32(RCC_BASE + 0x08u)
#define RCC_AHB1ENR     MMIO32(RCC_BASE + 0x30u)
#define RCC_AHB3ENR     MMIO32(RCC_BASE + 0x38u)
#define RCC_APB1ENR     MMIO32(RCC_BASE + 0x40u)

/* RCC_CR */
#define RCC_CR_HSION       (1u << 0)
#define RCC_CR_HSIRDY      (1u << 1)
#define RCC_CR_PLLON       (1u << 24)
#define RCC_CR_PLLRDY      (1u << 25)

/* RCC_CFGR */
#define RCC_CFGR_SW_PLL    (2u << 0)
#define RCC_CFGR_SWS_MASK  (3u << 2)

/* Peripheral clock enables */
#define RCC_AHB1ENR_GPIOBEN (1u << 1)
#define RCC_AHB1ENR_GPIOCEN (1u << 2)
#define RCC_AHB1ENR_GPIODEN (1u << 3)
#define RCC_AHB1ENR_GPIOEEN (1u << 4)
#define RCC_AHB1ENR_GPIOFEN (1u << 5)
#define RCC_AHB1ENR_GPIOGEN (1u << 6)

#define RCC_AHB3ENR_FMCEN   (1u << 0)
#define RCC_APB1ENR_PWREN   (1u << 28)

#endif