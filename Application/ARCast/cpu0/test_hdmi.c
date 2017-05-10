#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "debuglog.h"
#include "hal_hdmi_rx.h"
#include "hal_adc.h"

void command_hdmi(char *index)
{
    uint8_t u8_Index = strtoul(index, NULL, 0);
    switch (u8_Index)
    {
        case 0:
        {
            uint16_t u16_width;
            uint16_t u16_hight;
            uint8_t u8_framerate;
            uint32_t u32_sampleRate;

            HAL_HDMI_RX_GetAudioSampleRate(HAL_HDMI_RX_1, &u32_sampleRate);
            HAL_HDMI_RX_GetVideoFormat(HAL_HDMI_RX_1, &u16_width, &u16_hight, &u8_framerate);
            dlog_info("video width=%d u16_hight=%d u8_framerate=%d ", u16_width, u16_hight, u8_framerate);
            if (0 ==u32_sampleRate)
            {
                dlog_info("audio sampleRate=44P1K %d", u32_sampleRate);    
            }
            else if (2 ==u32_sampleRate)
            {
                dlog_info("audio sampleRate=48K   %d", u32_sampleRate);
            }
            else
            {
                dlog_info("audio sampleRate=unkown %d", u32_sampleRate);   
            }
            
            break;
        }
        case 1:
        {
            volatile uint16_t adc=HAL_ADC_Read(14);
            dlog_info("ADC14=%d", adc);
            adc=HAL_ADC_Read(15);
            dlog_info("ADC15=%d", adc);
            break;
        }
        default :
            return;
    }

}
