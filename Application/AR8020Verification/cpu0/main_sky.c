#include "debuglog.h"
#include "test_freertos.h"
#include "test_i2c_adv7611.h"
#include "pll_ctrl.h"
#include "command.h"
#include "test_usbd.h"
#include "test_sram.h"

extern USBD_HandleTypeDef USBD_Device;

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

void console_init(uint32_t uart_num, uint32_t baut_rate)
{
  serial_init(uart_num, baut_rate);
  dlog_init(uart_num);
  UartNum = uart_num;
  command_init();
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
  /* initialize the uart */
  console_init(0,115200);
  dlog_info("cpu0 start!!! \n");

  /* Enable the CPU Cache */
  CPU_CACHE_Enable();

  HAL_Init();

  dlog_info("HAL_Init done \n");

  //test_usbh();

  /* We should never get here as control is now taken by the scheduler */
  for( ;; )
  {
    if (command_getEnterStatus() == 1)
    {
      command_fulfill();
    }

    dlog_output(1);
  }
} 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
