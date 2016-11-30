#include "test_cpu0_application.h"
#include "usbd_hid_desc.h"
#include "usbd_hid.h"
#include "debuglog.h"
#include "interrupt.h"
#include "command.h"
#include "cmsis_os.h"
#include "raw_video_data.h"

extern USBD_HandleTypeDef USBD_Device;
osMessageQId usbVideoReadyEvent;


void Application_InitResource(void)
{
    command_init();

    dlog_info("create task\n");

    /* create usb video task */
    osThreadDef(USB_VIDEO_Thread, Application_USBVideoTask, 0, 0, 1024);
    osThreadCreate(osThread(USB_VIDEO_Thread), NULL);

    dlog_info("create message queue\n");

    /* create message queue */
    osMessageQDef(osqueue, 1, uint16_t);
    usbVideoReadyEvent = osMessageCreate(osMessageQ(osqueue), NULL);

    osKernelStart();
}


static void Application_USBVideoTask(void const * argument)
{
    uint32_t      index = 0;
    osEvent       event;

    dlog_info("start usb video task\n");

    /* register interrupt for usb device */
    reg_IrqHandle(OTG_INTR0_VECTOR_NUM, USB_LL_OTG0_IRQHandler);

    USBD_Init(&USBD_Device, &HID_Desc, 0);

    USBD_RegisterClass(&USBD_Device, USBD_HID_CLASS);

    USBD_Start(&USBD_Device);

    for (;;)
    {
        dlog_info("waiting for the video ready interrupt\n");

        /* wait for the video ready interrupt */
        event = osMessageGet(usbVideoReadyEvent, osWaitForever);

        if (osEventMessage == event.status)
        {
            dlog_info("receive value: %d\n", event.value.v);

            if (80 == index)
            {
                USBD_HID_SendReport(&USBD_Device, g_rawDataBuffer2, 488, HID_EPIN_VIDEO_ADDR);
                index = 0;

                continue;
            }

            USBD_HID_SendReport(&USBD_Device, g_rawDataBuffer[index], 512, HID_EPIN_VIDEO_ADDR);
            index++;
        }
    }
}
