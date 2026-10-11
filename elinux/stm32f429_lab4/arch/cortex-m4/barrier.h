#ifndef CORTEX_M4_BARRIER_H
#define CORTEX_M4_BARRIER_H

/* Data synchronization barrier: all earlier memory accesses have completed. */
static inline void dsb(void)
{
    __asm volatile("dsb 0xF" ::: "memory");
}

/* Instruction synchronization barrier: refetch, so new settings take effect. */
static inline void isb(void)
{
    __asm volatile("isb 0xF" ::: "memory");
}

#endif
