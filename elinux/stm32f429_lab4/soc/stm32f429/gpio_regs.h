#ifndef STM32F429_GPIO_REGS_H
#define STM32F429_GPIO_REGS_H

#include <stdint.h>
#include "arch/cortex-m4/mmio.h"
#include "soc/stm32f429/periph_base.h"

/*
 * General-purpose I/O (RM0090 section 8.4).
 * All ports share one layout, so the macros take a port base address:
 *
 *     GPIO_MODER(GPIOG_BASE) |= GPIO_MODER_VAL(13, GPIO_MODE_OUTPUT);
 *     GPIO_BSRR(GPIOG_BASE)   = GPIO_BSRR_SET(13);
 */

/* Port index 0 = A ... 10 = K, matching RCC_AHB1ENR_GPIOEN(n) and SYSCFG_EXTICR */
#define GPIO_PORT_BASE(n)   (GPIOA_BASE + ((uint32_t)(n) * 0x400u))
#define GPIO_PORT_A   0u
#define GPIO_PORT_B   1u
#define GPIO_PORT_C   2u
#define GPIO_PORT_D   3u
#define GPIO_PORT_E   4u
#define GPIO_PORT_F   5u
#define GPIO_PORT_G   6u
#define GPIO_PORT_H   7u
#define GPIO_PORT_I   8u
#define GPIO_PORT_J   9u
#define GPIO_PORT_K   10u

#define GPIO_MODER(base)    MMIO32((base) + 0x00u)
#define GPIO_OTYPER(base)   MMIO32((base) + 0x04u)
#define GPIO_OSPEEDR(base)  MMIO32((base) + 0x08u)
#define GPIO_PUPDR(base)    MMIO32((base) + 0x0Cu)
#define GPIO_IDR(base)      MMIO32((base) + 0x10u)
#define GPIO_ODR(base)      MMIO32((base) + 0x14u)
#define GPIO_BSRR(base)     MMIO32((base) + 0x18u)
#define GPIO_LCKR(base)     MMIO32((base) + 0x1Cu)
#define GPIO_AFRL(base)     MMIO32((base) + 0x20u)   /* pins 0..7  */
#define GPIO_AFRH(base)     MMIO32((base) + 0x24u)   /* pins 8..15 */
/* AFRL or AFRH, whichever holds this pin */
#define GPIO_AFR(base, pin) MMIO32((base) + 0x20u + (((pin) >> 3) << 2))

/* MODER / OSPEEDR / PUPDR: 2 bits per pin */
#define GPIO_MODE_INPUT   0u
#define GPIO_MODE_OUTPUT  1u
#define GPIO_MODE_AF      2u
#define GPIO_MODE_ANALOG  3u

#define GPIO_SPEED_LOW       0u
#define GPIO_SPEED_MEDIUM    1u
#define GPIO_SPEED_HIGH      2u
#define GPIO_SPEED_VERY_HIGH 3u

#define GPIO_PULL_NONE    0u
#define GPIO_PULL_UP      1u
#define GPIO_PULL_DOWN    2u

#define GPIO_2BIT_MASK(pin)      (3u << ((pin) * 2u))
#define GPIO_2BIT_VAL(pin, v)    (((v) & 3u) << ((pin) * 2u))
#define GPIO_MODER_MASK(pin)     GPIO_2BIT_MASK(pin)
#define GPIO_MODER_VAL(pin, m)   GPIO_2BIT_VAL(pin, m)

/* OTYPER: 1 bit per pin */
#define GPIO_OTYPE_PUSHPULL   0u
#define GPIO_OTYPE_OPENDRAIN  1u

/* AFR: 4 bits per pin, within AFRL (pins 0..7) or AFRH (pins 8..15) */
#define GPIO_AFR_SHIFT(pin)      (((pin) & 7u) * 4u)
#define GPIO_AFR_MASK(pin)       (0xFu << GPIO_AFR_SHIFT(pin))
#define GPIO_AFR_VAL(pin, af)    (((af) & 0xFu) << GPIO_AFR_SHIFT(pin))

/* BSRR: atomic set (low half) / reset (high half) */
#define GPIO_BSRR_SET(pin)    (1u << (pin))
#define GPIO_BSRR_RESET(pin)  (1u << ((pin) + 16u))

#define GPIO_LCKR_LCKK        (1u << 16)

/* Alternate function numbers (datasheet DS9405, table 12) */
#define GPIO_AF_SYSTEM    0u   /* MCO, SWD, TRACE, RTC_REFIN */
#define GPIO_AF_TIM1_2    1u
#define GPIO_AF_TIM3_5    2u
#define GPIO_AF_TIM8_11   3u
#define GPIO_AF_I2C       4u   /* I2C1..3                    */
#define GPIO_AF_SPI1_6    5u   /* SPI1, SPI2, SPI4, SPI5, SPI6 (+ some SPI3) */
#define GPIO_AF_SPI2_3    6u   /* SPI2/3 alt pins, SAI1      */
#define GPIO_AF_USART1_3  7u
#define GPIO_AF_UART4_8   8u   /* UART4/5, USART6, UART7/8   */
#define GPIO_AF_CAN_TIM12 9u   /* CAN1/2, TIM12..14, some LTDC pins */
#define GPIO_AF_OTG_FS    10u
#define GPIO_AF_ETH       11u
#define GPIO_AF_FMC       12u  /* also SDIO, OTG HS FS-mode */
#define GPIO_AF_DCMI      13u
#define GPIO_AF_LTDC      14u
#define GPIO_AF_EVENTOUT  15u

#endif
