#include "debuglog.h"
#include "test_freertos.h"
#include "test_i2c_adv7611.h"
#include "pll_ctrl.h"
#include "command.h"
#include "test_usbd.h"
#include "test_sram.h"
#include "sram.h"

extern USBD_HandleTypeDef USBD_Device;

extern volatile uint32_t   sramReady0;
extern volatile uint32_t   sramReady1;
extern volatile uint32_t   sendFinish;

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

void usb_data_dump(void)
{
    uint8_t         *buff0;
    uint8_t         *buff1;
    uint32_t         dataLen0;
    uint32_t         dataLen1;

    buff0           = SRAM_BUFF_0_ADDRESS;
    buff1           = SRAM_BUFF_1_ADDRESS;

    if ((sramReady0 == 1)||(sramReady1 == 1))
    {
        sendFinish = 0;

        if (sramReady0 == 1)
        {
            sramReady0      = 0;
            dataLen0        = SRAM_DATA_VALID_LEN_0;
            dataLen0        = (dataLen0 << 2);

            USBD_HID_SendReport(&USBD_Device, buff0, dataLen0);
            while(sendFinish == 0);
            SRAM_Ready0Confirm();
        }
        else if (sramReady1 == 1)
        {
            sramReady1 = 0;
            dataLen1        = SRAM_DATA_VALID_LEN_1;
            dataLen1        = (dataLen1 << 2);

            USBD_HID_SendReport(&USBD_Device, buff1, dataLen1);
            while(sendFinish == 0);
            SRAM_Ready1Confirm();
        }
    }
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
  sramReady0  = 0;
  sramReady1  = 0;
  sendFinish  = 1;

  BB_SPI_init();

  PLLCTRL_SetCoreClk(CPU0_CPU1_CORE_PLL_CLK, CPU0_ID);
  PLLCTRL_SetCoreClk(CPU2_CORE_PLL_CLK, CPU2_ID);

  /* initialize the uart */
  console_init(0,115200);
  dlog_info("cpu0 start!!! \n");

  /* Enable the CPU Cache */
  CPU_CACHE_Enable();

  HAL_Init();

  dlog_info("HAL_Init done \n");

  TestUsbd_InitHid();
  test_sram_init();

  /* We should never get here as control is now taken by the scheduler */
  for( ;; )
  {
    usb_data_dump();

    if (command_getEnterStatus() == 1)
    {
        command_fulfill();
    }

    dlog_output(1);
  }
} 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
