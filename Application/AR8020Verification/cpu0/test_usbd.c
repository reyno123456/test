#include "test_usbd.h"
#include "usbd_hid_desc.h"
#include "usbd_hid.h"
#include "debuglog.h"
#include "interrupt.h"
#include "command.h"
#include "raw_video_data.h"

USBD_HandleTypeDef USBD_Device;




void TestUsbd_InitHid(void)
{
    reg_IrqHandle(OTG_INTR0_VECTOR_NUM, USB_LL_OTG0_IRQHandler);

    USBD_Init(&USBD_Device, &HID_Desc, 0);

    USBD_RegisterClass(&USBD_Device, USBD_HID_CLASS);

    USBD_Start(&USBD_Device);

    dlog_info("usb device init done!\n");

    return;
}


