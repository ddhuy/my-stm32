#ifndef STM32F429_GPIO_REGS_H
#define STM32F429_GPIO_REGS_H

#include <stdint.h>
#include "mmio.h"

#define GPIOA_BASE  0x40020000u
#define GPIOB_BASE  0x40020400u
#define GPIOC_BASE  0x40020800u
#define GPIOD_BASE  0x40020C00u
#define GPIOE_BASE  0x40021000u
#define GPIOF_BASE  0x40021400u
#define GPIOG_BASE  0x40021800u

#define GPIO_MODER(base)    MMIO32((base) + 0x00u)
#define GPIO_OTYPER(base)   MMIO32((base) + 0x04u)
#define GPIO_OSPEEDR(base)  MMIO32((base) + 0x08u)
#define GPIO_PUPDR(base)    MMIO32((base) + 0x0Cu)
#define GPIO_IDR(base)      MMIO32((base) + 0x10u)
#define GPIO_ODR(base)      MMIO32((base) + 0x14u)
#define GPIO_BSRR(base)     MMIO32((base) + 0x18u)
#define GPIO_AFRL(base)     MMIO32((base) + 0x20u)
#define GPIO_AFRH(base)     MMIO32((base) + 0x24u)

#define GPIO_MODE_INPUT   0u
#define GPIO_MODE_OUTPUT  1u
#define GPIO_MODE_AF      2u
#define GPIO_MODE_ANALOG  3u

#define GPIO_AF_FMC       12u

#endif