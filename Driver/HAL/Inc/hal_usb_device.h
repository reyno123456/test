/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_usb_device.h
Description: The external HAL APIs to use the SRAM.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/12/22
History: 
        0.0.1    2016/12/22    The initial version of hal_usb_device.h
*****************************************************************************/

#ifndef __HAL_USB_DEVICE_H__
#define __HAL_USB_DEVICE_H__

#include <stdint.h>
#include "hal_ret_type.h"



typedef enum
{
    HAL_USB_DEVICE_PORT_0 = 0,
    HAL_USB_DEVICE_PORT_1,
} ENUM_HAL_USB_DEVICE_PORT;


/**
* @brief  config the usb as device controller
* @param  e_usbPort         usb port number: 0 or 1
* @retval   void
* @note  
*/
void HAL_USB_InitDevice(ENUM_HAL_USB_DEVICE_PORT e_usbPort);

/**
* @brief  reset the usb device controller for usb device hotplug
* @param  void * p         for sys event call-back
* @retval   void
* @note  
*/
void HAL_USB_ResetDevice(void * p);

/**
* @brief  send video data to the host
* @param  uint8_t    *buff                the buffer to send
*               uint32_t    u32_len          buffer length 
* @retval   void
* @note
*/
HAL_RET_T HAL_USB_DeviceSendVideo(uint8_t *buff, uint32_t u32_len);

/**
* @brief  send control data to the host
* @param  uint8_t    *buff                the buffer to send
*               uint32_t    u32_len          buffer length 
* @retval   void
* @note
*/
HAL_RET_T HAL_USB_DeviceSendCtrl(uint8_t *buff, uint32_t u32_len);

/**
* @brief   register the user callback function to receive host data
* @param  void (*pUsrFunc)(void *)     user callback function
* @retval   void
* @note
*/
void HAL_USB_RegisterUserProcess(void (*pUsrFunc)(void *));


#endif


