#include <errno.h>
#include <systime.h>
#include <timer.h>
#include <dm/uclass.h>


/*
 * System time: the generic layer above the timer uclass, as console.c is
 * above the serial uclass. Callers get time without naming a device.
 */

/**
 * The timer device behind systime_elapsed_ms() and systime_mdelay(); NULL
 * until systime_init() has succeeded.
 */
static struct udevice *timer_dev;


/**
 * The first timer device is the system timer: generic code cannot know the
 * SoC's name for it. Asking for it probes it, which starts the counter. The
 * variable is set only on success, so a failed call leaves time stopped.
 */
int systime_init(void)
{
    int ret;
    struct udevice *dev;

    ret = uclass_get_device(UCLASS_TIMER, 0, &dev);
    if (ret)
        return ret;

    timer_dev = dev;
    
    return SUCCESS;
}


/**
 * Unsigned subtraction, so the result is right across the 2^32 ms wrap.
 * Without a timer, no time passes: 0, whatever 'base' is. Calling
 * timer_get_ms(NULL) would follow a NULL device, which does not fault here.
 */
u32 systime_elapsed_ms(
    u32 base
)
{
    if (!timer_dev)
        return 0;

    return timer_get_ms(timer_dev) - base;
}


/**
 * Busy wait. Built on systime_elapsed_ms(), so it is wrap-safe; it never
 * returns if there is no timer.
 */
void systime_mdelay(
    u32 ms
)
{
    u32 start = systime_elapsed_ms(0);

    while (systime_elapsed_ms(start) < ms)
        ;
}
