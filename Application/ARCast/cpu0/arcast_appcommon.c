#include <stdint.h>
#include <string.h>

#include "debuglog.h"
#include "sys_event.h"
#include "memory_config.h"

#include "hal_ret_type.h"
#include "hal_encodemp3.h"
#include "hal_softi2s.h"
#include "hal_sram.h"
#include "hal_usb_device.h"
#include "hal_bb.h"
#include "hal.h"
#include "arcast_appcommon.h"

#define ARCAST_DEBUGE

#ifdef ARCAST_DEBUGE
#define DLOG_INFO(...) dlog_info(__VA_ARGS__)
#else
#define DLOG_INFO(...)
#endif

static STRU_ARCAST_AVFORMAT_SKY_TO_GROUND g_st_formatChange;
static STRU_ARCAST_AVFORMAT_SKY_TO_GROUND g_st_formatChangeTmp;
static STRU_ARCAST_AVFORMAT_USB_SEND      g_st_usbSendFormat;

static int8_t UsbSendFormat(STRU_ARCAST_AVFORMAT_SKY_TO_GROUND *p_format)
{
    g_st_usbSendFormat.u8_headArray[0] = 'a';
    g_st_usbSendFormat.u8_headArray[1] = 'r';
    g_st_usbSendFormat.u8_headArray[2] = 'c';
    g_st_usbSendFormat.u8_headArray[3] = 'a';
    g_st_usbSendFormat.u8_headArray[4] = 's';
    g_st_usbSendFormat.u8_headArray[5] = 't';
    g_st_usbSendFormat.u16_version = 0x0001;
    g_st_usbSendFormat.u8_audioOnOrOff = 1;
    g_st_usbSendFormat.u8_audioChannel = 2;
    g_st_usbSendFormat.u8_audioBitRate = 128;
    g_st_usbSendFormat.u16_audioSamplerate = p_format->u16_audioSamplerate;
    g_st_usbSendFormat.u16_videoHight = p_format->u16_videoHight;
    g_st_usbSendFormat.u16_videoWidth = p_format->u16_videoWidth;
    g_st_usbSendFormat.u8_videoFrameRate = p_format->u8_videoFrameRate;
    
	HAL_USB_CustomerSendData((uint8_t*)(&g_st_usbSendFormat), sizeof(STRU_ARCAST_AVFORMAT_USB_SEND));
}

static void rcvFormatHandler_ground(void *p)
{
    uint32_t u32_rcvLen = 0;
    uint32_t u32_sizeSTRU_ARCAST_AVFORMAT = sizeof(STRU_ARCAST_AVFORMAT_SKY_TO_GROUND);
    HAL_BB_UartComReceiveMsg(BB_UART_COM_SESSION_3, (uint8_t*)(&g_st_formatChangeTmp), u32_sizeSTRU_ARCAST_AVFORMAT, &u32_rcvLen);

    UsbSendFormat(&g_st_formatChangeTmp);


    DLOG_INFO("u16_audioSamplerate %d ",g_st_formatChangeTmp.u16_audioSamplerate);
    DLOG_INFO("u16_videoHight %d ",g_st_formatChangeTmp.u16_videoHight);
    DLOG_INFO("u16_videoWidth %d ",g_st_formatChangeTmp.u16_videoWidth);  
    DLOG_INFO("u8_videoFrameRate %d ",g_st_formatChangeTmp.u8_videoFrameRate);
}

static void rcvFormatHandler_sky(void *p)
{
    uint32_t u32_rcvLen = 0;
    uint32_t u32_sizeSTRU_ARCAST_AVFORMAT = sizeof(STRU_ARCAST_AVFORMAT_SKY_TO_GROUND);

    HAL_BB_UartComReceiveMsg(BB_UART_COM_SESSION_3, (uint8_t*)(&g_st_formatChangeTmp), u32_sizeSTRU_ARCAST_AVFORMAT, &u32_rcvLen);
    DLOG_INFO("re-send format info");
    HAL_BB_UartComSendMsg(BB_UART_COM_SESSION_3, (uint8_t*)(&g_st_formatChange), u32_sizeSTRU_ARCAST_AVFORMAT);
}

void Common_AVFORMAT_VideoSysEventCallBack(void* p)
{
    
    if ((g_st_formatChange.u8_videoFrameRate  != ((STRU_SysEvent_H264InputFormatChangeParameter*)p)->framerate) ||
        (g_st_formatChange.u16_videoHight  != ((STRU_SysEvent_H264InputFormatChangeParameter*)p)->hight) || 
        (g_st_formatChange.u16_videoWidth  != ((STRU_SysEvent_H264InputFormatChangeParameter*)p)->width))
    {   
        g_st_formatChange.u16_videoHight  = ((STRU_SysEvent_H264InputFormatChangeParameter*)p)->hight;
        g_st_formatChange.u16_videoWidth  = ((STRU_SysEvent_H264InputFormatChangeParameter*)p)->width;
        g_st_formatChange.u8_videoFrameRate  = ((STRU_SysEvent_H264InputFormatChangeParameter*)p)->framerate;
        DLOG_INFO("video width=%d hight=%d framerate=%d ",  g_st_formatChange.u16_videoWidth, 
                                                            g_st_formatChange.u16_videoHight, 
                                                            g_st_formatChange.u8_videoFrameRate);
        
        if ((0 != g_st_formatChange.u16_videoWidth) || (0 != g_st_formatChange.u16_videoHight) || (0 != g_st_formatChange.u8_videoFrameRate))
        {
            HAL_BB_UartComSendMsg(BB_UART_COM_SESSION_3, (uint8_t*)(&g_st_formatChange), sizeof(STRU_ARCAST_AVFORMAT_SKY_TO_GROUND)); 
        }
    }
}

void Common_AVFORMAT_AudioSysEventCallBack(void* p)
{
    HAL_MP3EncodePcmUnInit();
    STRU_MP3_ENCODE_CONFIGURE_WAVE st_audioConfig;
    if (HAL_SOFTI2S_ENCODE_IEC_48000 == ((STRU_SysEvent_AudioInputChangeParameter*)p)->u8_audioSampleRate)
    {                    
        st_audioConfig.e_samplerate = HAL_MP3_ENCODE_48000;
        g_st_formatChange.u16_audioSamplerate = 48000;
        DLOG_INFO("Audio Sample Rate 48000");
    }
    else if (HAL_SOFTI2S_ENCODE_IEC_44100 == ((STRU_SysEvent_AudioInputChangeParameter*)p)->u8_audioSampleRate)
    {
        st_audioConfig.e_samplerate = HAL_MP3_ENCODE_44100; 
        g_st_formatChange.u16_audioSamplerate = 44100;  
        DLOG_INFO("Audio Sample Rate 44100");                 
    }
    
    st_audioConfig.e_modes = HAL_MP3_ENCODE_STEREO;
    st_audioConfig.u32_rawDataAddr = AUDIO_DATA_START;
    st_audioConfig.u32_rawDataLenght = AUDIO_DATA_BUFF_SIZE;
    st_audioConfig.u32_encodeDataAddr = MPE3_ENCODER_DATA_ADDR;
    st_audioConfig.u32_newPcmDataFlagAddr = SRAM_MODULE_SHARE_AUDIO_PCM;
    st_audioConfig.u8_channel = 2;
    HAL_MP3EncodePcmInit(&st_audioConfig, ENUM_HAL_SRAM_DATA_PATH_REVERSE);
        
    HAL_BB_UartComSendMsg(BB_UART_COM_SESSION_3, (uint8_t*)(&g_st_formatChange), sizeof(STRU_ARCAST_AVFORMAT_SKY_TO_GROUND));
}


void Common_AVFORMATSysEventSKYInit(void)
{
    
    g_st_formatChange.u16_audioSamplerate = 48000;
    g_st_formatChange.u16_videoHight = 720;
    g_st_formatChange.u16_videoWidth = 1280;     
    g_st_formatChange.u8_videoFrameRate = 60;  

    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_H264_INPUT_FORMAT_CHANGE, Common_AVFORMAT_VideoSysEventCallBack);
    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_AUDIO_INPUT_CHANGE, Common_AVFORMAT_AudioSysEventCallBack);
    
    HAL_BB_UartComRemoteSessionInit();        
    HAL_BB_UartComRegisterSession(BB_UART_COM_SESSION_3, rcvFormatHandler_sky);

/*    
    HAL_BB_UartComSendMsg(BB_UART_COM_SESSION_3, (uint8_t*)(&g_st_formatChange), sizeof(STRU_ARCAST_AVFORMAT_SKY_TO_GROUND));
    
    while (0 != memcmp(&g_st_formatChangeTmp, &g_st_formatChange, u32_sizeSTRU_ARCAST_AVFORMAT))
    {
        HAL_BB_UartComSendMsg(BB_UART_COM_SESSION_3, (uint8_t*)(&g_st_formatChange), u32_sizeSTRU_ARCAST_AVFORMAT);
        HAL_Delay(100);
    }
*/

}

void Common_AVFORMATSysEventGroundInit(void)
{
    uint32_t u32_sizeSTRU_ARCAST_AVFORMAT = sizeof(STRU_ARCAST_AVFORMAT_SKY_TO_GROUND);

    g_st_formatChange.u16_audioSamplerate = 48000;
    g_st_formatChange.u16_videoHight = 720;
    g_st_formatChange.u16_videoWidth = 1280;
    g_st_formatChange.u8_videoFrameRate = 60;

    HAL_BB_UartComRemoteSessionInit();        
    HAL_BB_UartComRegisterSession(BB_UART_COM_SESSION_3, rcvFormatHandler_ground);
        
    HAL_BB_UartComSendMsg(BB_UART_COM_SESSION_3, (uint8_t*)(&g_st_formatChange), u32_sizeSTRU_ARCAST_AVFORMAT);

/*    while (0 != memcmp((void *)(&g_st_formatChangeTmp), (void *)(&g_st_formatChange), u32_sizeSTRU_ARCAST_AVFORMAT))
    {
        HAL_BB_UartComSendMsg(BB_UART_COM_SESSION_3, (uint8_t*)(&g_st_formatChange), u32_sizeSTRU_ARCAST_AVFORMAT);
        HAL_Delay(100);
    }*/    

}
