#ifndef MINIBOOT_MALLOC_H
#define MINIBOOT_MALLOC_H

#include <types.h>

/*
 * Simple allocator: memory is handed out from the heap front to back and
 * is never reused.
 */


/**
 * Allocate 'size' bytes, aligned for any basic type.
 * Returns NULL when the heap is exhausted.
 */
void *malloc(
    size_t size
);


/**
 * Allocate an array of 'num' elements of 'size' bytes each, set to zero.
 * Returns NULL when the heap is exhausted or num * size overflows.
 */
void *calloc(
    size_t num,
    size_t size
);


/**
 * Does nothing; it exists so that callers can be written properly.
 */
void free(
    void *ptr
);

#endif /* MINIBOOT_MALLOC_H */