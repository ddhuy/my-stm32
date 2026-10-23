#ifndef BOARD_INTERNAL_H
#define BOARD_INTERNAL_H

#include <stdint.h>

/* Helpers shared by the files in this board directory only. */

/* Route a pin to an alternate function (push-pull). */
void board_pin_af(uint32_t port, uint32_t pin, uint32_t af, uint32_t speed, uint32_t pull);

#endif
