#ifndef MINIBOOT_PANIC_H
#define MINIBOOT_PANIC_H


/**
 * Print the message, signal the error on the board, and stop for good. Never
 * returns.
 */
void panic(
    const char *msg
) __attribute__((noreturn));

#endif /* MINIBOOT_PANIC_H */