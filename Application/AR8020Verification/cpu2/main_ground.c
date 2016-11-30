#include "serial.h"
#include "debuglog.h"
#include "fpu.h"
#include "test_BB.h"
#include "command.h"
#include "sys_event.h"
#include "inter_core.h"

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
    //(*(volatile unsigned int *)0x40B0008C) = 0x00500000; //PATCH for FPGA version, PIN MUX for UART9
   
    console_init(2, 115200);
    dlog_info("main ground function start \n");
    
    FPU_AccessEnable();
    SysTicks_Init(166000);
    SysTicks_DelayMS(10); //delay to wait cpu0 bootup and set the PLL register

    InterCore_Init();

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

        SysTicks_DelayMS(20);
    }
} 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
