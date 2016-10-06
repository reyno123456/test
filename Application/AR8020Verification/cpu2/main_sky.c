#include "debuglog.h"
#include "serial.h"
#include "adv_7611.h"
#include "h264_encoder.h"
#include "BB_ctrl.h"

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

    //ADV_7611_Initial();
    H264_Encoder_Init();
    test_BB_SKY();

    for( ;; );
} 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
