#include "test_usbh.h"
#include "debuglog.h"
#include "hal_usb.h"


uint32_t    g_usbhUVCStarted = 0;


void USBH_ProcUVC(void)
{
    // if UVC is not started
    if ((HAL_USB_STATE_READY == HAL_USB_GetHostAppState())
      &&(0 == g_usbhUVCStarted))
    {
        g_usbhUVCStarted = 1;

        dlog_info("startuvc");

        HAL_USB_StartUVC();
    }
    else if ((HAL_USB_STATE_DISCONNECT == HAL_USB_GetHostAppState())&&
             (1 == g_usbhUVCStarted))
    {
        g_usbhUVCStarted = 0;
    }
}



