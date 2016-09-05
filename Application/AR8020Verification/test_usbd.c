#include "test_usbd.h"
#include "usbd_hid_desc.h"
#include "usbd_hid.h"
#include "debuglog.h"

USBD_HandleTypeDef USBD_Device;


void test_usbd_hid(void)
{
    dlog_info("0000\n");
    USBD_Init(&USBD_Device, &HID_Desc, 0);

    dlog_info("1111\n");

    USBD_RegisterClass(&USBD_Device, USBD_HID_CLASS);
    dlog_info("222222\n");

    USBD_Start(&USBD_Device);
    dlog_info("33333\n");

}
