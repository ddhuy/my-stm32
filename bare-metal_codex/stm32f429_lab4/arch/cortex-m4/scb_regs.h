#ifndef CORTEX_M4_SCB_REGS_H
#define CORTEX_M4_SCB_REGS_H

#include "mmio.h"
#include "core_base.h"

/* Cortex-M4 System Control Block (PM0214 section 4.4) */

#define SCB_ACTLR      MMIO32(SCS_BASE + 0x008u)
#define SCB_CPUID      MMIO32(SCB_BASE + 0x00u)
#define SCB_ICSR       MMIO32(SCB_BASE + 0x04u)
#define SCB_VTOR       MMIO32(SCB_BASE + 0x08u)
#define SCB_AIRCR      MMIO32(SCB_BASE + 0x0Cu)
#define SCB_SCR        MMIO32(SCB_BASE + 0x10u)
#define SCB_CCR        MMIO32(SCB_BASE + 0x14u)
#define SCB_SHPR1      MMIO32(SCB_BASE + 0x18u)
#define SCB_SHPR2      MMIO32(SCB_BASE + 0x1Cu)
#define SCB_SHPR3      MMIO32(SCB_BASE + 0x20u)
#define SCB_SHCSR      MMIO32(SCB_BASE + 0x24u)
#define SCB_CFSR       MMIO32(SCB_BASE + 0x28u)
#define SCB_HFSR       MMIO32(SCB_BASE + 0x2Cu)
#define SCB_DFSR       MMIO32(SCB_BASE + 0x30u)
#define SCB_MMFAR      MMIO32(SCB_BASE + 0x34u)
#define SCB_BFAR       MMIO32(SCB_BASE + 0x38u)
#define SCB_AFSR       MMIO32(SCB_BASE + 0x3Cu)
#define SCB_CPACR      MMIO32(SCB_BASE + 0x88u)

/*
 * System handler priority, one byte per exception number (4..15):
 *   4 MemManage, 5 BusFault, 6 UsageFault, 11 SVCall, 14 PendSV, 15 SysTick.
 * STM32F4 implements the top 4 bits only: write (prio << 4).
 */
#define SCB_SHPR_BYTE(exc)   MMIO8(SCB_BASE + 0x18u + ((exc) - 4u))
#define EXC_MEMMANAGE        4u
#define EXC_BUSFAULT         5u
#define EXC_USAGEFAULT       6u
#define EXC_SVCALL           11u
#define EXC_PENDSV           14u
#define EXC_SYSTICK          15u

/* SCB_ICSR */
#define SCB_ICSR_VECTACTIVE_MASK  (0x1FFu << 0)
#define SCB_ICSR_RETTOBASE        (1u << 11)
#define SCB_ICSR_VECTPENDING_MASK (0x7Fu << 12)
#define SCB_ICSR_ISRPENDING       (1u << 22)
#define SCB_ICSR_PENDSTCLR        (1u << 25)
#define SCB_ICSR_PENDSTSET        (1u << 26)
#define SCB_ICSR_PENDSVCLR        (1u << 27)
#define SCB_ICSR_PENDSVSET        (1u << 28)
#define SCB_ICSR_NMIPENDSET       (1u << 31)

/* SCB_AIRCR: every write must include VECTKEY */
#define SCB_AIRCR_VECTKEY         (0x05FAu << 16)
#define SCB_AIRCR_VECTKEY_MASK    (0xFFFFu << 16)
#define SCB_AIRCR_SYSRESETREQ     (1u << 2)
#define SCB_AIRCR_PRIGROUP_MASK   (7u << 8)
#define SCB_AIRCR_PRIGROUP(g)     (((g) & 7u) << 8)
#define SCB_AIRCR_ENDIANNESS      (1u << 15)

/* SCB_SCR */
#define SCB_SCR_SLEEPONEXIT       (1u << 1)
#define SCB_SCR_SLEEPDEEP         (1u << 2)
#define SCB_SCR_SEVONPEND         (1u << 4)

/* SCB_CCR */
#define SCB_CCR_NONBASETHRDENA    (1u << 0)
#define SCB_CCR_USERSETMPEND      (1u << 1)
#define SCB_CCR_UNALIGN_TRP       (1u << 3)
#define SCB_CCR_DIV_0_TRP         (1u << 4)
#define SCB_CCR_BFHFNMIGN         (1u << 8)
#define SCB_CCR_STKALIGN          (1u << 9)

/* SCB_SHCSR */
#define SCB_SHCSR_MEMFAULTACT     (1u << 0)
#define SCB_SHCSR_BUSFAULTACT     (1u << 1)
#define SCB_SHCSR_USGFAULTACT     (1u << 3)
#define SCB_SHCSR_SVCALLACT       (1u << 7)
#define SCB_SHCSR_MONITORACT      (1u << 8)
#define SCB_SHCSR_PENDSVACT       (1u << 10)
#define SCB_SHCSR_SYSTICKACT      (1u << 11)
#define SCB_SHCSR_USGFAULTPENDED  (1u << 12)
#define SCB_SHCSR_MEMFAULTPENDED  (1u << 13)
#define SCB_SHCSR_BUSFAULTPENDED  (1u << 14)
#define SCB_SHCSR_SVCALLPENDED    (1u << 15)
#define SCB_SHCSR_MEMFAULTENA     (1u << 16)
#define SCB_SHCSR_BUSFAULTENA     (1u << 17)
#define SCB_SHCSR_USGFAULTENA     (1u << 18)

/* SCB_CFSR = UFSR[31:16] | BFSR[15:8] | MMFSR[7:0]; write 1 to clear */
#define SCB_CFSR_IACCVIOL         (1u << 0)
#define SCB_CFSR_DACCVIOL         (1u << 1)
#define SCB_CFSR_MUNSTKERR        (1u << 3)
#define SCB_CFSR_MSTKERR          (1u << 4)
#define SCB_CFSR_MLSPERR          (1u << 5)
#define SCB_CFSR_MMARVALID        (1u << 7)
#define SCB_CFSR_IBUSERR          (1u << 8)
#define SCB_CFSR_PRECISERR        (1u << 9)
#define SCB_CFSR_IMPRECISERR      (1u << 10)
#define SCB_CFSR_UNSTKERR         (1u << 11)
#define SCB_CFSR_STKERR           (1u << 12)
#define SCB_CFSR_LSPERR           (1u << 13)
#define SCB_CFSR_BFARVALID        (1u << 15)
#define SCB_CFSR_UNDEFINSTR       (1u << 16)
#define SCB_CFSR_INVSTATE         (1u << 17)
#define SCB_CFSR_INVPC            (1u << 18)
#define SCB_CFSR_NOCP             (1u << 19)
#define SCB_CFSR_UNALIGNED        (1u << 24)
#define SCB_CFSR_DIVBYZERO        (1u << 25)

/* SCB_HFSR */
#define SCB_HFSR_VECTTBL          (1u << 1)
#define SCB_HFSR_FORCED           (1u << 30)
#define SCB_HFSR_DEBUGEVT         (1u << 31)

/* SCB_CPACR: enable the FPU (CP10 + CP11 full access), then DSB; ISB */
#define SCB_CPACR_CP10_FULL       (3u << 20)
#define SCB_CPACR_CP11_FULL       (3u << 22)
#define SCB_CPACR_FPU_ENABLE      (SCB_CPACR_CP10_FULL | SCB_CPACR_CP11_FULL)

/* FPU context control (PM0214 section 4.6) */
#define FPU_FPCCR      MMIO32(FPU_BASE + 0x04u)
#define FPU_FPCAR      MMIO32(FPU_BASE + 0x08u)
#define FPU_FPDSCR     MMIO32(FPU_BASE + 0x0Cu)

#define FPU_FPCCR_LSPACT          (1u << 0)
#define FPU_FPCCR_LSPEN           (1u << 30)   /* lazy stacking        */
#define FPU_FPCCR_ASPEN           (1u << 31)   /* automatic FP stacking */

#endif
