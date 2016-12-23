/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_usb.h
Description: The external HAL APIs to use the SRAM.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/12/22
History: 
        0.0.1    2016/12/22    The initial version of hal_usb.h
*****************************************************************************/

#ifndef __HAL_USB_H__
#define __HAL_USB_H__

#include <stdint.h>
#include "hal_ret_type.h"


typedef enum
{
    HAL_USB_PORT_0 = 0,
    HAL_USB_PORT_1,
} ENUM_HAL_USB_PORT;


typedef enum
{
    HAL_USB_STATE_IDLE = 0,
    HAL_USB_STATE_READY,
    HAL_USB_STATE_DISCONNECT,
} ENUM_HAL_USB_STATE;


/**
* @brief  Set the USB Host State for Application use.
* @param  e_usbHostAppState             indicate the usb host state
* @retval   void
* @note  
*/
void HAL_USB_SetHostAppState(ENUM_HAL_USB_STATE e_usbHostAppState);

/**
* @brief  Get the USB Host State for Application use.
* @param  void
* @retval   HAL_USB_STATE_IDLE                indicate the usb is IDLE
*               HAL_USB_STATE_READY             indicate the usb is READY
*               HAL_USB_STATE_DISCONNECT   indicate the usb is DISCONNECT
* @note  
*/
ENUM_HAL_USB_STATE HAL_USB_GetHostAppState(void);

/**
* @brief  polling the usb state-machine 
* @param  void
* @retval   void
* @note  
*/
void HAL_USB_HostProcess(void);

/**
* @brief  config the usb as host controller
* @param  e_usbPort         usb port number: 0 or 1
* @retval   void
* @note  
*/
void HAL_USB_InitHost(ENUM_HAL_USB_PORT e_usbPort);

/**
* @brief  config the usb as device controller
* @param  e_usbPort         usb port number: 0 or 1
* @retval   void
* @note  
*/
void HAL_USB_InitDevice(ENUM_HAL_USB_PORT e_usbPort);

/**
* @brief  reset the usb device controller for usb device hotplug
* @param  void * p         for sys event call-back
* @retval   void
* @note  
*/
void HAL_USB_ResetDevice(void * p);

/**
* @brief  start the USB Video for Application use
* @param  void
* @retval   void
* @note  
*/
void HAL_USB_StartUVC(void);


#endif
