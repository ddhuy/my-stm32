#include <errno.h>
#include <console.h>
#include <serial.h>
#include <dm/uclass.h>


/*
 * The serial device that carries the console; NULL until console_init()
 * has succeeded.
 */
static struct udevice *console_dev;


/**
 * The first serial device is the console. Asking for it probes it, which is
 * what starts the UART. The variable is set only on success, so a failed
 * call leaves the console off.
 */
int console_init(void)
{
    int ret;
    struct udevice *dev;
    
    ret = uclass_get_device(UCLASS_SERIAL, 0, &dev);
    if (ret)
        return ret;

    console_dev = dev;
    
    return SUCCESS;
}


/**
 * Terminals need CR+LF, so a carriage return goes out before each newline.
 */
void putc(
    char ch
)
{
    if (!console_dev)
        return;

    if (ch == '\n')
        serial_putc(console_dev, '\r');

    serial_putc(console_dev, ch);
}


/**
 * Goes through putc(), so the newline translation lives in one place.
 */
void puts(
    const char *s
)
{
    while ( *s )
        putc(*s++);
}


/**
 * Blocks until a character arrives.
 */
int getc(void)
{
    if (!console_dev)
        return -1;

    return serial_getc(console_dev);
}


/**
 * Polls the UART for a received character.
 */
int tstc(void)
{
    if (!console_dev)
        return 0;
    
    return serial_tstc(console_dev);
}
