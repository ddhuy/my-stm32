#ifndef MINIBOOT_CLOCK_H
#define MINIBOOT_CLOCK_H

#include <types.h>

/**
 * Turn on the clock of one peripheral
 */
void clock_enable(u32 clk_id);

/**
 * Frequency in Hz of the clock feeding one peripheral
 */
u32 clock_get_rate(u32 clk_id);

#endif /* MINIBOOT_CLOCK_H */
