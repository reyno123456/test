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
#include "test_usbh.h"


#define USB_VIDEO_BYPASS_SIZE_ONCE      (16384)
#define USB_VIDEO_BYPASS_DEST_ADDR      (0xB1000000)


/* USB Host Global Variables */
USBH_HandleTypeDef      hUSBHost;
FATFS                   USBH_fatfs;
FIL                     MyUSBFile;
osMessageQId            USBH_AppEvent;
USBH_BypassVideoCtrl    g_usbhBypassVideoCtrl = {0};



void USBH_BypassVideo(void)
{
    FRESULT             fileResult;
    uint32_t            bytesread;
    uint8_t            *destAddr;

    fileResult          = FR_OK;
    bytesread           = 0;
    destAddr            = USB_VIDEO_BYPASS_DEST_ADDR;

    dlog_info("enter USBH_BypassVideo Task!\n");

    while (1)
    {
        if (osOK == osSemaphoreWait(g_usbhBypassVideoCtrl.semID, osWaitForever))
        {
            while (1)
            {
                if ((APPLICATION_READY == Appli_state)
                  &&(1 == g_usbhBypassVideoCtrl.taskActivate))
                {
                    if (g_usbhBypassVideoCtrl.fileOpened == 0)
                    {
                        g_usbhBypassVideoCtrl.fileOpened = 1;

                        fileResult = f_open(&MyUSBFile, "0:usbtest.264", FA_READ);

                        if(fileResult != FR_OK)
                        {
                            dlog_error("open file error: %d\n", (uint32_t)fileResult);

                            break;
                        }
                    }

                    fileResult = f_read(&MyUSBFile, destAddr, USB_VIDEO_BYPASS_SIZE_ONCE, (void *)&bytesread);

                    osDelay(200);

                    if(fileResult != FR_OK)
                    {
                        g_usbhBypassVideoCtrl.fileOpened    = 0;
                        f_close(&MyUSBFile);

                        dlog_error("Cannot Read from the file \n");

                        break;
                    }

                    if (bytesread < USB_VIDEO_BYPASS_SIZE_ONCE)
                    {
                        dlog_info("a new round!\n");
                        g_usbhBypassVideoCtrl.fileOpened    = 0;
                        f_close(&MyUSBFile);
                    }
                }
                else
                {
                    if (1 == g_usbhBypassVideoCtrl.fileOpened)
                    {
                        g_usbhBypassVideoCtrl.fileOpened    = 0;
                        f_close(&MyUSBFile);
                    }

                    continue;
                }

            }

            g_usbhBypassVideoCtrl.taskActivate  = 0;
        }
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
            case USBH_APP_START_BYPASS_VIDEO:
                {
                    /* Need to start a new task */
                    if (0 == g_usbhBypassVideoCtrl.taskExist)
                    {
                        SRAM_SKY_BypassVideoConfig(0);

                        g_usbhBypassVideoCtrl.taskExist = 1;

                        osThreadDef(BypassTask, USBH_BypassVideo, osPriorityIdle, 0, 4 * 128);
                        g_usbhBypassVideoCtrl.threadID  = osThreadCreate(osThread(BypassTask), NULL);

                        if (g_usbhBypassVideoCtrl.threadID == NULL)
                        {
                            g_usbhBypassVideoCtrl.taskExist     = 0;

                            dlog_error("create Video Bypass Task error!\n");
                        }

                        osSemaphoreDef(bypassVideoSem);
                        g_usbhBypassVideoCtrl.semID     = osSemaphoreCreate(osSemaphore(bypassVideoSem), 1);
                    }

                    /* activate the task */
                    if (0 == g_usbhBypassVideoCtrl.taskActivate)
                    {
                        g_usbhBypassVideoCtrl.taskActivate  = 1;
                        osSemaphoreRelease(g_usbhBypassVideoCtrl.semID);
                    }

                    break;
                }

            case USBH_APP_STOP_BYPASS_VIDEO:
                {
                    dlog_info("stop bypassvideo task!\n");
                    g_usbhBypassVideoCtrl.taskActivate   = 0;
                }
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


