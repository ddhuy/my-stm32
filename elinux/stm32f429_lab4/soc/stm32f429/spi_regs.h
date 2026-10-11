#ifndef STM32F429_SPI_REGS_H
#define STM32F429_SPI_REGS_H

#include "arch/cortex-m4/mmio.h"
#include "soc/stm32f429/periph_base.h"

/*
 * SPI / I2S (RM0090 section 28.5).
 * Macros take the instance base, e.g. SPI_DR(SPI5_BASE).
 * SPI1/4/5/6 are clocked from PCLK2, SPI2/3 from PCLK1.
 */

#define SPI_CR1(base)      MMIO32((base) + 0x00u)
#define SPI_CR2(base)      MMIO32((base) + 0x04u)
#define SPI_SR(base)       MMIO32((base) + 0x08u)
#define SPI_DR(base)       MMIO32((base) + 0x0Cu)
#define SPI_DR8(base)      MMIO8((base) + 0x0Cu)
#define SPI_CRCPR(base)    MMIO32((base) + 0x10u)
#define SPI_RXCRCR(base)   MMIO32((base) + 0x14u)
#define SPI_TXCRCR(base)   MMIO32((base) + 0x18u)
#define SPI_I2SCFGR(base)  MMIO32((base) + 0x1Cu)
#define SPI_I2SPR(base)    MMIO32((base) + 0x20u)

/* SPI_CR1 */
#define SPI_CR1_CPHA       (1u << 0)
#define SPI_CR1_CPOL       (1u << 1)
#define SPI_CR1_MSTR       (1u << 2)
#define SPI_CR1_BR_MASK    (7u << 3)
#define SPI_CR1_BR_DIV2    (0u << 3)
#define SPI_CR1_BR_DIV4    (1u << 3)
#define SPI_CR1_BR_DIV8    (2u << 3)
#define SPI_CR1_BR_DIV16   (3u << 3)
#define SPI_CR1_BR_DIV32   (4u << 3)
#define SPI_CR1_BR_DIV64   (5u << 3)
#define SPI_CR1_BR_DIV128  (6u << 3)
#define SPI_CR1_BR_DIV256  (7u << 3)
#define SPI_CR1_SPE        (1u << 6)
#define SPI_CR1_LSBFIRST   (1u << 7)
#define SPI_CR1_SSI        (1u << 8)
#define SPI_CR1_SSM        (1u << 9)    /* software NSS: also set SSI in master mode */
#define SPI_CR1_RXONLY     (1u << 10)
#define SPI_CR1_DFF        (1u << 11)   /* 1 = 16-bit frames */
#define SPI_CR1_CRCNEXT    (1u << 12)
#define SPI_CR1_CRCEN      (1u << 13)
#define SPI_CR1_BIDIOE     (1u << 14)
#define SPI_CR1_BIDIMODE   (1u << 15)

/* SPI_CR2 */
#define SPI_CR2_RXDMAEN    (1u << 0)
#define SPI_CR2_TXDMAEN    (1u << 1)
#define SPI_CR2_SSOE       (1u << 2)
#define SPI_CR2_FRF        (1u << 4)    /* 1 = TI mode */
#define SPI_CR2_ERRIE      (1u << 5)
#define SPI_CR2_RXNEIE     (1u << 6)
#define SPI_CR2_TXEIE      (1u << 7)

/* SPI_SR */
#define SPI_SR_RXNE        (1u << 0)
#define SPI_SR_TXE         (1u << 1)
#define SPI_SR_CHSIDE      (1u << 2)
#define SPI_SR_UDR         (1u << 3)
#define SPI_SR_CRCERR      (1u << 4)
#define SPI_SR_MODF        (1u << 5)
#define SPI_SR_OVR         (1u << 6)
#define SPI_SR_BSY         (1u << 7)
#define SPI_SR_FRE         (1u << 8)

/* SPI_I2SCFGR */
#define SPI_I2SCFGR_I2SMOD (1u << 11)
#define SPI_I2SCFGR_I2SE   (1u << 10)

#endif
