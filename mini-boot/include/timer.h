#ifndef MINIBOOT_TIMER_H
#define MINIBOOT_TIMER_H

#include <types.h>


/**
 * Only pointers to it are used here, so a declaration is enough.
 */
struct udevice;


/**
 * The operations every timer driver implements. A driver fills in one
 * constant table and points its 'ops' field at it.
 *
 * get_ms: milliseconds since the timer was probed; wraps after about
 *         49 days (2^32 ms). Never blocks, never fails.
 */
struct timer_ops
{
    u32 (*get_ms) (struct udevice *dev);
};


/**
 * Milliseconds since the timer was probed. 'dev' is a probed device of
 * UCLASS_TIMER. Returns 0 if the driver does not provide the operation.
 */
u32 timer_get_ms (
    struct udevice *dev
);


#endif /* MINIBOOT_TIMER_H */
