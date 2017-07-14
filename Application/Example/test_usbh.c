#include <string.h>
#include <stdlib.h>
#include "test_usbh.h"
#include "debuglog.h"
#include "hal_usb_host.h"
#include "hal_usb_device.h"
#include "hal_sram.h"
#include "hal.h"



/* USB Host Global Variables */
USBH_BypassVideoCtrl            g_usbhBypassVideoCtrl;
USBH_AppCtrl                    g_usbhAppCtrl;
uint8_t                         g_u8ViewUVC = 0;
uint8_t                         u8_FrameBuff[153600];
USBH_UVC_TASK_STATE             g_eUVCTaskState = USBH_UVC_TASK_IDLE;
uint16_t                        g_u16UVCWidth = 0;
uint16_t                        g_u16UVCHeight = 0;
volatile uint8_t                g_u8UserSelectPixel = 0;
volatile uint8_t                g_u8SaveUVC = 0;
FIL                             uvcFile;
volatile uint8_t                g_u8ShowUVC = 0;
uint8_t                         g_u8UVCHeader[12];

void USBH_USBHostStatus(void const *argument)
{
    dlog_info("USBH_USBHostStatus TASK");

    while (1)
    {
        HAL_USB_HostProcess();

        HAL_Delay(5);
    }
}


void USBH_BypassVideo(void const *argument)
{
    FRESULT             fileResult;
    uint32_t            bytesread;
    uint8_t            *videoBuff;
    static FIL          usbhAppFile;
    uint8_t             i;
    static uint8_t      u8_usbPortId;

    fileResult          = FR_OK;
    bytesread           = 0;

    dlog_info("enter USBH_BypassVideo Task!\n");

    while (1)
    {
        if (osOK == osSemaphoreWait(g_usbhBypassVideoCtrl.semID, osWaitForever))
        {
            if (g_usbhBypassVideoCtrl.bypassChannel == 0)
            {
                videoBuff           = (uint8_t *)USB_VIDEO_BYPASS_CHANNEL_0_DEST_ADDR;
            }
            else
            {
                videoBuff           = (uint8_t *)USB_VIDEO_BYPASS_CHANNEL_1_DEST_ADDR;
            }

            for ( ;; )
            {
                if (g_usbhBypassVideoCtrl.taskActivate == 1)
                {
                    switch (g_usbhBypassVideoCtrl.taskState)
                    {
                    case USBH_VIDEO_BYPASS_TASK_IDLE:
                        u8_usbPortId = HAL_USB_GetMSCPort();

                        if (u8_usbPortId < HAL_USB_PORT_NUM)
                        {
                            dlog_info("use MSC Port: %d", u8_usbPortId);

                            g_usbhBypassVideoCtrl.taskState = USBH_VIDEO_BYPASS_TASK_START;
                        }

                        break;

                    case USBH_VIDEO_BYPASS_TASK_START:
                        if ((HAL_USB_HOST_STATE_READY == HAL_USB_GetHostAppState(u8_usbPortId))&&
                            (1 == g_usbhBypassVideoCtrl.taskActivate))
                        {
                            if (g_usbhBypassVideoCtrl.fileOpened == 0)
                            {
                                fileResult = f_open(&usbhAppFile, "0:usbtest.264", FA_READ);

                                if(fileResult == FR_OK)
                                {
                                    g_usbhBypassVideoCtrl.fileOpened = 1;

                                    g_usbhBypassVideoCtrl.taskState = USBH_VIDEO_BYPASS_TASK_TRANS;
                                }
                                else
                                {
                                    g_usbhBypassVideoCtrl.taskState = USBH_VIDEO_BYPASS_TASK_STOP;

                                    dlog_error("open file error: %d\n", (uint32_t)fileResult);
                                }
                            }
                            else
                            {
                                g_usbhBypassVideoCtrl.taskState = USBH_VIDEO_BYPASS_TASK_TRANS;
                            }
                        }
                        else
                        {
                            g_usbhBypassVideoCtrl.taskState = USBH_VIDEO_BYPASS_TASK_STOP;
                        }

                        break;

                    case USBH_VIDEO_BYPASS_TASK_TRANS:
                        if ((HAL_USB_HOST_STATE_READY == HAL_USB_GetHostAppState(u8_usbPortId))&&
                            (1 == g_usbhBypassVideoCtrl.taskActivate))
                        {
                            fileResult = f_read((&usbhAppFile), videoBuff, USB_VIDEO_BYPASS_SIZE_ONCE, (void *)&bytesread);

                            if(fileResult != FR_OK)
                            {
                                g_usbhBypassVideoCtrl.fileOpened    = 0;
                                f_close(&usbhAppFile);

                                dlog_error("Cannot Read from the file \n");
                            }

                            HAL_Delay(10);

                            if (bytesread < USB_VIDEO_BYPASS_SIZE_ONCE)
                            {
                                dlog_info("a new round!\n");
                                g_usbhBypassVideoCtrl.fileOpened    = 0;
                                f_close(&usbhAppFile);

                                g_usbhBypassVideoCtrl.taskState = USBH_VIDEO_BYPASS_TASK_START;
                            }
                        }
                        else
                        {
                            g_usbhBypassVideoCtrl.taskState = USBH_VIDEO_BYPASS_TASK_STOP;
                        }

                        break;

                    case USBH_VIDEO_BYPASS_TASK_STOP:
                        g_usbhBypassVideoCtrl.taskState = USBH_VIDEO_BYPASS_TASK_IDLE;

                        break;

                    default:
                        g_usbhBypassVideoCtrl.taskActivate  = 0;

                        break;
                    }
                }
                else
                {
                    break;
                }
            }
        }
    }
}


void USBH_MountUSBDisk(void)
{
    FRESULT             fileResult;
    static uint8_t      s_u8MountFlag = 0;

    if (s_u8MountFlag == 0)
    {
        FATFS_LinkDriver(&USBH_Driver, "0:/");

        fileResult = f_mount(&(g_usbhAppCtrl.usbhAppFatFs), "0:/", 0);

        if (fileResult != FR_OK)
        {
            dlog_error("mount fatfs error: %d\n", fileResult);

            return;
        }

        s_u8MountFlag = 1;
    }
    else
    {
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
                    if (g_usbhBypassVideoCtrl.bypassChannel == 0)
                    {
                        HAL_SRAM_EnableSkyBypassVideo(HAL_SRAM_VIDEO_CHANNEL_0);
                    }
                    else
                    {
                        HAL_SRAM_EnableSkyBypassVideo(HAL_SRAM_VIDEO_CHANNEL_1);
                    }

                    osSemaphoreRelease(g_usbhBypassVideoCtrl.semID);

                    break;
                }

                case USBH_APP_STOP_BYPASS_VIDEO:
                {
                    dlog_info("stop bypassvideo task!\n");

                    if (g_usbhBypassVideoCtrl.bypassChannel == 0)
                    {
                        HAL_SRAM_DisableSkyBypassVideo(HAL_SRAM_VIDEO_CHANNEL_0);
                    }
                    else
                    {
                        HAL_SRAM_DisableSkyBypassVideo(HAL_SRAM_VIDEO_CHANNEL_1);
                    }

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
    uint8_t                         i;
    static uint8_t                  u8_uvcPortId;
    static uint8_t                  u8_udiskPortId = HAL_USB_PORT_NUM;
    uint32_t                        u32_savedSize;
    uint8_t                        *u8_buff;
    uint8_t                         u8_buffCount;
    uint32_t                        u32_lastPacketLen;

    if ((HAL_USB_GetHostAppState(u8_uvcPortId) == HAL_USB_HOST_STATE_DISCONNECT)&&
        (g_eUVCTaskState != USBH_UVC_TASK_IDLE))
    {
        g_eUVCTaskState = USBH_UVC_TASK_DISCONNECT;
    }

    switch (g_eUVCTaskState)
    {
    case USBH_UVC_TASK_IDLE:
        u8_uvcPortId = HAL_USB_GetUVCPortId();

        if (u8_uvcPortId >= HAL_USB_PORT_NUM)
        {
            return;
        }

        if (HAL_USB_HOST_STATE_READY == HAL_USB_GetHostAppState(u8_uvcPortId))
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

            // set header of image
            // header format:
            // byte0 byte1 byte2 byte3 byte4 byte5 byte6 byte7 byte8   byte9   byte10 byte11
            // 0x00  0x00  0x00  0x00  0xFF  0xFF  0xFF  0xFF  format  pixel     reserved
            // format:  1: YUV       2: Y only
            // pixel:   1: 160*120   2: 320*240
            g_u8UVCHeader[0]            = 0x00;
            g_u8UVCHeader[1]            = 0x00;
            g_u8UVCHeader[2]            = 0x00;
            g_u8UVCHeader[3]            = 0x00;
            g_u8UVCHeader[4]            = 0xFF;
            g_u8UVCHeader[5]            = 0xFF;
            g_u8UVCHeader[6]            = 0xFF;
            g_u8UVCHeader[7]            = 0xFF;
            g_u8UVCHeader[8]            = 0x01;
            g_u8UVCHeader[9]            = 0x02;
            g_u8UVCHeader[10]           = 0x00;
            g_u8UVCHeader[11]           = 0x00;

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
        if (HAL_OK == HAL_USB_StartUVC(u16_UVCWidth,
                                       u16_UVCHeight,
                                       &u32_UVCFrameSize,
                                       u8_uvcPortId))
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

            if (g_u8SaveUVC == 1)
            {
                if (u8_udiskPortId >= HAL_USB_PORT_NUM)
                {
                    u8_udiskPortId = HAL_USB_GetMSCPort();

                    if (u8_udiskPortId >= HAL_USB_PORT_NUM)
                    {
                        dlog_error("udisk is not ready to save UVC data");

                        break;
                    }
                }

                f_write(&uvcFile, u8_FrameBuff, u32_UVCFrameSize, (void *)&u32_savedSize);

                if (u32_savedSize < u32_UVCFrameSize)
                {
                    dlog_error("save UVC data error: %d, %d", u32_savedSize, u32_UVCFrameSize);
                }
            }

            if (g_u8ShowUVC == 1)
            {
                /* usb transfer 8K Byte */
                u8_buffCount        = (u32_UVCFrameSize >> 13);
                u32_lastPacketLen   = (u32_UVCFrameSize & (UVC_TRANSFER_SIZE_ONCE - 1));

                HAL_USB_SendData(g_u8UVCHeader, sizeof(g_u8UVCHeader), (HAL_USB_PORT_1 - u8_uvcPortId), UVC_ENDPOINT_FOR_TRANSFER);

                u8_buff = u8_FrameBuff;

                for (i = 0; i < u8_buffCount; i++)
                {
                    HAL_USB_SendData(u8_buff, UVC_TRANSFER_SIZE_ONCE, (HAL_USB_PORT_1 - u8_uvcPortId), UVC_ENDPOINT_FOR_TRANSFER);

                    u8_buff += UVC_TRANSFER_SIZE_ONCE;
                }

                if (u32_lastPacketLen > 0)
                {
                    HAL_USB_SendData(u8_buff, u32_lastPacketLen, (HAL_USB_PORT_1 - u8_uvcPortId), UVC_ENDPOINT_FOR_TRANSFER);
                }
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
    uint8_t                         u8_uvcPortId;

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


void command_saveUVC(void)
{
    FRESULT fileResult;

    USBH_MountUSBDisk();

    fileResult = f_open(&uvcFile, "0:uvcdata.yuv", FA_CREATE_ALWAYS | FA_WRITE | FA_READ);

    if (fileResult != FR_OK)
    {
        dlog_error("create uvc file error: %d", fileResult);
        return;
    }

    dlog_info("start to save UVC data");

    g_u8SaveUVC = 1;
}


void command_stopSaveUVC(void)
{
    f_close(&uvcFile);

    dlog_info("save uvc data succeed");

    g_u8SaveUVC = 0;
}


void command_showUVC(void)
{
    g_u8ShowUVC++;

    if (g_u8ShowUVC >= 2)
    {
        g_u8ShowUVC = 0;
    }
}


void command_startBypassVideo(uint8_t *bypassChannel)
{
    USBH_APP_EVENT_DEF  usbhAppType;

    usbhAppType = USBH_APP_START_BYPASS_VIDEO;

    if (0 == g_usbhBypassVideoCtrl.taskActivate)
    {
        g_usbhBypassVideoCtrl.taskActivate  = 1;
        g_usbhBypassVideoCtrl.bypassChannel = strtoul(bypassChannel, NULL, 0);
        osMessagePut(g_usbhAppCtrl.usbhAppEvent, usbhAppType, 0);
    }
    else
    {
        dlog_error("Bypass Video Task is running\n");
    }
}


void command_stopBypassVideo(void)
{
    USBH_APP_EVENT_DEF  usbhAppType;

    usbhAppType = USBH_APP_STOP_BYPASS_VIDEO;

    if (1 == g_usbhBypassVideoCtrl.taskActivate)
    {
        g_usbhBypassVideoCtrl.taskActivate  = 0;
        osMessagePut(g_usbhAppCtrl.usbhAppEvent, usbhAppType, 0);
    }
    else
    {
        dlog_error("Bypass Video Task is not running\n");
    }
}


