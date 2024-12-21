.syntax unified
.cpu cortex-m4
.fpu softvfp
.thumb


.global vtable
.global Default_IRQHandler


/*
 * The vector table.
 */
.section .isr_vector,"a",%progbits
.type vtable, %object
.size vtable, .-vtable
vtable:
  // 0-15
  .word _estack
  .word Reset_Handler
  .word NMI_Handler
  .word HardFault_Handler
  .word MemManage_Handler
  .word BusFault_Handler
  .word UsageFault_Handler
  .word 0
  .word 0
  .word 0
  .word 0
  .word SVCall_Handler
  .word DebugMon_Handler
  .word 0
  .word PendSV_Handler
  .word SysTick_Handler
  /* External Interrupts */
  .word WDT_IRQHandler
  .word PVD_IRQHandler
  .word Timestamp_IRQHandler
  .word RTC_IRQHandler
  .word Flash_IRQHandler
  .word RCC_IRQHandler
  .word EXTI0_IRQHandler
  .word EXTI1_IRQHandler
  .word EXTI2_IRQHandler
  .word EXTI3_IRQHandler
  .word EXTI3_IRQHandler
  .word DMA1_Stream0_IRQHandler
  .word DMA1_Stream1_IRQHandler
  .word DMA1_Stream2_IRQHandler
  .word DMA1_Stream3_IRQHandler
  .word DMA1_Stream4_IRQHandler
  // 32-47
  .word DMA1_Stream5_IRQHandler
  .word DMA1_Stream6_IRQHandler
  .word ADC1_IRQHandler
  .word CAN1_Tx_IRQHandler
  .word CAN1_Rx0_IRQHandler
  .word CAN1_Rx1_IRQHandler
  .word CAN1_SCE_IRQHandler
  .word EXTI9_15_IRQHandler
  .word TIM1_BRK_TIM9_IRQHandler
  .word TIM1_UP_TIM10_IRQHandler
  .word TIM1_TRG_COM_TIM11_IRQHandler
  .word TIM1_CC_IRQHandler
  .word TIM2_IRQHandler
  .word TIM3_IRQHandler
  .word TIM4_IRQHandler
  .word I2C1_EV_IRQHandler
  // 48-59
  .word I2C1_ER_IRQHandler
  .word I2C2_EV_IRQHandler
  .word I2C2_ER_IRQHandler
  .word SPI1_IRQHandler
  .word SPI2_IRQHandler
  .word USART1_IRQHandler
  .word USART2_IRQHandler
  .word USART3_IRQHandler
  .word EXTI_15_10_IRQHandler
  .word RtcAlarm_Handler
  .word OTG_IRQHandler
  .word TIM8_BRK_TIM12_IRQHandler
  //
  .word I2C3_EV_IRQHandler
  .word I2C3_ER_IRQHandler
  .word OTG_HS_EP1_OUT_IRQHandler
  .word OTG_HS_EP1_IN_IRQHandler
  .word OTG_HS_WKUP_IRQHandler
  .word OTG_HS_IRQHandler
  .word DCMI_IRQHandler
  .word CRYP_IRQHandler
  .word HASH_RNG_IRQHandler
  .word FPU_IRQHandler
  .word UART7_IRQHandler
  .word UART8_IRQHandler
  .word SPI4_IRQHandler
  .word SPI5_IRQHandler
  .word SPI6_IRQHandler
  .word SAI1_IRQHandler
  .word LCD_TFT_IRQHandler
  .word LCD_TFT_Error_IRQHandler
  .word DMA2D_IRQHandler


  // (Location to boot from for RAM startup)
  //#define boot_ram_base  0xF108F85F
  //.word boot_ram_base

  /*
   * Setup weak aliases for each exception Handler to the
   * default one. These can be updated later, or just
   * overridden since they're weak refs.
   * The reset_Handler is set up separately.
   */
  .weak Reset_Handler
  .thumb_set Reset_Handler,Default_IRQHandler

  .weak NMI_Handler
  .thumb_set NMI_Handler,Default_IRQHandler

  .weak HardFault_Handler
  .thumb_set HardFault_Handler,Default_IRQHandler

  .weak MemManage_Handler
  .thumb_set MemManage_Handler,Default_IRQHandler

  .weak BusFault_Handler
  .thumb_set BusFault_Handler,Default_IRQHandler

  .weak UsageFault_Handler
  .thumb_set UsageFault_Handler,Default_IRQHandler

  .weak SVCall_Handler
  .thumb_set SVCall_Handler,Default_IRQHandler

  .weak DebugMon_Handler
  .thumb_set DebugMon_Handler,Default_IRQHandler

  .weak PendSV_Handler
  .thumb_set PendSV_Handler,Default_IRQHandler

  .weak SysTick_Handler
  .thumb_set SysTick_Handler,Default_IRQHandler

  .weak WDT_IRQHandler
  .thumb_set WDT_IRQHandler,Default_IRQHandler

  .weak PVD_IRQHandler
  .thumb_set PVD_IRQHandler,Default_IRQHandler

  .weak Timestamp_IRQHandler
  .thumb_set Timestamp_IRQHandler,Default_IRQHandler

  .weak RTC_IRQHandler
  .thumb_set RTC_IRQHandler,Default_IRQHandler

  .weak Flash_IRQHandler
  .thumb_set Flash_IRQHandler,Default_IRQHandler

  .weak RCC_IRQHandler
  .thumb_set RCC_IRQHandler,Default_IRQHandler

  .weak EXTI0_IRQHandler
  .thumb_set EXTI0_IRQHandler,Default_IRQHandler

  .weak EXTI1_IRQHandler
  .thumb_set EXTI1_IRQHandler,Default_IRQHandler

  .weak EXTI2_IRQHandler
  .thumb_set EXTI2_IRQHandler,Default_IRQHandler

  .weak EXTI3_IRQHandler
  .thumb_set EXTI3_IRQHandler,Default_IRQHandler

  .weak EXTI3_IRQHandler
  .thumb_set EXTI3_IRQHandler,Default_IRQHandler

  .weak DMA1_Stream0_IRQHandler
  .thumb_set DMA1_Stream0_IRQHandler,Default_IRQHandler

  .weak DMA1_Stream1_IRQHandler
  .thumb_set DMA1_Stream1_IRQHandler,Default_IRQHandler

  .weak DMA1_Stream2_IRQHandler
  .thumb_set DMA1_Stream2_IRQHandler,Default_IRQHandler

  .weak DMA1_Stream3_IRQHandler
  .thumb_set DMA1_Stream3_IRQHandler,Default_IRQHandler

  .weak DMA1_Stream4_IRQHandler
  .thumb_set DMA1_Stream4_IRQHandler,Default_IRQHandler

  .weak DMA1_Stream5_IRQHandler
  .thumb_set DMA1_Stream5_IRQHandler,Default_IRQHandler

  .weak DMA1_Stream6_IRQHandler
  .thumb_set DMA1_Stream6_IRQHandler,Default_IRQHandler

  .weak ADC1_IRQHandler
  .thumb_set ADC1_IRQHandler,Default_IRQHandler

  .weak CAN1_Tx_IRQHandler
  .thumb_set CAN1_Tx_IRQHandler,Default_IRQHandler

  .weak CAN1_Rx0_IRQHandler
  .thumb_set CAN1_Rx0_IRQHandler,Default_IRQHandler

  .weak CAN1_Rx1_IRQHandler
  .thumb_set CAN1_Rx1_IRQHandler,Default_IRQHandler

  .weak CAN1_SCE_IRQHandler
  .thumb_set CAN1_SCE_IRQHandler,Default_IRQHandler

  .weak EXTI9_15_IRQHandler
  .thumb_set EXTI9_15_IRQHandler,Default_IRQHandler

  .weak TIM1_BRK_TIM9_IRQHandler
  .thumb_set TIM1_BRK_TIM9_IRQHandler,Default_IRQHandler

  .weak TIM1_UP_TIM10_IRQHandler
  .thumb_set TIM1_UP_TIM10_IRQHandler,Default_IRQHandler

  .weak TIM1_TRG_COM_TIM11_IRQHandler
  .thumb_set TIM1_TRG_COM_TIM11_IRQHandler,Default_IRQHandler

  .weak TIM1_CC_IRQHandler
  .thumb_set TIM1_CC_IRQHandler,Default_IRQHandler

  .weak TIM2_IRQHandler
  .thumb_set TIM2_IRQHandler,Default_IRQHandler

  .weak TIM3_IRQHandler
  .thumb_set TIM3_IRQHandler,Default_IRQHandler

  .weak TIM4_IRQHandler
  .thumb_set TIM4_IRQHandler,Default_IRQHandler

  .weak I2C1_EV_IRQHandler
  .thumb_set I2C1_EV_IRQHandler,Default_IRQHandler

  .weak I2C1_ER_IRQHandler
  .thumb_set I2C1_ER_IRQHandler,Default_IRQHandler

  .weak I2C2_EV_IRQHandler
  .thumb_set I2C2_EV_IRQHandler,Default_IRQHandler

  .weak I2C2_ER_IRQHandler
  .thumb_set I2C2_ER_IRQHandler,Default_IRQHandler

  .weak SPI1_IRQHandler
  .thumb_set SPI1_IRQHandler,Default_IRQHandler

  .weak SPI2_IRQHandler
  .thumb_set SPI2_IRQHandler,Default_IRQHandler

  .weak USART1_IRQHandler
  .thumb_set USART1_IRQHandler,Default_IRQHandler

  .weak USART2_IRQHandler
  .thumb_set USART2_IRQHandler,Default_IRQHandler

  .weak USART3_IRQHandler
  .thumb_set USART3_IRQHandler,Default_IRQHandler

  .weak EXTI_15_10_IRQHandler
  .thumb_set EXTI_15_10_IRQHandler,Default_IRQHandler

  .weak RtcAlarm_Handler
  .thumb_set RtcAlarm_Handler,Default_IRQHandler

  .weak OTG_IRQHandler
  .thumb_set OTG_IRQHandler,Default_IRQHandler

  .weak TIM8_BRK_TIM12_IRQHandler
  .thumb_set TIM8_BRK_TIM12_IRQHandler,Default_IRQHandler

  .weak I2C3_EV_IRQHandler
  .thumb_set I2C3_EV_IRQHandler,Default_IRQHandler

  .weak I2C3_ER_IRQHandler
  .thumb_set I2C3_ER_IRQHandler,Default_IRQHandler

  .weak OTG_HS_EP1_OUT_IRQHandler
  .thumb_set OTG_HS_EP1_OUT_IRQHandler,Default_IRQHandler

  .weak OTG_HS_EP1_IN_IRQHandler
  .thumb_set OTG_HS_EP1_IN_IRQHandler,Default_IRQHandler

  .weak OTG_HS_WKUP_IRQHandler
  .thumb_set OTG_HS_WKUP_IRQHandler,Default_IRQHandler

  .weak OTG_HS_IRQHandler
  .thumb_set OTG_HS_IRQHandler,Default_IRQHandler

  .weak DCMI_IRQHandler
  .thumb_set DCMI_IRQHandler,Default_IRQHandler

  .weak CRYP_IRQHandler
  .thumb_set CRYP_IRQHandler,Default_IRQHandler

  .weak HASH_RNG_IRQHandler
  .thumb_set HASH_RNG_IRQHandler,Default_IRQHandler

  .weak FPU_IRQHandler
  .thumb_set FPU_IRQHandler,Default_IRQHandler

  .weak UART7_IRQHandler
  .thumb_set UART7_IRQHandler,Default_IRQHandler

  .weak UART8_IRQHandler
  .thumb_set UART8_IRQHandler,Default_IRQHandler

  .weak SPI4_IRQHandler
  .thumb_set SPI4_IRQHandler,Default_IRQHandler

  .weak SPI5_IRQHandler
  .thumb_set SPI5_IRQHandler,Default_IRQHandler

  .weak SPI6_IRQHandler
  .thumb_set SPI6_IRQHandler,Default_IRQHandler

  .weak SAI1_IRQHandler
  .thumb_set SAI1_IRQHandler,Default_IRQHandler

  .weak LCD_TFT_IRQHandler
  .thumb_set LCD_TFT_IRQHandler,Default_IRQHandler

  .weak LCD_TFT_Error_IRQHandler
  .thumb_set LCD_TFT_Error_IRQHandler,Default_IRQHandler

  .weak DMA2D_IRQHandler
  .thumb_set DMA2D_IRQHandler,Default_IRQHandler


/*
 * A 'Default' interrupt Handler. This is where interrupts
 * which are not otherwise configured will go.
 * It is an infinite loop, because...well, we weren't
 * expecting the interrupt, so what can we do?
 */
.section .text.Default_IRQHandler,"ax",%progbits
Default_IRQHandler:
  Infinitive_Loop:
    B Infinitive_Loop
.size Default_IRQHandler, .-Default_IRQHandler
