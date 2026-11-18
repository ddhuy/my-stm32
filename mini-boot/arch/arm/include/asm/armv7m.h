#ifndef MINIBOOT_ARCH_ARMV7M_H
#define MINIBOOT_ARCH_ARMV7M_H


/* System timer: same address on every ARMv7-M core; layout in its driver */
#define SYSTICK_BASE  (0xE000E010)

#define SCB_BASE  (0xE000ED00)
#define SCB_CFSR  (0x28)
#define SCB_HFSR  (0x2C)

#endif /* MINIBOOT_ARCH_ARMV7M_H */
