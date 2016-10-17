#include "debuglog.h"
#include "test_i2c_adv7611.h"
#include "test_h264_encoder.h"
#include "test_BB.h"
#include "command.h"

void console_init(uint32_t uart_num, uint32_t baut_rate)
{
  serial_init(uart_num, baut_rate);
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
    int tmp;
  
    //(*(volatile unsigned int *)0x40B0008C) = 0x00500000; //PATCH for FPGA version, PIN MUX for UART9
   
    console_init(2, 115200);
    dlog_info("main ground function start \n");
    
    test_BB();

    //ADV_7611_Initial();
    //H264_Encoder_Init();

      /* We should never get here as control is now taken by the scheduler */
    for( ;; )
    {
        if (command_getEnterStatus() == 1)
        {
            command_fulfill();
        }
    }
} 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
