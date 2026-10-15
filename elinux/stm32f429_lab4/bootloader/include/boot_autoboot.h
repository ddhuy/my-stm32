#ifndef BOOT_AUTOBOOT_H
#define BOOT_AUTOBOOT_H

#include <stdbool.h>

/*
 * Wait for the autoboot timeout.
 *
 * Returns:
 *   true  - timeout expired; boot application
 *   false - user pressed a key; enter console
 */
bool boot_autoboot_wait(void);

#endif // BOOT_AUTOBOOT_H