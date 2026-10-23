#ifndef CORTEX_M4_NVIC_REGS_H
#define CORTEX_M4_NVIC_REGS_H

#include <stdint.h>
#include "mmio.h"
#include "core_base.h"

/*
 * Nested vectored interrupt controller (PM0214 section 4.3).
 *
 * How many priority bits exist, and which IRQ number is which peripheral,
 * is decided by the chip vendor, so the SoC layer must define
 * NVIC_PRIO_BITS before including this file (see soc/irq.h).
 */

#ifndef NVIC_PRIO_BITS
#error "Include the SoC's irq.h instead: it defines NVIC_PRIO_BITS first"
#endif

#define NVIC_ISER(n)   MMIO32(NVIC_BASE + 0x000u + ((n) * 4u))   /* set-enable    */
#define NVIC_ICER(n)   MMIO32(NVIC_BASE + 0x080u + ((n) * 4u))   /* clear-enable  */
#define NVIC_ISPR(n)   MMIO32(NVIC_BASE + 0x100u + ((n) * 4u))   /* set-pending   */
#define NVIC_ICPR(n)   MMIO32(NVIC_BASE + 0x180u + ((n) * 4u))   /* clear-pending */
#define NVIC_IABR(n)   MMIO32(NVIC_BASE + 0x200u + ((n) * 4u))   /* active        */
#define NVIC_IPR(irq)  MMIO8(NVIC_BASE + 0x300u + (irq))          /* priority byte */
#define NVIC_STIR      MMIO32(SCS_BASE + 0xF00u)                  /* software trigger */

static inline void nvic_enable_irq(uint32_t irq)
{
    NVIC_ISER(irq >> 5) = 1u << (irq & 31u);
}

static inline void nvic_disable_irq(uint32_t irq)
{
    NVIC_ICER(irq >> 5) = 1u << (irq & 31u);
}

static inline void nvic_clear_pending(uint32_t irq)
{
    NVIC_ICPR(irq >> 5) = 1u << (irq & 31u);
}

static inline void nvic_set_priority(uint32_t irq, uint32_t prio)
{
    NVIC_IPR(irq) = (uint8_t)(prio << (8u - NVIC_PRIO_BITS));
}

#endif
