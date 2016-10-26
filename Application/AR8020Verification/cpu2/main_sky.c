#include "serial.h"
#include "debuglog.h"
#include "adv_7611.h"
#include "h264_encoder.h"
#include "test_BB.h"
#include "command.h"

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
    /* initialize the uart */
    console_init(2, 115200);   
    dlog_info("cpu2 start!!! \n");
    
    ADV_7611_Initial(0);
    ADV_7611_Initial(1);
    H264_Encoder_Init();
    
    test_BB_sky();

    for( ;; )
    {
        if (command_getEnterStatus() == 1)
        {
            command_fulfill();
        }

        dlog_output(1);
    }
} 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
