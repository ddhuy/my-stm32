#include "arch/cortex-m4/systick_regs.h"

#include "boot_time.h"


bool boot_time_init(uint32_t core_clock_hz)
{
    uint32_t clocks_per_ms = core_clock_hz / 1000u;

    if ((clocks_per_ms % 1000u) != 0u ||
        clocks_per_ms == 0u ||
        clocks_per_ms > SYSTICK_LOAD_MAX + 1u)
    {
        return false;   /* core clock is not a multiple of 1 kHz */
    }

    /* Stop SysTick during configuration */
    SYSTICK_CTRL = 0u;
    /* 1 countdown period = 1ms */
    SYSTICK_LOAD = clocks_per_ms - 1u;
    /* Clear the current counter and COUNTFLAG */
    SYSTICK_VAL = 0u;
    /* Enable counter using core clock; no interrupt. */
    SYSTICK_CTRL = SYSTICK_CTRL_ENABLE | SYSTICK_CTRL_CLKSOURCE;

    return true;
}

void boot_time_delay_1ms(void)
{
    /* Reading CTRL clears COUNTFLAG
     * Start with a fresh flag, then wait for the next tick.
     */
    (void)SYSTICK_CTRL;
    /* Wait for COUNTFLAG to be set, indicating the counter has counted down. */
    while ((SYSTICK_CTRL & SYSTICK_CTRL_COUNTFLAG) == 0u)
    {
        /* Wait */
    }
}

bool boot_time_tick(void)
{
    return (SYSTICK_CTRL & SYSTICK_CTRL_COUNTFLAG) != 0u;
}