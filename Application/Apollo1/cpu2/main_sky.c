#include "serial.h"
#include "debuglog.h"
#include "hal_h264.h"
#include "test_BB.h"
#include "command.h"
#include "hal_bb.h"
#include "sys_event.h"
#include "hal_sys_ctl.h"
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

    /* initialize the uart */
    console_init(2, 115200);   
    dlog_info("cpu2 start!!! \n");

    STRU_HAL_H264_CONFIG st_h264Cfg;
    st_h264Cfg.u8_view0En = 1;
    st_h264Cfg.u8_view0Gop = 10;
    st_h264Cfg.e_view0Br = HAL_H264_BITRATE_500K;
    st_h264Cfg.u8_view0BrEn = 1;
    st_h264Cfg.u8_view1En = 1;
    st_h264Cfg.u8_view1Gop = 10;
    st_h264Cfg.e_view1Br = HAL_H264_BITRATE_500K;
    st_h264Cfg.u8_view1BrEn = 1;
    HAL_H264_Init(st_h264Cfg);
    
    HAL_BB_init(BB_SKY_MODE );

    for( ;; )
    {
        SYS_EVENT_Process();
        
        DLOG_Process(NULL);
      
        HAL_Delay(20);
    }
} 

