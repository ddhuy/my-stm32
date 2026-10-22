#include <assert.h>

#include "bootloader/include/boot_xmodem_io.h"

static unsigned ticks, reads, mode;
bool boot_time_tick(void) { ++ticks; return true; }
bool console_try_getc(char *out)
{
    ++reads;
    /* Delayed bursts restart the quiet interval; continuous input hits limit. */
    bool available = mode == 1 || (mode == 2 && (ticks == 3 || ticks == 6));
    *out = 'x';
    return available;
}
void console_putc(char c) { (void)c; assert(false); }

int main(void)
{
    assert(!boot_xmodem_io_drain(0, 10));
    assert(!boot_xmodem_io_drain(10, 9));
    assert(ticks == 0 && reads == 0);
    assert(boot_xmodem_io_drain(4, 20));
    assert(reads == 4);
    ticks = reads = 0; mode = 1;
    assert(!boot_xmodem_io_drain(4, 20));
    assert(reads == 20);
    ticks = reads = 0; mode = 2;
    assert(boot_xmodem_io_drain(4, 20));
    assert(reads == 10);
    ticks = reads = 0;
    assert(!boot_xmodem_io_drain(4, 9));
    assert(reads == 9);
    return 0;
}
