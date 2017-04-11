#include "debuglog.h"
#include "serial.h"
#include "command.h"
#include "test_usbh.h"
#include "cmsis_os.h"
#include "sys_event.h"
#include "bb_spi.h"
#include "hal.h"
#include "hal_gpio.h"
#include "hal_bb.h"
#include "hal_hdmi_rx.h"
#include "hal_usb_otg.h"
#include "hal_sys_ctl.h"
#include "wireless_interface.h"
#include "hal_nv.h"
#include "hal_usb_host.h"
#include "hal_encodemp3.h"
#include "hal_softi2s.h"
#include "systicks.h"
#include "memory_config.h"

void console_init(uint32_t uart_num, uint32_t baut_rate)
{
    HAL_UART_Init(DEBUG_LOG_UART_PORT, HAL_UART_BAUDR_115200, NULL);
    dlog_init(command_run, DLOG_SERVER_PROCESSOR);
}

void HDMI_powerOn(void)
{
    HAL_GPIO_OutPut(HAL_GPIO_NUM59);
    HAL_GPIO_SetPin(HAL_GPIO_NUM59, HAL_GPIO_PIN_SET);
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */

int main(void)
{
    STRU_HAL_SYS_CTL_CONFIG *pst_cfg;
    HAL_SYS_CTL_GetConfig( &pst_cfg);
    pst_cfg->u8_workMode = 0;
    HAL_SYS_CTL_Init(pst_cfg);
    /* initialize the uart */
    console_init(0,115200);
    dlog_info("cpu0 start!!! \n");

    HAL_GPIO_InPut(HAL_GPIO_NUM99);

    HAL_USB_ConfigPHY();

    HDMI_powerOn();

    STRU_HDMI_CONFIGURE        st_configure;
    st_configure.e_getFormatMethod = HAL_HDMI_POLLING;

    st_configure.u8_interruptGpio = HAL_GPIO_NUM64;
    st_configure.u8_hdmiToEncoderCh = 0;
    HAL_HDMI_RX_Init(HAL_HDMI_RX_1, &st_configure);

    STRU_MP3_ENCODE_CONFIGURE_WAVE st_audioConfig;
    st_audioConfig.e_samplerate = HAL_MP3_ENCODE_44100;
    st_audioConfig.e_modes = HAL_MP3_ENCODE_STEREO;
    st_audioConfig.u32_rawDataAddr = AUDIO_DATA_START;
    st_audioConfig.u32_rawDataLenght = AUDIO_DATA_BUFF_SIZE;
    st_audioConfig.u32_encodeDataAddr = MPE3_ENCODER_DATA_ADDR;
    st_audioConfig.u32_newPcmDataFlagAddr = SRAM_MODULE_SHARE_AUDIO_PCM;
    st_audioConfig.u8_channel = 2;

    HAL_USB_InitOTG(HAL_USB_PORT_0);

    HAL_NV_Init();

    USBH_MountUSBDisk();
    
    HAL_MP3EncodePcmInit(&st_audioConfig);

    uint32_t u32_audioSampleRate=0xf;
    uint32_t u32_audioSampleRateTmp=0;
    volatile uint32_t *pu32_newAudioSampleRate=(uint32_t *)(SRAM_MODULE_SHARE_AUDIO_RATE);
    *pu32_newAudioSampleRate=0xf;
    
    /* We should never get here as control is now taken by the scheduler */
    for( ;; )
    {
        HAL_HDMI_RX_GetAudioSampleRate(HAL_HDMI_RX_1,&u32_audioSampleRate);
        if ((*pu32_newAudioSampleRate) != u32_audioSampleRate)
        {
            u32_audioSampleRateTmp++;
            if (u32_audioSampleRateTmp >3)
            {
                u32_audioSampleRateTmp=0;
                *pu32_newAudioSampleRate = u32_audioSampleRate;
                HAL_MP3EncodePcmUnInit();
                if (2 == u32_audioSampleRate)
                {                    
                    st_audioConfig.e_samplerate = HAL_MP3_ENCODE_48000;
                    dlog_info("Audio Sample Rate 48000");
                }
                else if (0 == u32_audioSampleRate)
                {
                    st_audioConfig.e_samplerate = HAL_MP3_ENCODE_44100;   
                    dlog_info("Audio Sample Rate 44100");                 
                }
                HAL_MP3EncodePcmInit(&st_audioConfig);
            }                         
        }
        else
        {
            u32_audioSampleRateTmp=0;
        }
        //HAL_USB_HostProcess();
        HAL_MP3EncodePcm();
        SYS_EVENT_Process();
        DLOG_Process(NULL);
    }
} 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
