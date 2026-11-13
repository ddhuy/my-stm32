#include <init.h>
#include <panic.h>
#include <printf.h>


/**
 * Last resort: report, light the error LED through the board hook, and halt.
 */
void panic(const char *msg)
{
    printf("PANIC: %s\n", msg);
    board_panic();

    for(;;)
        ;
}
