#include <init.h>
#include <panic.h>
#include <printf.h>


/**
 * 
 */
void panic(const char *msg)
{
    printf("PANIC: %s\n", msg);
    board_panic();

    for(;;)
        ;
}
