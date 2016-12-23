#include "serial.h"
#include "debuglog.h"
#include "systicks.h"
#include "fpu.h"
#include "adv_7611.h"
#include "hal_h264.h"
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

    FPU_AccessEnable();
    SysTicks_Init(166000);
    SysTicks_DelayMS(10); //delay to wait cpu0 bootup and set the PLL register
    InterCore_Init();
    
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

