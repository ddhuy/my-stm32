#ifndef CORTEX_M4_CORE_BASE_H
#define CORTEX_M4_CORE_BASE_H

/*
 * Cortex-M4 private peripheral bus (ARMv7-M architecture, PM0214).
 * These addresses are the same on every Cortex-M4, whoever made the chip.
 */
#define DWT_BASE            0xE0001000u
#define SCS_BASE            0xE000E000u   /* System Control Space */
#define SYSTICK_BASE        0xE000E010u
#define NVIC_BASE           0xE000E100u
#define SCB_BASE            0xE000ED00u
#define MPU_BASE            0xE000ED90u
#define FPU_BASE            0xE000EF30u

#endif
