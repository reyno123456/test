#include "usbh_uvc.h"
#include "debuglog.h"
#include "systicks.h"

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
uint8_t                 g_UVCRecvBuffer[UVC_VIDEO_EP_MAX_SIZE];
uint8_t                 g_UVCVideoBuffer[UVC_VIDEO_BUFF_FRAME_NUM][UVC_VIDEO_BUFF_SIZE_PER_FRAME];
uint8_t                 g_u8curVideoBuffIndex = 0;
uint32_t                g_u32recvBuffPos = 0;
UVC_BuffStateTypeDef    g_enumUVCBuffState[UVC_VIDEO_BUFF_FRAME_NUM];


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

        // video interface
        interface = USBH_FindInterface(phost, 14, 2, 0);

        if(interface == 0xFF) /* Not Valid Interface */
        {
            dlog_info("Cannot Find the interface for %s class.", phost->pActiveClass->Name);

            status = USBH_FAIL;
        }
        else
        {
            USBH_SelectInterface(phost, interface);

            //UVC_Handle->VideoEp      = (phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].bEndpointAddress);
            //UVC_Handle->VideoEpSize  = phost->device.CfgDesc.Itf_Desc[phost->device.current_interface].Ep_Desc[0].wMaxPacketSize;

            UVC_Handle->VideoEp      = UVC_VIDEO_EP;
            UVC_Handle->VideoEpSize  = UVC_VIDEO_EP_MAX_SIZE;
        }

        UVC_Handle->uvc_state   = UVC_STATE_INIT;
        UVC_Handle->CtrlPipe    = USBH_AllocPipe(phost, UVC_Handle->CtrlEp);
        UVC_Handle->VideoPipe   = USBH_AllocPipe(phost, UVC_Handle->VideoEp);

        dlog_info("ctrlpipe: %d, videopipe: %d", UVC_Handle->CtrlPipe, UVC_Handle->VideoPipe);

        /* Open the new channels */
        USBH_OpenPipe  (phost,
                        UVC_Handle->CtrlPipe,
                        UVC_Handle->CtrlEp,
                        phost->device.address,
                        phost->device.speed,
                        USB_EP_TYPE_INTR,
                        UVC_Handle->CtrlEpSize);

        USBH_OpenPipe  (phost,
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
    UVC_HandleTypeDef *UVC_Handle =  (UVC_HandleTypeDef *) phost->pActiveClass->pData;

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

    return USBH_OK;
}


static USBH_StatusTypeDef USBH_UVC_ClassRequest(USBH_HandleTypeDef *phost)
{
    UVC_HandleTypeDef        *UVC_Handle =  (UVC_HandleTypeDef *)phost->pActiveClass->pData;
    USBH_StatusTypeDef        status     = USBH_BUSY;

    switch (UVC_Handle->uvc_state)
    {
    case UVC_STATE_INIT:

    case UVC_STATE_SET_INTERFACE1:
        if (USBH_OK == USBH_SetInterface(phost, 1, 0))
        {
            UVC_Handle->uvc_state = UVC_STATE_SET_INTERFACE2;
        }
        break;

    case UVC_STATE_SET_INTERFACE2:
        if (USBH_OK == USBH_SetInterface(phost, 1, 0))
        {
            UVC_Handle->uvc_state = UVC_STATE_GET_CSPARAM;

            UVC_Handle->uvc_getParamState   = UVC_STATE_GET_LEN;
            UVC_Handle->uvc_CSCount           = 1;
        }
        break;

    case UVC_STATE_GET_CSPARAM:

        if (USBH_OK == USBH_UVC_GetCSParam(phost))
        {
            UVC_Handle->uvc_state           = UVC_STATE_GET_BRIGHTNESS;
            UVC_Handle->uvc_getParamState   = UVC_STATE_GET_INFO;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_GET_BRIGHTNESS:

        if (USBH_OK == USBH_UVC_GetBrightness(phost))
        {
            UVC_Handle->uvc_state           = UVC_STATE_GET_CONTRAST;
            UVC_Handle->uvc_getParamState   = UVC_STATE_GET_INFO;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_GET_CONTRAST:

        if (USBH_OK == USBH_UVC_GetContrast(phost))
        {
            UVC_Handle->uvc_state           = UVC_STATE_GET_HUE;
            UVC_Handle->uvc_getParamState   = UVC_STATE_GET_INFO;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_GET_HUE:

        if (USBH_OK == USBH_UVC_GetHUE(phost))
        {
            UVC_Handle->uvc_state           = UVC_STATE_GET_SATUATION;
            UVC_Handle->uvc_getParamState   = UVC_STATE_GET_INFO;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_GET_SATUATION:

        if (USBH_OK == USBH_UVC_GetSaturation(phost))
        {
            UVC_Handle->uvc_state           = UVC_STATE_GET_SHARPNESS;
            UVC_Handle->uvc_getParamState   = UVC_STATE_GET_INFO;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_GET_SHARPNESS:

        if (USBH_OK == USBH_UVC_GetSharpness(phost))
        {
            UVC_Handle->uvc_state           = UVC_STATE_GET_GAMMA;
            UVC_Handle->uvc_getParamState   = UVC_STATE_GET_INFO;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_GET_GAMMA:

        if (USBH_OK == USBH_UVC_GetGamma(phost))
        {
            UVC_Handle->uvc_state           = UVC_STATE_GET_WHITE_BALANCE;
            UVC_Handle->uvc_getParamState   = UVC_STATE_GET_INFO;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_GET_WHITE_BALANCE:

        if (USBH_OK == USBH_UVC_GetWhite(phost))
        {
            UVC_Handle->uvc_state           = UVC_STATE_GET_BACKLIGHT;
            UVC_Handle->uvc_getParamState   = UVC_STATE_GET_INFO;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_GET_BACKLIGHT:

        if (USBH_OK == USBH_UVC_GetBack(phost))
        {
            UVC_Handle->uvc_state           = UVC_STATE_GET_POWER_LINE;
            UVC_Handle->uvc_getParamState   = UVC_STATE_GET_INFO;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_GET_POWER_LINE:

        if (USBH_OK == USBH_UVC_GetPowerLine(phost))
        {
            UVC_Handle->uvc_state           = UVC_STATE_PROBE1;
            UVC_Handle->uvc_getParamState   = UVC_PROBE_STATE_GET_CUR;
            UVC_Handle->probeCount          = 0;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_PROBE1:
        if (USBH_OK == USBH_UVC_Probe(phost))
        {
            UVC_Handle->uvc_state           = UVC_STATE_PROBE2;
            UVC_Handle->uvc_getParamState   = UVC_PROBE_STATE_GET_CUR;
            UVC_Handle->probeCount          = 0;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_PROBE2:

        if (USBH_OK == USBH_UVC_Probe(phost))
        {
            UVC_Handle->uvc_state           = UVC_STATE_CS_FINISH;

            status = USBH_OK;

            phost->pUser(phost, HOST_USER_CLASS_ACTIVE); 
        }

        break;

    default:
        break;

    }

    return status; 
}


static USBH_StatusTypeDef USBH_UVC_Process (USBH_HandleTypeDef *phost)
{
    UVC_HandleTypeDef        *UVC_Handle =  (UVC_HandleTypeDef *)phost->pActiveClass->pData;
    USBH_StatusTypeDef        status = USBH_BUSY;

    switch (UVC_Handle->uvc_state)
    {
    case UVC_STATE_PROBE3:
        if (USBH_OK == USBH_UVC_Probe(phost))
        {
            UVC_Handle->uvc_state           = UVC_STATE_COMMIT;
            UVC_Handle->uvc_getParamState   = UVC_PROBE_STATE_SET_CUR;
            UVC_Handle->probeCount          = 0;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }
        break;

    case UVC_STATE_COMMIT:
        if (USBH_OK == USBH_UVC_Commit(phost))
        {
            UVC_Handle->uvc_state           = UVC_STATE_ERROR;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
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
    case UVC_STATE_GET_LEN:
        phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
        phost->Control.setup.b.bRequest = UVC_GET_LEN;
        phost->Control.setup.b.wValue.w = (uint16_t)((UVC_Handle->uvc_CSCount)<<8);
        phost->Control.setup.b.wIndex.w = 0x0300;
        phost->Control.setup.b.wLength.w = 0x0002;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_INFO;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }
        break;

    case UVC_STATE_GET_INFO:
        phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
        phost->Control.setup.b.bRequest = UVC_GET_INFO;
        phost->Control.setup.b.wValue.w = (uint16_t)((UVC_Handle->uvc_CSCount)<<8);
        phost->Control.setup.b.wIndex.w = 0x0300;
        phost->Control.setup.b.wLength.w = 0x0001;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_MIN;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }
        break;

    case UVC_STATE_GET_MIN:
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

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_MAX;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;
        
    case UVC_STATE_GET_MAX:
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

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_RES;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_GET_RES:
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

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_DEF;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_GET_DEF:
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

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_CSCount++;

            if (UVC_Handle->uvc_CSCount >= 6)
            {
                status  = USBH_OK;
            }
            else
            {
                UVC_Handle->uvc_getParamState = UVC_STATE_GET_LEN;
            }

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    default:
        break;

    }

    return status;
}


static USBH_StatusTypeDef USBH_UVC_GetBrightness(USBH_HandleTypeDef *phost)
{
    UVC_HandleTypeDef        *UVC_Handle =  (UVC_HandleTypeDef *)phost->pActiveClass->pData;
    USBH_StatusTypeDef        status = USBH_BUSY;

    phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
    phost->Control.setup.b.wValue.w = 0x0200;
    phost->Control.setup.b.wIndex.w = 0x0200;
    phost->Control.setup.b.wLength.w = 0x0002;

    switch (UVC_Handle->uvc_getParamState)
    {
    case UVC_STATE_GET_INFO:
        phost->Control.setup.b.bRequest = UVC_GET_INFO;
        phost->Control.setup.b.wLength.w = 0x0001;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_MIN;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }
        break;

    case UVC_STATE_GET_MIN:
        phost->Control.setup.b.bRequest = UVC_GET_MIN;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_MAX;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;
        
    case UVC_STATE_GET_MAX:
        phost->Control.setup.b.bRequest = UVC_GET_MAX;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_RES;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_GET_RES:
        phost->Control.setup.b.bRequest = UVC_GET_RES;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_DEF;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_GET_DEF:
        phost->Control.setup.b.bRequest = UVC_GET_DEF;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            status  = USBH_OK;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    default:
        break;

    }

    return status;
}


static USBH_StatusTypeDef USBH_UVC_GetContrast(USBH_HandleTypeDef *phost)
{
    UVC_HandleTypeDef        *UVC_Handle =  (UVC_HandleTypeDef *)phost->pActiveClass->pData;
    USBH_StatusTypeDef        status = USBH_BUSY;

    phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
    phost->Control.setup.b.wValue.w = 0x0300;
    phost->Control.setup.b.wIndex.w = 0x0200;
    phost->Control.setup.b.wLength.w = 0x0002;

    switch (UVC_Handle->uvc_getParamState)
    {
    case UVC_STATE_GET_INFO:
        phost->Control.setup.b.bRequest = UVC_GET_INFO;
        phost->Control.setup.b.wLength.w = 0x0001;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_MIN;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }
        break;

    case UVC_STATE_GET_MIN:
        phost->Control.setup.b.bRequest = UVC_GET_MIN;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_MAX;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;
        
    case UVC_STATE_GET_MAX:
        phost->Control.setup.b.bRequest = UVC_GET_MAX;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_RES;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_GET_RES:
        phost->Control.setup.b.bRequest = UVC_GET_RES;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_DEF;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_GET_DEF:
        phost->Control.setup.b.bRequest = UVC_GET_DEF;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            status  = USBH_OK;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    default:
        break;

    }

    return status;
}


static USBH_StatusTypeDef USBH_UVC_GetHUE(USBH_HandleTypeDef *phost)
{
    UVC_HandleTypeDef        *UVC_Handle =  (UVC_HandleTypeDef *)phost->pActiveClass->pData;
    USBH_StatusTypeDef        status = USBH_BUSY;

    phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
    phost->Control.setup.b.wValue.w = 0x0600;
    phost->Control.setup.b.wIndex.w = 0x0200;
    phost->Control.setup.b.wLength.w = 0x0002;

    switch (UVC_Handle->uvc_getParamState)
    {
    case UVC_STATE_GET_INFO:
        phost->Control.setup.b.bRequest = UVC_GET_INFO;
        phost->Control.setup.b.wLength.w = 0x0001;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_MIN;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }
        break;

    case UVC_STATE_GET_MIN:
        phost->Control.setup.b.bRequest = UVC_GET_MIN;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_MAX;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;
        
    case UVC_STATE_GET_MAX:
        phost->Control.setup.b.bRequest = UVC_GET_MAX;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_RES;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_GET_RES:
        phost->Control.setup.b.bRequest = UVC_GET_RES;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_DEF;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_GET_DEF:
        phost->Control.setup.b.bRequest = UVC_GET_DEF;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            status  = USBH_OK;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    default:
        break;

    }

    return status;
}


static USBH_StatusTypeDef USBH_UVC_GetSaturation(USBH_HandleTypeDef *phost)
{
    UVC_HandleTypeDef        *UVC_Handle =  (UVC_HandleTypeDef *)phost->pActiveClass->pData;
    USBH_StatusTypeDef        status = USBH_BUSY;

    phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
    phost->Control.setup.b.wValue.w = 0x0700;
    phost->Control.setup.b.wIndex.w = 0x0200;
    phost->Control.setup.b.wLength.w = 0x0002;

    switch (UVC_Handle->uvc_getParamState)
    {
    case UVC_STATE_GET_INFO:
        phost->Control.setup.b.bRequest = UVC_GET_INFO;
        phost->Control.setup.b.wLength.w = 0x0001;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_MIN;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }
        break;

    case UVC_STATE_GET_MIN:
        phost->Control.setup.b.bRequest = UVC_GET_MIN;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_MAX;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;
        
    case UVC_STATE_GET_MAX:
        phost->Control.setup.b.bRequest = UVC_GET_MAX;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_RES;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_GET_RES:
        phost->Control.setup.b.bRequest = UVC_GET_RES;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_DEF;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_GET_DEF:
        phost->Control.setup.b.bRequest = UVC_GET_DEF;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            status  = USBH_OK;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    default:
        break;

    }

    return status;
}


static USBH_StatusTypeDef USBH_UVC_GetSharpness(USBH_HandleTypeDef *phost)
{
    UVC_HandleTypeDef        *UVC_Handle =  (UVC_HandleTypeDef *)phost->pActiveClass->pData;
    USBH_StatusTypeDef        status = USBH_BUSY;

    phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
    phost->Control.setup.b.wValue.w = 0x0800;
    phost->Control.setup.b.wIndex.w = 0x0200;
    phost->Control.setup.b.wLength.w = 0x0002;

    switch (UVC_Handle->uvc_getParamState)
    {
    case UVC_STATE_GET_INFO:
        phost->Control.setup.b.bRequest = UVC_GET_INFO;
        phost->Control.setup.b.wLength.w = 0x0001;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_MIN;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }
        break;

    case UVC_STATE_GET_MIN:
        phost->Control.setup.b.bRequest = UVC_GET_MIN;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_MAX;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;
        
    case UVC_STATE_GET_MAX:
        phost->Control.setup.b.bRequest = UVC_GET_MAX;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_RES;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_GET_RES:
        phost->Control.setup.b.bRequest = UVC_GET_RES;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_DEF;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_GET_DEF:
        phost->Control.setup.b.bRequest = UVC_GET_DEF;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            status  = USBH_OK;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    default:
        break;

    }

    return status;
}


static USBH_StatusTypeDef USBH_UVC_GetGamma(USBH_HandleTypeDef *phost)
{
    UVC_HandleTypeDef        *UVC_Handle =  (UVC_HandleTypeDef *)phost->pActiveClass->pData;
    USBH_StatusTypeDef        status = USBH_BUSY;

    phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
    phost->Control.setup.b.wValue.w = 0x0900;
    phost->Control.setup.b.wIndex.w = 0x0200;
    phost->Control.setup.b.wLength.w = 0x0002;

    switch (UVC_Handle->uvc_getParamState)
    {
    case UVC_STATE_GET_INFO:
        phost->Control.setup.b.bRequest = UVC_GET_INFO;
        phost->Control.setup.b.wLength.w = 0x0001;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_MIN;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }
        break;

    case UVC_STATE_GET_MIN:
        phost->Control.setup.b.bRequest = UVC_GET_MIN;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_MAX;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;
        
    case UVC_STATE_GET_MAX:
        phost->Control.setup.b.bRequest = UVC_GET_MAX;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_RES;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_GET_RES:
        phost->Control.setup.b.bRequest = UVC_GET_RES;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_DEF;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_GET_DEF:
        phost->Control.setup.b.bRequest = UVC_GET_DEF;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            status  = USBH_OK;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    default:
        break;

    }

    return status;
}


static USBH_StatusTypeDef USBH_UVC_GetWhite(USBH_HandleTypeDef *phost)
{
    UVC_HandleTypeDef        *UVC_Handle =  (UVC_HandleTypeDef *)phost->pActiveClass->pData;
    USBH_StatusTypeDef        status = USBH_BUSY;

    phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
    phost->Control.setup.b.wValue.w = 0x0a00;
    phost->Control.setup.b.wIndex.w = 0x0200;
    phost->Control.setup.b.wLength.w = 0x0002;

    switch (UVC_Handle->uvc_getParamState)
    {
    case UVC_STATE_GET_INFO:
        phost->Control.setup.b.bRequest = UVC_GET_INFO;
        phost->Control.setup.b.wLength.w = 0x0001;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_MIN;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }
        break;

    case UVC_STATE_GET_MIN:
        phost->Control.setup.b.bRequest = UVC_GET_MIN;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_MAX;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;
        
    case UVC_STATE_GET_MAX:
        phost->Control.setup.b.bRequest = UVC_GET_MAX;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_RES;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_GET_RES:
        phost->Control.setup.b.bRequest = UVC_GET_RES;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_DEF;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_GET_DEF:
        phost->Control.setup.b.bRequest = UVC_GET_DEF;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            status  = USBH_OK;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    default:
        break;

    }

    return status;
}


static USBH_StatusTypeDef USBH_UVC_GetBack(USBH_HandleTypeDef *phost)
{
    UVC_HandleTypeDef        *UVC_Handle =  (UVC_HandleTypeDef *)phost->pActiveClass->pData;
    USBH_StatusTypeDef        status = USBH_BUSY;

    phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
    phost->Control.setup.b.wValue.w = 0x0100;
    phost->Control.setup.b.wIndex.w = 0x0200;
    phost->Control.setup.b.wLength.w = 0x0002;

    switch (UVC_Handle->uvc_getParamState)
    {
    case UVC_STATE_GET_INFO:
        phost->Control.setup.b.bRequest = UVC_GET_INFO;
        phost->Control.setup.b.wLength.w = 0x0001;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_MIN;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }
        break;

    case UVC_STATE_GET_MIN:
        phost->Control.setup.b.bRequest = UVC_GET_MIN;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_MAX;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;
        
    case UVC_STATE_GET_MAX:
        phost->Control.setup.b.bRequest = UVC_GET_MAX;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_RES;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_GET_RES:
        phost->Control.setup.b.bRequest = UVC_GET_RES;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_DEF;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_GET_DEF:
        phost->Control.setup.b.bRequest = UVC_GET_DEF;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            status  = USBH_OK;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    default:
        break;

    }

    return status;
}


static USBH_StatusTypeDef USBH_UVC_GetPowerLine(USBH_HandleTypeDef *phost)
{
    UVC_HandleTypeDef        *UVC_Handle =  (UVC_HandleTypeDef *)phost->pActiveClass->pData;
    USBH_StatusTypeDef        status = USBH_BUSY;

    phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
    phost->Control.setup.b.wValue.w = 0x0500;
    phost->Control.setup.b.wIndex.w = 0x0200;
    phost->Control.setup.b.wLength.w = 0x0001;

    switch (UVC_Handle->uvc_getParamState)
    {
    case UVC_STATE_GET_INFO:
        phost->Control.setup.b.bRequest = UVC_GET_INFO;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_MIN;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }
        break;

    case UVC_STATE_GET_MIN:
        phost->Control.setup.b.bRequest = UVC_GET_MIN;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_MAX;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;
        
    case UVC_STATE_GET_MAX:
        phost->Control.setup.b.bRequest = UVC_GET_MAX;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_RES;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_GET_RES:
        phost->Control.setup.b.bRequest = UVC_GET_RES;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_GET_DEF;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_GET_DEF:
        phost->Control.setup.b.bRequest = UVC_GET_DEF;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->mem) , 8))
        {
            status  = USBH_OK;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    default:
        break;

    }

    return status;
}


static USBH_StatusTypeDef USBH_UVC_Probe(USBH_HandleTypeDef *phost)
{
    UVC_HandleTypeDef        *UVC_Handle =  (UVC_HandleTypeDef *)phost->pActiveClass->pData;
    USBH_StatusTypeDef        status = USBH_BUSY;

    phost->Control.setup.b.wValue.w = 0x0100;
    phost->Control.setup.b.wIndex.w = 0x0001;
    phost->Control.setup.b.wLength.w = 0x001A;

    switch (UVC_Handle->uvc_getParamState)
    {
    case UVC_PROBE_STATE_GET_CUR:
        phost->Control.setup.b.bRequest = UVC_GET_CUR;
        phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->cur_mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_PROBE_STATE_GET_MAX;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }
        break;

    case UVC_PROBE_STATE_GET_MAX:
        phost->Control.setup.b.bRequest = UVC_GET_MAX;
        phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->max_mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_PROBE_STATE_GET_MIN;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_PROBE_STATE_GET_MIN:
        phost->Control.setup.b.bRequest = UVC_GET_MIN;
        phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->min_mem) , 8))
        {
            if (UVC_Handle->probeCount >= 1)
            {
                UVC_Handle->probeCount = 0;

                status = USBH_OK;
            }
            else
            {
                UVC_Handle->uvc_getParamState = UVC_PROBE_STATE_SET_CUR;
            }

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_PROBE_STATE_SET_CUR:
        phost->Control.setup.b.bRequest = UVC_SET_CUR;
        phost->Control.setup.b.bmRequestType = USB_H2D | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
        UVC_Handle->cur_mem[23] = 0;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->cur_mem) , 26))
        {
            UVC_Handle->probeCount++;

            UVC_Handle->uvc_getParamState = UVC_PROBE_STATE_GET_CUR;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    default:
        break;

    }

    return status;
}


static USBH_StatusTypeDef USBH_UVC_StillProbe(USBH_HandleTypeDef *phost)
{
    UVC_HandleTypeDef        *UVC_Handle =  (UVC_HandleTypeDef *)phost->pActiveClass->pData;
    USBH_StatusTypeDef        status = USBH_BUSY;

    phost->Control.setup.b.wValue.w = 0x0300;
    phost->Control.setup.b.wIndex.w = 0x0001;
    phost->Control.setup.b.wLength.w = 0x000B;

    switch (UVC_Handle->uvc_getParamState)
    {
    case UVC_PROBE_STATE_GET_CUR:
        phost->Control.setup.b.bRequest = UVC_GET_CUR;
        phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->still_probe_cur_mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_PROBE_STATE_GET_MIN;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }
        break;

    case UVC_PROBE_STATE_GET_MIN:
        phost->Control.setup.b.bRequest = UVC_GET_MIN;
        phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->min_mem) , 8))
        {
            UVC_Handle->uvc_getParamState = UVC_PROBE_STATE_GET_MAX;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_PROBE_STATE_GET_MAX:
        phost->Control.setup.b.bRequest = UVC_GET_MAX;
        phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->max_mem) , 8))
        {
            if (UVC_Handle->probeCount >= 1)
            {
                UVC_Handle->probeCount = 0;

                status = USBH_OK;
            }
            else
            {
                UVC_Handle->uvc_getParamState = UVC_PROBE_STATE_SET_CUR;
            }

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_PROBE_STATE_SET_CUR:
        phost->Control.setup.b.bRequest = UVC_SET_CUR;
        phost->Control.setup.b.bmRequestType = USB_H2D | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;

        UVC_Handle->still_probe_cur_mem[7] = 0;
        UVC_Handle->still_probe_cur_mem[8] = 0;
        UVC_Handle->still_probe_cur_mem[9] = 0;
        UVC_Handle->still_probe_cur_mem[10] = 0;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->still_probe_cur_mem) , 11))
        {
            UVC_Handle->probeCount++;

            UVC_Handle->uvc_getParamState = UVC_PROBE_STATE_GET_CUR;

#if (USBH_USE_OS == 1)
            osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
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

    switch (UVC_Handle->uvc_getParamState)
    {
    case UVC_PROBE_STATE_SET_CUR:
        phost->Control.setup.b.bRequest = UVC_SET_CUR;
        phost->Control.setup.b.bmRequestType = USB_H2D | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
        phost->Control.setup.b.wValue.w = 0x0100;
        phost->Control.setup.b.wIndex.w = 0x0001;
        phost->Control.setup.b.wLength.w = 0x001A;

        UVC_Handle->cur_mem[23] = 0;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->cur_mem) , 26))
        {

            UVC_Handle->uvc_getParamState = UVC_PROBE_STATE_GET_CUR;

#if (USBH_USE_OS == 1)
            osMessagePut(phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_PROBE_STATE_GET_CUR:
        phost->Control.setup.b.bRequest = UVC_GET_CUR;
        phost->Control.setup.b.bmRequestType = USB_D2H | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
        phost->Control.setup.b.wValue.w = 0x0100;
        phost->Control.setup.b.wIndex.w = 0x0001;
        phost->Control.setup.b.wLength.w = 0x001A;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->cur_mem) , 26))
        {
            UVC_Handle->uvc_getParamState = UVC_PROBE_STATE_SET_CUR_COMMIT;

#if (USBH_USE_OS == 1)
            osMessagePut(phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }
        break;

    case UVC_PROBE_STATE_SET_CUR_COMMIT:
        phost->Control.setup.b.bRequest = UVC_SET_CUR;
        phost->Control.setup.b.bmRequestType = USB_H2D | USB_REQ_RECIPIENT_INTERFACE | USB_REQ_TYPE_CLASS;
        phost->Control.setup.b.wValue.w = 0x0200;
        phost->Control.setup.b.wIndex.w = 0x0001;
        phost->Control.setup.b.wLength.w = 0x001A;

        if (USBH_OK == USBH_CtlReq(phost, (uint8_t *)(UVC_Handle->cur_mem) , 26))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_SET_INTERFACE1;

#if (USBH_USE_OS == 1)
            osMessagePut(phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }

        break;

    case UVC_STATE_SET_INTERFACE1:
        if (USBH_OK == USBH_SetInterface(phost, 1, 6))
        {
            UVC_Handle->uvc_getParamState = UVC_STATE_VIDEO_PLAY;

#if (USBH_USE_OS == 1)
            osMessagePut(phost->os_event, USBH_CLASS_EVENT, 0);
#endif
        }
        break;

    case UVC_STATE_VIDEO_PLAY:
        UVC_Handle->palying  = 1;
        USBH_IsocReceiveData(phost, g_UVCRecvBuffer, 1024, UVC_Handle->VideoPipe);

        status = USBH_OK;
#if (USBH_USE_OS == 1)
        osMessagePut(phost->os_event, USBH_CLASS_EVENT, 0);
#endif

        break;

    default:
        break;

    }

    return status;
}


void USBH_UVC_StartView(USBH_HandleTypeDef *phost)
{
    UVC_HandleTypeDef        *UVC_Handle =  (UVC_HandleTypeDef *)  phost->pActiveClass->pData;

    UVC_Handle->uvc_state           = UVC_STATE_PROBE3;
    UVC_Handle->uvc_getParamState   = UVC_PROBE_STATE_GET_CUR;
    UVC_Handle->probeCount          = 0;

#if (USBH_USE_OS == 1)
    osMessagePut ( phost->os_event, USBH_CLASS_EVENT, 0);
#endif
}


static void USBH_UVC_UrbDone(USBH_HandleTypeDef *phost)
{
    UVC_HandleTypeDef      *UVC_Handle;
    uint32_t                u32_recvSize;

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
            }
            else
            {
                if ((g_u32recvBuffPos + u32_recvSize) == UVC_VIDEO_BUFF_SIZE_PER_FRAME)
                {
                    //dma_transfer((uint32_t *)(g_UVCRecvBuffer + UVC_HEADER_SIZE), (uint32_t *)(g_UVCVideoBuffer[g_u8curVideoBuffIndex] + g_u32recvBuffPos), u32_recvSize);
                    memcpy((void *)(g_UVCVideoBuffer[g_u8curVideoBuffIndex] + g_u32recvBuffPos),
                           (void *)(g_UVCRecvBuffer + UVC_HEADER_SIZE),
                           u32_recvSize);

                    USBH_UVC_SetBuffState(g_u8curVideoBuffIndex, UVC_VIDEO_BUFF_VALID);

                    g_u8curVideoBuffIndex = ((++g_u8curVideoBuffIndex)%UVC_VIDEO_BUFF_FRAME_NUM);

                    if (USBH_UVC_GetBuffState(g_u8curVideoBuffIndex) != UVC_VIDEO_BUFF_IN_USING)
                    {
                        USBH_UVC_SetBuffState(g_u8curVideoBuffIndex, UVC_VIDEO_BUFF_EMPTY);
                    }
                    else
                    {
                        g_u8curVideoBuffIndex = ((++g_u8curVideoBuffIndex)%UVC_VIDEO_BUFF_FRAME_NUM);
                    }
                }

                g_u32recvBuffPos = 0;
            }

        }

    }

    USBH_IsocReceiveData(phost, g_UVCRecvBuffer, UVC_VIDEO_EP_MAX_SIZE, UVC_Handle->VideoPipe);
}


USBH_StatusTypeDef USBH_UVC_GetBuff(uint8_t *destAddr)
{
    uint8_t   i;

    for (i = 0; i < UVC_VIDEO_BUFF_FRAME_NUM; i++)
    {
        if (UVC_VIDEO_BUFF_VALID == USBH_UVC_GetBuffState(i))
        {
            USBH_UVC_SetBuffState(i, UVC_VIDEO_BUFF_IN_USING);

            //dma_transfer((uint32_t *)g_UVCVideoBuffer[i], (uint32_t *)destAddr, UVC_VIDEO_BUFF_SIZE_PER_FRAME);
            memcpy((void *)destAddr,
                   (void *)g_UVCVideoBuffer[i],
                   UVC_VIDEO_BUFF_SIZE_PER_FRAME);

            USBH_UVC_SetBuffState(i, UVC_VIDEO_BUFF_EMPTY);

            break;
        }
    }

    if (i >= UVC_VIDEO_BUFF_FRAME_NUM)
    {
        dlog_error("buff is empty");

        return USBH_FAIL;
    }

    return USBH_OK;
}


static void USBH_UVC_SetBuffState(uint8_t u8_buffIndex, UVC_BuffStateTypeDef e_buffState)
{
    g_enumUVCBuffState[u8_buffIndex] = e_buffState;
}


static UVC_BuffStateTypeDef USBH_UVC_GetBuffState(uint8_t u8_buffIndex)
{
    return g_enumUVCBuffState[u8_buffIndex];
}


