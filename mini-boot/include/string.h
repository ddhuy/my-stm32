#ifndef MINIBOOT_STRING_H
#define MINIBOOT_STRING_H

#include <types.h>


/**
 * Fill the first 'n' bytes of 'dst' with the byte 'c'. Returns 'dst'.
 */
void  *memset(
    void *dst, 
    int c, 
    size_t n
);


/**
 * Copy 'n' bytes from 'src' to 'dst'. The two areas must not overlap.
 * Returns 'dst'.
 */
void  *memcpy(
    void *dst, 
    const void *src, 
    size_t n
);


/**
 * Length of the string 's', not counting the terminating zero.
 */
size_t strlen(
    const char *s
);


/**
 * Compare two strings. Returns 0 if they are equal, a negative value if 'a'
 * sorts before 'b', a positive value if it sorts after.
 */
int strcmp(
    const char *a, 
    const char *b
);


/**
 * Like strcmp(), but compares at most 'n' characters.
 */
int strncmp(
    const char *a, 
    const char *b, 
    size_t n
);


#endif /* MINIBOOT_STRING_H */