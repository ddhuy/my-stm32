#ifndef STM32F429_EXTI_REGS_H
#define STM32F429_EXTI_REGS_H

#include "arch/cortex-m4/mmio.h"
#include "soc/stm32f429/periph_base.h"

/*
 * External interrupt/event controller (RM0090 section 12.3).
 * Bit n of every register is EXTI line n.
 *   Lines 0..15 : GPIO pin n (port chosen in SYSCFG_EXTICR)
 *   Line 16 PVD, 17 RTC alarm, 18 USB OTG FS wakeup, 19 Ethernet wakeup,
 *   20 USB OTG HS wakeup, 21 RTC tamper/timestamp, 22 RTC wakeup
 */

#define EXTI_IMR       MMIO32(EXTI_BASE + 0x00u)   /* interrupt mask (1 = enabled) */
#define EXTI_EMR       MMIO32(EXTI_BASE + 0x04u)   /* event mask                   */
#define EXTI_RTSR      MMIO32(EXTI_BASE + 0x08u)   /* rising-edge trigger          */
#define EXTI_FTSR      MMIO32(EXTI_BASE + 0x0Cu)   /* falling-edge trigger         */
#define EXTI_SWIER     MMIO32(EXTI_BASE + 0x10u)   /* software trigger             */
#define EXTI_PR        MMIO32(EXTI_BASE + 0x14u)   /* pending; write 1 to clear    */

#define EXTI_LINE(n)   (1u << (n))

#endif
