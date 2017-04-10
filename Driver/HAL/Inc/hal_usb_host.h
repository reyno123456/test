/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_usb_host.h
Description: The external HAL APIs to use the SRAM.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/12/22
History: 
        0.0.1    2016/12/22    The initial version of hal_usb_host.h
*****************************************************************************/

#ifndef __HAL_USB_HOST_H__
#define __HAL_USB_HOST_H__


#include <stdint.h>
#include "hal_ret_type.h"
#include "hal_usb_otg.h"


#define HAL_USB_UVC_MAX_FRAME_FORMATS_NUM     20

typedef enum
{
    HAL_USB_HOST_STATE_IDLE = 0,
    HAL_USB_HOST_STATE_READY,
    HAL_USB_HOST_STATE_DISCONNECT,
} ENUM_HAL_USB_HOST_STATE;


typedef enum
{
    HAL_USB_HOST_CLASS_MSC = 0,
    HAL_USB_HOST_CLASS_UVC,
    HAL_USB_HOST_CLASS_NONE,
} ENUM_HAL_USB_HOST_CLASS;


typedef enum
{
    HAL_UVC_GET_LEN = 0,
    HAL_UVC_GET_INFO,
    HAL_UVC_GET_MIN,
    HAL_UVC_GET_MAX,
    HAL_UVC_GET_RES,
    HAL_UVC_GET_DEF,
} ENUM_HAL_UVC_GET_PARAM_TYPE;


typedef struct
{
    uint16_t    u16_width[HAL_USB_UVC_MAX_FRAME_FORMATS_NUM];
    uint16_t    u16_height[HAL_USB_UVC_MAX_FRAME_FORMATS_NUM];
    uint32_t    u32_sizePerFrame[HAL_USB_UVC_MAX_FRAME_FORMATS_NUM];
    uint8_t     u8_frameIndex[HAL_USB_UVC_MAX_FRAME_FORMATS_NUM];
} STRU_UVC_VIDEO_FRAME_FORMAT;


/**
* @brief  Set the USB Host State for Application use.
* @param  e_usbHostAppState             indicate the usb host state
* @retval   void
* @note  
*/
void HAL_USB_SetHostAppState(ENUM_HAL_USB_HOST_STATE e_usbHostAppState);

/**
* @brief  Get the USB Host State for Application use.
* @param  void
* @retval   HAL_USB_STATE_IDLE                indicate the usb is IDLE
*               HAL_USB_STATE_READY             indicate the usb is READY
*               HAL_USB_STATE_DISCONNECT   indicate the usb is DISCONNECT
* @note  
*/
ENUM_HAL_USB_HOST_STATE HAL_USB_GetHostAppState(void);

/**
* @brief  polling the usb state-machine 
* @param  void
* @retval   void
* @note  
*/
void HAL_USB_HostProcess(void);

/**
* @brief  config the usb as host controller
* @param  e_usbPort            usb port number: 0 or 1
*               e_usbHostClass    usb class, MSC or UVC
* @retval   void
* @note  
*/
void HAL_USB_InitHost(ENUM_HAL_USB_PORT e_usbPort);

/**
* @brief  start the USB Video for Application use
* @param  uint16_t u16_width                  user input frame width
*               uint16_t u16_height                 user input frame height
*               uint32_t *u32_frameSize          return the frame size
* @retval   void
* @note  
*/
HAL_RET_T HAL_USB_StartUVC(uint16_t u16_width,
                           uint16_t u16_height,
                           uint32_t *u32_frameSize);

/**
* @brief  get the latest frame buffer
* @param  uint8_t  *u8_buff    the dest buffer to storage the video frame
* @retval   HAL_USB_ERR_BUFF_IS_EMPTY   : means the buffer pool is empty
*               HAL_OK                                      : means successfully get one video frame
* @note  
*/
HAL_RET_T HAL_USB_GetVideoFrame(uint8_t *u8_buff, uint32_t *u32_frameNum, uint32_t *u32_frameSize);

/**
* @brief  get the formats this camera support
* @param  
* @retval   STRU_UVC_VIDEO_FRAME_FORMAT *stVideoFrameFormat
* @note  
*/
void HAL_USB_GetVideoFormats(STRU_UVC_VIDEO_FRAME_FORMAT *stVideoFrameFormat);


/**
* @brief  get the UVC Porc Unit Control Mask, such as HUE,Backlight, white balance and so on
* @param  void
* @retval  void
* @note  
*/
uint32_t HAL_USB_GetUVCProcUnitControls(void);


uint32_t HAL_USB_GetUVCExtUnitControls(void);


/**
* @brief    configure the USB Controller to enter into TEST MODE
* @param  void
* @retval   void
* @note  
*/
void HAL_USB_EnterUSBHostTestMode(void);

void HAL_USB_TransferUVCToGrd(uint8_t *buff, uint32_t dataLen);

ENUM_HAL_USB_HOST_CLASS HAL_USB_CurUsbClassType(void);



#endif

