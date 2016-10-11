#include "debuglog.h"
#include "test_freertos.h"
#include "test_i2c_adv7611.h"
#include "pll_ctrl.h"

void *malloc(size_t size)
{
    return pvPortMalloc(size);
}

void free(void* p)
{
    vPortFree(p);
}

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

  PLLCTRL_SetCoreClk(CORE_PLL_CLK);
  serial_init(0, 115200);
  dlog_info("cpu0 start!!! \n");

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
  
  command_init();

  TestUsbd_InitHid();

  /* We should never get here as control is now taken by the scheduler */
  for( ;; )
  {
    if (command_getEnterStatus() == 1)
    {
      command_fulfill();
    }
  }
} 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
