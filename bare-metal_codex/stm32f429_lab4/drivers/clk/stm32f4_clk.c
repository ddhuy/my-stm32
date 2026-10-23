#include <stdbool.h>
#include <stdint.h>

#include "soc/stm32f429/rcc_regs.h"
#include "soc/stm32f429/pwr_regs.h"
#include "soc/stm32f429/flash_regs.h"

#include "drivers/clk/stm32f4_clk.h"


/* Iterations to wait for a clock or flash flag; startup takes far less. */
#define CLK_TIMEOUT 1000000u

/* Evaluates cond until true or until timeout iterations pass; yields the final cond. */
#define POLL_UNTIL(cond, timeout)                       \
    ({                                                  \
        uint32_t __n = (timeout);                       \
        while (!(cond) && --__n) {                      \
        }                                               \
        (bool)(cond);                                   \
    })

bool stm32f4_clk_init(const stm32f4_clk_config_t *cfg)
{
    RCC_APB1ENR |= RCC_APB1ENR_PWREN;
    (void)RCC_APB1ENR;

    PWR_CR = (PWR_CR & ~PWR_CR_VOS_MASK) | cfg->vos;

    /* Raise flash wait states before raising the clock. */
    FLASH_ACR = FLASH_ACR_DCEN | FLASH_ACR_ICEN | FLASH_ACR_PRFTEN | cfg->flash_latency;
    if (!POLL_UNTIL((FLASH_ACR & FLASH_ACR_LATENCY_MASK) == cfg->flash_latency, CLK_TIMEOUT))
        return false;

    /* The PLL cannot be turned off while it drives SYSCLK, so run from HSI first. */
    RCC_CFGR = (RCC_CFGR & ~RCC_CFGR_SW_MASK) | RCC_CFGR_SW_HSI;
    if (!POLL_UNTIL((RCC_CFGR & RCC_CFGR_SWS_MASK) == RCC_CFGR_SWS_HSI, CLK_TIMEOUT))
        return false;

    if (cfg->use_hse)
    {
        RCC_CR |= RCC_CR_HSEON;
        if (!POLL_UNTIL(RCC_CR & RCC_CR_HSERDY, CLK_TIMEOUT))
            return false;  /* no crystal, or it does not start */
    }

    RCC_CFGR = (RCC_CFGR & ~(RCC_CFGR_HPRE_MASK | RCC_CFGR_PPRE1_MASK | RCC_CFGR_PPRE2_MASK)) |
               cfg->cfgr_prescalers;

    RCC_CR &= ~RCC_CR_PLLON;
    if (!POLL_UNTIL(!(RCC_CR & RCC_CR_PLLRDY), CLK_TIMEOUT))
        return false;

    /* Read-modify-write keeps the reserved bits at their reset value. */
    RCC_PLLCFGR = (RCC_PLLCFGR & ~(RCC_PLLCFGR_PLLM_MASK | RCC_PLLCFGR_PLLN_MASK |
                                   RCC_PLLCFGR_PLLP_MASK | RCC_PLLCFGR_PLLQ_MASK |
                                   RCC_PLLCFGR_PLLSRC_HSE)) |
                  (cfg->use_hse ? RCC_PLLCFGR_PLLSRC_HSE : 0u) |
                  RCC_PLLCFGR_PLLM(cfg->pllm) | RCC_PLLCFGR_PLLN(cfg->plln) |
                  RCC_PLLCFGR_PLLP(cfg->pllp) | RCC_PLLCFGR_PLLQ(cfg->pllq);

    RCC_CR |= RCC_CR_PLLON;
    if (!POLL_UNTIL(RCC_CR & RCC_CR_PLLRDY, CLK_TIMEOUT))
        return false;

    RCC_CFGR = (RCC_CFGR & ~RCC_CFGR_SW_MASK) | RCC_CFGR_SW_PLL;
    if (!POLL_UNTIL((RCC_CFGR & RCC_CFGR_SWS_MASK) == RCC_CFGR_SWS_PLL, CLK_TIMEOUT))
        return false;

    return true;
}
