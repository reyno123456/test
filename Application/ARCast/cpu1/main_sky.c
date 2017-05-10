#include "debuglog.h"
#include "command.h"
#include "sys_event.h"
#include "serial.h"
#include "hal.h"
#include "hal_sys_ctl.h"
#include "hal_softi2s.h"
#include "hal_uart.h"

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
    STRU_HAL_SOFTI2S_INIT st_audioConfig = {AUDIO_DATA_START,HAL_GPIO_NUM35,HAL_GPIO_NUM70,HAL_GPIO_NUM20};    
    dlog_info("cpu1 start!!!"); 
   /* while(1)
    {
      SYS_EVENT_Process();
    }*/
    /* We should never get here as control is now taken by the scheduler */
    HAL_SOFTI2S_Init(&st_audioConfig);
    HAL_SOFTI2S_Funct();
} 

