#include <stdint.h>

#include "soc/stm32f429/tim_regs.h"
#include "drivers/led/stm32_led.h"


volatile uint32_t tim2_irq_count = 0;


void tim2_irq_handler(void)
{
    if (TIM2_SR & TIM_SR_UIF)
    {
        TIM2_SR &= ~TIM_SR_UIF;
    
        tim2_irq_count++;

        if ((tim2_irq_count % 5u) == 0)
        {
            stm32_led_toggle();
        }
    }
}
