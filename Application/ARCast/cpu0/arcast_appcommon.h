#ifndef __ARCAST_APPCOMMON_H__
#define __ARCAST_APPCOMMON_H__

typedef struct 
{
    uint8_t  u8_headArray[6];
    uint16_t u16_version;
    uint16_t u16_audioSamplerate;
    uint16_t u16_videoHight;
    uint16_t u16_videoWidth;        
    uint8_t  u8_videoFrameRate;    
    uint8_t  u8_audioOnOrOff;
    uint8_t  u8_audioChannel;
    uint8_t  u8_audioBitRate;
    uint8_t  u8_reserveArray[6];
}STRU_ARCAST_AVFORMAT;

void Common_AVFORMATSysEventGroundInit(void);
void Common_AVFORMATSysEventSKYInit(void);

#endif