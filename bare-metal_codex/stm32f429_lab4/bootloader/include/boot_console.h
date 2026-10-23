#ifndef BOOT_CONSOLE_H
#define BOOT_CONSOLE_H

typedef enum {
    BOOT_CONSOLE_NONE = 0,
    BOOT_CONSOLE_BOOT,
} boot_console_action_t;

/*
 * Run the interactive console until an actionable
 * command is entered.
 */
boot_console_action_t boot_console_run(void);

#endif // BOOT_CONSOLE_H