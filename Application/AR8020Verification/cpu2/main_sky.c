#include "serial.h"
#include "debuglog.h"
#include "systicks.h"
#include "adv_7611.h"
#include "h264_encoder.h"
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
    /* initialize the uart */
    console_init(2, 115200);   
    dlog_info("cpu2 start!!! \n");

    SysTicks_Init(166000);

    InterCore_Init();
    
    ADV_7611_Initial(0);
    ADV_7611_Initial(1);
    H264_Encoder_Init();
    
    test_BB_sky();

    for( ;; )
    {
        SYS_EVENT_Process();
        
        if (command_getEnterStatus() == 1)
        {
            command_fulfill();
        }

        dlog_output(200);
        SysTicks_DelayMS(20);
    }
} 

