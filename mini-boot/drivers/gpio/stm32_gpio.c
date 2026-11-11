#include <gpio.h>
#include <asm/io.h>

/* STM32 GPIO port register layout */
#define GPIO_MODER      0x00
#define GPIO_IDR        0x10
#define GPIO_BSRR       0x18
#define GPIO_AFRL       0x20
#define GPIO_AFRH       0x24

/*
 * MODER holds a 2-bit mode field for each of the 16 pins:
 * pin 0 uses bits 1:0, pin 1 bits 3:2, ... pin 15 bits 31:30.
 */
#define GPIO_MODER_BITS_PER_PIN 2
#define GPIO_MODER_SHIFT(pin)   ((pin) * GPIO_MODER_BITS_PER_PIN)
#define GPIO_MODER_MASK(pin)    (0x3UL << GPIO_MODER_SHIFT(pin))

#define GPIO_MODE_INPUT         0x0UL
#define GPIO_MODE_OUTPUT        0x1UL
#define GPIO_MODE_ALTERNATE     0x2UL
#define GPIO_MODE_ANALOG        0x3UL
#define GPIO_MODER_MODE(pin, mode)  ((mode) << GPIO_MODER_SHIFT(pin))

/*
 * BSRR is write-only: writing 1 to bit n (0-15) sets pin n,
 * writing 1 to bit n + 16 resets it. Zeros are ignored.
 */
#define GPIO_BSRR_RESET_SHIFT   16
#define GPIO_BSRR_SET(pin)      BIT(pin)
#define GPIO_BSRR_RESET(pin)    BIT((pin) + GPIO_BSRR_RESET_SHIFT)

/*
 * The alternate function number is a 4-bit field per pin, so 16 pins need
 * two registers: AFRL holds pins 0-7, AFRH pins 8-15.
 */
#define GPIO_AFR_BITS_PER_PIN   4
#define GPIO_AFR_PINS_PER_REG   8
#define GPIO_AFR(pin)           ((pin) < GPIO_AFR_PINS_PER_REG ? \
                                 GPIO_AFRL : GPIO_AFRH)
#define GPIO_AFR_SHIFT(pin)     (((pin) % GPIO_AFR_PINS_PER_REG) * \
                                 GPIO_AFR_BITS_PER_PIN)
#define GPIO_AFR_MASK(pin)      (0xFUL << GPIO_AFR_SHIFT(pin))
#define GPIO_AFR_AF(pin, af)    ((u32)(af) << GPIO_AFR_SHIFT(pin))

/**
 * 
 */
void gpio_set_value(
    uintptr_t base,
    unsigned int pin,
    int value)
{
    /* BSRR changes one pin without a read-modify-write of ODR */
    writel(value ? GPIO_BSRR_SET(pin) : GPIO_BSRR_RESET(pin),
           base + GPIO_BSRR);
}

/**
 * 
 */
int gpio_get_value(
    uintptr_t base,
    unsigned int pin)
{
    return (readl(base + GPIO_IDR) >> pin) & 1;
}

/**
 * 
 */
void gpio_direction_output(
    uintptr_t base,
    unsigned int pin,
    int value)
{
    /* set the level first so the pin does not glitch when it turns output */
    gpio_set_value(base, pin, value);
    clrsetbits(base + GPIO_MODER,
               GPIO_MODER_MASK(pin),
               GPIO_MODER_MODE(pin, GPIO_MODE_OUTPUT));
}

/**
 * 
 */
void gpio_set_alternative(
    uintptr_t base,
    unsigned int pin,
    unsigned int af)
{
    /* choose the function first, then switch the pin over to it */
    clrsetbits(base + GPIO_AFR(pin),
               GPIO_AFR_MASK(pin),
               GPIO_AFR_AF(pin, af));
    clrsetbits(base + GPIO_MODER,
               GPIO_MODER_MASK(pin),
               GPIO_MODER_MODE(pin, GPIO_MODE_ALTERNATE));
}
