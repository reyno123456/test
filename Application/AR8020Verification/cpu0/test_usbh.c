#include "test_usbh.h"
#include "debuglog.h"
#include "hal_usb_host.h"
#include "hal_usb_device.h"
#include "hal_sram.h"


#define USB_VIDEO_BYPASS_SIZE_ONCE      (8192)
#define USB_VIDEO_BYPASS_DEST_ADDR      (0xB1000000)


/* USB Host Global Variables */
USBH_BypassVideoCtrl            g_usbhBypassVideoCtrl;
USBH_AppCtrl                    g_usbhAppCtrl;


void USBH_USBHostStatus(void const *argument)
{
    dlog_info("USBH_USBHostStatus TASK");

    while (1)
    {
        HAL_USB_HostProcess();

        osDelay(10);
    }
}


void USBH_BypassVideo(void const *argument)
{
    FRESULT             fileResult;
    uint32_t            bytesread;
    uint8_t            *videoBuff;

    fileResult          = FR_OK;
    bytesread           = 0;
    videoBuff           = (uint8_t *)USB_VIDEO_BYPASS_DEST_ADDR;

    dlog_info("enter USBH_BypassVideo Task!\n");

    while (1)
    {
        if (osOK == osSemaphoreWait(g_usbhBypassVideoCtrl.semID, osWaitForever))
        {
            while (1)
            {
                if ((HAL_USB_HOST_STATE_READY == HAL_USB_GetHostAppState())&&
                    (1 == g_usbhBypassVideoCtrl.taskActivate))
                {
                    if (g_usbhBypassVideoCtrl.fileOpened == 0)
                    {
                        g_usbhBypassVideoCtrl.fileOpened = 1;

                        fileResult = f_open(&(g_usbhAppCtrl.usbhAppFile), "0:usbtest.264", FA_READ);

                        if(fileResult != FR_OK)
                        {
                            dlog_error("open file error: %d\n", (uint32_t)fileResult);

                            break;
                        }
                    }

                    fileResult = f_read(&(g_usbhAppCtrl.usbhAppFile), videoBuff, USB_VIDEO_BYPASS_SIZE_ONCE, (void *)&bytesread);

                    if(fileResult != FR_OK)
                    {
                        g_usbhBypassVideoCtrl.fileOpened    = 0;
                        f_close(&(g_usbhAppCtrl.usbhAppFile));

                        dlog_error("Cannot Read from the file \n");

                        continue;
                    }

                    osDelay(10);

                    if (bytesread < USB_VIDEO_BYPASS_SIZE_ONCE)
                    {
                        dlog_info("a new round!\n");
                        g_usbhBypassVideoCtrl.fileOpened    = 0;
                        f_close(&(g_usbhAppCtrl.usbhAppFile));
                    }
                }
                else
                {
                    if (1 == g_usbhBypassVideoCtrl.fileOpened)
                    {
                        g_usbhBypassVideoCtrl.fileOpened    = 0;
                        f_close(&(g_usbhAppCtrl.usbhAppFile));
                    }

                    break;
                }
            }

            g_usbhBypassVideoCtrl.taskActivate  = 0;
        }
    }
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


void USB_MainTask(void const *argument)
{
    osEvent event;

    dlog_info("main task");

    while (1)
    {
        event = osMessageGet(g_usbhAppCtrl.usbhAppEvent, osWaitForever);

        if (event.status == osEventMessage)
        {
            switch (event.value.v)
            {
                case USBH_APP_START_BYPASS_VIDEO:
                {
                    /* Need to start a new task */
                    if (0 == g_usbhBypassVideoCtrl.taskExist)
                    {
                        USBH_MountUSBDisk();

                        osThreadDef(BypassTask, USBH_BypassVideo, osPriorityIdle, 0, 4 * 128);
                        g_usbhBypassVideoCtrl.threadID  = osThreadCreate(osThread(BypassTask), NULL);

                        if (NULL == g_usbhBypassVideoCtrl.threadID)
                        {
                            dlog_error("create Video Bypass Task error!\n");

                            break;
                        }

                        g_usbhBypassVideoCtrl.taskExist = 1;

                        osSemaphoreDef(bypassVideoSem);
                        g_usbhBypassVideoCtrl.semID     = osSemaphoreCreate(osSemaphore(bypassVideoSem), 1);

                        if (NULL == g_usbhBypassVideoCtrl.semID)
                        {
                            osThreadTerminate(g_usbhBypassVideoCtrl.threadID);
                            g_usbhBypassVideoCtrl.taskExist = 0;

                            dlog_error("create Video Bypass Semaphore error!\n");

                            break;
                        }
                    }

                    /* activate the task */
                    HAL_SRAM_EnableSkyBypassVideo(HAL_SRAM_VIDEO_CHANNEL_0);

                    osSemaphoreRelease(g_usbhBypassVideoCtrl.semID);

                    break;
                }

                case USBH_APP_STOP_BYPASS_VIDEO:
                {
                    dlog_info("stop bypassvideo task!\n");

                    HAL_SRAM_DisableSkyBypassVideo(HAL_SRAM_VIDEO_CHANNEL_0);

                    break;
                }

                default:
                    break;
            }
        }
    }
}


