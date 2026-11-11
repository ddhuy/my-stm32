#ifndef MINIBOOT_GPIO_H
#define MINIBOOT_GPIO_H


#include <types.h>

/*
 * Generic GPIO API. A port is identified by its base address for now;
 * with the driver model this becomes a struct udevice pointer.
 */


/**
 * Make the pin a push-pull output driving 'value'
 */
void gpio_direction_output(
    uintptr_t base,
    unsigned int pin,
    int value
);

/**
 * Hand the pin to a peripheral; 'af' is the alternate function number
 */
void gpio_set_alternative(
    uintptr_t base,
    unsigned int pin,
    unsigned int af
);

/**
 * Set level on the pin: 0 or 1
 */
void gpio_set_value(
    uintptr_t base,
    unsigned int pin,
    int value
);

/**
 * Get level of the pin: 0 or 1
 */
int gpio_get_value(
    uintptr_t base,
    unsigned int pin
);


#endif /* MINIBOOT_GPIO_H */