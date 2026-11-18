#ifndef MINIBOOT_TIME_H
#define MINIBOOT_TIME_H

#include <types.h>

/*
 * System time in milliseconds, from the first device of UCLASS_TIMER. A
 * time value is always u32 and wraps after about 49.7 days; compare
 * elapsed times (get_timer(start) >= timeout), never absolute ones.
 */

/**
 * Probe the system timer. Returns 0, or the error from the uclass.
 * Nothing below works before it has succeeded.
 */
int timer_init(void);


/**
 * Milliseconds since 'base'; get_timer(0) is the current time. Returns 0
 * before timer_init().
 */
u32 get_timer(
    u32 base
);


/**
 * Wait at least ms - 1 milliseconds (the start may fall anywhere in a
 * tick). Called before timer_init() it never returns.
 */
void mdelay(
    u32 ms
);

#endif /* MINIBOOT_TIME_H */
