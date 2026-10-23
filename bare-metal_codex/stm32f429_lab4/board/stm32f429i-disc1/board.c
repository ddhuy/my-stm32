#include <stdbool.h>
#include <stdint.h>

#include "soc/stm32f429/rcc_regs.h"
#include "soc/stm32f429/pwr_regs.h"
#include "soc/stm32f429/flash_regs.h"
#include "soc/stm32f429/gpio_regs.h"
#include "drivers/clk/stm32f4_clk.h"
#include "drivers/serial/stm32_usart.h"

#include "board/stm32f429i-disc1/board.h"
#include "board/stm32f429i-disc1/board_internal.h"


/*
 * PLL from HSI (16 MHz):
 *   16 MHz / M  =   2 MHz  VCO input (1..2 MHz)
 *    2 MHz * N  = 336 MHz  VCO output (100..432 MHz)
 *  336 MHz / P  = 168 MHz  SYSCLK
 *  336 MHz / Q  =  48 MHz  USB / SDIO
 */
#define PLL_M   8u
#define PLL_N   168u
#define PLL_P   2u
#define PLL_Q   7u

_Static_assert(HSI_HZ / PLL_M * PLL_N / PLL_P == CLOCK_SYSCLK_HZ,
               "PLL settings do not produce CLOCK_SYSCLK_HZ");
_Static_assert(CLOCK_PCLK1_HZ <= 45000000u && CLOCK_PCLK2_HZ <= 90000000u,
               "APB clocks above the STM32F429 limits");

static const stm32f4_clk_config_t clk_config = {
    .use_hse         = false,
    .pllm            = PLL_M,
    .plln            = PLL_N,
    .pllp            = PLL_P,
    .pllq            = PLL_Q,
    /* Divisors are stated once, in board.h, next to the CLOCK_*_HZ they produce */
    .cfgr_prescalers = RCC_CFGR_HPRE(BOARD_AHB_DIV) | RCC_CFGR_PPRE1(BOARD_APB1_DIV) |
                       RCC_CFGR_PPRE2(BOARD_APB2_DIV),
    .flash_latency   = FLASH_ACR_LATENCY_5WS,   /* 150..180 MHz at 3.3 V */
    .vos             = PWR_CR_VOS_SCALE1,       /* needed above 144 MHz  */
};


void board_pin_af(uint32_t port, uint32_t pin, uint32_t af, uint32_t speed, uint32_t pull)
{
    GPIO_MODER(port) = (GPIO_MODER(port) & ~GPIO_MODER_MASK(pin)) |
                       GPIO_MODER_VAL(pin, GPIO_MODE_AF);
    GPIO_OTYPER(port) &= ~(1u << pin);  /* push-pull */
    GPIO_OSPEEDR(port) = (GPIO_OSPEEDR(port) & ~GPIO_2BIT_MASK(pin)) |
                         GPIO_2BIT_VAL(pin, speed);
    GPIO_PUPDR(port) = (GPIO_PUPDR(port) & ~GPIO_2BIT_MASK(pin)) |
                       GPIO_2BIT_VAL(pin, pull);
    GPIO_AFR(port, pin) = (GPIO_AFR(port, pin) & ~GPIO_AFR_MASK(pin)) |
                          GPIO_AFR_VAL(pin, af);
}

bool board_clock_init(void)
{
    return stm32f4_clk_init(&clk_config);
}

void board_console_init(uint32_t pclk_hz, uint32_t baud)
{
    RCC_AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC_APB2ENR |= RCC_APB2ENR_USART1EN;
    (void)RCC_APB2ENR;

    board_pin_af(VCP_TX_PORT, VCP_TX_PIN, GPIO_AF_USART1_3, GPIO_SPEED_HIGH, GPIO_PULL_NONE);
    board_pin_af(VCP_RX_PORT, VCP_RX_PIN, GPIO_AF_USART1_3, GPIO_SPEED_HIGH,
                 GPIO_PULL_UP);  /* line idles high */

    stm32_usart_init(CONSOLE_USART, pclk_hz, baud);
}
