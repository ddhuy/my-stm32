#ifndef MINIBOOT_INIT_H
#define MINIBOOT_INIT_H

/*
 * Hooks that every board implements in board/<board>/board.c. Generic code
 * calls them without knowing which board it runs on.
 */

/**
 * Bring up the board: LEDs and the console. Called first thing in main().
 */
void board_init(void);


/**
 * Toggle the heartbeat LED. Temporary: goes away once main() can look up a
 * GPIO device.
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