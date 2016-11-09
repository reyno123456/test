#include "test_usbh.h"
#include "debuglog.h"
#include "ff.h"
#include "usbh_conf.h"
#include "usbh_def.h"
#include "usbh_core.h"
#include "usbh_msc.h"
#include "ff_gen_drv.h"
#include "usbh_diskio.h"
#include "interrupt.h"
#include "cmsis_os.h"
#include "systicks.h"

//#define USB_VIDEO_BYPASS_SIZE_ONCE      (8192)
#define USB_VIDEO_BYPASS_SIZE_ONCE      (16384)
#define USB_VIDEO_BYPASS_DEST_ADDR      (0xB1000000)

/* USB Host Global Variables */
USBH_HandleTypeDef   hUSBHost;
FATFS                USBH_fatfs;
FIL                  MyUSBFile;
uint8_t              fileOpened = 0;
osMessageQId         USBH_AppEvent;


void USBH_BypassVideo(void)
{
    FRESULT             fileResult;
    uint32_t            bytesread;
    uint8_t            *destAddr;

    bytesread           = 0;
    destAddr            = USB_VIDEO_BYPASS_DEST_ADDR;

    if (APPLICATION_READY == Appli_state)
    {
        if (fileOpened == 0)
        {
            fileOpened = 1;

            fileResult = f_open(&MyUSBFile, "0:usbtest.264", FA_READ);

            if(fileResult != FR_OK)
            {
                dlog_error("open file error: %d\n", (uint32_t)fileResult);

                return;
            }
        }

        fileResult = f_read(&MyUSBFile, destAddr, USB_VIDEO_BYPASS_SIZE_ONCE, (void *)&bytesread);

        if(fileResult != FR_OK)
        {
            dlog_error("Cannot Read from the file \n");

            f_close(&MyUSBFile);

            return;
        }

        if (bytesread < USB_VIDEO_BYPASS_SIZE_ONCE)
        {
            fileOpened = 0;

            f_close(&MyUSBFile);
        }

        return;
    }
    else
    {
        fileOpened = 0;

        f_close(&MyUSBFile);
    }
}


void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id)
{
    switch(id)
    {
    case HOST_USER_SELECT_CONFIGURATION:
        break;

    case HOST_USER_DISCONNECTION:
        Appli_state = APPLICATION_DISCONNECT;
        break;

    case HOST_USER_CLASS_ACTIVE:
        Appli_state = APPLICATION_READY;
        break;

    case HOST_USER_CONNECTION:
        break;

    default:
        break;
    }
}


void USBH_ApplicationInit(void)
{
    reg_IrqHandle(OTG_INTR0_VECTOR_NUM, USB_LL_OTG0_IRQHandler);

    USBH_Init(&hUSBHost, USBH_UserProcess, 0);

    USBH_RegisterClass(&hUSBHost, USBH_MSC_CLASS);

    USBH_Start(&hUSBHost);
}


void USBH_MountUSBDisk(void)
{
    FRESULT   fileResult;

    FATFS_LinkDriver(&USBH_Driver, "0:/");

    fileResult = f_mount(&USBH_fatfs, "0:/", 0);

    if (fileResult != FR_OK)
    {
        dlog_error("mount fatfs error: %d\n", fileResult);

        return;
    }
}

void USBH_MainTask(void)
{
    osEvent event;

    USBH_ApplicationInit();

    USBH_MountUSBDisk();

    dlog_info("usb host main task\n");

    while (1)
    {
        event = osMessageGet(USBH_AppEvent, osWaitForever);

        if (event.status == osEventMessage)
        {
            switch (event.value.v)
            {
            case USBH_APP_BYPASS_VIDEO:
                USBH_BypassVideo();
                break;

            case USBH_APP_CREATE_FILE:
                break;

            case USBH_APP_READ_FILE:
                break;

            case USBH_APP_CREATE_FOLDER:
                break;

            case USBH_APP_SHOW_DIR:
                break;

            default:
                break;
            }
        }
    }
}


