#include <asm/io.h>
#include <mach/stm32f429.h>
#include <clock.h>


/*
 * RCC register layout. The enable registers are consecutive, one per bus,
 * in the order of the CLK_BUS_* numbers in mach/clock_ids.h.
 */
#define RCC_AHB1ENR     0x30
#define RCC_ENR(bus)    (RCC_AHB1ENR + 4 * (bus))

/* Internal RC oscillator: the system clock after reset */
#define HSI_HZ          16000000

/**
 * Turn on the clock of one peripheral
 */
void clock_enable(u32 clk_id)
{
    uintptr_t enr = RCC_BASE + RCC_ENR(clk_id / 32);

    setbits(enr, BIT(clk_id % 32));
    /* the clock needs two cycles before the peripheral can be accessed */
    (void)readl(enr);
}

/**
 * Frequency in Hz of the clock feeding one peripheral
 */
u32 clock_get_rate(u32 clk_id)
{
    /*
     * No PLL setup yet: the system runs on HSI and every bus prescaler is
     * at its reset value of 1, so all peripherals see the same clock.
     */
    (void)clk_id;

    return HSI_HZ;
}
