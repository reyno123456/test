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


typedef enum
{
    HAL_USB_HOST_PORT_0 = 0,
    HAL_USB_HOST_PORT_1,
} ENUM_HAL_USB_HOST_PORT;


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
} ENUM_HAL_USB_HOST_CLASS;


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
void HAL_USB_InitHost(ENUM_HAL_USB_HOST_PORT e_usbPort, ENUM_HAL_USB_HOST_CLASS e_usbHostClass);

/**
* @brief  start the USB Video for Application use
* @param  void
* @retval   void
* @note  
*/
void HAL_USB_StartUVC(void);

/**
* @brief  get the latest frame buffer
* @param  uint8_t  *u8_buff    the dest buffer to storage the video frame
* @retval   HAL_USB_ERR_BUFF_IS_EMPTY   : means the buffer pool is empty
*               HAL_OK                                      : means successfully get one video frame
* @note  
*/
HAL_RET_T HAL_USB_GetVideoFrame(uint8_t *u8_buff);


#endif

