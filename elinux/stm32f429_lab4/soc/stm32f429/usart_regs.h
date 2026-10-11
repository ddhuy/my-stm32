#ifndef STM32F429_USART_REGS_H
#define STM32F429_USART_REGS_H

#include "arch/cortex-m4/mmio.h"
#include "soc/stm32f429/periph_base.h"

/*
 * USART / UART (RM0090 section 30.6).
 * Macros take the instance base, e.g. USART_DR(USART1_BASE).
 * USART1/6 are clocked from PCLK2 (APB2), the others from PCLK1 (APB1).
 */

#define USART_SR(base)    MMIO32((base) + 0x00u)
#define USART_DR(base)    MMIO32((base) + 0x04u)
#define USART_BRR(base)   MMIO32((base) + 0x08u)
#define USART_CR1(base)   MMIO32((base) + 0x0Cu)
#define USART_CR2(base)   MMIO32((base) + 0x10u)
#define USART_CR3(base)   MMIO32((base) + 0x14u)
#define USART_GTPR(base)  MMIO32((base) + 0x18u)

/* USART_SR */
#define USART_SR_PE       (1u << 0)
#define USART_SR_FE       (1u << 1)
#define USART_SR_NF       (1u << 2)
#define USART_SR_ORE      (1u << 3)
#define USART_SR_IDLE     (1u << 4)
#define USART_SR_RXNE     (1u << 5)   /* byte received, read DR           */
#define USART_SR_TC       (1u << 6)   /* transmission complete (shift reg) */
#define USART_SR_TXE      (1u << 7)   /* DR empty, may write next byte    */
#define USART_SR_LBD      (1u << 8)
#define USART_SR_CTS      (1u << 9)

/* USART_CR1 */
#define USART_CR1_SBK     (1u << 0)
#define USART_CR1_RWU     (1u << 1)
#define USART_CR1_RE      (1u << 2)
#define USART_CR1_TE      (1u << 3)
#define USART_CR1_IDLEIE  (1u << 4)
#define USART_CR1_RXNEIE  (1u << 5)
#define USART_CR1_TCIE    (1u << 6)
#define USART_CR1_TXEIE   (1u << 7)
#define USART_CR1_PEIE    (1u << 8)
#define USART_CR1_PS      (1u << 9)    /* 1 = odd parity  */
#define USART_CR1_PCE     (1u << 10)
#define USART_CR1_WAKE    (1u << 11)
#define USART_CR1_M       (1u << 12)   /* 1 = 9 data bits */
#define USART_CR1_UE      (1u << 13)
#define USART_CR1_OVER8   (1u << 15)

/* USART_CR2 */
#define USART_CR2_ADD_MASK  (0xFu << 0)
#define USART_CR2_LBDL      (1u << 5)
#define USART_CR2_LBDIE     (1u << 6)
#define USART_CR2_LBCL      (1u << 8)
#define USART_CR2_CPHA      (1u << 9)
#define USART_CR2_CPOL      (1u << 10)
#define USART_CR2_CLKEN     (1u << 11)
#define USART_CR2_STOP_MASK (3u << 12)
#define USART_CR2_STOP_1BIT   (0u << 12)
#define USART_CR2_STOP_0_5BIT (1u << 12)
#define USART_CR2_STOP_2BIT   (2u << 12)
#define USART_CR2_STOP_1_5BIT (3u << 12)
#define USART_CR2_LINEN     (1u << 14)

/* USART_CR3 */
#define USART_CR3_EIE     (1u << 0)
#define USART_CR3_IREN    (1u << 1)
#define USART_CR3_IRLP    (1u << 2)
#define USART_CR3_HDSEL   (1u << 3)
#define USART_CR3_NACK    (1u << 4)
#define USART_CR3_SCEN    (1u << 5)
#define USART_CR3_DMAR    (1u << 6)
#define USART_CR3_DMAT    (1u << 7)
#define USART_CR3_RTSE    (1u << 8)
#define USART_CR3_CTSE    (1u << 9)
#define USART_CR3_CTSIE   (1u << 10)
#define USART_CR3_ONEBIT  (1u << 11)

/*
 * USART_BRR: mantissa [15:4], fraction [3:0].
 * With OVER8 = 0 (16x oversampling) that is simply f_ck / baud, rounded:
 *     USART_BRR(USART1_BASE) = USART_BRR_VALUE(90000000u, 115200u);
 */
#define USART_BRR_VALUE(fck, baud)  (((fck) + ((baud) / 2u)) / (baud))

#endif
