#ifndef MINIBOOT_INIT_H
#define MINIBOOT_INIT_H

/*
 * Hooks that every board implements in board/<board>/board.c. Generic code
 * calls them without knowing which board it runs on.
 */

/**
 * Bring up the board: LEDs and the console pins. Called after dm_init(),
 * because the board reaches its pins through GPIO devices, and before
 * console_init().
 *
 * Returns 0, or the error from looking up a GPIO port or configuring a pin.
 */
int board_init(void);


/**
 * Toggle the heartbeat LED. A board hook because which pin carries the LED
 * is a board fact.
 */
void board_led_toggle(void);


/**
 * Print the boot banner with the board's name and facts.
 */
void board_print_info(void);


/**
 * Signal a fatal error to the user (red LED). Called by panic().
 */
void board_panic(void);

#endif /* MINIBOOT_INIT_H */