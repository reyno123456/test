#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hal_ret_type.h"
#include "hal_encodemp3.h"
#include "debuglog.h"
#include "systicks.h"
#include "cpu_info.h"
#include "dma.h"
#include "hal_sram.h"
#include "layer3.h"

        
   
shine_t        st_s;
uint32_t u32_rawDataLenght;
uint32_t u32_rawDataAddr;
uint32_t u32_encodeDataAddr;
int32_t s32_samples_per_pass;
uint32_t u32_frameSize;
volatile uint32_t *pu32_newPcmDataFlagAddr;

HAL_BOOL_T HAL_MP3EncodePcmInit(const STRU_MP3_ENCODE_CONFIGURE_WAVE *st_mp3EncodeConfg)
{          
    shine_config_t st_config; 
    pu32_newPcmDataFlagAddr=(uint32_t *)(st_mp3EncodeConfg->u32_newPcmDataFlagAddr);
    (*pu32_newPcmDataFlagAddr) = 0;

    u32_rawDataLenght = (st_mp3EncodeConfg->u32_rawDataLenght);
    u32_rawDataAddr = st_mp3EncodeConfg->u32_rawDataAddr;
    u32_encodeDataAddr = st_mp3EncodeConfg->u32_encodeDataAddr;
    shine_set_config_mpeg_defaults(&(st_config.mpeg));

    st_config.wave.channels = st_mp3EncodeConfg->u8_channel;
    st_config.wave.samplerate = st_mp3EncodeConfg->e_samplerate;
    st_config.mpeg.mode = st_mp3EncodeConfg->e_modes;
    st_config.mpeg.bitr = 64;
    st_s = shine_initialise(&st_config);
    if (NULL == st_s)
    {
        return HAL_FALSE;
    }

    s32_samples_per_pass = shine_samples_per_pass(st_s);
    u32_frameSize = sizeof(short int) * s32_samples_per_pass * st_config.wave.channels; 

    HAL_SRAM_EnableSkyBypassVideo(HAL_SRAM_VIDEO_CHANNEL_1);

    return  HAL_TRUE;
}

void HAL_MP3EncodePcm(void)
{   
    if (0 != (*pu32_newPcmDataFlagAddr))
    {
        uint32_t u32_tmpRawDataLenght = u32_rawDataLenght/2;
        uint32_t u32_tmpEncodeDataAddr = u32_encodeDataAddr+(113*u32_frameSize)*(*pu32_newPcmDataFlagAddr-1);
        uint32_t u32_tmpRawDataAddr = u32_rawDataAddr+0x7F200*(*pu32_newPcmDataFlagAddr-1);
        int s32_encodeLenght = 0;
        uint8_t  *pu8_data = NULL;
        uint16_t *pu16_rawDataAddr = (uint16_t *)(u32_tmpRawDataAddr);
        uint8_t  *pu8_encodeDataAddr = (uint8_t *)(u32_tmpEncodeDataAddr);
        uint32_t tick=0;
        tick = SysTicks_GetTickCount();
        while (u32_tmpRawDataLenght)
        {
            
            pu8_data = shine_encode_buffer_interleaved(st_s, pu16_rawDataAddr, &s32_encodeLenght);         
            memcpy((uint8_t *)0xB1800000,pu8_data,s32_encodeLenght);
            
            u32_tmpRawDataLenght -= u32_frameSize;                     
            u32_tmpRawDataAddr+= u32_frameSize;            
            pu16_rawDataAddr = (uint16_t *)(u32_tmpRawDataAddr);
        }
        dlog_info("encode mp3 ok %d %d \n", SysTicks_GetTickCount()-tick,(*pu32_newPcmDataFlagAddr));
        (*pu32_newPcmDataFlagAddr) = 0;
        

    }    
}