#ifndef __USBH_UVC_H
#define __USBH_UVC_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "usbh_core.h"


#define UVC_CLASS                   14


extern USBH_ClassTypeDef            UVC_Class;
#define USBH_UVC_CLASS             &UVC_Class


#define UVC_SET_CUR                         0x01
#define UVC_GET_CUR                         0x81
#define UVC_GET_MIN                         0x82
#define UVC_GET_MAX                         0x83
#define UVC_GET_RES                         0x84
#define UVC_GET_LEN                         0x85
#define UVC_GET_INFO                        0x86
#define UVC_GET_DEF                         0x87


#define UVC_VIDEO_EP                        0x81
#define UVC_VIDEO_EP_MAX_SIZE               0x400
#define UVC_CTRL_EP                         0x83

#define UVC_VIDEO_MAX_SIZE_PER_SOF          0xC00

#define UVC_VIDEO_BUFF_FRAME_NUM            0x2
//#define UVC_VIDEO_BUFF_FRAME_SIZE           76800
#define UVC_VIDEO_BUFF_FRAME_SIZE           153600

//#define UVC_VIDEO_BUFF_SIZE_PER_FRAME       38400   // 160 * 120 * 2

#define UVC_HEADER_SPECIAL_CHAR             0xC
#define UVC_HEADER_SIZE                     0xC

#define UVC_HEADER_FRAME_START              0x01
#define UVC_HEADER_FRAME_END                0x02


#define USB_DESC_TYPE_CS_INTERFACE          0x24

#define USB_UVC_FORMAT_UNCOMPRESSED         0x04
#define USB_UVC_FRAME_UNCOMPRESSED          0x05

#define USB_UVC_PROCESSING_UNIT             0x05
#define USB_UVC_EXTENSION_UNIT              0x06


#define USB_UVC_SUBCLASS_VIDEO_CONTROL      0x01
#define USB_UVC_SUBCLASS_VIDEO_STREAMING    0x02


#define USB_UVC_MAX_FRAME_FORMATS_NUM       20

#define USB_UVC_RECV_BUFFER_ADDR            0x21000000
#define USB_UVC_VIDEO_BUFFER_ADDR           0x21000800

#define USB_UVC_PARAM_BRIGHTNESS                0x0001
#define USB_UVC_PARAN_CONTRAST                  0x0002
#define USB_UVC_PARAN_HUE                       0x0004
#define USB_UVC_PARAN_SATURATION                0x0008
#define USB_UVC_PARAN_SHARPNESS                 0x0010
#define USB_UVC_PARAN_GAMMA                     0x0020
#define USB_UVC_PARAN_WHITE_BALANCE_TEMP        0x0040
#define USB_UVC_PARAN_WHITE_BALANCE_COMP        0x0080
#define USB_UVC_PARAN_BACKLIGHT_COMP            0x0100
#define USB_UVC_PARAN_GAIN                      0x0200
#define USB_UVC_PARAN_PWR_LINE_FREQ             0x0400
#define USB_UVC_PARAN_HUE_AUTO                  0x0800
#define USB_UVC_PARAN_WHITE_BALANCE_TEMP_AUTO   0x1000
#define USB_UVC_PARAN_WHITE_BALANCE_COMP_AUTO   0x2000
#define USB_UVC_PARAN_DIGITAL_MULTI             0x4000
#define USB_UVC_PARAN_DIGITAL_MULTI_LIMIT       0x8000

typedef enum
{
    UVC_STATE_SET_INTERFACE = 0,
    UVC_STATE_PROBE,
    UVC_STATE_COMMIT,
    UVC_STATE_VIDEO_PLAY,
    UVC_STATE_ERROR,
    UVC_PROBE_STATE_GET_CUR,
    UVC_PROBE_STATE_GET_MAX,
    UVC_PROBE_STATE_GET_MIN,
    UVC_PROBE_STATE_SET_CUR,
    UVC_PROBE_STATE_GET_CUR_COMMIT,
    UVC_PROBE_STATE_SET_CUR_COMMIT,
    UVC_RECV_CTRL_DATA,
    UVC_RECV_CTRL_OK,
}
UVC_StateTypeDef;

typedef enum
{
    UVC_PARAM_TYPE_GET_CUR   = 0x81,
    UVC_PARAM_TYPE_GET_MIN   = 0x82,
    UVC_PARAM_TYPE_GET_MAX   = 0x83,
    UVC_PARAM_TYPE_GET_RES   = 0x84,
    UVC_PARAM_TYPE_GET_LEN   = 0x85,
    UVC_PARAM_TYPE_GET_INFO  = 0x86,
    UVC_PARAM_TYPE_GET_DEF   = 0x87,    
} UVC_GetParamTypeDef;



typedef enum
{
    UVC_VIDEO_BUFF_EMPTY = 0,
    UVC_VIDEO_BUFF_VALID,
    UVC_VIDEO_BUFF_IN_USING,
}
UVC_BuffStateTypeDef;



typedef struct _UVC_Process
{
    uint8_t                     CtrlPipe;
    uint8_t                     CtrlEp;
    uint16_t                    CtrlEpSize;
    uint8_t                     VideoPipe;
    uint8_t                     VideoEp;
    uint16_t                    VideoEpSize;
    UVC_StateTypeDef            uvc_state;
    UVC_StateTypeDef            uvc_probeState;
    UVC_GetParamTypeDef         uvc_getParamState;
    uint16_t                    uvc_CSCount;
    uint8_t                     probeCount;
    uint8_t                     interface;
    uint8_t                     u8_selFrameIndex;
    uint8_t                     u8_selInterface;
    uint8_t                     u8_selAltInterface;
}
UVC_HandleTypeDef;


typedef struct _UVCInterfaceAssociation
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bFirstInterface;
    uint8_t  bInterfaceCount;
    uint8_t  bFunctionClass;
    uint8_t  bFunctionSubClass;
    uint8_t  bFunctionProtocol;
    uint8_t  iFunction;
} USBH_UVCInterfaceAssociationDescriptor;


typedef struct _UVCInterface
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bInterfaceNumber;
    uint8_t  bAlternateSetting;
    uint8_t  bNumEndpoints;
    uint8_t  bInterfaceClass;
    uint8_t  bInterfaceSubClass;
    uint8_t  bInterfaceProtocol;
    uint8_t  iInterface;
} USBH_UVCInterfaceDescriptor;


typedef struct _VCProcessingUnitInterface
{
    uint8_t    bLength;
    uint8_t    bDescriptorType;
    uint8_t    bDescriptorSubtype;
    uint8_t    bUnitID;
    uint8_t    bSourceID;
    uint16_t   wMaxMultiplier;
    uint8_t    bControlSize;
    uint8_t    bmControls[4];
    uint8_t    iProcessing;
    uint8_t    bmVideoStandards;
} USBH_VCProcessingUnitInterfaceDescriptor;


typedef struct _VCExtensionUnitInterface
{
    uint8_t    bLength;
    uint8_t    bDescriptorType;
    uint8_t    bDescriptorSubtype;
    uint8_t    bUnitID;
    uint8_t    guidExtensionCode[16];
    uint8_t    bNumControls;
    uint8_t    bNrInPins;
    uint8_t    baSourceID[4];
    uint8_t    bControlSize;
    uint8_t    bmControls[4];
    uint8_t    iExtension;
} USBH_VCExtensionUnitInterfaceDescriptor;


typedef struct _UVCFormatUncompressedTypeDef
{
    uint8_t  bLength;
    uint8_t  bDescriptorType;
    uint8_t  bDescriptorSubtype;
    uint8_t  bFormatIndex;
    uint8_t  bNumFrameDescriptors;
    uint8_t  guidFormat[16];
    uint8_t  bBitsPerPixel;
    uint8_t  bDefaultFrameIndex;
    uint8_t  bAspectRatioX;
    uint8_t  bAspectRatioY;
    uint8_t  bmInterlaceFlags;
    uint8_t  bCopyProtect;
} USBH_UVCFormatUncompressedTypeDef;


typedef struct _UVCFrameUncompressedTypeDef
{
    uint8_t   bLength;
    uint8_t   bDescriptorType;
    uint8_t   bDescriptorSubtype;
    uint8_t   bFrameIndex;
    uint8_t   bmCapabilities;
    uint16_t  wWidth;
    uint16_t  wHeight;
    uint32_t  dwMinBitRate;
    uint32_t  dwMaxBitRate;
    uint32_t  dwMaxVideoFrameBufferSize;
    uint32_t  dwDefaultFrameInterval;
    uint8_t   bFrameIntervalType;
    uint32_t  dwFrameInterval;
} USBH_UVCFrameUncompressedTypeDef;


typedef uint32_t (*USBH_UVC_PARAM_HANDLER)(USBH_HandleTypeDef *phost, UVC_GetParamTypeDef paramType);


static USBH_StatusTypeDef USBH_UVC_InterfaceInit (USBH_HandleTypeDef *phost);
USBH_StatusTypeDef USBH_UVC_InterfaceDeInit (USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_UVC_ClassRequest(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_UVC_Process (USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_UVC_SOFProcess (USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_UVC_GetCSParam(USBH_HandleTypeDef *phost);

static uint32_t USBH_UVC_GetBrightness(USBH_HandleTypeDef *phost,
                                      UVC_GetParamTypeDef paramType);
static uint32_t USBH_UVC_GetContrast(USBH_HandleTypeDef *phost,
                                    UVC_GetParamTypeDef paramType);
static uint32_t USBH_UVC_GetHUE(USBH_HandleTypeDef *phost,
                                 UVC_GetParamTypeDef paramType);
static uint32_t USBH_UVC_GetSaturation(USBH_HandleTypeDef *phost,
                                      UVC_GetParamTypeDef paramType);
static uint32_t USBH_UVC_GetSharpness(USBH_HandleTypeDef *phost,
                                      UVC_GetParamTypeDef paramType);
static uint32_t USBH_UVC_GetGamma(USBH_HandleTypeDef *phost,
                                   UVC_GetParamTypeDef paramType);
static uint32_t USBH_UVC_GetWhiteTemp(USBH_HandleTypeDef *phost,
                                      UVC_GetParamTypeDef paramType);
static uint32_t USBH_UVC_GetBack(USBH_HandleTypeDef *phost,
                                  UVC_GetParamTypeDef paramType);
static uint32_t USBH_UVC_GetPowerLine(USBH_HandleTypeDef *phost,
                                      UVC_GetParamTypeDef paramType);
static USBH_StatusTypeDef USBH_UVC_Probe(USBH_HandleTypeDef *phost);
static USBH_StatusTypeDef USBH_UVC_Commit(USBH_HandleTypeDef *phost);
uint8_t USBH_UVC_StartView(USBH_HandleTypeDef *phost, uint8_t u8_frameIndex);
static void USBH_UVC_UrbDone(USBH_HandleTypeDef *phost);
uint8_t *USBH_UVC_GetBuff(uint32_t *frameNumber, uint32_t *frameSize);
void USBH_UVC_SetBuffStateByAddr(uint8_t *u8_buffAddr, UVC_BuffStateTypeDef e_buffState);
static void USBH_UVC_SetBuffState(uint8_t u8_buffIndex, UVC_BuffStateTypeDef e_buffState);
static UVC_BuffStateTypeDef USBH_UVC_GetBuffState(uint8_t u8_buffIndex);
static void USBH_ParseFormatUncompDesc(uint8_t *buf, uint8_t index);
static void USBH_ParseFrameUncompDesc(uint8_t *buf, uint8_t index);
void USBH_UVC_GetVideoFormatList(USBH_HandleTypeDef *phost);
static uint8_t USBH_UVC_FindStreamingInterface(USBH_HandleTypeDef *phost);
uint16_t USBH_UVC_GetFrameWidth(uint8_t index);
uint16_t USBH_UVC_GetFrameHeight(uint8_t index);
uint8_t USBH_UVC_GetFrameIndex(uint8_t index);
uint32_t USBH_UVC_GetFrameSize(uint8_t frameIndex);
uint32_t USBH_UVC_GetProcUnitControls(void);
uint32_t USBH_UVC_GetExtUnitControls(void);
uint32_t USBH_UVC_ProcUnitParamHandler(USBH_HandleTypeDef *phost, uint8_t index, UVC_GetParamTypeDef enParamType);
static void USBH_UVC_SetBuffNumber(uint8_t u8_buffIndex, uint32_t u32_bufferNum);
static uint32_t USBH_UVC_GetBuffNumber(uint8_t u8_buffIndex);


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

