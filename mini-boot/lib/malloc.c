#include <malloc.h>
#include <string.h>


/* Every block is aligned for the largest basic type (u64 / double) */
#define MALLOC_ALIGN 8

/*
 * Heap limits from link.ld; only the addresses of these symbols matter,
 * they hold no data
 */
extern u8 _sheap[];
extern u8 _eheap[];

/* Next free byte; everything below it is in use */
static u8 *heap_next = _sheap;


/**
 * Bump allocation: return the current position and move it forward.
 */
void *malloc(
    size_t size
)
{
    u8 *ptr = heap_next;
    size_t avail = (size_t) (_eheap - heap_next);

    /* Reject a huge size before rounding: the addition below would wrap */
    if ( size > avail )
        return NULL;

    /* Round up the size to the nearest multiple of MALLOC_ALIGN */
    size = (size + MALLOC_ALIGN - 1) & ~(MALLOC_ALIGN - 1);
    /* Not enough room left in the heap */
    if ( size > avail )
        return NULL;

    heap_next = heap_next + size;

    return (void *) ptr;
}


/**
 * malloc() plus zeroing. The check rejects a num * size that does not fit
 * in size_t.
 */
void *calloc(
    size_t num,
    size_t size
)
{
    void *ptr;

    if (size && num > SIZE_MAX / size)
        return NULL;
    
    ptr = malloc(num * size);
    if (ptr)
        memset(ptr, 0, num * size);

    return ptr;
}


/**
 * Memory is never reused, so there is nothing to do.
 */
void free(
    void *ptr
)
{
    (void) ptr;
}
