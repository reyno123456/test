#include <string.h>
#include <stdlib.h>
#include "test_usbh.h"
#include "debuglog.h"
#include "hal_usb_host.h"
#include "hal_usb_device.h"
#include "hal_sram.h"
#include "hal.h"


#define USB_VIDEO_BYPASS_SIZE_ONCE      (8192)
#define USB_VIDEO_BYPASS_DEST_ADDR      (0xB1000000)


/* USB Host Global Variables */
USBH_BypassVideoCtrl            g_usbhBypassVideoCtrl;
USBH_AppCtrl                    g_usbhAppCtrl;
uint8_t                         g_u8ViewUVC = 0;
uint8_t                         u8_FrameBuff[153600];
USBH_UVC_TASK_STATE             g_eUVCTaskState = USBH_UVC_TASK_IDLE;
uint16_t                        g_u16UVCWidth = 0;
uint16_t                        g_u16UVCHeight = 0;
volatile uint8_t                g_u8UserSelectPixel = 0;

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



void USBH_ProcUVC(void)
{
    STRU_UVC_VIDEO_FRAME_FORMAT     stVideoFrameFormat;
    uint32_t                        u32_uvcFrameNum;
    static uint16_t                 u16_UVCWidth;
    static uint16_t                 u16_UVCHeight;
    static uint32_t                 u32_UVCFrameSize;
    static uint8_t                  u8_UVCFrameIndex;
    uint32_t                        i;

    if (HAL_USB_GetHostAppState() == HAL_USB_HOST_STATE_DISCONNECT)
    {
        g_eUVCTaskState = USBH_UVC_TASK_DISCONNECT;
    }

    switch (g_eUVCTaskState)
    {
    case USBH_UVC_TASK_IDLE:
        if ((HAL_USB_HOST_STATE_READY == HAL_USB_GetHostAppState())&&
            (HAL_USB_HOST_CLASS_UVC == HAL_USB_CurUsbClassType()))
        {
            // get supported formats first
            HAL_USB_UVCGetVideoFormats(&stVideoFrameFormat);

            u16_UVCHeight           = 240;
            u16_UVCWidth            = 320;

            if (g_u8UserSelectPixel)
            {
                g_u8UserSelectPixel = 0;

                u16_UVCHeight       = g_u16UVCHeight;
                u16_UVCWidth        = g_u16UVCWidth;
            }

            for (i = 0; i < HAL_USB_UVC_MAX_FRAME_FORMATS_NUM; i++)
            {
                if ((u16_UVCHeight == stVideoFrameFormat.u16_height[i])&&
                    (u16_UVCWidth == stVideoFrameFormat.u16_width[i]))
                {
                    u8_UVCFrameIndex = stVideoFrameFormat.u8_frameIndex[i];

                    break;
                }
            }

            if (i >= HAL_USB_UVC_MAX_FRAME_FORMATS_NUM)
            {
                dlog_error("format info incorect: width: %d, height: %d", 
                                u16_UVCWidth, u16_UVCHeight);

                return;
            }

            g_eUVCTaskState = USBH_UVC_TASK_START;
        }

        break;

    case USBH_UVC_TASK_START:
        if (HAL_OK == HAL_USB_StartUVC(u16_UVCWidth, u16_UVCHeight, &u32_UVCFrameSize))
        {
            dlog_info("start UVC OK!");
        }
        else
        {
            dlog_error("app start UVC fail");

            return;
        }

        g_eUVCTaskState = USBH_UVC_TASK_GET_FRAME;

        break;

    case USBH_UVC_TASK_GET_FRAME:
        if (HAL_OK == HAL_USB_UVCGetVideoFrame(u8_FrameBuff))
        {
            g_eUVCTaskState = USBH_UVC_TASK_CHECK_FRAME_READY;
        }

        break;

    case USBH_UVC_TASK_CHECK_FRAME_READY:
        if (HAL_OK == HAL_USB_UVCCheckFrameReady(&u32_uvcFrameNum, &u32_UVCFrameSize))
        {
            g_eUVCTaskState = USBH_UVC_TASK_GET_FRAME;

            // do something USER need, such as transfer to ground , or optical flow process
            if (g_u8ViewUVC == 1)
            {
                HAL_USB_TransferUVCToGrd(u8_FrameBuff, u32_UVCFrameSize, u16_UVCWidth, u16_UVCHeight, ENUM_UVC_DATA_YUV);
            }
        }

        break;

    case USBH_UVC_TASK_DISCONNECT:

        g_eUVCTaskState = USBH_UVC_TASK_IDLE;

        break;
    }

}


void command_ViewUVC(void)
{
    HAL_SRAM_EnableSkyBypassVideo(HAL_SRAM_VIDEO_CHANNEL_1);

    g_u8ViewUVC = 1;
}


void command_startUVC(char *width, char *height)
{
    uint32_t                        u32_width = strtoul(width, NULL, 0);
    uint32_t                        u32_height = strtoul(height, NULL, 0);
    STRU_UVC_VIDEO_FRAME_FORMAT     stVideoFrameFormat;

    HAL_USB_UVCGetVideoFormats(&stVideoFrameFormat);

    g_eUVCTaskState     = USBH_UVC_TASK_DISCONNECT;

    g_u16UVCWidth       = (uint16_t)u32_width;
    g_u16UVCHeight      = (uint16_t)u32_height;

    g_u8UserSelectPixel = 1;
}


void USBH_UVCTask(void const *argument)
{
    dlog_info("UVC Task");

    while (1)
    {
        USBH_ProcUVC();

        HAL_Delay(1);
    }
}

