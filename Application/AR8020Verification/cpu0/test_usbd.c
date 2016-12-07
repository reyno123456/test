#include "test_usbd.h"
#include "usbd_hid_desc.h"
#include "usbd_hid.h"
#include "debuglog.h"
#include "interrupt.h"
#include "command.h"
#include "cmsis_os.h"
#include "test_sram.h"
#include "test_usbh.h"

USBD_HandleTypeDef          USBD_Device;
osMessageQId                USBD_AppEvent;

USBH_HandleTypeDef              hUSBHost;
USBH_BypassVideoCtrl            g_usbhBypassVideoCtrl;
USBH_AppCtrl                    g_usbhAppCtrl;


void USBD_ApplicationInit(void)
{
    reg_IrqHandle(OTG_INTR0_VECTOR_NUM, USB_LL_OTG0_IRQHandler);

    USBD_Init(&USBD_Device, &HID_Desc, 0);

    USBD_RegisterClass(&USBD_Device, USBD_HID_CLASS);

    USBD_Start(&USBD_Device);

    dlog_info("usb device init done!\n");

    return;
}

static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id)
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
void USBH_HApplicationInit(void)
{
    reg_IrqHandle(OTG_INTR0_VECTOR_NUM, USB_LL_OTG0_IRQHandler);

    USBH_Init(&hUSBHost, USBH_UserProcess, 0);

    USBH_RegisterClass(&hUSBHost, USBH_MSC_CLASS);

    USBH_Start(&hUSBHost);
    dlog_info("usb host init done!\n");
    return;
}


void USBD_MainTask(void const *argument)
{
    osEvent     event;
    uint8_t     buffer[30] = "test video or ctrl!";

    USBD_ApplicationInit();
    USBH_HApplicationInit();

    USBH_MountUSBDisk();

    test_sram_init();

    while (1)
    {
        event = osMessageGet(USBD_AppEvent, osWaitForever);

        if (event.status == osEventMessage)
        {
            switch (event.value.v)
            {
            case USBD_APP_SEND_CTRL:
                {
                    dlog_info("send ctrl info\n");

                    if (USBD_OK != USBD_HID_SendReport(&USBD_Device, buffer, sizeof(buffer), HID_EPIN_CTRL_ADDR))
                    {
                        dlog_error("send fail!\n");
                    }
                }
                break;

            case USBD_APP_SEND_VIDEO:
                {
                    dlog_info("send video info\n");

                    if (USBD_OK != USBD_HID_SendReport(&USBD_Device, buffer, sizeof(buffer), HID_EPIN_VIDEO_ADDR))
                    {
                        dlog_error("send fail!\n");
                    }
                }
                break;

            default:
                break;
            }
        }
    }
}


