MEMORY
{
    FLASH (rx)  : ORIGIN = 0x08000000, LENGTH = 2048K
    RAM   (rwx) : ORIGIN = 0x20000000, LENGTH = 192K
}

/*
 * How this board divides its RAM: bytes for malloc(), and the minimum kept
 * free for the stack.
 */
_heap_size  = 16K;
_stack_size = 8K;