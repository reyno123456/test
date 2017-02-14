/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_usb.c
Description: The external HAL APIs to use the USB Host.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/12/22
History: 
        0.0.1    2016/12/21    The initial version of hal_usb_host.c
*****************************************************************************/
#include "hal_usb_host.h"
#include "usbh_def.h"
#include "usbh_core.h"
#include "usbh_uvc.h"
#include "usbh_msc.h"
#include "interrupt.h"
#include "hal_nvic.h"


static ENUM_HAL_USB_HOST_STATE   s_eUSBHostAppState;
USBH_HandleTypeDef               hUSBHost;



/**
* @brief  Set the USB Host State for Application use.
* @param  e_usbHostAppState             indicate the usb host state
* @retval   void
* @note  
*/
void HAL_USB_SetHostAppState(ENUM_HAL_USB_HOST_STATE e_usbHostAppState)
{
    s_eUSBHostAppState = e_usbHostAppState;
}


/**
* @brief  Get the USB Host State for Application use.
* @param  void
* @retval   HAL_USB_STATE_IDLE                indicate the usb is IDLE
*               HAL_USB_STATE_READY             indicate the usb is READY
*               HAL_USB_STATE_DISCONNECT   indicate the usb is DISCONNECT
* @note  
*/
ENUM_HAL_USB_HOST_STATE HAL_USB_GetHostAppState(void)
{
    return s_eUSBHostAppState;
}


/**
* @brief  polling the usb state-machine 
* @param  void
* @retval   void
* @note  
*/
void HAL_USB_HostProcess(void)
{
    USBH_Process(&hUSBHost);
}


/**
* @brief  the entrance of usb state change, called when state change happened
* @param  phost       the handler of usb host
*               id            the usb host state
* @retval   void
* @note  
*/
static void USB_HostAppState(USBH_HandleTypeDef *phost, uint8_t id)
{
    switch(id)
    {
        case HOST_USER_SELECT_CONFIGURATION:
            break;

        case HOST_USER_DISCONNECTION:
            HAL_USB_SetHostAppState(HAL_USB_HOST_STATE_DISCONNECT);
            break;

        case HOST_USER_CLASS_ACTIVE:
            HAL_USB_SetHostAppState(HAL_USB_HOST_STATE_READY);
            break;

        case HOST_USER_CONNECTION:
            break;

        default:
            break;
    }

}


/**
* @brief  config the usb as host controller
* @param  e_usbPort            usb port number: 0 or 1
*               e_usbHostClass    usb class, MSC or UVC
* @retval   void
* @note  
*/
void HAL_USB_InitHost(ENUM_HAL_USB_PORT e_usbPort, ENUM_HAL_USB_HOST_CLASS e_usbHostClass)
{
    if (HAL_USB_PORT_0 == e_usbPort)
    {
        reg_IrqHandle(OTG_INTR0_VECTOR_NUM, USB_LL_OTG0_IRQHandler, NULL);
        INTR_NVIC_SetIRQPriority(OTG_INTR0_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_OTG_INITR0,0));
    }
    else if (HAL_USB_PORT_1 == e_usbPort)
    {
        reg_IrqHandle(OTG_INTR1_VECTOR_NUM, USB_LL_OTG1_IRQHandler, NULL);
        INTR_NVIC_SetIRQPriority(OTG_INTR1_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_OTG_INITR1,0));

    }

    USBH_Init(&hUSBHost, USB_HostAppState, (uint8_t)e_usbPort);

    if (HAL_USB_HOST_CLASS_MSC == e_usbHostClass)
    {
        USBH_RegisterClass(&hUSBHost, USBH_MSC_CLASS);
    }
    else if (HAL_USB_HOST_CLASS_UVC == e_usbHostClass)
    {
        USBH_RegisterClass(&hUSBHost, USBH_UVC_CLASS);
    }

    USBH_Start(&hUSBHost);
}


/**
* @brief  start the USB Video for Application use
* @param  void
* @retval   void
* @note  
*/
void HAL_USB_StartUVC(void)
{
    USBH_UVC_StartView(&hUSBHost);
}


/**
* @brief  get the latest frame buffer
* @param  uint8_t  *u8_buff    the dest buffer to storage the video frame
* @retval   HAL_USB_ERR_BUFF_IS_EMPTY   : means the buffer pool is empty
*               HAL_OK                                      : means successfully get one video frame
* @note  
*/
HAL_RET_T HAL_USB_GetVideoFrame(uint8_t *u8_buff)
{
    HAL_RET_T    ret = HAL_OK;

    if (USBH_OK != USBH_UVC_GetBuff(u8_buff))
    {
        ret = HAL_USB_ERR_BUFF_IS_EMPTY;
    }

    return ret;
}


/**
* @brief    configure the USB Controller to enter into TEST MODE
* @param  void
* @retval   void
* @note  
*/
void HAL_USB_EnterUSBHostTestMode(void)
{
    USB_LL_EnterHostTestMode(USB_OTG0_HS);
}


