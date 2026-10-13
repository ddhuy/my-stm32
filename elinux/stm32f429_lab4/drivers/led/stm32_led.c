#include "stm32_led.h"

#include "board/stm32f429i-disc1/board.h"
#include "soc/stm32f429/gpio_regs.h"
#include "soc/stm32f429/rcc_regs.h"


void stm32_led_init(void)
{
    /* Enable the GPIO clock for the LED pin. */
    RCC_AHB1ENR |= RCC_AHB1ENR_GPIOGEN;
    (void)RCC_AHB1ENR;
    /* Reset the pin to ensure it's low */
    GPIO_BSRR(LED_GREEN_PORT) = GPIO_BSRR_RESET(LED_GREEN_PIN);
    /* Configure output type: push-pull */
    GPIO_OTYPER(LED_GREEN_PORT) &= ~(1u << LED_GREEN_PIN);
    /* No internal pull-up or pull-down */
    GPIO_PUPDR(LED_GREEN_PORT) &= ~GPIO_2BIT_MASK(LED_GREEN_PIN);
    /* Configure PG13 as a general-purpose output pin */
    GPIO_MODER(LED_GREEN_PORT) &= ~GPIO_2BIT_MASK(LED_GREEN_PIN);
    GPIO_MODER(LED_GREEN_PORT) |= GPIO_MODER_VAL(LED_GREEN_PIN, GPIO_MODE_OUTPUT);
}

void stm32_led_on(void)
{
    GPIO_BSRR(LED_GREEN_PORT) = GPIO_BSRR_SET(LED_GREEN_PIN); // Set PG13 high
}

void led_off(void)
{
    GPIO_BSRR(LED_GREEN_PORT) = GPIO_BSRR_RESET(LED_GREEN_PIN); // Set PG13 low
}

void stm32_led_toggle(void)
{
    GPIO_ODR(LED_GREEN_PORT) ^= (1u << LED_GREEN_PIN); // Toggle PG13
}