#include "debuglog.h"
#include "command.h"
#include "sys_event.h"
#include "hal.h"
#include "hal_sys_ctl.h"


void CONSOLE_Init(void)
{
    dlog_init(command_run, DLOG_CLIENT_PROCESSOR);
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
    dlog_info("cpu1 start!!! \n");

    /* We should never get here as control is now taken by the scheduler */
    for( ;; )
    {
        SYS_EVENT_Process();
        HAL_Delay(20);
    }
} 
