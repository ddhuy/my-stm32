#ifndef STM32F429_DMA_REGS_H
#define STM32F429_DMA_REGS_H

#include "arch/cortex-m4/mmio.h"
#include "soc/stm32f429/periph_base.h"

/*
 * DMA controller (RM0090 section 10.5).
 * Two controllers (DMA1_BASE, DMA2_BASE), each with 8 streams (0..7).
 * Only DMA2 can do memory-to-memory. Neither can reach CCM RAM.
 * Request mapping (stream + channel per peripheral): RM0090 tables 42/43.
 */

#define DMA_LISR(base)    MMIO32((base) + 0x00u)   /* streams 0..3 flags */
#define DMA_HISR(base)    MMIO32((base) + 0x04u)   /* streams 4..7 flags */
#define DMA_LIFCR(base)   MMIO32((base) + 0x08u)   /* write 1 to clear   */
#define DMA_HIFCR(base)   MMIO32((base) + 0x0Cu)

/* Per-stream registers, s = 0..7 */
#define DMA_STREAM_BASE(base, s) ((base) + 0x10u + ((s) * 0x18u))
#define DMA_SxCR(base, s)    MMIO32(DMA_STREAM_BASE(base, s) + 0x00u)
#define DMA_SxNDTR(base, s)  MMIO32(DMA_STREAM_BASE(base, s) + 0x04u)
#define DMA_SxPAR(base, s)   MMIO32(DMA_STREAM_BASE(base, s) + 0x08u)
#define DMA_SxM0AR(base, s)  MMIO32(DMA_STREAM_BASE(base, s) + 0x0Cu)
#define DMA_SxM1AR(base, s)  MMIO32(DMA_STREAM_BASE(base, s) + 0x10u)
#define DMA_SxFCR(base, s)   MMIO32(DMA_STREAM_BASE(base, s) + 0x14u)

/*
 * Interrupt status / clear for stream s: picks LISR/HISR (LIFCR/HIFCR)
 * and the bit offset of that stream's 6-bit group (0, 6, 16, 22).
 *
 *     if (DMA_ISR(DMA2_BASE, 7) & DMA_FLAG_TCIF(7)) {
 *         DMA_IFCR(DMA2_BASE, 7) = DMA_FLAG_TCIF(7);
 *     }
 */
#define DMA_ISR(base, s)     MMIO32((base) + 0x00u + (((s) >> 2) << 2))
#define DMA_IFCR(base, s)    MMIO32((base) + 0x08u + (((s) >> 2) << 2))
#define DMA_FLAG_SHIFT(s)    ((((s) & 1u) * 6u) + ((((s) >> 1) & 1u) * 16u))
#define DMA_FLAG_FEIF(s)     (1u << (DMA_FLAG_SHIFT(s) + 0u))   /* FIFO error        */
#define DMA_FLAG_DMEIF(s)    (1u << (DMA_FLAG_SHIFT(s) + 2u))   /* direct mode error */
#define DMA_FLAG_TEIF(s)     (1u << (DMA_FLAG_SHIFT(s) + 3u))   /* transfer error    */
#define DMA_FLAG_HTIF(s)     (1u << (DMA_FLAG_SHIFT(s) + 4u))   /* half transfer     */
#define DMA_FLAG_TCIF(s)     (1u << (DMA_FLAG_SHIFT(s) + 5u))   /* transfer complete */
#define DMA_FLAG_ALL(s)      (0x3Du << DMA_FLAG_SHIFT(s))

/* DMA_SxCR */
#define DMA_SxCR_EN          (1u << 0)    /* clear and wait until it reads 0 before reconfiguring */
#define DMA_SxCR_DMEIE       (1u << 1)
#define DMA_SxCR_TEIE        (1u << 2)
#define DMA_SxCR_HTIE        (1u << 3)
#define DMA_SxCR_TCIE        (1u << 4)
#define DMA_SxCR_PFCTRL      (1u << 5)
#define DMA_SxCR_DIR_MASK    (3u << 6)
#define DMA_SxCR_DIR_P2M     (0u << 6)
#define DMA_SxCR_DIR_M2P     (1u << 6)
#define DMA_SxCR_DIR_M2M     (2u << 6)
#define DMA_SxCR_CIRC        (1u << 8)
#define DMA_SxCR_PINC        (1u << 9)
#define DMA_SxCR_MINC        (1u << 10)
#define DMA_SxCR_PSIZE_8     (0u << 11)
#define DMA_SxCR_PSIZE_16    (1u << 11)
#define DMA_SxCR_PSIZE_32    (2u << 11)
#define DMA_SxCR_PSIZE_MASK  (3u << 11)
#define DMA_SxCR_MSIZE_8     (0u << 13)
#define DMA_SxCR_MSIZE_16    (1u << 13)
#define DMA_SxCR_MSIZE_32    (2u << 13)
#define DMA_SxCR_MSIZE_MASK  (3u << 13)
#define DMA_SxCR_PINCOS      (1u << 15)
#define DMA_SxCR_PL_MASK     (3u << 16)
#define DMA_SxCR_PL_LOW      (0u << 16)
#define DMA_SxCR_PL_MEDIUM   (1u << 16)
#define DMA_SxCR_PL_HIGH     (2u << 16)
#define DMA_SxCR_PL_VERYHIGH (3u << 16)
#define DMA_SxCR_DBM         (1u << 18)
#define DMA_SxCR_CT          (1u << 19)
#define DMA_SxCR_PBURST_MASK (3u << 21)
#define DMA_SxCR_MBURST_MASK (3u << 23)
#define DMA_SxCR_CHSEL_MASK  (7u << 25)
#define DMA_SxCR_CHSEL(ch)   (((ch) & 7u) << 25)

/* DMA_SxFCR */
#define DMA_SxFCR_FTH_MASK   (3u << 0)
#define DMA_SxFCR_FTH_1_4    (0u << 0)
#define DMA_SxFCR_FTH_1_2    (1u << 0)
#define DMA_SxFCR_FTH_3_4    (2u << 0)
#define DMA_SxFCR_FTH_FULL   (3u << 0)
#define DMA_SxFCR_DMDIS      (1u << 2)    /* 1 = FIFO mode, 0 = direct mode */
#define DMA_SxFCR_FS_MASK    (7u << 3)
#define DMA_SxFCR_FEIE       (1u << 7)

#endif
