#include <string.h>


/**
 * 
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
 * 
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
 * 
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
 * 
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
 * 
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