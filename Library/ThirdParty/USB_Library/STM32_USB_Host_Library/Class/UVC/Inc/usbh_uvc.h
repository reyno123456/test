#ifndef __USBH_UVC_H
#define __USBH_UVC_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "usbh_core.h"


#define UVC_CLASS             14


extern USBH_ClassTypeDef        UVC_Class;
#define USBH_UVC_CLASS         &UVC_Class


#define UVC_SET_CUR             0x01
#define UVC_GET_CUR             0x81
#define UVC_GET_MIN             0x82
#define UVC_GET_MAX             0x83
#define UVC_GET_RES             0x84
#define UVC_GET_LEN             0x85
#define UVC_GET_INFO            0x86
#define UVC_GET_DEF             0x87


typedef enum
{
    UVC_STATE_INIT = 0,
    UVC_STATE_IDLE,
    UVC_STATE_SET_INTERFACE1,
    UVC_STATE_SET_INTERFACE2,
    UVC_STATE_GET_CSPARAM,
    UVC_STATE_GET_BRIGHTNESS,
    UVC_STATE_GET_CONTRAST,
    UVC_STATE_GET_HUE,
    UVC_STATE_GET_SATUATION,
    UVC_STATE_GET_SHARPNESS,
    UVC_STATE_GET_GAMMA,
    UVC_STATE_GET_WHITE_BALANCE,
    UVC_STATE_GET_BACKLIGHT,
    UVC_STATE_GET_POWER_LINE,
    UVC_STATE_PROBE1,
    UVC_STATE_PROBE2,
    UVC_STATE_CS_FINISH,
    UVC_STATE_PROBE3,
    UVC_STATE_COMMIT,
    UVC_STATE_VIDEO_PLAY,
    UVC_STATE_VIDEO_WAIT,
    UVC_STATE_ERROR,
}
UVC_StateTypeDef;

typedef enum
{
    UVC_STATE_GET_LEN = 0,
    UVC_STATE_GET_INFO,
    UVC_STATE_GET_MIN,
    UVC_STATE_GET_MAX,
    UVC_STATE_GET_RES,
    UVC_STATE_GET_DEF,
}
UVC_GetCSParamStateTypeDef;


typedef enum
{
    UVC_PROBE_STATE_GET_CUR = 0,
    UVC_PROBE_STATE_GET_MAX,
    UVC_PROBE_STATE_GET_MIN,
    UVC_PROBE_STATE_SET_CUR,
    UVC_PROBE_STATE_SET_CUR_COMMIT,
}
UVC_ProbeStateTypeDef;



typedef struct _UVC_Process
{
    uint8_t                     CtrlPipe;
    uint8_t                     CtrlEp;
    uint16_t                    CtrlEpSize;
    uint8_t                     VideoPipe;
    uint8_t                     VideoEp;
    uint16_t                    VideoEpSize;
    UVC_StateTypeDef            uvc_state;
    UVC_GetCSParamStateTypeDef  uvc_getParamState;
    uint16_t                    uvc_CSCount;
    uint8_t                     mem[8];
    uint8_t                     cur_mem[28];
    uint8_t                     max_mem[28];
    uint8_t                     min_mem[28];
    uint8_t                     still_probe_cur_mem[12];
    uint8_t                     probeCount;
    uint8_t                     interface;
    uint8_t                     palying;
    uint32_t                    sofCount;
}
UVC_HandleTypeDef;


static USBH_StatusTypeDef USBH_UVC_InterfaceInit (USBH_HandleTypeDef *phost);
USBH_StatusTypeDef USBH_UVC_InterfaceDeInit (USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_UVC_ClassRequest(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_UVC_Process (USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_UVC_SOFProcess (USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_UVC_GetCSParam(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_UVC_GetBrightness(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_UVC_GetContrast(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_UVC_GetHUE(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_UVC_GetSaturation(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_UVC_GetSharpness(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_UVC_GetGamma(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_UVC_GetWhite(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_UVC_GetBack(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_UVC_GetPowerLine(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_UVC_Probe(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_UVC_StillProbe(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_UVC_Commit(USBH_HandleTypeDef *phost);
void USBH_UVC_StartView(USBH_HandleTypeDef *phost);


#endif /* __USBH_AUDIO_H */

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 

/**
  * @}
  */ 
/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

