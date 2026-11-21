MEMORY
{
    FLASH (rx) : ORIGIN = 0x00000000, LENGTH = 256K
    RAM  (rwx) : ORIGIN = 0x20000000, LENGTH = 64K
}

/*
 * How this board divides its RAM: bytes for malloc(), and the minimum kept
 * free for the stack.
 */
_heap_size  = 16K;
_stack_size = 8K;
