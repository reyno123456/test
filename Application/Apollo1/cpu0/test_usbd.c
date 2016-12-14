#include "test_usbd.h"
#include "usbd_hid_desc.h"
#include "usbd_hid.h"
#include "debuglog.h"
#include "interrupt.h"
#include "command.h"
#include "cmsis_os.h"
#include "test_sram.h"
#include "test_usbh.h"
#include "sys_event.h"
#include "sram.h"

USBD_HandleTypeDef          USBD_Device;
osMessageQId                USBD_AppEvent;



void USBD_ApplicationInit(void)
{
    reg_IrqHandle(OTG_INTR0_VECTOR_NUM, USB_LL_OTG0_IRQHandler);

    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_USB_PLUG_OUT, USBD_RestartUSBDevice);

    USBD_Init(&USBD_Device, &HID_Desc, 0);

    USBD_RegisterClass(&USBD_Device, USBD_HID_CLASS);

    USBD_Start(&USBD_Device);

    dlog_info("usb device init done!\n");

    return;
}



void USBD_MainTask(void const *argument)
{
    osEvent     event;
    uint8_t     buffer[30] = "test video or ctrl!";

    USBD_ApplicationInit();
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


void USBD_RestartUSBDevice(void * p)
{
    dlog_info("restart USB Device");

    USBD_LL_Init(&USBD_Device);
    HAL_PCD_Start(USBD_Device.pData);

    if (1 == sramReady0)
    {
        SRAM_Ready0Confirm();
    }
    
    if (1 == sramReady1)
    {
        SRAM_Ready1Confirm();
    }
}


