#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hal_ret_type.h"
#include "hal_encodemp3.h"
#include "hal_softi2s.h"
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
volatile uint32_t g_u32_dstAddress=MPE3_ENCODER_DATA_ADDR;

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
    st_s = shine_initialise(&st_config);
    if (NULL == st_s)
    {
        return HAL_FALSE;
    }

    s32_samples_per_pass = shine_samples_per_pass(st_s);
    u32_frameSize = sizeof(short int) * s32_samples_per_pass * st_config.wave.channels; 

    HAL_SRAM_EnableSkyBypassVideo(HAL_SRAM_VIDEO_CHANNEL_1);
    dlog_info("encode mp3 init  %x %x %x %d\n", u32_rawDataLenght,u32_rawDataAddr,u32_encodeDataAddr,u32_frameSize);
    return  HAL_TRUE;
}

HAL_BOOL_T HAL_MP3EncodePcmUnInit(void)
{
    shine_close(st_s);
    dlog_info("encode mp3 uninit");
}


void HAL_MP3EncodePcm(void)
{   
    if (0 != (*pu32_newPcmDataFlagAddr))
    {
        uint32_t u32_tmpRawDataLenght = u32_rawDataLenght;
        uint32_t u32_tmpEncodeDataAddr = u32_encodeDataAddr+(AUDIO_DATA_BUFF_COUNT*u32_frameSize)*(*pu32_newPcmDataFlagAddr-1);
        uint32_t u32_tmpRawDataAddr = u32_rawDataAddr+AUDIO_DATA_BUFF_SIZE*(*pu32_newPcmDataFlagAddr-1);
        int s32_encodeLenght = 0;
        uint8_t  *pu8_data = NULL;
        uint16_t *pu16_rawDataAddr = (uint16_t *)(u32_tmpRawDataAddr);
        uint8_t  *pu8_encodeDataAddr = (uint8_t *)(u32_tmpEncodeDataAddr);
        uint32_t tick=0;
        uint32_t i=0;
        uint8_t ch[AUDIO_DATA_BUFF_COUNT*420]={0};

        /*if ((MPE3_ENCODER_DATA_ADDR+192*5418) <= g_u32_dstAddress)
        {
            g_u32_dstAddress=MPE3_ENCODER_DATA_ADDR;
        }*/
        
		//tick = SysTicks_GetTickCount();
        while (u32_tmpRawDataLenght)
        {
            
            pu8_data = shine_encode_buffer_interleaved(st_s, pu16_rawDataAddr, &s32_encodeLenght);                   
            u32_tmpRawDataLenght -= u32_frameSize;                     
            u32_tmpRawDataAddr+= u32_frameSize;            
            pu16_rawDataAddr = (uint16_t *)(u32_tmpRawDataAddr);
            memcpy(&ch[i],pu8_data,s32_encodeLenght);
            i+=s32_encodeLenght;
        }
        
        //DMA_transfer((uint32_t)ch+DTCM_CPU0_DMA_ADDR_OFFSET, AUDIO_BYPASS_START,i, CHAN0, LINK_LIST_ITEM); 

        memcpy((uint8_t *)AUDIO_BYPASS_START,ch,i);                   
        //memcpy((uint8_t *)g_u32_dstAddress,ch,i);                    
        //dlog_info("encode mp3 ok %d %d %x\n", SysTicks_GetTickCount()-tick,(*pu32_newPcmDataFlagAddr),g_u32_dstAddress);
        
        g_u32_dstAddress+=i;
        (*pu32_newPcmDataFlagAddr) = 0;

    }    

}
