#include "test_usbh.h"
#include "debuglog.h"
#include "hal_usb_host.h"


uint32_t    g_usbhUVCStarted = 0;

uint8_t     g_usbhAppBuff[38400];

void USBH_ProcUVC(void)
{
    // if UVC is not started
    if (HAL_USB_HOST_STATE_READY == HAL_USB_GetHostAppState())
    {
        if (0 == g_usbhUVCStarted)
        {
            g_usbhUVCStarted = 1;

            dlog_info("startuvc");

            HAL_USB_StartUVC();
        }
        else
        {
            if (HAL_OK == HAL_USB_GetVideoFrame(g_usbhAppBuff))
            {
                dlog_info("app start: %02x, %02x, %02x, %02x",
                                            g_usbhAppBuff[0],
                                            g_usbhAppBuff[1],
                                            g_usbhAppBuff[2],
                                            g_usbhAppBuff[3]);

                dlog_info("app end: %02x, %02x, %02x, %02x",
                                            g_usbhAppBuff[38396],
                                            g_usbhAppBuff[38397],
                                            g_usbhAppBuff[38398],
                                            g_usbhAppBuff[38399]);
            }
        }
    }
    else if ((HAL_USB_HOST_STATE_DISCONNECT == HAL_USB_GetHostAppState())&&
             (1 == g_usbhUVCStarted))
    {
        g_usbhUVCStarted = 0;
    }
}



