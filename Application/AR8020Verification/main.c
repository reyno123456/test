#include "test_freertos.h"

/**
 * @brief  CPU L1-Cache enable.
 * @param  None
 * @retval None
 */
static void CPU_CACHE_Enable(void)
{
  /* Enable I-Cache */
  SCB_EnableICache();

  /* Enable D-Cache */
  SCB_EnableDCache();
}


/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  int tmp;
  
  serial_init();
  serial_puts("main function start \n");

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

  serial_puts("HAL_Init done \n");

  /* FreeRTOS task test*/
  TestTask();

  /* We should never get here as control is now taken by the scheduler */
  for( ;; );
} 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
