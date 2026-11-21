#include <asm/io.h>
#include <errno.h>
#include <clk.h>
#include <dm/device.h>
#include <dm/driver.h>
#include <platdata/stm32f4_rcc.h>


/*
 * STM32F4 RCC (reset and clock control): the clock provider. RM0090
 * register layout, offsets from plat->base. The enable registers are
 * consecutive, one per bus, in the order of the STM32F4_CLK_BUS_* numbers
 * in platdata/stm32f4_rcc.h, with two reserved words in between (buses 3
 * and 6).
 */
#define RCC_AHB1ENR     0x30
#define RCC_ENR(bus)    (RCC_AHB1ENR + 4 * (bus))

/* Bus numbers with no enable register: the reserved words 0x3C and 0x48 */
#define RCC_BUS_RSVD1   3
#define RCC_BUS_RSVD2   6

/* An id is (bus * 32 + bit); see STM32F4_CLK_ID() */
#define CLK_ID_BUS(id)  ((id) / 32)
#define CLK_ID_BIT(id)  ((id) % 32)

/* Internal RC oscillator: the system clock after reset */
#define HSI_HZ          16000000


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
 * No PLL setup yet: the system runs on HSI and every bus prescaler is at
 * its reset value of 1, so every clock has the same rate. Item 3 Part D
 * replaces this with a per-bus computation.
 */
static u32 stm32f4_rcc_get_rate(
    struct clk *clk
)
{
    (void)clk;

    return HSI_HZ;
}


/**
 * This driver's implementation of the clock uclass operations.
 */
static const struct clk_ops stm32f4_rcc_ops = {
    .enable   = &stm32f4_rcc_enable,
    .get_rate = &stm32f4_rcc_get_rate,
};


/**
 * Driver model entry. No probe yet: HSI is the reset clock, so there is
 * nothing to set up (item 3 Part D adds the PLL setup here). No private
 * data.
 */
DM_DRIVER(stm32f4_rcc) = {
    .name = "stm32f4_rcc",
    .id   = UCLASS_CLK,

    .ops = &stm32f4_rcc_ops,
};
