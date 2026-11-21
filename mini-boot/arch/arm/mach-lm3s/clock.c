#include <clk.h>
#include <clock.h>
#include <platdata/lm3s_sysctl.h>


/*
 * Transitional shim: the old include/clock.h API, implemented on top of
 * the clock uclass, for the callers not yet converted (SysTick, the
 * banner). The register code lives only in drivers/clk/lm3s_sysctl.c.
 * Deleted when the last caller uses struct clk (item 3, Part C).
 *
 * Never call these from the lm3s_sysctl driver itself: the lookup would
 * probe it again and recurse.
 */

/**
 * Turn on the clock of one peripheral
 */
void clock_enable(u32 clk_id)
{
    struct clk clk;

    if (clk_get_by_name(LM3S_SYSCTL_NAME, clk_id, &clk))
        return;

    (void)clk_enable(&clk);
}


/**
 * Frequency in Hz of the clock feeding one peripheral; 0 if the provider
 * cannot be found or probed
 */
u32 clock_get_rate(u32 clk_id)
{
    struct clk clk;

    if (clk_get_by_name(LM3S_SYSCTL_NAME, clk_id, &clk))
        return 0;

    return clk_get_rate(&clk);
}
