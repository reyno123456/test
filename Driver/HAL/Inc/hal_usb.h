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




void HAL_USB_InitHost(ENUM_HAL_USB_PORT e_usbPort);
void HAL_USB_SetHostAppState(ENUM_HAL_USB_STATE e_usbHostAppState);
ENUM_HAL_USB_STATE HAL_USB_GetHostAppState(void);
void HAL_USB_HostProcess(void);
void HAL_USB_InitDevice(ENUM_HAL_USB_PORT e_usbPort);
void HAL_USB_ResetDevice(void * p);
void HAL_USB_StartUVC(void);



#endif

