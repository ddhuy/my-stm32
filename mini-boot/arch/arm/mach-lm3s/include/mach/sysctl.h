#ifndef MINIBOOT_MACH_LM3S_SYSCTL_H
#define MINIBOOT_MACH_LM3S_SYSCTL_H

#include <types.h>


/**
 * Run the system clock from the PLL at sysclk_hz, with a crystal of xtal_hz.
 * Called by the board before any driver reads a clock rate.
 *
 * Returns 0; -EINVAL for an unsupported crystal or a rate that is not
 * 200 MHz / (4..16); -ETIMEDOUT if the PLL does not lock.
 */
int lm3s_clock_init(
    u32 xtal_hz,
    u32 sysclk_hz
);

#endif /* MINIBOOT_MACH_LM3S_SYSCTL_H */