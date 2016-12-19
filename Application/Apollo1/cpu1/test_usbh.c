#include "test_usbh.h"
#include "debuglog.h"
#include "interrupt.h"
#include "systicks.h"


/* USB Host Global Variables */
USBH_HandleTypeDef              hUSBHost;
USBH_AppCtrl                    g_usbhAppCtrl;
uint32_t                        g_usbhUVCStarted = 0;


void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id)
{
    switch(id)
    {
    case HOST_USER_SELECT_CONFIGURATION:
        break;

    case HOST_USER_DISCONNECTION:
        g_usbhAppCtrl.usbhAppState  = APPLICATION_DISCONNECT;
        break;

    case HOST_USER_CLASS_ACTIVE:
        g_usbhAppCtrl.usbhAppState  = APPLICATION_READY;
        break;

    case HOST_USER_CONNECTION:
        break;

    default:
        break;
    }
}


void USBH_ApplicationInit(void)
{
    reg_IrqHandle(OTG_INTR1_VECTOR_NUM, USB_LL_OTG1_IRQHandler);

    USBH_Init(&hUSBHost, USBH_UserProcess, 1);

    USBH_RegisterClass(&hUSBHost, USBH_UVC_CLASS);

    USBH_Start(&hUSBHost);
}


void USBH_MountUSBDisk(void)
{
    FRESULT   fileResult;

    FATFS_LinkDriver(&USBH_Driver, "0:/");

    fileResult = f_mount(&(g_usbhAppCtrl.usbhAppFatFs), "0:/", 0);

    if (fileResult != FR_OK)
    {
        dlog_error("mount fatfs error: %d\n", fileResult);

        return;
    }
}


void USBH_USBHostStatus(void)
{
    USBH_Process(&hUSBHost);
}


void USBH_ProcUVC(void)
{
    // if UVC is not started
    if ((APPLICATION_READY == g_usbhAppCtrl.usbhAppState)
      &&(0 == g_usbhUVCStarted))
    {
        g_usbhUVCStarted = 1;

        dlog_info("startuvc");

        USBH_UVC_StartView(&hUSBHost);
    }
    else if (APPLICATION_DISCONNECT == g_usbhAppCtrl.usbhAppState)
    {
        g_usbhUVCStarted = 0;
    }
}



