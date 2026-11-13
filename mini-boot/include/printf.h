#ifndef MINIBOOT_PRINTF_H
#define MINIBOOT_PRINTF_H


/**
 * Formatted output to the console.
 *
 * Supported: %c %s %d %u %x %p %%, with an optional width and zero padding
 * (%8x, %08x). An 'l' length modifier is accepted and ignored, since int and
 * long are both 32 bits here.
 *
 * Returns the number of characters written.
 */
int printf(
    const char *fmt,
    ...) __attribute__((format(printf, 1, 2)));

#endif /* MINIBOOT_PRINTF_H */