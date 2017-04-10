#include "usbh_uvc.h"
#include "debuglog.h"
#include "systicks.h"
#include "dma.h"
#include "cpu_info.h"

USBH_ClassTypeDef  UVC_Class = 
{
  "UVC",
  UVC_CLASS,  
  USBH_UVC_InterfaceInit,
  USBH_UVC_InterfaceDeInit,
  USBH_UVC_ClassRequest,
  USBH_UVC_Process,
  USBH_UVC_SOFProcess,
  NULL,
};

UVC_HandleTypeDef       g_stUVCHandle;
uint8_t                *g_UVCRecvBuffer;
uint8_t                *g_UVCVideoBuffer[UVC_VIDEO_BUFF_FRAME_NUM];
uint32_t                g_u32BufferFrameNumber[UVC_VIDEO_BUFF_FRAME_NUM];
uint8_t                 g_u8curVideoBuffIndex = 0;
uint32_t                g_u32recvBuffPos = 0;
UVC_BuffStateTypeDef    g_enumUVCBuffState[UVC_VIDEO_BUFF_FRAME_NUM];
uint32_t                g_u32UVCVideoBuffSizePerFrame;


USBH_UVCFormatUncompressedTypeDef           g_stUVCFormatUncomp[USB_UVC_MAX_FRAME_FORMATS_NUM];
USBH_UVCFrameUncompressedTypeDef            g_stUVCFrameUncomp[USB_UVC_MAX_FRAME_FORMATS_NUM];
USBH_VCProcessingUnitInterfaceDescriptor    g_stProcessingUnitDesc;
USBH_VCExtensionUnitInterfaceDescriptor     g_stExtensionUnitDesc;

uint8_t                 g_req_mem[8];
uint8_t                 g_cur_mem[28];
uint8_t                 g_max_mem[28];
uint8_t                 g_min_mem[28];


static USBH_UVC_PARAM_HANDLER hUsbhUVCParamHandler[16] = 
{
    USBH_UVC_GetBrightness,
    USBH_UVC_GetContrast,
    USBH_UVC_GetHUE,
    USBH_UVC_GetSaturation,
    USBH_UVC_GetSharpness,
    USBH_UVC_GetGamma,
    USBH_UVC_GetWhiteTemp,
    NULL,
    USBH_UVC_GetBack,
    NULL,
    USBH_UVC_GetPowerLine,
    NULL,
    NULL,
    NULL,
    NULL,
    NULL
};


static USBH_StatusTypeDef USBH_UVC_InterfaceInit (USBH_HandleTypeDef *phost)
{
    USBH_StatusTypeDef      status = USBH_FAIL ;
    UVC_HandleTypeDef      *UVC_Handle;
    uint8_t                 interface;

    // ctrl interface
    interface = USBH_FindInterface(phost, 14, 1, 0);

    if(interface == 0xFF) /* Not Valid Interface */
    {
        dlog_info("Cannot Find the interface for %s class.", phost->pActiveClass->Name);

        status = USBH_FAIL;
    }
    else
    {
        USBH_SelectInterface(phost, interface);

        phost->pActiveClass->pData = &g_stUVCHandle;
        UVC_Handle =  (UVC_HandleTypeDef *) phost->pActiveClass->pData;
        USBH_memset(UVC_Handle, 0, sizeof(UVC_HandleTypeDef));

        UVC_Handle =  (UVC_HandleTypeDef *) phost->pActiveClass->pData;

        UVC_Handle->CtrlEp      = (phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].bEndpointAddress);
        UVC_Handle->CtrlEpSize  = phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].wMaxPacketSize;

        dlog_info("CtrlEp: %d, CtrlEpSize: %d", 
                    UVC_Handle->CtrlEp,
                    UVC_Handle->CtrlEpSize);

        // video interface
        interface = USBH_UVC_FindStreamingInterface(phost);

        if(interface == 0xFF) /* Not Valid Interface */
        {
            dlog_info("Cannot Find the interface for %s class.", phost->pActiveClass->Name);

            status = USBH_FAIL;
        }
        else
        {
            UVC_Handle->VideoEp      = (phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].bEndpointAddress);
            UVC_Handle->VideoEpSize  = UVC_VIDEO_EP_MAX_SIZE;

            UVC_Handle->u8_selInterface = phost->device.CfgDesc.Itf_Desc[interface].bInterfaceNumber;
            UVC_Handle->u8_selAltInterface = phost->device.CfgDesc.Itf_Desc[interface].bAlternateSetting;
#if 0
            dlog_info("xxxxxxxxxxxxxxxxxxxx0x%02x", UVC_Handle->VideoEp);
            dlog_info("interface: %d", interface);
            dlog_info("size: %d",phost->device.CfgDesc.Itf_Desc[interface].Ep_Desc[0].wMaxPacketSize);

            dlog_info("u8_selInterface: %d, u8_selAltInterface: %d", 
                        phost->device.CfgDesc.Itf_Desc[interface].bInterfaceNumber,
                        phost->device.CfgDesc.Itf_Desc[interface].bAlternateSetting);
#endif
        }

        UVC_Handle->CtrlPipe    = USBH_AllocPipe(phost, UVC_Handle->CtrlEp);
        UVC_Handle->VideoPipe   = USBH_AllocPipe(phost, UVC_Handle->VideoEp);

        dlog_info("ctrlpipe: %d, videopipe: %d", UVC_Handle->CtrlPipe, UVC_Handle->VideoPipe);

        /* Open the new channels */
        USBH_OpenPipe(phost,
                      UVC_Handle->CtrlPipe,
                      UVC_Handle->CtrlEp,
                      phost->device.address,
                      phost->device.speed,
                      USB_EP_TYPE_INTR,
                      UVC_Handle->CtrlEpSize);

        USBH_OpenPipe(phost,
                      UVC_Handle->VideoPipe,
                      UVC_Handle->VideoEp,
                      phost->device.address,
                      phost->device.speed,
                      USB_EP_TYPE_ISOC,
                      UVC_Handle->VideoEpSize);

        USBH_LL_SetToggle(phost, UVC_Handle->CtrlPipe, 0);
        USBH_LL_SetToggle(phost, UVC_Handle->VideoPipe, 0);

        phost->isocURBDone = USBH_UVC_UrbDone;

        status = USBH_OK;
    }

    return status;
}


USBH_StatusTypeDef USBH_UVC_InterfaceDeInit (USBH_HandleTypeDef *phost)
{
    uint8_t                     i;
    UVC_HandleTypeDef          *UVC_Handle =  (UVC_HandleTypeDef *) phost->pActiveClass->pData;

    USBH_ClosePipe(phost, UVC_Handle->CtrlPipe);
    USBH_FreePipe(phost, UVC_Handle->CtrlPipe);
    UVC_Handle->CtrlPipe = 0;

    USBH_ClosePipe(phost, UVC_Handle->VideoPipe);
    USBH_FreePipe  (phost, UVC_Handle->VideoPipe);
    UVC_Handle->VideoPipe = 0;

    USBH_memset((void *)UVC_Handle, 0, sizeof(UVC_HandleTypeDef));
    phost->pActiveClass->pData = 0;

    USBH_UVC_SetBuffState(0, 0);
    USBH_UVC_SetBuffState(1, 0);

    g_u8curVideoBuffIndex = 0;
    g_u32recvBuffPos = 0;

    free(g_UVCRecvBuffer);

    for (i = 0; i < UVC_VIDEO_BUFF_FRAME_NUM; i++)
    {
        free(g_UVCVideoBuffer[i]);
    }

    return USBH_OK;
}


static USBH_StatusTypeDef USBH_UVC_ClassRequest(USBH_HandleTypeDef *phost)
{
    UVC_HandleTypeDef        *UVC_Handle =  (UVC_HandleTypeDef *)phost->pActiveClass->pData;
    USBH_StatusTypeDef        status     = USBH_BUSY;

    if (USBH_OK == USBH_SetInterface(phost, 1, 0))
    {
        status = USBH_OK;

        phost->pUser(phost, HOST_USER_CLASS_ACTIVE);
    }

    return status; 
}


static USBH_StatusTypeDef USBH_UVC_Process (USBH_HandleTypeDef *phost)
{
    UVC_HandleTypeDef        *UVC_Handle =  (UVC_HandleTypeDef *)phost->pActiveClass->pData;
    USBH_StatusTypeDef        status = USBH_BUSY;

    switch (UVC_Handle->uvc_state)
    {
    case UVC_STATE_PROBE:
        if (USBH_OK == USBH_UVC_Probe(phost))
        {
            UVC_Handle->uvc_state           = UVC_STATE_COMMIT;
            UVC_Handle->uvc_probeState      = UVC_PROBE_STATE_GET_CUR;
            UVC_Handle->probeCount          = 0;
        }
        break;

    case UVC_STATE_COMMIT:
        if (USBH_OK == USBH_UVC_Commit(phost))
        {
            UVC_Handle->uvc_state           = UVC_STATE_ERROR;
        }
        break;

    default:
        break;
    }

    return status;
}


static USBH_StatusTypeDef USBH_UVC_SOFProcess (USBH_HandleTypeDef *phost)
{
    return USBH_OK;
}


static USBH_StatusTypeDef USBH_UVC_GetCSParam(USBH_HandleTypeDef *phost)
{
    UVC_HandleTypeDef        *UVC_Handle =  (UVC_HandleTypeDef *)phost->pActiveClass->pData;
    USBH_StatusTypeDef        status = USBH_BUSY;

    switch (UVC_Handle->uvc_getParamState)
    {
    case UVC_PARAM_TYPE_GET_LEN:
        phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
        phost->Control.setup.b.bRequest = UVC_GET_LEN;
        phost->Control.setup.b.wValue.w = (uint16_t)((UVC_Handle->uvc_CSCount)<<8);
        phost->Control.setup.b.wIndex.w = 0x0300;
        phost->Control.setup.b.wLength.w = 0x0002;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)g_req_mem , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_PARAM_TYPE_GET_INFO;
        }
        break;

    case UVC_PARAM_TYPE_GET_INFO:
        phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
        phost->Control.setup.b.bRequest = UVC_GET_INFO;
        phost->Control.setup.b.wValue.w = (uint16_t)((UVC_Handle->uvc_CSCount)<<8);
        phost->Control.setup.b.wIndex.w = 0x0300;
        phost->Control.setup.b.wLength.w = 0x0001;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)g_req_mem, 8))
        {
            UVC_Handle->uvc_getParamState = UVC_PARAM_TYPE_GET_MIN;
        }
        break;

    case UVC_PARAM_TYPE_GET_MIN:
        phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
        phost->Control.setup.b.bRequest = UVC_GET_MIN;
        phost->Control.setup.b.wValue.w = (uint16_t)((UVC_Handle->uvc_CSCount)<<8);
        phost->Control.setup.b.wIndex.w = 0x0300;

        if (UVC_Handle->uvc_CSCount == 1)
        {
            phost->Control.setup.b.wLength.w = 0x0004;
        }
        else if (UVC_Handle->uvc_CSCount == 2)
        {
            phost->Control.setup.b.wLength.w = 0x0008;
        }
        else
        {
            phost->Control.setup.b.wLength.w = 0x000B;
        }

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)g_req_mem, 8))
        {
            UVC_Handle->uvc_getParamState = UVC_PARAM_TYPE_GET_MAX;
        }

        break;
        
    case UVC_PARAM_TYPE_GET_MAX:
        phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
        phost->Control.setup.b.bRequest = UVC_GET_MAX;
        phost->Control.setup.b.wValue.w = (uint16_t)((UVC_Handle->uvc_CSCount)<<8);
        phost->Control.setup.b.wIndex.w = 0x0300;

        if (UVC_Handle->uvc_CSCount == 1)
        {
            phost->Control.setup.b.wLength.w = 0x0004;
        }
        else if (UVC_Handle->uvc_CSCount == 2)
        {
            phost->Control.setup.b.wLength.w = 0x0008;
        }
        else
        {
            phost->Control.setup.b.wLength.w = 0x000B;
        }

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)g_req_mem, 8))
        {
            UVC_Handle->uvc_getParamState = UVC_PARAM_TYPE_GET_RES;
        }

        break;

    case UVC_PARAM_TYPE_GET_RES:
        phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
        phost->Control.setup.b.bRequest = UVC_GET_RES;
        phost->Control.setup.b.wValue.w = (uint16_t)((UVC_Handle->uvc_CSCount)<<8);
        phost->Control.setup.b.wIndex.w = 0x0300;

        if (UVC_Handle->uvc_CSCount == 1)
        {
            phost->Control.setup.b.wLength.w = 0x0004;
        }
        else if (UVC_Handle->uvc_CSCount == 2)
        {
            phost->Control.setup.b.wLength.w = 0x0008;
        }
        else
        {
            phost->Control.setup.b.wLength.w = 0x000B;
        }

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)g_req_mem, 8))
        {
            UVC_Handle->uvc_getParamState = UVC_PARAM_TYPE_GET_DEF;
        }

        break;

    case UVC_PARAM_TYPE_GET_DEF:
        phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
        phost->Control.setup.b.bRequest = UVC_GET_DEF;
        phost->Control.setup.b.wValue.w = (uint16_t)((UVC_Handle->uvc_CSCount)<<8);
        phost->Control.setup.b.wIndex.w = 0x0300;

        if (UVC_Handle->uvc_CSCount == 1)
        {
            phost->Control.setup.b.wLength.w = 0x0004;
        }
        else if (UVC_Handle->uvc_CSCount == 2)
        {
            phost->Control.setup.b.wLength.w = 0x0008;
        }
        else
        {
            phost->Control.setup.b.wLength.w = 0x000B;
        }

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)g_req_mem, 8))
        {
            UVC_Handle->uvc_CSCount++;

            if (UVC_Handle->uvc_CSCount >= 6)
            {
                status  = USBH_OK;
            }
            else
            {
                UVC_Handle->uvc_getParamState = UVC_PARAM_TYPE_GET_LEN;
            }
        }

        break;

    default:
        break;

    }

    return status;
}


static uint32_t USBH_UVC_GetBrightness(USBH_HandleTypeDef *phost,
                                      UVC_GetParamTypeDef paramType)
{
    UVC_HandleTypeDef        *UVC_Handle =  (UVC_HandleTypeDef *)phost->pActiveClass->pData;
    uint32_t                  ret = 0xFFFFFFFF;

    phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
    phost->Control.setup.b.wValue.w = 0x0200;
    phost->Control.setup.b.wIndex.w = 0x0200;
    phost->Control.setup.b.bRequest = paramType;

    if (UVC_PARAM_TYPE_GET_INFO == paramType)
    {
        phost->Control.setup.b.wLength.w = 0x0001;
    }
    else
    {
        phost->Control.setup.b.wLength.w = 0x0002;
    }

    memset((void *)g_req_mem, 0, 8);

    if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)g_req_mem, 8))
    {
        ret = LE32(g_req_mem);
    }

    return ret;
}


static uint32_t USBH_UVC_GetContrast(USBH_HandleTypeDef *phost,
                                    UVC_GetParamTypeDef paramType)
{
    UVC_HandleTypeDef        *UVC_Handle =  (UVC_HandleTypeDef *)phost->pActiveClass->pData;
    uint32_t                  ret = 0xFFFFFFFF;

    phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
    phost->Control.setup.b.wValue.w = 0x0300;
    phost->Control.setup.b.wIndex.w = 0x0200;
    phost->Control.setup.b.bRequest = paramType;

    if (UVC_PARAM_TYPE_GET_INFO == paramType)
    {
        phost->Control.setup.b.wLength.w = 0x0001;
    }
    else
    {
        phost->Control.setup.b.wLength.w = 0x0002;
    }

    memset((void *)g_req_mem, 0, 8);

    if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)g_req_mem, 8))
    {
        ret = LE32(g_req_mem);
    }

    return ret;
}


static uint32_t USBH_UVC_GetHUE(USBH_HandleTypeDef *phost,
                                 UVC_GetParamTypeDef paramType)
{
    UVC_HandleTypeDef        *UVC_Handle =  (UVC_HandleTypeDef *)phost->pActiveClass->pData;
    uint32_t                  ret = 0xFFFFFFFF;

    phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
    phost->Control.setup.b.wValue.w = 0x0600;
    phost->Control.setup.b.wIndex.w = 0x0200;
    phost->Control.setup.b.bRequest = paramType;

    if (UVC_PARAM_TYPE_GET_INFO == paramType)
    {
        phost->Control.setup.b.wLength.w = 0x0001;
    }
    else
    {
        phost->Control.setup.b.wLength.w = 0x0002;
    }

    memset((void *)g_req_mem, 0, 8);

    if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)g_req_mem, 8))
    {
        ret = LE32(g_req_mem);
    }

    return ret;
}


static uint32_t USBH_UVC_GetSaturation(USBH_HandleTypeDef *phost,
                                      UVC_GetParamTypeDef paramType)
{
    UVC_HandleTypeDef        *UVC_Handle =  (UVC_HandleTypeDef *)phost->pActiveClass->pData;
    uint32_t                  ret = 0xFFFFFFFF;

    phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
    phost->Control.setup.b.wValue.w = 0x0700;
    phost->Control.setup.b.wIndex.w = 0x0200;
    phost->Control.setup.b.bRequest = paramType;

    if (UVC_PARAM_TYPE_GET_INFO == paramType)
    {
        phost->Control.setup.b.wLength.w = 0x0001;
    }
    else
    {
        phost->Control.setup.b.wLength.w = 0x0002;
    }

    memset((void *)g_req_mem, 0, 8);

    if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)g_req_mem, 8))
    {
        ret = LE32(g_req_mem);
    }

    return ret;
}


static uint32_t USBH_UVC_GetSharpness(USBH_HandleTypeDef *phost,
                                      UVC_GetParamTypeDef paramType)
{
    UVC_HandleTypeDef        *UVC_Handle =  (UVC_HandleTypeDef *)phost->pActiveClass->pData;
    uint32_t                  ret = 0xFFFFFFFF;

    phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
    phost->Control.setup.b.wValue.w = 0x0800;
    phost->Control.setup.b.wIndex.w = 0x0200;
    phost->Control.setup.b.bRequest = paramType;

    if (UVC_PARAM_TYPE_GET_INFO == paramType)
    {
        phost->Control.setup.b.wLength.w = 0x0001;
    }
    else
    {
        phost->Control.setup.b.wLength.w = 0x0002;
    }

    memset((void *)g_req_mem, 0, 8);

    if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)g_req_mem, 8))
    {
        ret = LE32(g_req_mem);
    }

    return ret;
}


static uint32_t USBH_UVC_GetGamma(USBH_HandleTypeDef *phost,
                                   UVC_GetParamTypeDef paramType)
{
    UVC_HandleTypeDef        *UVC_Handle =  (UVC_HandleTypeDef *)phost->pActiveClass->pData;
    uint32_t                  ret = 0xFFFFFFFF;

    phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
    phost->Control.setup.b.wValue.w = 0x0900;
    phost->Control.setup.b.wIndex.w = 0x0200;
    phost->Control.setup.b.bRequest = paramType;

    if (UVC_PARAM_TYPE_GET_INFO == paramType)
    {
        phost->Control.setup.b.wLength.w = 0x0001;
    }
    else
    {
        phost->Control.setup.b.wLength.w = 0x0002;
    }

    memset((void *)g_req_mem, 0, 8);

    if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)g_req_mem, 8))
    {
        ret = LE32(g_req_mem);
    }

    return ret;
}


static uint32_t USBH_UVC_GetWhiteTemp(USBH_HandleTypeDef *phost,
                                      UVC_GetParamTypeDef paramType)
{
    UVC_HandleTypeDef        *UVC_Handle =  (UVC_HandleTypeDef *)phost->pActiveClass->pData;
    uint32_t                  ret = 0xFFFFFFFF;

    phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
    phost->Control.setup.b.wValue.w = 0x0a00;
    phost->Control.setup.b.wIndex.w = 0x0200;
    phost->Control.setup.b.bRequest = paramType;

    if (UVC_PARAM_TYPE_GET_INFO == paramType)
    {
        phost->Control.setup.b.wLength.w = 0x0001;
    }
    else
    {
        phost->Control.setup.b.wLength.w = 0x0002;
    }

    memset((void *)g_req_mem, 0, 8);

    if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)g_req_mem, 8))
    {
        ret = LE32(g_req_mem);
    }

    return ret;
}


static uint32_t USBH_UVC_GetBack(USBH_HandleTypeDef *phost,
                                  UVC_GetParamTypeDef paramType)
{
    UVC_HandleTypeDef        *UVC_Handle =  (UVC_HandleTypeDef *)phost->pActiveClass->pData;
    uint32_t                  ret = 0xFFFFFFFF;

    phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
    phost->Control.setup.b.wValue.w = 0x0100;
    phost->Control.setup.b.wIndex.w = 0x0200;
    phost->Control.setup.b.bRequest = paramType;

    if (UVC_PARAM_TYPE_GET_INFO == paramType)
    {
        phost->Control.setup.b.wLength.w = 0x0001;
    }
    else
    {
        phost->Control.setup.b.wLength.w = 0x0002;
    }

    if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)g_req_mem, 8))
    {
        ret = LE32(g_req_mem);
    }

    return ret;
}


static uint32_t USBH_UVC_GetPowerLine(USBH_HandleTypeDef *phost,
                                      UVC_GetParamTypeDef paramType)
{
    UVC_HandleTypeDef        *UVC_Handle =  (UVC_HandleTypeDef *)phost->pActiveClass->pData;
    uint32_t                  ret = 0xFFFFFFFF;

    phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
    phost->Control.setup.b.wValue.w = 0x0500;
    phost->Control.setup.b.wIndex.w = 0x0200;
    phost->Control.setup.b.bRequest = paramType;

    if (UVC_PARAM_TYPE_GET_INFO == paramType)
    {
        phost->Control.setup.b.wLength.w = 0x0001;
    }
    else
    {
        phost->Control.setup.b.wLength.w = 0x0002;
    }

    memset((void *)g_req_mem, 0, 8);

    if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)g_req_mem, 8))
    {
        ret = LE32(g_req_mem);
    }

    return ret;

}


static USBH_StatusTypeDef USBH_UVC_Probe(USBH_HandleTypeDef *phost)
{
    UVC_HandleTypeDef        *UVC_Handle =  (UVC_HandleTypeDef *)phost->pActiveClass->pData;
    USBH_StatusTypeDef        status = USBH_BUSY;

    phost->Control.setup.b.wValue.w = 0x0100;
    phost->Control.setup.b.wIndex.w = 0x0001;
    phost->Control.setup.b.wLength.w = 0x001A;

    switch (UVC_Handle->uvc_probeState)
    {
    case UVC_PROBE_STATE_GET_CUR:
        phost->Control.setup.b.bRequest = UVC_GET_CUR;
        phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)g_cur_mem, 26))
        {
            UVC_Handle->uvc_probeState = UVC_PROBE_STATE_GET_MAX;
        }
        break;

    case UVC_PROBE_STATE_GET_MAX:
        phost->Control.setup.b.bRequest = UVC_GET_MAX;
        phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(g_max_mem) , 26))
        {
            UVC_Handle->uvc_probeState = UVC_PROBE_STATE_GET_MIN;
        }

        break;

    case UVC_PROBE_STATE_GET_MIN:
        phost->Control.setup.b.bRequest = UVC_GET_MIN;
        phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(g_min_mem) , 26))
        {
            if (UVC_Handle->probeCount >= 1)
            {
                UVC_Handle->probeCount = 0;

                status = USBH_OK;
            }
            else
            {
                UVC_Handle->uvc_probeState = UVC_PROBE_STATE_SET_CUR;
            }
        }

        break;

    case UVC_PROBE_STATE_SET_CUR:
        phost->Control.setup.b.bRequest = UVC_SET_CUR;
        phost->Control.setup.b.bmRequestType = USB_H2D | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
//        g_cur_mem[23] = 0;
        g_max_mem[3] = UVC_Handle->u8_selFrameIndex;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(g_max_mem) , 26))
        {
            UVC_Handle->probeCount++;

            UVC_Handle->uvc_probeState = UVC_PROBE_STATE_GET_CUR;
        }

        break;

    default:
        break;

    }

    return status;
}


static USBH_StatusTypeDef USBH_UVC_Commit(USBH_HandleTypeDef *phost)
{
    UVC_HandleTypeDef        *UVC_Handle =  (UVC_HandleTypeDef *)phost->pActiveClass->pData;
    USBH_StatusTypeDef        status = USBH_BUSY;

    switch (UVC_Handle->uvc_probeState)
    {
    case UVC_PROBE_STATE_GET_CUR:
        phost->Control.setup.b.bRequest = UVC_GET_CUR;
        phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
        phost->Control.setup.b.wValue.w = 0x0100;
        phost->Control.setup.b.wIndex.w = 0x0001;
        phost->Control.setup.b.wLength.w = 0x001A;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(g_cur_mem) , 26))
        {
            UVC_Handle->uvc_probeState = UVC_PROBE_STATE_SET_CUR;
        }
        break;

    case UVC_PROBE_STATE_SET_CUR:
        phost->Control.setup.b.bRequest = UVC_SET_CUR;
        phost->Control.setup.b.bmRequestType = USB_H2D | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
        phost->Control.setup.b.wValue.w = 0x0100;
        phost->Control.setup.b.wIndex.w = 0x0001;
        phost->Control.setup.b.wLength.w = 0x001A;

        //g_cur_mem[23] = 0;
        //g_max_mem[3]  = UVC_Handle->u8_selFrameIndex;

        //dlog_info("g_cur_mem[3]: %d", g_max_mem[3]);

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(g_max_mem) , 26))
        {
            UVC_Handle->uvc_probeState = UVC_PROBE_STATE_SET_CUR_COMMIT;
        }

        break;

    case UVC_PROBE_STATE_GET_CUR_COMMIT:
        phost->Control.setup.b.bRequest = UVC_GET_CUR;
        phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
        phost->Control.setup.b.wValue.w = 0x0100;
        phost->Control.setup.b.wIndex.w = 0x0001;
        phost->Control.setup.b.wLength.w = 0x001A;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(g_cur_mem) , 26))
        {
            UVC_Handle->uvc_probeState = UVC_PROBE_STATE_SET_CUR_COMMIT;
        }

        break;

    case UVC_PROBE_STATE_SET_CUR_COMMIT:

        phost->Control.setup.b.bRequest = UVC_SET_CUR;
        phost->Control.setup.b.bmRequestType = USB_H2D | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
        phost->Control.setup.b.wValue.w = 0x0200;
        phost->Control.setup.b.wIndex.w = 0x0001;
        phost->Control.setup.b.wLength.w = 0x001A;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(g_max_mem) , 26))
        {
            UVC_Handle->uvc_probeState = UVC_STATE_SET_INTERFACE;
        }

        break;

    case UVC_STATE_SET_INTERFACE:
        if (USBH_OK == USBH_SetInterface(phost,
                                         UVC_Handle->u8_selInterface,
                                         UVC_Handle->u8_selAltInterface))
        {
            UVC_Handle->uvc_probeState = UVC_STATE_VIDEO_PLAY;
        }
        break;

    case UVC_STATE_VIDEO_PLAY:
        //USBH_IsocReceiveData(phost, g_UVCRecvBuffer, UVC_VIDEO_EP_MAX_SIZE, UVC_Handle->VideoPipe);
        USBH_IsocReceiveData(phost, g_UVCRecvBuffer, UVC_VIDEO_MAX_SIZE_PER_SOF, UVC_Handle->VideoPipe);

        status = USBH_OK;

        break;

    default:
        break;

    }

    return status;
}


uint8_t USBH_UVC_StartView(USBH_HandleTypeDef *phost, uint8_t u8_frameIndex)
{
    uint8_t                   i;
    UVC_HandleTypeDef        *UVC_Handle =  (UVC_HandleTypeDef *)  phost->pActiveClass->pData;

    UVC_Handle->uvc_state           = UVC_STATE_PROBE;
    UVC_Handle->uvc_probeState      = UVC_PROBE_STATE_GET_CUR;
    UVC_Handle->probeCount          = 0;
    UVC_Handle->u8_selFrameIndex    = u8_frameIndex;

    g_u32UVCVideoBuffSizePerFrame   = USBH_UVC_GetFrameSize(u8_frameIndex);

    g_UVCRecvBuffer                 = (uint8_t *)malloc(UVC_VIDEO_EP_MAX_SIZE);

    if (g_UVCRecvBuffer != NULL)
    {
        dlog_info("malloc g_UVCRecvBuffer: 0x%08x", g_UVCRecvBuffer);
    }
    else
    {
        dlog_error("malloc g_UVCRecvBuffer error");

        return 0xFF;
    }

    for (i = 0; i < UVC_VIDEO_BUFF_FRAME_NUM; i++)
    {
        g_UVCVideoBuffer[i]         = (uint8_t *)malloc(g_u32UVCVideoBuffSizePerFrame);

        if (g_UVCVideoBuffer[i] != NULL)
        {
            dlog_info("malloc g_UVCVideoBuffer[%d]: 0x%08x", i, g_UVCVideoBuffer[i]);
        }
        else
        {
            dlog_error("malloc g_UVCVideoBuffer[%d] error");

            return 0xFF;
        }
    }

    dlog_info("g_u32UVCVideoBuffSizePerFrame: %d", g_u32UVCVideoBuffSizePerFrame);

    return 0;
}


static void USBH_UVC_UrbDone(USBH_HandleTypeDef *phost)
{
    UVC_HandleTypeDef      *UVC_Handle;
    uint32_t                u32_recvSize;
    static uint32_t         u32_frameNumber = 0;

    UVC_Handle =  (UVC_HandleTypeDef *) phost->pActiveClass->pData;

    if (g_UVCRecvBuffer[0] != UVC_HEADER_SPECIAL_CHAR)
    {
        dlog_error("this is not UVC package");
    }
    else
    {
        u32_recvSize = USBH_LL_GetLastXferSize(phost, UVC_Handle->VideoPipe);

        if (u32_recvSize > UVC_HEADER_SIZE)
        {
            u32_recvSize -= UVC_HEADER_SIZE;

            if (UVC_HEADER_FRAME_END != (UVC_HEADER_FRAME_END & g_UVCRecvBuffer[1]))
            {
                //dma_transfer((uint32_t *)(g_UVCRecvBuffer + UVC_HEADER_SIZE), (uint32_t *)(g_UVCVideoBuffer[g_u8curVideoBuffIndex] + g_u32recvBuffPos), u32_recvSize);
                memcpy((void *)(g_UVCVideoBuffer[g_u8curVideoBuffIndex] + g_u32recvBuffPos),
                       (void *)(g_UVCRecvBuffer + UVC_HEADER_SIZE),
                       u32_recvSize);

                g_u32recvBuffPos += u32_recvSize;

                /* buffer size is overflow, refill this buffer */
                if (g_u32recvBuffPos >= g_u32UVCVideoBuffSizePerFrame)
                {
                    dlog_error("buffer %d is overflow", g_u8curVideoBuffIndex);
                    g_u32recvBuffPos = 0;
                }
            }
            else
            {
                if ((g_u32recvBuffPos + u32_recvSize) == g_u32UVCVideoBuffSizePerFrame)
                {
                    memcpy((void *)(g_UVCVideoBuffer[g_u8curVideoBuffIndex] + g_u32recvBuffPos),
                           (void *)(g_UVCRecvBuffer + UVC_HEADER_SIZE),
                           u32_recvSize);

                    USBH_UVC_SetBuffState(g_u8curVideoBuffIndex, UVC_VIDEO_BUFF_VALID);

                    USBH_UVC_SetBuffNumber(g_u8curVideoBuffIndex, u32_frameNumber);

                    g_u8curVideoBuffIndex = ((++g_u8curVideoBuffIndex)%UVC_VIDEO_BUFF_FRAME_NUM);

                    if (USBH_UVC_GetBuffState(g_u8curVideoBuffIndex) != UVC_VIDEO_BUFF_IN_USING)
                    {
                        USBH_UVC_SetBuffState(g_u8curVideoBuffIndex, UVC_VIDEO_BUFF_EMPTY);
                    }
                    else
                    {
                        g_u8curVideoBuffIndex = ((++g_u8curVideoBuffIndex)%UVC_VIDEO_BUFF_FRAME_NUM);
                    }

                    u32_frameNumber++;
                }
                else
                {
                    dlog_error("Lost Video Data");
                }

                g_u32recvBuffPos = 0;
            }

        }
    }

//    USBH_IsocReceiveData(phost, g_UVCRecvBuffer, UVC_VIDEO_EP_MAX_SIZE, UVC_Handle->VideoPipe);
    USBH_IsocReceiveData(phost, g_UVCRecvBuffer, UVC_VIDEO_MAX_SIZE_PER_SOF, UVC_Handle->VideoPipe);
}


uint8_t *USBH_UVC_GetBuff(uint32_t *frameNumber, uint32_t *frameSize)
{
    uint8_t             i;

    for (i = 0; i < UVC_VIDEO_BUFF_FRAME_NUM; i++)
    {
        if (UVC_VIDEO_BUFF_VALID == USBH_UVC_GetBuffState(i))
        {
            USBH_UVC_SetBuffState(i, UVC_VIDEO_BUFF_IN_USING);

            *frameNumber    = USBH_UVC_GetBuffNumber(i);
            *frameSize      = g_u32UVCVideoBuffSizePerFrame;

            return g_UVCVideoBuffer[i];
        }
    }

    return NULL;
}


void USBH_UVC_SetBuffStateByAddr(uint8_t *u8_buffAddr, UVC_BuffStateTypeDef e_buffState)
{
    uint8_t             i = 0;

    for (i = 0; i < UVC_VIDEO_BUFF_FRAME_NUM; i++)
    {
        if (g_UVCVideoBuffer[i] == u8_buffAddr)
        {
            break;
        }
    }

    g_enumUVCBuffState[i]   = e_buffState;
}

static void USBH_UVC_SetBuffState(uint8_t u8_buffIndex, UVC_BuffStateTypeDef e_buffState)
{
    g_enumUVCBuffState[u8_buffIndex] = e_buffState;
}

static UVC_BuffStateTypeDef USBH_UVC_GetBuffState(uint8_t u8_buffIndex)
{
    return g_enumUVCBuffState[u8_buffIndex];
}

static void USBH_UVC_SetBuffNumber(uint8_t u8_buffIndex, uint32_t u32_bufferNum)
{
    g_u32BufferFrameNumber[u8_buffIndex] = u32_bufferNum;
}


static uint32_t USBH_UVC_GetBuffNumber(uint8_t u8_buffIndex)
{
    return g_u32BufferFrameNumber[u8_buffIndex];
}


static void  USBH_ParseFormatUncompDesc(uint8_t *buf, uint8_t index)
{
    uint8_t                i;

    g_stUVCFormatUncomp[index].bLength               = *(uint8_t  *) (buf + 0);
    g_stUVCFormatUncomp[index].bDescriptorType       = *(uint8_t  *) (buf + 1);
    g_stUVCFormatUncomp[index].bDescriptorSubtype    = *(uint8_t  *) (buf + 2);
    g_stUVCFormatUncomp[index].bFormatIndex          = *(uint8_t  *) (buf + 3);
    g_stUVCFormatUncomp[index].bNumFrameDescriptors  = *(uint8_t  *) (buf + 4);

    for (i = 0; i < 16; i++)
    {
        g_stUVCFormatUncomp[index].guidFormat[i]     = *(uint8_t  *) (buf + (5 + i));
    }

    g_stUVCFormatUncomp[index].bBitsPerPixel         = *(uint8_t  *) (buf + 21);
    g_stUVCFormatUncomp[index].bDefaultFrameIndex    = *(uint8_t  *) (buf + 22);
    g_stUVCFormatUncomp[index].bAspectRatioX         = *(uint8_t  *) (buf + 23);
    g_stUVCFormatUncomp[index].bAspectRatioY         = *(uint8_t  *) (buf + 24);
    g_stUVCFormatUncomp[index].bmInterlaceFlags      = *(uint8_t  *) (buf + 25);
    g_stUVCFormatUncomp[index].bCopyProtect          = *(uint8_t  *) (buf + 26);
}

static void  USBH_ParseFrameUncompDesc(uint8_t *buf, uint8_t index)
{
    g_stUVCFrameUncomp[index].bLength                    = *(uint8_t  *) (buf + 0);
    g_stUVCFrameUncomp[index].bDescriptorType            = *(uint8_t  *) (buf + 1);
    g_stUVCFrameUncomp[index].bDescriptorSubtype         = *(uint8_t  *) (buf + 2);
    g_stUVCFrameUncomp[index].bFrameIndex                = *(uint8_t  *) (buf + 3);
    g_stUVCFrameUncomp[index].bmCapabilities             = *(uint8_t  *) (buf + 4);
    g_stUVCFrameUncomp[index].wWidth                     = LE16(buf + 5);
    g_stUVCFrameUncomp[index].wHeight                    = LE16(buf + 7);
    g_stUVCFrameUncomp[index].dwMinBitRate               = LE32(buf + 9);
    g_stUVCFrameUncomp[index].dwMaxBitRate               = LE32(buf + 13);
    g_stUVCFrameUncomp[index].dwMaxVideoFrameBufferSize  = LE32(buf + 17);
    g_stUVCFrameUncomp[index].dwDefaultFrameInterval     = LE32(buf + 21);
    g_stUVCFrameUncomp[index].bFrameIntervalType         = *(uint8_t  *) (buf + 25);
    g_stUVCFrameUncomp[index].dwFrameInterval            = LE32(buf + 26);
}

uint16_t USBH_UVC_GetFrameWidth(uint8_t index)
{
    return g_stUVCFrameUncomp[index].wWidth;
}

uint16_t USBH_UVC_GetFrameHeight(uint8_t index)
{
    return g_stUVCFrameUncomp[index].wHeight;
}

uint8_t USBH_UVC_GetFrameIndex(uint8_t index)
{
    return g_stUVCFrameUncomp[index].bFrameIndex;
}

uint32_t USBH_UVC_GetFrameSize(uint8_t frameIndex)
{
    uint8_t         i;
    uint32_t        frameSize;
    uint8_t         bytePerPixel;

    for (i = 0; i < USB_UVC_MAX_FRAME_FORMATS_NUM; i++)
    {
        if (frameIndex == g_stUVCFrameUncomp[i].bFrameIndex)
        {
            break;
        }
    }

    if (i >= USB_UVC_MAX_FRAME_FORMATS_NUM)
    {
        i = 0;
    }

    frameSize       = (g_stUVCFrameUncomp[i].wWidth * g_stUVCFrameUncomp[i].wHeight);

    bytePerPixel    = (g_stUVCFormatUncomp[0].bBitsPerPixel >> 3);

    if (bytePerPixel != 0)
    {
        frameSize   = (frameSize * bytePerPixel);
    }

    dlog_info("frameSize: %d", frameSize);

    return frameSize;
}

static void USBH_ParseProcessingUnitDesc(uint8_t * buf)
{
    uint8_t                 i;

    g_stProcessingUnitDesc.bLength              = *(uint8_t  *) (buf + 0);
    g_stProcessingUnitDesc.bDescriptorType      = *(uint8_t  *) (buf + 1);
    g_stProcessingUnitDesc.bDescriptorSubtype   = *(uint8_t  *) (buf + 2);
    g_stProcessingUnitDesc.bUnitID              = *(uint8_t  *) (buf + 3);
    g_stProcessingUnitDesc.bSourceID            = *(uint8_t  *) (buf + 4);
    g_stProcessingUnitDesc.wMaxMultiplier       = LE16(buf + 5);
    g_stProcessingUnitDesc.bControlSize         = *(uint8_t  *) (buf + 7);

    for (i = 0; i < g_stProcessingUnitDesc.bControlSize; i++)
    {
        g_stProcessingUnitDesc.bmControls[i]    = *(uint8_t  *) (buf + (8 + i));
    }

    g_stProcessingUnitDesc.iProcessing          = *(uint8_t  *) (buf + (8 + i));
    g_stProcessingUnitDesc.bmVideoStandards     = *(uint8_t  *) (buf + (9 + i));

}


static void USBH_ParseExtensionUnitDesc(uint8_t * buf)
{
    uint8_t                 i;

    g_stExtensionUnitDesc.bLength              = *(uint8_t  *) (buf + 0);
    g_stExtensionUnitDesc.bDescriptorType      = *(uint8_t  *) (buf + 1);
    g_stExtensionUnitDesc.bDescriptorSubtype   = *(uint8_t  *) (buf + 2);
    g_stExtensionUnitDesc.bUnitID              = *(uint8_t  *) (buf + 3);

    for (i = 0; i < 16; i++)
    {
        g_stExtensionUnitDesc.guidExtensionCode[i]
                                               = *(uint8_t  *) (buf + (4 + i));
    }

    g_stExtensionUnitDesc.bNumControls         = *(uint8_t  *) (buf + 20);
    g_stExtensionUnitDesc.bNrInPins            = *(uint8_t  *) (buf + 21);

    for (i = 0; i < g_stExtensionUnitDesc.bNrInPins; i++)
    {
        g_stExtensionUnitDesc.baSourceID[i]    = *(uint8_t  *) (buf + (22 + i));
    }

    g_stExtensionUnitDesc.bControlSize         = *(uint8_t  *) (buf + (22 + g_stExtensionUnitDesc.bNrInPins));

    for (i = 0; i < g_stExtensionUnitDesc.bControlSize; i++)
    {
        g_stExtensionUnitDesc.bmControls[i]    = *(uint8_t  *) (buf + ((23 + g_stExtensionUnitDesc.bNrInPins) + i));
    }

    g_stExtensionUnitDesc.iExtension           = *(uint8_t  *) (buf + ((23 + g_stExtensionUnitDesc.bNrInPins) + g_stExtensionUnitDesc.bControlSize));

    dlog_info("bLength: %d, bDescriptorType: %d, bDescriptorSubtype: %d, bUnitID: %d, guidExtensionCode: %d",
                g_stExtensionUnitDesc.bLength,
                g_stExtensionUnitDesc.bDescriptorType,
                g_stExtensionUnitDesc.bDescriptorSubtype,
                g_stExtensionUnitDesc.bUnitID,
                g_stExtensionUnitDesc.guidExtensionCode[0]);

    dlog_info("bNumControls: %d, bNrInPins: %d, baSourceID[0]: %d, bControlSize: %d, bmControls[0]: %d, iExtension: %d",
                g_stExtensionUnitDesc.bNumControls,
                g_stExtensionUnitDesc.bNrInPins,
                g_stExtensionUnitDesc.baSourceID[0],
                g_stExtensionUnitDesc.bControlSize,
                g_stExtensionUnitDesc.bmControls[0],
                g_stExtensionUnitDesc.iExtension);

}


void USBH_UVC_GetVideoFormatList(USBH_HandleTypeDef *phost)
{
    USBH_DescHeader_t                 *pdesc;
    uint16_t                           ptr = 0;
    uint8_t                           *buff;
    uint8_t                            formatUncompIndex = 0;
    uint8_t                            frameUncompIndex = 0;
    USBH_CfgDescTypeDef               *cfg_desc;
    USBH_UVCInterfaceDescriptor       *if_desc;

    cfg_desc                      = &(phost->device.CfgDesc);
    pdesc                         = (USBH_DescHeader_t *)(&(phost->device.CfgDesc_Raw));

    while (ptr < cfg_desc->wTotalLength)
    {
        pdesc = USBH_GetNextDesc((uint8_t *)pdesc, &ptr);

        if (pdesc->bDescriptorType   == USB_DESC_TYPE_INTERFACE)
        {
            if_desc               = (USBH_UVCInterfaceDescriptor *)pdesc;
        }

        if (if_desc->bInterfaceSubClass == USB_UVC_SUBCLASS_VIDEO_STREAMING)
        {
            if (pdesc->bDescriptorType   == USB_DESC_TYPE_CS_INTERFACE)
            {
                buff    = (uint8_t *)pdesc;

                if (buff[2] == USB_UVC_FORMAT_UNCOMPRESSED)
                {
                    USBH_ParseFormatUncompDesc(buff, formatUncompIndex);

//                    dlog_info("formatUncompIndex: %d", formatUncompIndex);

                    formatUncompIndex++;
                }
                else if (buff[2] == USB_UVC_FRAME_UNCOMPRESSED)
                {
                    USBH_ParseFrameUncompDesc(buff, frameUncompIndex);

//                    dlog_info("frameUncompIndex: %d", frameUncompIndex);

                    frameUncompIndex++;
                }
            }
        }
        else if (if_desc->bInterfaceSubClass == USB_UVC_SUBCLASS_VIDEO_CONTROL)
        {
            if (pdesc->bDescriptorType   == USB_DESC_TYPE_CS_INTERFACE)
            {
                buff    = (uint8_t *)pdesc;

                if (buff[2] == USB_UVC_PROCESSING_UNIT)
                {
                    USBH_ParseProcessingUnitDesc(buff);
                }
                else if (buff[2] == USB_UVC_EXTENSION_UNIT)
                {
                    USBH_ParseExtensionUnitDesc(buff);
                }
            }
        }
    }
}


static uint8_t USBH_UVC_FindStreamingInterface(USBH_HandleTypeDef *phost)
{
    uint8_t                       i;
    USBH_CfgDescTypeDef          *cfg_desc;
    USBH_InterfaceDescTypeDef    *interface_desc;

    cfg_desc                      = &(phost->device.CfgDesc);

    for (i = 2; i < USBH_MAX_NUM_INTERFACES; i++)
    {
        interface_desc              = &cfg_desc->Itf_Desc[i];
#if 1
        dlog_info("bInterfaceClass: %d", interface_desc->bInterfaceClass);
        dlog_info("bInterfaceSubClass: %d", interface_desc->bInterfaceSubClass);
        dlog_info("bNumEndpoints: %d", interface_desc->bNumEndpoints);
        dlog_info("bInterfaceNumber: %d", interface_desc->bInterfaceNumber);
        dlog_info("bAlternateSetting: %d", interface_desc->bAlternateSetting);
        dlog_info("wMaxPacketSize: %d", interface_desc->Ep_Desc[0].wMaxPacketSize);

        dlog_output(200);
#endif
        if ((interface_desc->bInterfaceClass == UVC_CLASS)&&
            (interface_desc->bInterfaceSubClass == USB_UVC_SUBCLASS_VIDEO_STREAMING)&&
            (interface_desc->bNumEndpoints > 0))
        {
            continue;
        }
        else
        {
            break;
        }
    }

    return (i-1);
}


uint32_t USBH_UVC_GetProcUnitControls(void)
{
    uint32_t                u32_procUnitControls;

    u32_procUnitControls    = LE32(&g_stProcessingUnitDesc.bmControls[0]);

    dlog_info("u32_procUnitControls: %08x", u32_procUnitControls);

    return u32_procUnitControls;
}


uint32_t USBH_UVC_GetExtUnitControls(void)
{
    uint32_t                u32_extUnitControls;

    u32_extUnitControls     = LE32(&g_stExtensionUnitDesc.bmControls[0]);

    dlog_info("u32_extUnitControls: %08x", u32_extUnitControls);

    return u32_extUnitControls;
}


uint32_t USBH_UVC_ProcUnitParamHandler(USBH_HandleTypeDef *phost, uint8_t index, UVC_GetParamTypeDef enParamType)
{
    uint32_t      timeout = 0;
    if ((index < 16)&&
        (hUsbhUVCParamHandler[index] != NULL))
    {
        while (0xFFFFFFFF == hUsbhUVCParamHandler[index](phost, enParamType))
        {
            timeout++;

            if (timeout >= 200000)
            {
                dlog_error("get uvc param timeout");
                break;
            }
        }
    }
}



