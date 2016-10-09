#include "debuglog.h"
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
    serial_init(1, 115200);
    dlog_info("cpu1 start!!! \n");
    CPU_CACHE_Enable();
   
    uint32_t test_reg = Reg_Read32((uint32_t)0xe000ed08);
    dlog_info("test_reg = %x\n", test_reg);

    HAL_NVIC_SetPriority(Uart0_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(Uart0_IRQn);
    HAL_Init();
    dlog_info("HAL_Init done \n");

    command_init();
    /* We should never get here as control is now taken by the scheduler */
    for( ;; )
    {
      if (command_getEnterStatus() == 1)
      {
        command_fulfill();
      }
    }
} 

