#include <string.h>
#include <stdlib.h>
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
uint8_t                         g_u8ViewUVC = 0;


void USBH_USBHostStatus(void const *argument)
{
    dlog_info("USBH_USBHostStatus TASK");

    while (1)
    {
        HAL_USB_HostProcess();

        USBH_ProcUVC();

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
                if ((HAL_USB_HOST_STATE_READY == HAL_USB_GetHostAppState())
                  &&(1 == g_usbhBypassVideoCtrl.taskActivate))
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


uint8_t  u8_FrameBuff[76800];
void USBH_ProcUVC(void)
{
    STRU_UVC_VIDEO_FRAME_FORMAT     stVideoFrameFormat;
    uint32_t                        u32_uvcFrameNum;
    uint16_t                        u16_width;
    uint16_t                        u16_height;
    uint32_t                        u32_frameSize;
    static uint8_t                  s_usbhUVCStarted = 0;

    // if UVC is started
    if ((HAL_USB_HOST_STATE_READY == HAL_USB_GetHostAppState())&&
        (HAL_USB_HOST_CLASS_UVC == HAL_USB_CurUsbClassType()))
    {
        if (0 == s_usbhUVCStarted)
        {
            // get supported formats first
            HAL_USB_GetVideoFormats(&stVideoFrameFormat);

            // set frame width and height
            u16_width               = 160;
            u16_height              = 120;

            // start uvc
            if (HAL_OK == HAL_USB_StartUVC(u16_width, u16_height, &u32_frameSize))
            {
                s_usbhUVCStarted = 1;
            }
            else
            {
                dlog_error("app start UVC fail");
            }
        }
        else
        {
            //get a YUV frame, size should be u16_width * u16_height * 2
            if (HAL_OK == HAL_USB_GetVideoFrame(u8_FrameBuff, &u32_uvcFrameNum, &u32_frameSize, ENUM_UVC_DATA_Y))
            {
                if (g_u8ViewUVC == 1)
                {
                    HAL_USB_TransferUVCToGrd(u8_FrameBuff, u32_frameSize, u16_width, u16_height, ENUM_UVC_DATA_Y);
                }
            }
            else
            {
                dlog_error("get video buffer error");
            }
        }
    }
    else if (HAL_USB_HOST_STATE_DISCONNECT == HAL_USB_GetHostAppState())
    {
        s_usbhUVCStarted = 0;
    }
}


void command_ViewUVC(void)
{
    HAL_SRAM_EnableSkyBypassVideo(HAL_SRAM_VIDEO_CHANNEL_1);

    g_u8ViewUVC = 1;
}




