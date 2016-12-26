#include "serial.h"
#include "debuglog.h"
#include "test_BB.h"
#include "command.h"
#include "sys_event.h"
#include "hal_sys_ctl.h"
#include "hal.h"

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
    HAL_SYS_CTL_Init(NULL);
   
    console_init(2, 115200);
    dlog_info("main ground function start \n");
    
    test_BB_grd();

    /* We should never get here as control is now taken by the scheduler */
    for( ;; )
    {
        SYS_EVENT_Process();

        if (command_getEnterStatus() == 1)
        {
            command_fulfill();
        }

        dlog_output(100);

        HAL_Delay(20);
    }
} 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
