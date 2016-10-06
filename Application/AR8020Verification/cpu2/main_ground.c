#include "debuglog.h"
#include "serial.h"
#include "test_i2c_adv7611.h"
#include "test_h264_encoder.h"
#include "test_BB.h"
#include "test_BBctrl.h"

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    int tmp;
  
    (*(volatile unsigned int *)0x40B0008C) = 0x00500000; //PATCH for FPGA version, PIN MUX for UART9
   
    serial_init(9, 115200);
    dlog_info("main function start \n");
    
    command_init();
    
    test_BB_Grd();

    //ADV_7611_Initial();
    //H264_Encoder_Init();

    for( ;; );
} 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
