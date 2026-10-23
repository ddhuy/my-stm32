#ifndef STM32F4_CLK_H
#define STM32F4_CLK_H

#include <stdbool.h>
#include <stdint.h>

/*
 * System clock setup for STM32F4: PLL -> SYSCLK, bus prescalers,
 * flash wait states and regulator scale. The board supplies the numbers.
 *
 *   f_VCO  = f_src / pllm * plln     (f_src / pllm must be 1..2 MHz)
 *   SYSCLK = f_VCO / pllp            (pllp = 2, 4, 6 or 8)
 *   USB    = f_VCO / pllq            (48 MHz if USB is used)
 */
typedef struct
{
    bool     use_hse;          /* PLL source: HSE crystal (true) or HSI (false) */
    uint32_t pllm;
    uint32_t plln;
    uint32_t pllp;
    uint32_t pllq;
    uint32_t cfgr_prescalers;  /* RCC_CFGR_HPRE_DIVx | RCC_CFGR_PPRE1_DIVx | RCC_CFGR_PPRE2_DIVx */
    uint32_t flash_latency;    /* FLASH_ACR_LATENCY_xWS for the new HCLK */
    uint32_t vos;              /* PWR_CR_VOS_SCALEx */
} stm32f4_clk_config_t;

/*
 * Expects to be raising the clock (flash wait states are set first).
 * Returns false if a clock or flag does not become ready (e.g. no HSE crystal).
 */
bool stm32f4_clk_init(const stm32f4_clk_config_t *cfg);

#endif // STM32F4_CLK_H
