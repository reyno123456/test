#include "serial.h"
#include "debuglog.h"
#include "command.h"
#include "sys_event.h"
#include "hal_sys_ctl.h"
#include "hal_bb.h"
#include "hal.h"

void console_init(uint32_t uart_num, uint32_t baut_rate)
{
    dlog_init(command_run);
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{ 
    HAL_SYS_CTL_Init(NULL);
   
    console_init(2, 115200);
    dlog_info("main ground function start \n");
    
    HAL_BB_InitGround();

    /* We should never get here as control is now taken by the scheduler */
    for( ;; )
    {
        SYS_EVENT_Process();

        DLOG_Process(NULL);
      
        HAL_Delay(20);
    }
} 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
