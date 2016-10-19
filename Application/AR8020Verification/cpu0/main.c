#include "debuglog.h"
#include "test_freertos.h"
#include "test_i2c_adv7611.h"
#include "pll_ctrl.h"
#include "command.h"
#include "test_usbd.h"
#include "test_sram.h"

extern USBD_HandleTypeDef USBD_Device;

volatile uint32_t   sramReady0;
volatile uint32_t   sramReady1;
volatile uint32_t   sendFinish;

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

  sramReady0  = 0;
  sramReady1  = 0;
  sendFinish = 0;

  PLLCTRL_SetCoreClk(CORE_PLL_CLK);
  /* initialize the uart */
  console_init(0,115200);
  dlog_info("cpu0 start!!! \n");

  /* Enable the CPU Cache */
  CPU_CACHE_Enable();

  HAL_Init();

  dlog_info("HAL_Init done \n");
  

  TestUsbd_InitHid();
  test_sram_init();

  while (1)
  {
      if ((sramReady0 == 1)||(sramReady1 == 1))
      {
      //    for (index = 0; index < 16; index++)
      //    {
          sendFinish = 0;
      //        USBD_HID_SendReport(&USBD_Device, (uint8_t *)(0x21002000 + ( index << 9 )), 8192);
      //        (sendCount)++;
      //    }

          if (sramReady0 == 1)
          {
              sramReady0 = 0;
              USBD_HID_SendReport(&USBD_Device, (uint8_t *)0x21000000, 8192);
              while(sendFinish == 0);
              SRAM_Ready0Confirm();
          }
          else if (sramReady1 == 1)
          {
              sramReady1 = 0;
              USBD_HID_SendReport(&USBD_Device, (uint8_t *)0x21002000, 8192);
              while(sendFinish == 0);
              SRAM_Ready1Confirm();
          }

      }
	   /* We should never get here as control is now taken by the scheduler */
	 if (command_getEnterStatus() == 1)
	 {
	   command_fulfill();
	 }	  
  }

 
  for( ;; )
  {

  }
} 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
