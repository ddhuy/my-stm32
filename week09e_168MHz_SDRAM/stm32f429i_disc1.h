#ifndef STM32F429I_DISC_H
#define STM32F429I_DISC_H


#define RCC_AHB1ENR (*(volatile uint32_t *) 0x40023830)
#define RCC_APB2ENR (*(volatile uint32_t *) 0x40023844)

#define GPIOA_MODER (*(volatile uint32_t *) 0x40020000)
#define GPIOA_AFRH  (*(volatile uint32_t *) 0x40020024)

#define GPIOG_MODER (*(volatile uint32_t *) 0x40021800)
#define GPIOG_ODR   (*(volatile uint32_t *) 0x40021814)

#define USART1_SR   (*(volatile uint32_t *) 0x40011000)
#define USART1_DR   (*(volatile uint32_t *) 0x40011004)
#define USART1_BRR  (*(volatile uint32_t *) 0x40011008)
#define USART1_CR1  (*(volatile uint32_t *) 0x4001100C)

#endif // STM32F429I_DISC_H