#include <string.h>


/**
 * The compiler may emit calls to this on its own (to zero a struct, for
 * example), so it must keep the standard name and signature.
 */
void  *memset(
    void *dst, 
    int c, 
    size_t n
)
{
    u8 *d = (u8 *) dst;

    while (n--)
        *d++ = (u8) c;

    return dst;
}


/**
 * As with memset(), the compiler may call this itself, for struct copies.
 */
void  *memcpy(
    void *dst, 
    const void *src, 
    size_t n
)
{
    u8 *d = (u8 *) dst;
    const u8 *s = (const u8 *) src;

    while (n--)
        *d++ = *s++;

    return dst;
}


/**
 * Count characters up to the terminating zero.
 */
size_t strlen(
    const char *s
)
{
    size_t len = 0;

    while (*s++)
        len++;

    return len;
}


/**
 * Stops at the first difference or at the end of 'a'. Bytes are compared as
 * unsigned, so the result does not depend on whether char is signed.
 */
int strcmp(
    const char *a, 
    const char *b
)
{
    while ( *a && (*a == *b) )
    {
        a++;
        b++;
    }

    return ((u8) *a) - ((u8) *b);
}


/**
 * As strcmp(), stopping after 'n' characters. Reaching the limit means the
 * strings are equal as far as asked.
 */
int strncmp(
    const char *a, 
    const char *b, 
    size_t n
)
{
    while ( n && *a && (*a == *b) )
    {
        n--;
        a++;
        b++;
    }

    return n ? ((u8) *a) - ((u8) *b) : 0;
}