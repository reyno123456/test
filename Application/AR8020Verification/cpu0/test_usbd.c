#include "test_usbd.h"
#include "usbd_hid_desc.h"
#include "usbd_hid.h"
#include "debuglog.h"
#include "interrupt.h"
#include "command.h"
#include "raw_video_data.h"

USBD_HandleTypeDef USBD_Device;


extern uint32_t g_sendUSBFlag;

#if 0
void test_usbd_hid(void)
{
 //   reg_IrqHandle(USB_OTG0_VECTOR_NUM, USB_OTG0_IRQHandler);
    uint32_t index = 0;

    command_init();

    reg_IrqHandle(OTG_INTR0_VECTOR_NUM, USB_LL_OTG0_IRQHandler);

    dlog_info("0000\n");
    USBD_Init(&USBD_Device, &HID_Desc, 0);

    dlog_info("1111\n");

    USBD_RegisterClass(&USBD_Device, USBD_HID_CLASS);
    dlog_info("222222\n");

    USBD_Start(&USBD_Device);
    dlog_info("33333\n");

    while (1)
    {
        if (g_sendUSBFlag)
        {
            dlog_info("SEND BUFF\n");

            USBD_HID_SendReport(&USBD_Device, g_rawDataBuffer[index], 512);
            index++;

            if (index == 80)
            {
                index = 0;
                USBD_HID_SendReport(&USBD_Device, g_rawDataBuffer2, 488);
            }
        }
    }
}
#endif

void TestUsbd_InitHid(void)
{
    reg_IrqHandle(OTG_INTR0_VECTOR_NUM, USB_LL_OTG0_IRQHandler);

    dlog_info("0000\n");
    USBD_Init(&USBD_Device, &HID_Desc, 0);

    dlog_info("1111\n");

    USBD_RegisterClass(&USBD_Device, USBD_HID_CLASS);
    dlog_info("222222\n");

    USBD_Start(&USBD_Device);
    dlog_info("33333\n");

	return;
}


