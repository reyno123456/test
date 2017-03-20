#include "debuglog.h"
#include "command.h"
#include "sys_event.h"
#include "stm32f746xx.h"
#include "hal.h"
#include "hal_sys_ctl.h"
#include "hal_uart.h"

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

void CONSOLE_Init(void)
{
    //HAL_UART_Init(DEBUG_LOG_UART_PORT, HAL_UART_BAUDR_115200, NULL);
    DLOG_Init(command_run, DLOG_CLIENT_PROCESSOR);
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    HAL_SYS_CTL_Init(NULL);

    /* initialize the uart */
    CONSOLE_Init();
    DLOG_Info("cpu1 start!!! \n");

    CPU_CACHE_Enable();

    /* We should never get here as control is now taken by the scheduler */
    for( ;; )
    {
       ;
    }
} 

