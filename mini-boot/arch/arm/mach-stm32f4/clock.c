#include <clk.h>
#include <clock.h>
#include <platdata/stm32f4_rcc.h>


/**
 * Turn on the clock of one peripheral
 */
void clock_enable(u32 clk_id)
{
    struct clk clk;

    if (clk_get_by_name(STM32F4_RCC_NAME, clk_id, &clk))
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

    if (clk_get_by_name(STM32F4_RCC_NAME, clk_id, &clk))
        return 0;

    return clk_get_rate(&clk);
}
