#include "debuglog.h"
#include "command.h"
#include "sys_event.h"
#include "stm32f746xx.h"
#include "serial.h"
#include "hal.h"
#include "hal_sys_ctl.h"
#include "hal_softi2s.h"

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
    CPU_CACHE_Enable();
    STRU_HAL_SOFTI2S_INIT st_audioConfig = {AUDIO_DATA_START,HAL_GPIO_NUM64,HAL_GPIO_NUM96,HAL_GPIO_NUM100};  
    dlog_info("cpu1 start!!! %d \n",HAL_SOFTI2S_Init(&st_audioConfig)); 

    /* We should never get here as control is now taken by the scheduler */
    HAL_SOFTI2S_Funct();
} 

