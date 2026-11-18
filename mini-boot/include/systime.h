#ifndef MINIBOOT_SYSTIME_H
#define MINIBOOT_SYSTIME_H

#include <types.h>

/*
 * System time in milliseconds, from the first device of UCLASS_TIMER. A
 * time value is always u32 and wraps after about 49.7 days; compare
 * elapsed times (systime_elapsed_ms(start) >= timeout), never absolute ones.
 */

/**
 * Probe the system timer. Returns 0, or the error from the uclass.
 * Nothing below works before it has succeeded.
 */
int systime_init(void);


/**
 * Milliseconds since 'base'; systime_elapsed_ms(0) is the current time.
 * Returns 0 before systime_init().
 */
u32 systime_elapsed_ms(
    u32 base
);


/**
 * Wait at least ms - 1 milliseconds (the start may fall anywhere in a
 * tick). Called before systime_init() it never returns.
 */
void systime_mdelay(
    u32 ms
);

#endif /* MINIBOOT_SYSTIME_H */
