/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_usb.c
Description: The external HAL APIs to use the USB.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/12/22
History: 
        0.0.1    2016/12/21    The initial version of hal_usb.c
*****************************************************************************/

#include <stdint.h>
#include "hal_usb.h"
#include "interrupt.h"
#include "sys_event.h"
#include "usbh_def.h"
#include "usbh_core.h"
#include "usbh_uvc.h"
#include "usbh_msc.h"
#include "usbd_def.h"
#include "usbd_core.h"
#include "usbd_hid.h"
#include "usbd_hid_desc.h"
#include "sram.h"


static ENUM_HAL_USB_STATE   s_eUSBHostAppState;
USBH_HandleTypeDef          hUSBHost;
extern USBD_HandleTypeDef   USBD_Device;



void HAL_USB_SetHostAppState(ENUM_HAL_USB_STATE e_usbHostAppState)
{
    s_eUSBHostAppState = e_usbHostAppState;
}


ENUM_HAL_USB_STATE HAL_USB_GetHostAppState(void)
{
    return s_eUSBHostAppState;
}


void HAL_USB_HostProcess(void)
{
    USBH_Process(&hUSBHost);
}


static void USB_HostAppState(USBH_HandleTypeDef *phost, uint8_t id)
{
    switch(id)
    {
        case HOST_USER_SELECT_CONFIGURATION:
            break;

        case HOST_USER_DISCONNECTION:
            HAL_USB_SetHostAppState(HAL_USB_STATE_DISCONNECT);
            break;

        case HOST_USER_CLASS_ACTIVE:
            HAL_USB_SetHostAppState(HAL_USB_STATE_READY);
            break;

        case HOST_USER_CONNECTION:
            break;

        default:
            break;
    }

}


void HAL_USB_InitHost(ENUM_HAL_USB_PORT e_usbPort)
{
    if (HAL_USB_PORT_0 == e_usbPort)
    {
        reg_IrqHandle(OTG_INTR0_VECTOR_NUM, USB_LL_OTG0_IRQHandler);
    }
    else if (HAL_USB_PORT_1 == e_usbPort)
    {
        reg_IrqHandle(OTG_INTR1_VECTOR_NUM, USB_LL_OTG1_IRQHandler);
    }

    USBH_Init(&hUSBHost, USB_HostAppState, (uint8_t)e_usbPort);

    USBH_RegisterClass(&hUSBHost, USBH_MSC_CLASS);

    USBH_Start(&hUSBHost);
}


void HAL_USB_InitDevice(ENUM_HAL_USB_PORT e_usbPort)
{
    if (HAL_USB_PORT_0 == e_usbPort)
    {
        reg_IrqHandle(OTG_INTR0_VECTOR_NUM, USB_LL_OTG0_IRQHandler);
    }
    else if (HAL_USB_PORT_1 == e_usbPort)
    {
        reg_IrqHandle(OTG_INTR1_VECTOR_NUM, USB_LL_OTG1_IRQHandler);
    }

    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_USB_PLUG_OUT, HAL_USB_ResetDevice);

    USBD_Init(&USBD_Device, &HID_Desc, 0);

    USBD_RegisterClass(&USBD_Device, USBD_HID_CLASS);

    USBD_Start(&USBD_Device);

    return;
}


void HAL_USB_ResetDevice(void * p)
{
    USBD_LL_Init(&USBD_Device);
    HAL_PCD_Start(USBD_Device.pData);

    SRAM_Ready0Confirm();

    SRAM_Ready1Confirm();

    return;
}


void HAL_USB_StartUVC(void)
{
    USBH_UVC_StartView(&hUSBHost);
}


