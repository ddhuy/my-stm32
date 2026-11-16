#ifndef MINIBOOT_CONSOLE_H
#define MINIBOOT_CONSOLE_H


/**
 * Start the console on the first serial device. Call it after dm_init().
 *
 * Returns 0, or -ENODEV if the board has no serial device, or the error
 * from probing it.
 */
int console_init(void);


/**
 * Write one character. A newline is sent as CR+LF. Output is dropped until
 * console_init() has been called.
 */
void putc(
    char ch
);


/**
 * Write a string. Unlike the C library puts(), no newline is appended.
 */
void puts(
    const char *s
);


/**
 * Wait for a character and return it, or -1 if there is no console yet.
 */
int getc(void);


/**
 * Non-zero if a received character is waiting. Never blocks.
 */
int tstc(void);


#endif /* MINIBOOT_CONSOLE_H */