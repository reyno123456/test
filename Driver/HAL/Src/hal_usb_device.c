/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_usb.c
Description: The external HAL APIs to use the USB Device.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/12/22
History: 
        0.0.1    2016/12/21    The initial version of hal_usb_device.c
*****************************************************************************/
#include "hal_usb_device.h"
#include "usbd_def.h"
#include "usbd_core.h"
#include "usbd_hid.h"
#include "usbd_hid_desc.h"
#include "sram.h"
#include "sys_event.h"
#include "interrupt.h"
#include "hal_nvic.h"
#include "debuglog.h"

extern USBD_HandleTypeDef   USBD_Device;
USBD_HID_ItfTypeDef         g_stUsbdHidItf;


/**
* @brief  config the usb as device controller
* @param  e_usbPort         usb port number: 0 or 1
* @retval   void
* @note  
*/
void HAL_USB_InitDevice(ENUM_HAL_USB_DEVICE_PORT e_usbPort)
{
    if (HAL_USB_DEVICE_PORT_0 == e_usbPort)
    {
        reg_IrqHandle(OTG_INTR0_VECTOR_NUM, USB_LL_OTG0_IRQHandler, NULL);
        INTR_NVIC_SetIRQPriority(OTG_INTR0_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_OTG_INITR0,0));
    }
    else if (HAL_USB_DEVICE_PORT_1 == e_usbPort)
    {
        reg_IrqHandle(OTG_INTR1_VECTOR_NUM, USB_LL_OTG1_IRQHandler, NULL);
        INTR_NVIC_SetIRQPriority(OTG_INTR1_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_OTG_INITR1,0));
    }

    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_USB_PLUG_OUT, HAL_USB_ResetDevice);

    USBD_Init(&USBD_Device, &HID_Desc, (uint8_t)e_usbPort);

    USBD_RegisterClass(&USBD_Device, USBD_HID_CLASS);

    USBD_Start(&USBD_Device);

    return;
}


/**
* @brief  reset the usb device controller for usb device hotplug
* @param  void * p         for sys event call-back
* @retval   void
* @note  
*/
void HAL_USB_ResetDevice(void * p)
{
    USBD_LL_Init(&USBD_Device);
    HAL_PCD_Start(USBD_Device.pData);

    SRAM_Ready0Confirm();

    SRAM_Ready1Confirm();

    return;
}


/**
* @brief  send video data to the host
* @param  uint8_t    *buff                the buffer to send
*               uint32_t    u32_len          buffer length 
* @retval   void
* @note
*/
HAL_RET_T HAL_USB_DeviceSendVideo(uint8_t *buff, uint32_t u32_len)
{
    uint8_t       ret;

    ret = USBD_HID_SendReport(&USBD_Device, buff, u32_len, HID_EPIN_VIDEO_ADDR);

    if (ret != USBD_OK)
    {
        if (USBD_BUSY == ret)
        {
            dlog_error("send video busy");

            return HAL_USB_ERR_DEVICE_BUSY;
        }
        else
        {
            dlog_error("send video not configured");

            return HAL_USB_ERR_DEVICE_NOT_CONGIURED;
        }
    }

    return HAL_OK;
}


/**
* @brief  send control data to the host
* @param  uint8_t    *buff                the buffer to send
*               uint32_t    u32_len          buffer length 
* @retval   void
* @note
*/
HAL_RET_T HAL_USB_DeviceSendCtrl(uint8_t *buff, uint32_t u32_len)
{
    uint8_t       ret;

    if (USB_OTG_IS_BIG_ENDIAN())
    {
        USB_LL_ConvertEndian(buff, buff, u32_len);
    }

    ret = USBD_HID_SendReport(&USBD_Device, buff, u32_len, HID_EPIN_CTRL_ADDR);

    if (ret != USBD_OK)
    {
        if (USBD_BUSY == ret)
        {
            dlog_error("send ctrl busy");

            return HAL_USB_ERR_DEVICE_BUSY;
        }
        else
        {
            dlog_error("send ctrl not configured");

            return HAL_USB_ERR_DEVICE_NOT_CONGIURED;
        }
    }

    return HAL_OK;
}


/**
* @brief   register the user callback function to receive host data
* @param  void (*pUsrFunc)(void *)     user callback function
* @retval   void
* @note
*/
void HAL_USB_RegisterUserProcess(void (*pUsrFunc)(void *))
{
    g_stUsbdHidItf.dataOut  = pUsrFunc;

    USBD_HID_RegisterInterface(&USBD_Device, &g_stUsbdHidItf);
}


