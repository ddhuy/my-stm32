#ifndef MINIBOOT_ARCH_IO_H
#define MINIBOOT_ARCH_IO_H


#include <types.h>


/**
 * Memory-mapped register access.
 *
 * The address is a runtime value (base + offset), so one driver can serve
 * any instance of a peripheral. Argument order follows Linux / U-Boot:
 * writel(value, address).
 */
static inline u32 readl(uintptr_t addr)
{
    return *(volatile u32 *)addr;
}

/**
 * 
 */
static inline void writel(u32 val, uintptr_t addr)
{
    *(volatile u32 *)addr = val;
}

/**
 * Read-modify-write helpers: clear the bits in 'clr', then set those in 'set'
 */
static inline void clrsetbits(uintptr_t addr, u32 clr, u32 set)
{
    writel((readl(addr) & ~clr) | set, addr);
}

/**
 * 
 */
static inline void setbits(uintptr_t addr, u32 set)
{
    clrsetbits(addr, 0, set);
}

/**
 * 
 */
static inline void clrbits(uintptr_t addr, u32 clr)
{
    clrsetbits(addr, clr, 0);
}


#endif /* MINIBOOT_ARCH_IO_H */