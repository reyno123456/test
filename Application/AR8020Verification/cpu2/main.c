#include "debuglog.h"
#include "serial.h"
#include "test_i2c_adv7611.h"
#include "test_h264_encoder.h"
#include "test_BB.h"

void USB_LL_OTG0_IRQHandler(void)
{
}

/**
 * @brief  CPU L1-Cache enable.
 * @param  None
 * @retval None
 */
#if 0
static void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}
#endif

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    int tmp;
  
    (*(volatile unsigned int *)0x40B0008C) = 0x00500000; //PATCH for FPGA version, PIN MUX for UART9
   
    serial_init(9, 115200);
    dlog_info("main function start \n");
    
    //test_BB_SKY();
    //dlog_info("test_BB_SKY Done \n");
    
    test_h264_encoder();

#if 0
    /* Enable the CPU Cache */
    CPU_CACHE_Enable();

    /* STM32F7xx HAL library initialization:
       Configure the Flash ART accelerator on ITCM interface
       Configure the Systick to generate an interrupt each 1 msec
       Set NVIC Group Priority to 4
       Low Level Initialization
     */
    HAL_NVIC_SetPriority(Uart0_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(Uart0_IRQn);
    HAL_Init();

    dlog_info("HAL_Init done \n");
#endif

  /* We should never get here as control is now taken by the scheduler */
  for( ;; );
} 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
