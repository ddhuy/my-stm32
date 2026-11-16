#ifndef MINIBOOT_SERIAL_H
#define MINIBOOT_SERIAL_H

#include <types.h>


struct udevice;


/**
 * The operations every serial driver implements. A driver fills in one
 * constant table and points its 'ops' field at it.
 *
 * putc: send one character, waiting for room; returns 0 or -Exxx
 * getc: wait for a character and return it (0-255)
 * tstc: non-zero if a received character is waiting; never blocks
 */
struct serial_ops {
    int (*putc) (struct udevice *dev, char ch);
    int (*getc) (struct udevice *dev);
    int (*tstc) (struct udevice *dev);
};


/*
 * Generic serial API. 'dev' is a probed device of UCLASS_SERIAL, as returned
 * by uclass_get_device(); probing is what sets the UART up.
 */

/**
 * Send one character. Blocks until the transmitter accepts it.
 *
 * Returns 0, or -ENOSYS if the driver does not provide the operation.
 */
int serial_putc(
    struct udevice *dev,
    char ch
);


/**
 * Wait for a character and return it (0-255), or -ENOSYS if the driver does
 * not provide the operation.
 */
int serial_getc(
    struct udevice *dev
);


/**
 * Non-zero if a received character is waiting. Never blocks.
 */
int serial_tstc(
    struct udevice *dev
);

#endif /* MINIBOOT_SERIAL_H */