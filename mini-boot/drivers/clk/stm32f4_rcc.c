#include <asm/io.h>
#include <errno.h>
#include <clk.h>
#include <dm/device.h>
#include <dm/driver.h>
#include <platdata/stm32f4_rcc.h>


/*
 * STM32F4 RCC (reset and clock control): the clock provider. Probe runs
 * the system clock from the PLL fed by HSE; the operations gate
 * peripheral clocks and report each bus's rate. RM0090 register layout,
 * offsets from plat->base. The enable registers are consecutive, one per
 * bus, in the order of the STM32F4_CLK_BUS_* numbers in
 * platdata/stm32f4_rcc.h, with two reserved words in between (buses 3
 * and 6).
 */
#define RCC_CR          0x00
#define RCC_PLLCFGR     0x04
#define RCC_CFGR        0x08
#define RCC_AHB1ENR     0x30
#define RCC_ENR(bus)    (RCC_AHB1ENR + 4 * (bus))

/* Bus numbers with no enable register: the reserved words 0x3C and 0x48 */
#define RCC_BUS_RSVD1   3
#define RCC_BUS_RSVD2   6

#define RCC_CR_HSEON    BIT(16)
#define RCC_CR_HSERDY   BIT(17)
#define RCC_CR_PLLON    BIT(24)
#define RCC_CR_PLLRDY   BIT(25)

/* PLLCFGR. Bit 29 is reserved and set at reset: change only these fields */
#define PLLCFGR_M_SHIFT     0
#define PLLCFGR_M_MASK      (0x3F << PLLCFGR_M_SHIFT)
#define PLLCFGR_N_SHIFT     6
#define PLLCFGR_N_MASK      (0x1FF << PLLCFGR_N_SHIFT)
#define PLLCFGR_P_SHIFT     16
#define PLLCFGR_P_MASK      (0x3 << PLLCFGR_P_SHIFT)
#define PLLCFGR_SRC_HSE     BIT(22)
#define PLLCFGR_Q_SHIFT     24
#define PLLCFGR_Q_MASK      (0xF << PLLCFGR_Q_SHIFT)

/* CFGR: system clock switch (SW) and its status (SWS) */
#define CFGR_SW_SHIFT       0
#define CFGR_SW_MASK        (0x3 << CFGR_SW_SHIFT)
#define CFGR_SWS_SHIFT      2
#define CFGR_SWS_MASK       (0x3 << CFGR_SWS_SHIFT)
#define CFGR_CLK_HSI        0
#define CFGR_CLK_HSE        1
#define CFGR_CLK_PLL        2

/* CFGR: AHB prescaler (HPRE) and the two APB prescalers (PPRE1, PPRE2) */
#define CFGR_HPRE_SHIFT     4
#define CFGR_HPRE_MASK      (0xF << CFGR_HPRE_SHIFT)
#define CFGR_PPRE1_SHIFT    10
#define CFGR_PPRE2_SHIFT    13
#define CFGR_PPRE_MASK      0x7     /* before shifting */

/*
 * Prescaler encodings. HPRE: 0xxx = /1, 1000..1111 = /2 /4 /8 /16 /64
 * /128 /256 /512 (no /32). PPRE: 0xx = /1, 100..111 = /2 /4 /8 /16.
 */
#define HPRE_DIV            0x8     /* set: divided, low bits say how */
#define HPRE_SEL_MASK       0x7
#define PPRE_DIV            0x4
#define PPRE_SEL_MASK       0x3
#define PPRE_SHIFT_MAX      4       /* /16 */

/* RCC_APB1ENR: the PWR interface clock, needed to write PWR_CR */
#define APB1ENR_PWREN       BIT(28)

/* PWR, offsets from plat->pwr_base. Scale 1 is needed above 144 MHz */
#define PWR_CR              0x00
#define PWR_CR_VOS_SHIFT    14
#define PWR_CR_VOS_MASK     (0x3 << PWR_CR_VOS_SHIFT)
#define PWR_CR_VOS_SCALE1   (0x3 << PWR_CR_VOS_SHIFT)

/* Flash interface, offsets from plat->flash_base */
#define FLASH_ACR           0x00
#define FLASH_ACR_LATENCY_MASK  0xF
#define FLASH_ACR_PRFTEN    BIT(8)
#define FLASH_ACR_ICEN      BIT(9)
#define FLASH_ACR_DCEN      BIT(10)

/* An id is (bus * 32 + bit); see STM32F4_CLK_ID() */
#define CLK_ID_BUS(id)      ((id) / 32)
#define CLK_ID_BIT(id)      ((id) % 32)

/* Internal RC oscillator: the system clock after reset */
#define HSI_HZ              16000000

/*
 * PLL limits (RM0090 and the STM32F429 datasheet). The VCO input (HSE /
 * M) may be 1-2 MHz; 2 MHz is chosen for the lowest jitter. SYSCLK = VCO
 * / P with P fixed at 2 (field value 0); USB = VCO / Q.
 */
#define PLL_IN_HZ           2000000
#define PLLM_MIN            2
#define PLLM_MAX            63
#define PLLN_MIN            50
#define PLLN_MAX            432
#define PLLP                2
#define PLLP_FIELD          ((PLLP / 2) - 1)
#define PLLQ_MIN            2
#define PLLQ_MAX            15
#define VCO_MIN_HZ          100000000
#define VCO_MAX_HZ          432000000
#define USB_MAX_HZ          48000000

/* Chip limits without over-drive (180 MHz would need it) */
#define SYSCLK_MAX_HZ       168000000
#define PCLK1_MAX_HZ        42000000
#define PCLK2_MAX_HZ        84000000

/* One flash wait state per 30 MHz of HCLK, at 2.7-3.6 V */
#define FLASH_WS_HZ         30000000

/*
 * No timer exists yet (probe runs from board_init(), before
 * systime_init()), so each wait is bounded by a loop count. HSE starts in
 * about 2 ms, the PLL locks in well under that; this allows far more.
 */
#define RCC_WAIT_LOOPS      100000


/**
 * Wait until the bits 'mask' of the register at 'addr' read 'val'.
 *
 * Returns 0, or -ETIMEDOUT after RCC_WAIT_LOOPS reads.
 */
static int rcc_wait(
    uintptr_t addr,
    u32 mask,
    u32 val
)
{
    u32 loops = RCC_WAIT_LOOPS;

    while ((readl(addr) & mask) != val)
        if (--loops == 0)
            return -ETIMEDOUT;

    return SUCCESS;
}


/**
 * The PPRE field that divides 'hclk' by the smallest power of two that
 * brings it to 'max' or below, in '*field'.
 *
 * Returns 0, or -EINVAL if even /16 is too fast.
 */
static int ppre_field(
    u32 hclk,
    u32 max,
    u32 *field
)
{
    u32 shift;

    for (shift = 0; shift <= PPRE_SHIFT_MAX; shift++) {
        if ((hclk >> shift) <= max) {
            *field = shift ? (PPRE_DIV | (shift - 1)) : 0;
            return SUCCESS;
        }
    }

    return -EINVAL;
}


/**
 * How far a PPRE field shifts the clock right: 0 for /1, 1-4 for /2-/16.
 */
static u32 ppre_shift(
    u32 field
)
{
    return (field & PPRE_DIV) ? (field & PPRE_SEL_MASK) + 1 : 0;
}


/**
 * How far an HPRE field shifts the clock right. /32 does not exist, so
 * the four largest dividers shift one further than their position says.
 */
static u32 hpre_shift(
    u32 field
)
{
    static const u8 shifts[] = { 1, 2, 3, 4, 6, 7, 8, 9 };

    return (field & HPRE_DIV) ? shifts[field & HPRE_SEL_MASK] : 0;
}


/**
 * Run the system clock from the PLL at plat->sysclk_hz, fed by the
 * crystal of plat->hse_hz. Every factor is computed and checked first, so
 * a refusal touches no register; then the RM0090 order: HSE on, voltage
 * scale, flash wait states and bus prescalers (all before the clock goes
 * up), PLL on, switch.
 *
 * Also owns two fields outside the RCC that exist only because of the
 * clock: PWR_CR VOS and FLASH_ACR LATENCY (U-Boot's clk_stm32f.c does the
 * same). Expects the reset state: the PLL off (its factors cannot change
 * while it runs).
 *
 * Never calls the clk_* API: that would look this device up and probe it
 * again, before 'probed' is set, and recurse until the stack overflows.
 *
 * Returns 0; -EINVAL if a factor is out of range (the clock stays on
 * HSI); -ETIMEDOUT if HSE, the PLL or the switch does not become ready;
 * -EIO if the flash does not take its wait states.
 */
static int stm32f4_rcc_probe(
    struct udevice *dev
)
{
    const struct stm32f4_rcc_plat *plat = dev->plat;
    int ret;

    /* 1. Compute and check: reject rather than round */
    u32 sysclk = plat->sysclk_hz;
    if (sysclk == 0 || sysclk > SYSCLK_MAX_HZ)
        return -EINVAL;

    if (plat->hse_hz % PLL_IN_HZ)
        return -EINVAL;
    u32 m = plat->hse_hz / PLL_IN_HZ;
    if (m < PLLM_MIN || m > PLLM_MAX)
        return -EINVAL;

    if ((sysclk * PLLP) % PLL_IN_HZ)
        return -EINVAL;
    u32 n = sysclk * PLLP / PLL_IN_HZ;
    if (n < PLLN_MIN || n > PLLN_MAX)
        return -EINVAL;

    u32 vco = PLL_IN_HZ * n;
    if (vco < VCO_MIN_HZ || vco > VCO_MAX_HZ)
        return -EINVAL;

    /* Rounded up, so that USB stays at or below its maximum */
    u32 q = (vco + USB_MAX_HZ - 1) / USB_MAX_HZ;
    if (q < PLLQ_MIN || q > PLLQ_MAX)
        return -EINVAL;

    u32 ws = (sysclk - 1) / FLASH_WS_HZ;

    /* HCLK = SYSCLK (HPRE /1), so the APB dividers are taken from it */
    u32 ppre1, ppre2;
    ret = ppre_field(sysclk, PCLK1_MAX_HZ, &ppre1);
    if (ret)
        return ret;
    ret = ppre_field(sysclk, PCLK2_MAX_HZ, &ppre2);
    if (ret)
        return ret;

    /* 2. HSE on */
    setbits(plat->base + RCC_CR, RCC_CR_HSEON);
    ret = rcc_wait(plat->base + RCC_CR, RCC_CR_HSERDY, RCC_CR_HSERDY);
    if (ret)
        return ret;

    /*
     * 3. Voltage scale 1. PWR_CR is writable only with the PWR clock on;
     * the read-back gives it the cycles it needs before that write.
     */
    uintptr_t apb1enr = plat->base + RCC_ENR(STM32F4_CLK_BUS_APB1);
    setbits(apb1enr, APB1ENR_PWREN);
    (void)readl(apb1enr);
    clrsetbits(plat->pwr_base + PWR_CR, PWR_CR_VOS_MASK, PWR_CR_VOS_SCALE1);

    /*
     * 4. Flash wait states before the clock goes up: too few, and the CPU
     * fetches wrong instructions. Read back: the write takes effect only
     * once the register reads the new value.
     */
    uintptr_t acr = plat->flash_base + FLASH_ACR;
    writel(ws | FLASH_ACR_PRFTEN | FLASH_ACR_ICEN | FLASH_ACR_DCEN, acr);
    if ((readl(acr) & FLASH_ACR_LATENCY_MASK) != ws)
        return -EIO;

    /* 5. Bus prescalers while still on HSI, so APB1 never runs too fast */
    clrsetbits(plat->base + RCC_CFGR,
               CFGR_HPRE_MASK |
               (CFGR_PPRE_MASK << CFGR_PPRE1_SHIFT) |
               (CFGR_PPRE_MASK << CFGR_PPRE2_SHIFT),
               (ppre1 << CFGR_PPRE1_SHIFT) | (ppre2 << CFGR_PPRE2_SHIFT));

    /* 6. PLL factors, then on */
    clrsetbits(plat->base + RCC_PLLCFGR,
               PLLCFGR_M_MASK | PLLCFGR_N_MASK | PLLCFGR_P_MASK |
               PLLCFGR_SRC_HSE | PLLCFGR_Q_MASK,
               (m << PLLCFGR_M_SHIFT) | (n << PLLCFGR_N_SHIFT) |
               (PLLP_FIELD << PLLCFGR_P_SHIFT) | PLLCFGR_SRC_HSE |
               (q << PLLCFGR_Q_SHIFT));

    setbits(plat->base + RCC_CR, RCC_CR_PLLON);
    ret = rcc_wait(plat->base + RCC_CR, RCC_CR_PLLRDY, RCC_CR_PLLRDY);
    if (ret)
        return ret;

    /* 7. Switch the system clock over to the PLL */
    clrsetbits(plat->base + RCC_CFGR, CFGR_SW_MASK,
               CFGR_CLK_PLL << CFGR_SW_SHIFT);

    return rcc_wait(plat->base + RCC_CFGR, CFGR_SWS_MASK,
                    CFGR_CLK_PLL << CFGR_SWS_SHIFT);
}


/**
 * Turn on one peripheral clock: set its bit in the bus's enable register.
 * Core clocks (STM32F4_CLK_BUS_CORE) have no gate, so there is nothing to
 * do. A bus with no enable register (reserved, or above the core group)
 * is refused rather than writing to a reserved RCC word.
 */
static int stm32f4_rcc_enable(
    struct clk *clk
)
{
    const struct stm32f4_rcc_plat *plat = clk->dev->plat;
    u32 bus = CLK_ID_BUS(clk->id);

    if (bus == STM32F4_CLK_BUS_CORE)
        return SUCCESS;
    if (bus == RCC_BUS_RSVD1 || bus == RCC_BUS_RSVD2 ||
        bus > STM32F4_CLK_BUS_CORE)
        return -EINVAL;

    uintptr_t enr = plat->base + RCC_ENR(bus);

    setbits(enr, BIT(CLK_ID_BIT(clk->id)));
    /* the clock needs two cycles before the peripheral can be accessed */
    (void)readl(enr);

    return SUCCESS;
}


/**
 * The SYSCLK the registers say is running: from SWS, and for the PLL from
 * its source and factors in PLLCFGR. 0 if unknown.
 */
static u32 rcc_sysclk(
    const struct stm32f4_rcc_plat *plat
)
{
    u32 cfgr = readl(plat->base + RCC_CFGR);

    switch ((cfgr & CFGR_SWS_MASK) >> CFGR_SWS_SHIFT) {
    case CFGR_CLK_HSI:
        return HSI_HZ;
    case CFGR_CLK_HSE:
        return plat->hse_hz;
    case CFGR_CLK_PLL:
        break;
    default:
        return 0;
    }

    u32 pllcfgr = readl(plat->base + RCC_PLLCFGR);
    u32 src = (pllcfgr & PLLCFGR_SRC_HSE) ? plat->hse_hz : HSI_HZ;
    u32 m = (pllcfgr & PLLCFGR_M_MASK) >> PLLCFGR_M_SHIFT;
    u32 n = (pllcfgr & PLLCFGR_N_MASK) >> PLLCFGR_N_SHIFT;
    u32 p = (((pllcfgr & PLLCFGR_P_MASK) >> PLLCFGR_P_SHIFT) + 1) * 2;

    if (m == 0)
        return 0;

    return src / m * n / p;
}


/**
 * The rate of the bus the clock hangs on, computed from the registers
 * rather than taken from plat, so that it stays true whatever probe got
 * to: AHB buses and the core group run at HCLK, APB1 at PCLK1, APB2 at
 * PCLK2. 0 for an id on no bus. (APB timers run at twice PCLK when the
 * APB is divided; no timer uses this yet.)
 */
static u32 stm32f4_rcc_get_rate(
    struct clk *clk
)
{
    const struct stm32f4_rcc_plat *plat = clk->dev->plat;
    u32 cfgr = readl(plat->base + RCC_CFGR);
    u32 hclk = rcc_sysclk(plat) >>
               hpre_shift((cfgr & CFGR_HPRE_MASK) >> CFGR_HPRE_SHIFT);

    switch (CLK_ID_BUS(clk->id)) {
    case STM32F4_CLK_BUS_AHB1:
    case STM32F4_CLK_BUS_AHB2:
    case STM32F4_CLK_BUS_AHB3:
    case STM32F4_CLK_BUS_CORE:
        return hclk;
    case STM32F4_CLK_BUS_APB1:
        return hclk >> ppre_shift((cfgr >> CFGR_PPRE1_SHIFT) & CFGR_PPRE_MASK);
    case STM32F4_CLK_BUS_APB2:
        return hclk >> ppre_shift((cfgr >> CFGR_PPRE2_SHIFT) & CFGR_PPRE_MASK);
    default:
        return 0;
    }
}


/**
 * This driver's implementation of the clock uclass operations.
 */
static const struct clk_ops stm32f4_rcc_ops = {
    .enable   = &stm32f4_rcc_enable,
    .get_rate = &stm32f4_rcc_get_rate,
};


/**
 * Driver model entry. Probe sets up the PLL, on the first clock request
 * (the LED port, from board_init()). No private data.
 */
DM_DRIVER(stm32f4_rcc) = {
    .name = "stm32f4_rcc",
    .id   = UCLASS_CLK,

    .probe = stm32f4_rcc_probe,

    .ops = &stm32f4_rcc_ops,
};
