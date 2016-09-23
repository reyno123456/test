/**
  ******************************************************************************
  * @file      start.s
  * @author    Min Zhao
  * @Version    V1.1.0
  * @Date       14-September-2016
  ******************************************************************************
  */
#include "debuglog.h"
#include "interrupt.h"
#include "interrupt_internal.h"

.equ  ITCM1,        0x00000000
.equ  DTCM1_DATA,   0x20004000
.equ  HEAP_START,   0x20010000
.equ  HEAP_END,     0x20000000
.equ  DATA_START,   0x20018000


.syntax unified
.cpu cortex-m7
.fpu softvfp
.thumb

.global  vectors
.global  Default_Handler

/* defined in linker script */
/* start address for the text section */
.word  _text_start
/* end address for the text section */
.word  _text_end
/* start address for the .data/rodata/bss section. */
.word  _data_start
/* end address for the .data/rodata/bss section. */
.word  _data_end

/**
 * @brief  This is the code that gets called when the processor first
 *          starts execution following a reset event. Only the absolutely
 *          necessary set is performed, after which the application
 *          supplied main() routine is called. 
 * @param  None
 * @retval : None
*/
    .section  .start.Reset_Handler
  .weak  Reset_Handler
  .type  Reset_Handler, %function
Reset_Handler:  
  ldr  sp,  =_estack             /* set stack pointer */

/* copy the CPU2 data from ITCM2 to DTCM2 */
  movs r1, #0
  ldr  r4, =_data_start           /* The start addr of data image */ 
  b  LoopCopyData
CopyInitData:
  ldr  r3, =_data_start
  ldr  r3, [r3, r1]
  str  r3, [r0, r1]
  adds  r1, r1, #4
LoopCopyData:
  ldr  r0, =DTCM1_DATA                 /* The start addr of Dtcm0 */
  ldr  r3, =_data_end             /* The end addr of data image */
  adds  r2, r4, r1
  cmp  r2, r3
  bcc  CopyInitData
/* branch to main */
  bl main

/**
 * @brief  This is the code that gets called when the processor receives an 
 *         unexpected interrupt.  This simply enters an infinite loop, preserving
 *         the system state for examination by a debugger.
 * @param  None     
 * @retval None       
*/
  .section .start.Default_Handler, "ax", %progbits
Default_Handler:
Infinite_Loop:
  b  Infinite_Loop
  .size  Default_Handler, .-Default_Handler

/******************************************************************************
*
* The minimal vector table for a Cortex M3. Note that the proper constructs
* must be placed on this to ensure that it ends up at physical address
* 0x0000.0000.
* 
*******************************************************************************/
  .section  .isr_vectors, "a", %progbits
  .type   vectors, %object
  .size   vectors, .-vectors
   
  vectors:
  .word     _estack
  .word     Reset_Handler
  .word     Default_Handler
  .word     HardFault_Handler
  .word     Default_Handler
  .word     Default_Handler
  .word     Default_Handler
  .word     Default_Handler
  .word     Default_Handler
  .word     Default_Handler
  .word     Default_Handler
  .word     SVC_Handler
  .word     Default_Handler
  .word     Default_Handler
  .word     PendSV_Handler
  .word     Default_Handler
  .word     UART0_IRQHandler
  .word     UART1_IRQHandler
  .word     UART2_IRQHandler
  
/*******************************************************************************
*
* Provide weak aliases for each Exception handler to the Default_Handler. 
* As they are weak aliases, any function with the same name will override 
* this definition.
* 
*******************************************************************************/
   .weak      NMI_Handler
   .thumb_set NMI_Handler,Default_Handler
  
   .weak      HardFault_Handler
   .thumb_set HardFault_Handler,Default_Handler
  
   .weak      MemManage_Handler
   .thumb_set MemManage_Handler,Default_Handler
  
   .weak      BusFault_Handler
   .thumb_set BusFault_Handler,Default_Handler

   .weak      UsageFault_Handler
   .thumb_set UsageFault_Handler,Default_Handler

   .weak      SVC_Handler
   .thumb_set SVC_Handler,Default_Handler

   .weak      DebugMon_Handler
   .thumb_set DebugMon_Handler,Default_Handler

   .weak      PendSV_Handler
   .thumb_set PendSV_Handler,Default_Handler

   .weak      SysTick_Handler
   .thumb_set SysTick_Handler,Default_Handler              
  
   .weak      WWDG_IRQHandler                   
   .thumb_set WWDG_IRQHandler,Default_Handler      
                  
   .weak      PVD_IRQHandler      
   .thumb_set PVD_IRQHandler,Default_Handler
               
   .weak      TAMP_STAMP_IRQHandler            
   .thumb_set TAMP_STAMP_IRQHandler,Default_Handler
            
   .weak      RTC_WKUP_IRQHandler                  
   .thumb_set RTC_WKUP_IRQHandler,Default_Handler
            
   .weak      FLASH_IRQHandler         
   .thumb_set FLASH_IRQHandler,Default_Handler
                  
   .weak      RCC_IRQHandler      
   .thumb_set RCC_IRQHandler,Default_Handler
                  
   .weak      EXTI0_IRQHandler         
   .thumb_set EXTI0_IRQHandler,Default_Handler
                  
   .weak      EXTI1_IRQHandler         
   .thumb_set EXTI1_IRQHandler,Default_Handler
                     
   .weak      EXTI2_IRQHandler         
   .thumb_set EXTI2_IRQHandler,Default_Handler 
                 
   .weak      EXTI3_IRQHandler         
   .thumb_set EXTI3_IRQHandler,Default_Handler
                        
   .weak      EXTI4_IRQHandler         
   .thumb_set EXTI4_IRQHandler,Default_Handler
                  
   .weak      DMA1_Stream0_IRQHandler               
   .thumb_set DMA1_Stream0_IRQHandler,Default_Handler
         
   .weak      DMA1_Stream1_IRQHandler               
   .thumb_set DMA1_Stream1_IRQHandler,Default_Handler
                  
   .weak      DMA1_Stream2_IRQHandler               
   .thumb_set DMA1_Stream2_IRQHandler,Default_Handler
                  
   .weak      DMA1_Stream3_IRQHandler               
   .thumb_set DMA1_Stream3_IRQHandler,Default_Handler 
                 
   .weak      DMA1_Stream4_IRQHandler              
   .thumb_set DMA1_Stream4_IRQHandler,Default_Handler
                  
   .weak      DMA1_Stream5_IRQHandler               
   .thumb_set DMA1_Stream5_IRQHandler,Default_Handler
                  
   .weak      DMA1_Stream6_IRQHandler               
   .thumb_set DMA1_Stream6_IRQHandler,Default_Handler
                  
   .weak      ADC_IRQHandler      
   .thumb_set ADC_IRQHandler,Default_Handler
               
   .weak      CAN1_TX_IRQHandler   
   .thumb_set CAN1_TX_IRQHandler,Default_Handler
            
   .weak      CAN1_RX0_IRQHandler                  
   .thumb_set CAN1_RX0_IRQHandler,Default_Handler
                           
   .weak      CAN1_RX1_IRQHandler                  
   .thumb_set CAN1_RX1_IRQHandler,Default_Handler
            
   .weak      CAN1_SCE_IRQHandler                  
   .thumb_set CAN1_SCE_IRQHandler,Default_Handler
            
   .weak      EXTI9_5_IRQHandler   
   .thumb_set EXTI9_5_IRQHandler,Default_Handler
            
   .weak      TIM1_BRK_TIM9_IRQHandler            
   .thumb_set TIM1_BRK_TIM9_IRQHandler,Default_Handler
            
   .weak      TIM1_UP_TIM10_IRQHandler            
   .thumb_set TIM1_UP_TIM10_IRQHandler,Default_Handler

   .weak      TIM1_TRG_COM_TIM11_IRQHandler      
   .thumb_set TIM1_TRG_COM_TIM11_IRQHandler,Default_Handler
      
   .weak      TIM1_CC_IRQHandler   
   .thumb_set TIM1_CC_IRQHandler,Default_Handler
                  
   .weak      TIM2_IRQHandler            
   .thumb_set TIM2_IRQHandler,Default_Handler
                  
   .weak      TIM3_IRQHandler            
   .thumb_set TIM3_IRQHandler,Default_Handler
                  
   .weak      TIM4_IRQHandler            
   .thumb_set TIM4_IRQHandler,Default_Handler
                  
   .weak      I2C1_EV_IRQHandler   
   .thumb_set I2C1_EV_IRQHandler,Default_Handler
                     
   .weak      I2C1_ER_IRQHandler   
   .thumb_set I2C1_ER_IRQHandler,Default_Handler
                     
   .weak      I2C2_EV_IRQHandler   
   .thumb_set I2C2_EV_IRQHandler,Default_Handler
                  
   .weak      I2C2_ER_IRQHandler   
   .thumb_set I2C2_ER_IRQHandler,Default_Handler
                           
   .weak      SPI1_IRQHandler            
   .thumb_set SPI1_IRQHandler,Default_Handler
                        
   .weak      SPI2_IRQHandler            
   .thumb_set SPI2_IRQHandler,Default_Handler
                  
   .weak      USART1_IRQHandler      
   .thumb_set USART1_IRQHandler,Default_Handler
                     
   .weak      USART2_IRQHandler      
   .thumb_set USART2_IRQHandler,Default_Handler
                     
   .weak      USART3_IRQHandler      
   .thumb_set USART3_IRQHandler,Default_Handler
                  
   .weak      EXTI15_10_IRQHandler               
   .thumb_set EXTI15_10_IRQHandler,Default_Handler
               
   .weak      RTC_Alarm_IRQHandler               
   .thumb_set RTC_Alarm_IRQHandler,Default_Handler
            
   .weak      OTG_FS_WKUP_IRQHandler         
   .thumb_set OTG_FS_WKUP_IRQHandler,Default_Handler
            
   .weak      TIM8_BRK_TIM12_IRQHandler         
   .thumb_set TIM8_BRK_TIM12_IRQHandler,Default_Handler
         
   .weak      TIM8_UP_TIM13_IRQHandler            
   .thumb_set TIM8_UP_TIM13_IRQHandler,Default_Handler
         
   .weak      TIM8_TRG_COM_TIM14_IRQHandler      
   .thumb_set TIM8_TRG_COM_TIM14_IRQHandler,Default_Handler
      
   .weak      TIM8_CC_IRQHandler   
   .thumb_set TIM8_CC_IRQHandler,Default_Handler
                  
   .weak      DMA1_Stream7_IRQHandler               
   .thumb_set DMA1_Stream7_IRQHandler,Default_Handler
                     
   .weak      FMC_IRQHandler            
   .thumb_set FMC_IRQHandler,Default_Handler
                     
   .weak      SDMMC1_IRQHandler            
   .thumb_set SDMMC1_IRQHandler,Default_Handler
                     
   .weak      TIM5_IRQHandler            
   .thumb_set TIM5_IRQHandler,Default_Handler
                     
   .weak      SPI3_IRQHandler            
   .thumb_set SPI3_IRQHandler,Default_Handler
   
   .weak      UART0_IRQHandler         
   .thumb_set UART0_IRQHandler,Default_Handler

   .weak      UART1_IRQHandler         
   .thumb_set UART1_IRQHandler,Default_Handler

   .weak      UART2_IRQHandler         
   .thumb_set UART2_IRQHandler,Default_Handler

   .weak      UART3_IRQHandler         
   .thumb_set UART3_IRQHandler,Default_Handler                  
   
   .weak      UART4_IRQHandler         
   .thumb_set UART4_IRQHandler,Default_Handler

   .weak      UART5_IRQHandler         
   .thumb_set UART5_IRQHandler,Default_Handler

   .weak      UART6_IRQHandler         
   .thumb_set UART6_IRQHandler,Default_Handler

   .weak      UART7_IRQHandler         
   .thumb_set UART7_IRQHandler,Default_Handler

   .weak      UART8_IRQHandler         
   .thumb_set UART8_IRQHandler,Default_Handler

   .weak      TIMER0_IRQHandler
   .thumb_set TIMER0_IRQHandler,Default_Handler

   .weak      TIM6_DAC_IRQHandler                  
   .thumb_set TIM6_DAC_IRQHandler,Default_Handler 
               
   .weak      TIM7_IRQHandler            
   .thumb_set TIM7_IRQHandler,Default_Handler
         
   .weak      DMA2_Stream0_IRQHandler               
   .thumb_set DMA2_Stream0_IRQHandler,Default_Handler
               
   .weak      DMA2_Stream1_IRQHandler               
   .thumb_set DMA2_Stream1_IRQHandler,Default_Handler
                  
   .weak      DMA2_Stream2_IRQHandler               
   .thumb_set DMA2_Stream2_IRQHandler,Default_Handler
            
   .weak      DMA2_Stream3_IRQHandler               
   .thumb_set DMA2_Stream3_IRQHandler,Default_Handler
            
   .weak      DMA2_Stream4_IRQHandler               
   .thumb_set DMA2_Stream4_IRQHandler,Default_Handler
   
   .weak      DMA2_Stream4_IRQHandler               
   .thumb_set DMA2_Stream4_IRQHandler,Default_Handler   

   .weak      ETH_IRQHandler   
   .thumb_set ETH_IRQHandler,Default_Handler
   
   .weak      ETH_WKUP_IRQHandler   
   .thumb_set ETH_WKUP_IRQHandler,Default_Handler

   .weak      CAN2_TX_IRQHandler   
   .thumb_set CAN2_TX_IRQHandler,Default_Handler   
                           
   .weak      CAN2_RX0_IRQHandler                  
   .thumb_set CAN2_RX0_IRQHandler,Default_Handler
                           
   .weak      CAN2_RX1_IRQHandler                  
   .thumb_set CAN2_RX1_IRQHandler,Default_Handler
                           
   .weak      CAN2_SCE_IRQHandler                  
   .thumb_set CAN2_SCE_IRQHandler,Default_Handler
                           
   .weak      OTG_FS_IRQHandler      
   .thumb_set OTG_FS_IRQHandler,Default_Handler
                     
   .weak      DMA2_Stream5_IRQHandler               
   .thumb_set DMA2_Stream5_IRQHandler,Default_Handler
                  
   .weak      DMA2_Stream6_IRQHandler               
   .thumb_set DMA2_Stream6_IRQHandler,Default_Handler
                  
   .weak      DMA2_Stream7_IRQHandler               
   .thumb_set DMA2_Stream7_IRQHandler,Default_Handler
                  
   .weak      USART6_IRQHandler      
   .thumb_set USART6_IRQHandler,Default_Handler
                        
   .weak      I2C3_EV_IRQHandler   
   .thumb_set I2C3_EV_IRQHandler,Default_Handler
                        
   .weak      I2C3_ER_IRQHandler   
   .thumb_set I2C3_ER_IRQHandler,Default_Handler
                        
   .weak      OTG_HS_EP1_OUT_IRQHandler         
   .thumb_set OTG_HS_EP1_OUT_IRQHandler,Default_Handler
               
   .weak      OTG_HS_EP1_IN_IRQHandler            
   .thumb_set OTG_HS_EP1_IN_IRQHandler,Default_Handler
               
   .weak      OTG_HS_WKUP_IRQHandler         
   .thumb_set OTG_HS_WKUP_IRQHandler,Default_Handler
            
   .weak      OTG_HS_IRQHandler      
   .thumb_set OTG_HS_IRQHandler,Default_Handler
                  
   .weak      DCMI_IRQHandler            
   .thumb_set DCMI_IRQHandler,Default_Handler

   .weak      CRYP_IRQHandler            
   .thumb_set CRYP_IRQHandler,Default_Handler

   .weak      HASH_RNG_IRQHandler            
   .thumb_set HASH_RNG_IRQHandler,Default_Handler   

   .weak      FPU_IRQHandler                  
   .thumb_set FPU_IRQHandler,Default_Handler

   .weak      UART7_IRQHandler                  
   .thumb_set UART7_IRQHandler,Default_Handler

   .weak      UART8_IRQHandler                  
   .thumb_set UART8_IRQHandler,Default_Handler   

   .weak      SPI4_IRQHandler            
   .thumb_set SPI4_IRQHandler,Default_Handler
   
   .weak      SPI5_IRQHandler            
   .thumb_set SPI5_IRQHandler,Default_Handler

   .weak      SPI6_IRQHandler            
   .thumb_set SPI6_IRQHandler,Default_Handler   

   .weak      SAI1_IRQHandler            
   .thumb_set SAI1_IRQHandler,Default_Handler
   
   .weak      LTDC_IRQHandler            
   .thumb_set LTDC_IRQHandler,Default_Handler

   .weak      LTDC_ER_IRQHandler            
   .thumb_set LTDC_ER_IRQHandler,Default_Handler

   .weak      DMA2D_IRQHandler            
   .thumb_set DMA2D_IRQHandler,Default_Handler   

   .weak      SAI2_IRQHandler            
   .thumb_set SAI2_IRQHandler,Default_Handler
   
   .weak      QUADSPI_IRQHandler            
   .thumb_set QUADSPI_IRQHandler,Default_Handler
 
   .weak      LPTIM1_IRQHandler            
   .thumb_set LPTIM1_IRQHandler,Default_Handler

   .weak      CEC_IRQHandler            
   .thumb_set CEC_IRQHandler,Default_Handler
   
   .weak      I2C4_EV_IRQHandler            
   .thumb_set I2C4_EV_IRQHandler,Default_Handler 
 
   .weak      I2C4_ER_IRQHandler            
   .thumb_set I2C4_ER_IRQHandler,Default_Handler
   
   .weak      SPDIF_RX_IRQHandler            
   .thumb_set SPDIF_RX_IRQHandler,Default_Handler 

