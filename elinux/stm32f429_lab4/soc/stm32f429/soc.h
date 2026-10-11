#ifndef STM32F429_H
#define STM32F429_H

/* Convenience header: every STM32F429 register definition in one include. */

#include "periph_base.h"

/* Cortex-M4 core, as configured by ST */
#include "arch/cortex-m4/scb_regs.h"
#include "arch/cortex-m4/systick_regs.h"
#include "arch/cortex-m4/mpu_regs.h"
#include "arch/cortex-m4/dwt_regs.h"
#include "irq.h"            /* NVIC + STM32F429 IRQ numbers */

/* Clocks, power, flash */
#include "rcc_regs.h"
#include "pwr_regs.h"
#include "flash_regs.h"

/* I/O and interrupts */
#include "gpio_regs.h"
#include "syscfg_regs.h"
#include "exti_regs.h"

/* Peripherals */
#include "usart_regs.h"
#include "spi_regs.h"
#include "i2c_regs.h"
#include "tim_regs.h"
#include "dma_regs.h"
#include "fmc_regs.h"
#include "ltdc_regs.h"

#endif
