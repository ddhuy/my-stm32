#ifndef BOOT_TIME_H
#define BOOT_TIME_H

#include <stdbool.h>
#include <stdint.h>

/* Initialize SysTick as a polling timebase */
bool boot_time_init(uint32_t core_clock_hz);

/* Wait approximately one millisecond */
void boot_time_delay_1ms(void);

/* Check if a millisecond has passed */
bool boot_time_tick(void);

#endif // BOOT_TIME_H