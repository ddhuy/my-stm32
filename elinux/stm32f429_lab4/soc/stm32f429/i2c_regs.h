#ifndef STM32F429_I2C_REGS_H
#define STM32F429_I2C_REGS_H

#include "arch/cortex-m4/mmio.h"
#include "soc/stm32f429/periph_base.h"

/*
 * I2C (RM0090 section 27.6). All instances run from PCLK1.
 * Macros take the instance base, e.g. I2C_DR(I2C3_BASE).
 */

#define I2C_CR1(base)    MMIO32((base) + 0x00u)
#define I2C_CR2(base)    MMIO32((base) + 0x04u)
#define I2C_OAR1(base)   MMIO32((base) + 0x08u)
#define I2C_OAR2(base)   MMIO32((base) + 0x0Cu)
#define I2C_DR(base)     MMIO32((base) + 0x10u)
#define I2C_SR1(base)    MMIO32((base) + 0x14u)
#define I2C_SR2(base)    MMIO32((base) + 0x18u)
#define I2C_CCR(base)    MMIO32((base) + 0x1Cu)
#define I2C_TRISE(base)  MMIO32((base) + 0x20u)
#define I2C_FLTR(base)   MMIO32((base) + 0x24u)

/* I2C_CR1 */
#define I2C_CR1_PE         (1u << 0)
#define I2C_CR1_SMBUS      (1u << 1)
#define I2C_CR1_SMBTYPE    (1u << 3)
#define I2C_CR1_ENARP      (1u << 4)
#define I2C_CR1_ENPEC      (1u << 5)
#define I2C_CR1_ENGC       (1u << 6)
#define I2C_CR1_NOSTRETCH  (1u << 7)
#define I2C_CR1_START      (1u << 8)
#define I2C_CR1_STOP       (1u << 9)
#define I2C_CR1_ACK        (1u << 10)
#define I2C_CR1_POS        (1u << 11)
#define I2C_CR1_PEC        (1u << 12)
#define I2C_CR1_ALERT      (1u << 13)
#define I2C_CR1_SWRST      (1u << 15)

/* I2C_CR2 */
#define I2C_CR2_FREQ_MASK  (0x3Fu << 0)
#define I2C_CR2_FREQ(mhz)  (((mhz) & 0x3Fu) << 0)   /* PCLK1 in MHz, 2..45 */
#define I2C_CR2_ITERREN    (1u << 8)
#define I2C_CR2_ITEVTEN    (1u << 9)
#define I2C_CR2_ITBUFEN    (1u << 10)
#define I2C_CR2_DMAEN      (1u << 11)
#define I2C_CR2_LAST       (1u << 12)

/* I2C_OAR1: bit 14 must be kept at 1 by software */
#define I2C_OAR1_ADD7(a)   (((a) & 0x7Fu) << 1)
#define I2C_OAR1_BIT14     (1u << 14)
#define I2C_OAR1_ADDMODE   (1u << 15)

/* I2C_SR1 */
#define I2C_SR1_SB         (1u << 0)    /* start sent                 */
#define I2C_SR1_ADDR       (1u << 1)    /* address ACKed; clear by reading SR1 then SR2 */
#define I2C_SR1_BTF        (1u << 2)
#define I2C_SR1_ADD10      (1u << 3)
#define I2C_SR1_STOPF      (1u << 4)
#define I2C_SR1_RXNE       (1u << 6)
#define I2C_SR1_TXE        (1u << 7)
#define I2C_SR1_BERR       (1u << 8)
#define I2C_SR1_ARLO       (1u << 9)
#define I2C_SR1_AF         (1u << 10)   /* NACK received              */
#define I2C_SR1_OVR        (1u << 11)
#define I2C_SR1_PECERR     (1u << 12)
#define I2C_SR1_TIMEOUT    (1u << 14)
#define I2C_SR1_SMBALERT   (1u << 15)

/* I2C_SR2 */
#define I2C_SR2_MSL        (1u << 0)
#define I2C_SR2_BUSY       (1u << 1)
#define I2C_SR2_TRA        (1u << 2)
#define I2C_SR2_GENCALL    (1u << 4)
#define I2C_SR2_SMBDEFAULT (1u << 5)
#define I2C_SR2_SMBHOST    (1u << 6)
#define I2C_SR2_DUALF      (1u << 7)

/*
 * I2C_CCR
 *   Standard mode (100 kHz): CCR = PCLK1 / (2 * 100 kHz), TRISE = MHz + 1
 *   Fast mode (400 kHz, DUTY=0): CCR = PCLK1 / (3 * 400 kHz),
 *                                TRISE = (MHz * 300 / 1000) + 1
 */
#define I2C_CCR_CCR_MASK   (0xFFFu << 0)
#define I2C_CCR_DUTY       (1u << 14)
#define I2C_CCR_FS         (1u << 15)

/* I2C_FLTR */
#define I2C_FLTR_DNF_MASK  (0xFu << 0)
#define I2C_FLTR_ANOFF     (1u << 4)

#endif
