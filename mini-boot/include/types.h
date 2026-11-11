#ifndef MINIBOOT_TYPES_H
#define MINIBOOT_TYPES_H


#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


typedef signed char   s8;
typedef signed short  s16;
typedef signed int    s32;

typedef unsigned char  u8;
typedef unsigned short u16;
typedef unsigned int   u32;


#define BIT(n)         (1UL << (n))
#define ARRAY_SIZE(a)  (sizeof(a) / sizeof((a)[0]))


#endif /* MINIBOOT_TYPES_H */