#ifndef MINIBOOT_PRINTF_H
#define MINIBOOT_PRINTF_H


/**
 * 
 */
int printf(
    const char *fmt,
    ...) __attribute__((format(printf, 1, 2)));

#endif /* MINIBOOT_PRINTF_H */