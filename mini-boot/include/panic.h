#ifndef MINIBOOT_PANIC_H
#define MINIBOOT_PANIC_H


/**
 * 
 */
void panic(
    const char *msg
) __attribute__((noreturn));

#endif /* MINIBOOT_PANIC_H */