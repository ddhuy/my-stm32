#ifndef STM32F429_LTDC_REGS_H
#define STM32F429_LTDC_REGS_H

#include "arch/cortex-m4/mmio.h"
#include "soc/stm32f429/periph_base.h"

/*
 * LCD-TFT display controller (RM0090 section 16.7).
 * Enable RCC_APB2ENR_LTDCEN; the pixel clock comes from PLLSAI.
 * Timing registers hold cumulative values minus 1:
 *   SSCR  = (HSYNC-1, VSYNC-1)
 *   BPCR  = (HSYNC+HBP-1, VSYNC+VBP-1)
 *   AWCR  = (HSYNC+HBP+W-1, VSYNC+VBP+H-1)
 *   TWCR  = (HSYNC+HBP+W+HFP-1, VSYNC+VBP+H+VFP-1)
 */

#define LTDC_SSCR      MMIO32(LTDC_BASE + 0x08u)
#define LTDC_BPCR      MMIO32(LTDC_BASE + 0x0Cu)
#define LTDC_AWCR      MMIO32(LTDC_BASE + 0x10u)
#define LTDC_TWCR      MMIO32(LTDC_BASE + 0x14u)
#define LTDC_GCR       MMIO32(LTDC_BASE + 0x18u)
#define LTDC_SRCR      MMIO32(LTDC_BASE + 0x24u)
#define LTDC_BCCR      MMIO32(LTDC_BASE + 0x2Cu)
#define LTDC_IER       MMIO32(LTDC_BASE + 0x34u)
#define LTDC_ISR       MMIO32(LTDC_BASE + 0x38u)
#define LTDC_ICR       MMIO32(LTDC_BASE + 0x3Cu)
#define LTDC_LIPCR     MMIO32(LTDC_BASE + 0x40u)
#define LTDC_CPSR      MMIO32(LTDC_BASE + 0x44u)
#define LTDC_CDSR      MMIO32(LTDC_BASE + 0x48u)

/* Horizontal value in [27:16], vertical in [10:0] for SSCR..TWCR */
#define LTDC_HV(h, v)  ((((h) & 0xFFFu) << 16) | ((v) & 0x7FFu))

/* LTDC_GCR */
#define LTDC_GCR_LTDCEN    (1u << 0)
#define LTDC_GCR_DEN       (1u << 16)
#define LTDC_GCR_PCPOL     (1u << 28)
#define LTDC_GCR_DEPOL     (1u << 29)
#define LTDC_GCR_VSPOL     (1u << 30)
#define LTDC_GCR_HSPOL     (1u << 31)

/* LTDC_SRCR: shadow reload */
#define LTDC_SRCR_IMR      (1u << 0)   /* immediately          */
#define LTDC_SRCR_VBR      (1u << 1)   /* at vertical blanking */

/* LTDC_IER / LTDC_ISR / LTDC_ICR */
#define LTDC_IT_LI         (1u << 0)   /* line                 */
#define LTDC_IT_FU         (1u << 1)   /* FIFO underrun        */
#define LTDC_IT_TERR       (1u << 2)   /* transfer error       */
#define LTDC_IT_RR         (1u << 3)   /* register reload      */

/* Layer registers, layer n = 1 or 2 */
#define LTDC_LAYER_BASE(n) (LTDC_BASE + 0x84u + (((n) - 1u) * 0x80u))
#define LTDC_LxCR(n)       MMIO32(LTDC_LAYER_BASE(n) + 0x00u)
#define LTDC_LxWHPCR(n)    MMIO32(LTDC_LAYER_BASE(n) + 0x04u)   /* [27:16] stop, [11:0] start */
#define LTDC_LxWVPCR(n)    MMIO32(LTDC_LAYER_BASE(n) + 0x08u)   /* [26:16] stop, [10:0] start */
#define LTDC_LxCKCR(n)     MMIO32(LTDC_LAYER_BASE(n) + 0x0Cu)
#define LTDC_LxPFCR(n)     MMIO32(LTDC_LAYER_BASE(n) + 0x10u)
#define LTDC_LxCACR(n)     MMIO32(LTDC_LAYER_BASE(n) + 0x14u)   /* constant alpha 0..255 */
#define LTDC_LxDCCR(n)     MMIO32(LTDC_LAYER_BASE(n) + 0x18u)
#define LTDC_LxBFCR(n)     MMIO32(LTDC_LAYER_BASE(n) + 0x1Cu)
#define LTDC_LxCFBAR(n)    MMIO32(LTDC_LAYER_BASE(n) + 0x28u)   /* frame buffer address */
#define LTDC_LxCFBLR(n)    MMIO32(LTDC_LAYER_BASE(n) + 0x2Cu)   /* [28:16] pitch, [12:0] line length + 3 */
#define LTDC_LxCFBLNR(n)   MMIO32(LTDC_LAYER_BASE(n) + 0x30u)   /* number of lines */
#define LTDC_LxCLUTWR(n)   MMIO32(LTDC_LAYER_BASE(n) + 0x40u)

/* LTDC_LxCR */
#define LTDC_LxCR_LEN      (1u << 0)
#define LTDC_LxCR_COLKEN   (1u << 1)
#define LTDC_LxCR_CLUTEN   (1u << 4)

/* LTDC_LxPFCR pixel formats */
#define LTDC_PF_ARGB8888   0u
#define LTDC_PF_RGB888     1u
#define LTDC_PF_RGB565     2u
#define LTDC_PF_ARGB1555   3u
#define LTDC_PF_ARGB4444   4u
#define LTDC_PF_L8         5u
#define LTDC_PF_AL44       6u
#define LTDC_PF_AL88       7u

/* LTDC_LxBFCR blending factors */
#define LTDC_BF1_CONST_ALPHA       (4u << 8)
#define LTDC_BF1_PIXEL_X_CONST     (6u << 8)
#define LTDC_BF2_CONST_ALPHA       (5u << 0)
#define LTDC_BF2_PIXEL_X_CONST     (7u << 0)

#endif
