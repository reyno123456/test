#include "test_usbd.h"
#include "usbd_hid_desc.h"
#include "usbd_hid.h"
#include "debuglog.h"
#include "interrupt.h"
#include "command.h"
#include "cmsis_os.h"


USBD_HandleTypeDef          USBD_Device;
osMessageQId                USBD_AppEvent;


void USBD_ApplicationInit(void)
{
    reg_IrqHandle(OTG_INTR0_VECTOR_NUM, USB_LL_OTG0_IRQHandler);

    USBD_Init(&USBD_Device, &HID_Desc, 0);

    USBD_RegisterClass(&USBD_Device, USBD_HID_CLASS);

    USBD_Start(&USBD_Device);

    dlog_info("usb device init done!\n");

    return;
}


void USBD_MainTask(void)
{
    osEvent event;

    USBD_ApplicationInit();

    test_sram_init();

    while (1)
    {
        event = osMessageGet(USBD_AppEvent, osWaitForever);

        if (event.status == osEventMessage)
        {
            switch (event.value.v)
            {
            case USBD_TASK_SRAM_0_READY:

                break;

            case USBD_TASK_SRAM_1_READY:

                break;

            case USBD_TASK_DEV_DISCONN:

                break;

            default:
                break;
            }
        }
    }
}


