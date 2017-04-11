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
#include "debuglog.h"
#include "hal_dma.h"
#include "cpu_info.h"

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
void HAL_USB_InitHost(ENUM_HAL_USB_PORT e_usbPort)
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

    //support MSC
    USBH_RegisterClass(&hUSBHost, USBH_MSC_CLASS);

    //support UVC
    USBH_RegisterClass(&hUSBHost, USBH_UVC_CLASS);

    USBH_Start(&hUSBHost);
}


/**
* @brief  start the USB Video for Application use
* @param  void
* @retval   void
* @note  
*/
HAL_RET_T HAL_USB_StartUVC(uint16_t u16_width,
                           uint16_t u16_height,
                           uint32_t *u32_frameSize)
{
    HAL_RET_T           u8_ret = HAL_OK;
    uint8_t             u8_frameIndex;
    uint8_t             i;

    if ((u16_width == 0)||(u16_height == 0))
    {
        dlog_error("width or height can not be ZERO");

        return HAL_USB_ERR_USBH_UVC_INVALID_PARAM;
    }

    for (i = 0; i < HAL_USB_UVC_MAX_FRAME_FORMATS_NUM; i++)
    {
        if ((u16_width == USBH_UVC_GetFrameWidth(i))&&
            (u16_height == USBH_UVC_GetFrameHeight(i)))
        {
            break;
        }
    }

    if (i < HAL_USB_UVC_MAX_FRAME_FORMATS_NUM)
    {
        u8_frameIndex       = USBH_UVC_GetFrameIndex(i);
        *u32_frameSize      = USBH_UVC_GetFrameSize(u8_frameIndex);

        dlog_info("u8_frameIndex, u32_frameSize: %d, %d", u8_frameIndex, *u32_frameSize);

        if (0 == USBH_UVC_StartView(&hUSBHost, u8_frameIndex))
        {
            dlog_info("START UVC OK");
        }
        else
        {
            dlog_error("START UVC FAIL");

            u8_ret          = HAL_USB_ERR_USBH_UVC_START_ERROR;
        }
    }
    else
    {
        u8_ret              = HAL_USB_ERR_USBH_UVC_FORMAT_ERROR;
    }

    return u8_ret;
}


/**
* @brief  get the latest frame buffer
* @param  uint8_t  *u8_buff    the dest buffer to storage the video frame
* @retval   HAL_USB_ERR_BUFF_IS_EMPTY   : means the buffer pool is empty
*               HAL_OK                                      : means successfully get one video frame
* @note  
*/
HAL_RET_T HAL_USB_GetVideoFrame(uint8_t *u8_buff, uint32_t *u32_frameNum, uint32_t *u32_frameSize)
{
    HAL_RET_T               ret = HAL_USB_ERR_BUFF_IS_EMPTY;
    uint8_t                *u8_frameBuff = NULL;
    uint32_t                u32_srcAddr;
    uint32_t                u32_destAddr;
    uint32_t                u32_addrOffset = 0;

    u8_frameBuff            = USBH_UVC_GetBuff(u32_frameNum, u32_frameSize);

    if (NULL != u8_frameBuff)
    {
        u32_srcAddr             = (uint32_t)u8_frameBuff;
        u32_destAddr            = (uint32_t)u8_buff;

        if (ENUM_CPU0_ID == CPUINFO_GetLocalCpuId())
        {
            u32_addrOffset  = DTCM_CPU0_DMA_ADDR_OFFSET;
        }
        else if (ENUM_CPU1_ID == CPUINFO_GetLocalCpuId())
        {
            u32_addrOffset  = DTCM_CPU1_DMA_ADDR_OFFSET;
        }

        if ((u32_srcAddr > DTCM_START_ADDR)&&
            (u32_srcAddr <= DTCM_END_ADDR))
        {
            u32_srcAddr    += u32_addrOffset;
        }

        if ((u32_destAddr > DTCM_START_ADDR)&&
            (u32_destAddr <= DTCM_END_ADDR))
        {
            u32_destAddr    += u32_addrOffset;
        }

        HAL_DMA_Start(u32_srcAddr,
                      u32_destAddr,
                      *u32_frameSize,
                      DMA_AUTO,
                      DMA_LINK_LIST_ITEM);

        CPUINFO_DCacheInvalidateByAddr((uint32_t *)u8_buff, *u32_frameSize);

        USBH_UVC_SetBuffStateByAddr(u8_frameBuff, UVC_VIDEO_BUFF_EMPTY);

        ret = HAL_OK;
    }

    return ret;
}


/**
* @brief  get formats the camera support
* @param  STRU_UVC_VIDEO_FRAME_FORMAT *stVideoFrameFormat
* @retval   void
* @note  
*/
void HAL_USB_GetVideoFormats(STRU_UVC_VIDEO_FRAME_FORMAT *stVideoFrameFormat)
{
    uint8_t         i;

    USBH_UVC_GetVideoFormatList(&hUSBHost);

    for (i = 0; i < HAL_USB_UVC_MAX_FRAME_FORMATS_NUM; i++)
    {
        stVideoFrameFormat->u16_width[i]       = USBH_UVC_GetFrameWidth(i);
        stVideoFrameFormat->u16_height[i]      = USBH_UVC_GetFrameHeight(i);
        stVideoFrameFormat->u8_frameIndex[i]   = USBH_UVC_GetFrameIndex(i);

        dlog_info("i: %d, width: %d, height: %d, frameIndex: %d",
                  i,
                  stVideoFrameFormat->u16_width[i],
                  stVideoFrameFormat->u16_height[i],
                  stVideoFrameFormat->u8_frameIndex[i]);
    }
}


/**
* @brief  get the control items supported by Processing Unit, such as white balance, hue, and so on
* @param  void
* @retval   uint32_t, bit maps
* @note  
*/
uint32_t HAL_USB_GetUVCProcUnitControls(void)
{
    uint32_t        ret;

    ret = USBH_UVC_GetProcUnitControls();

    return ret;
}


/**
* @brief  get the control items supported by Extension Unit
* @param  void
* @retval   uint32_t, bit maps
* @note  
*/
uint32_t HAL_USB_GetUVCExtUnitControls(void)
{
    uint32_t        ret;

    ret = USBH_UVC_GetExtUnitControls();

    return ret;
}


/**
* @brief  get the control param from Processing Unit
* @param  void
* @retval   uint32_t, bit maps
* @note
*/
uint32_t HAL_USB_GetProcUnitParam(uint8_t index, uint8_t type)
{
    USBH_UVC_ProcUnitParamHandler(&hUSBHost, index, type);
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


void HAL_USB_TransferUVCToGrd(uint8_t *buff, uint32_t dataLen)
{
    static uint8_t          u8_frameInterval = 0;

    u8_frameInterval++;

    if (u8_frameInterval >= 5)
    {
        u8_frameInterval    = 0;

        //copy to baseband
        memcpy((void *)0xB1800000,
               (void *)buff,
               dataLen);
    }
}


ENUM_HAL_USB_HOST_CLASS HAL_USB_CurUsbClassType(void)
{
    USBH_ClassTypeDef           *activeClass;
    ENUM_HAL_USB_HOST_CLASS      enHostClass;

    activeClass            = hUSBHost.pActiveClass;

    switch (activeClass->ClassCode)
    {
    case USB_MSC_CLASS:
        enHostClass        = HAL_USB_HOST_CLASS_MSC;
        break;

    case UVC_CLASS:
        enHostClass        = HAL_USB_HOST_CLASS_UVC;
        break;

    default:
        enHostClass        = HAL_USB_HOST_CLASS_NONE;
        break;
    }

    return enHostClass;
}




