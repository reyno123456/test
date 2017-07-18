#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "uvc_task.h"
#include "test_usbh.h"
#include "hal_usb_host.h"
#include "hal_usb_device.h"
#include "hal_sram.h"
#include "debuglog.h"
#include "hal.h"


uint8_t                         g_u8ViewUVC = 0;
volatile uint8_t                g_u8SaveUVC = 0;
volatile uint8_t                g_u8ShowUVC = 0;

uint8_t                         u8_FrameBuff[153600];
USBH_UVC_TASK_STATE             g_eUVCTaskState = USBH_UVC_TASK_IDLE;
uint16_t                        g_u16UVCWidth = 0;
uint16_t                        g_u16UVCHeight = 0;
volatile uint8_t                g_u8UserSelectPixel = 0;
FIL                             uvcFile;
uint8_t                         g_u8UVCHeader[12];


void USBH_UVCTask(void const *argument)
{
    dlog_info("UVC Task");

    while (1)
    {
        USBH_ProcUVC();

        HAL_Delay(1);
    }
}


void USBH_ProcUVC(void)
{
    static STRU_UVC_SUPPORT_FORMAT_LIST     stVideoFrameFormat;
    uint32_t                                u32_uvcFrameNum;
    static uint16_t                         u16_UVCWidth;
    static uint16_t                         u16_UVCHeight;
    static ENUM_HAL_USB_UVC_DATA_TYPE       e_UVCDataType;
    static uint32_t                         u32_UVCFrameSize;
    static uint8_t                          u8_UVCFrameIndex;
    uint8_t                                 i;
    static uint8_t                          u8_uvcPortId;
    static uint8_t                          u8_udiskPortId = HAL_USB_PORT_NUM;
    uint32_t                                u32_savedSize;
    uint8_t                                *u8_buff;
    uint8_t                                 u8_buffCount;
    uint32_t                                u32_lastPacketLen;

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
            memset((void *)&stVideoFrameFormat, 0, sizeof(STRU_UVC_SUPPORT_FORMAT_LIST));

            // get supported formats first
            HAL_USB_UVCGetVideoFormats(&stVideoFrameFormat);

            if (g_u8UserSelectPixel)
            {
                g_u8UserSelectPixel = 0;

                u16_UVCHeight       = g_u16UVCHeight;
                u16_UVCWidth        = g_u16UVCWidth;
                e_UVCDataType       = ENUM_UVC_DATA_YUV;
            }
            else
            {
                // H264 UVC
                //u16_UVCHeight       = 720;
                //u16_UVCWidth        = 1280;
                //e_UVCDataType       = ENUM_UVC_DATA_H264;

                // YUV UVC
                u16_UVCHeight       = 240;
                u16_UVCWidth        = 320;
                e_UVCDataType       = ENUM_UVC_DATA_YUV;
            }

            for (i = 0; i < stVideoFrameFormat.u16_frameNum; i++)
            {
                if ((stVideoFrameFormat.st_uvcFrameFormat[i].u16_height == u16_UVCHeight)&&
                    (stVideoFrameFormat.st_uvcFrameFormat[i].u16_width == u16_UVCWidth)&&
                    (stVideoFrameFormat.st_uvcFrameFormat[i].e_dataType == e_UVCDataType))
                {
                    break;
                }
            }

            if (i < stVideoFrameFormat.u16_frameNum)
            {
                g_eUVCTaskState = USBH_UVC_TASK_START;
            }
            else
            {
                dlog_error("no this format: %d * %d", u16_UVCWidth, u16_UVCHeight);
            }
        }

        break;

    case USBH_UVC_TASK_START:
        if (HAL_OK == HAL_USB_StartUVC(u16_UVCWidth,
                                       u16_UVCHeight,
                                       &u32_UVCFrameSize,
                                       e_UVCDataType,
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
    uint8_t                         u8_uvcPortId;

    g_eUVCTaskState     = USBH_UVC_TASK_DISCONNECT;

    g_u16UVCWidth       = (uint16_t)u32_width;
    g_u16UVCHeight      = (uint16_t)u32_height;

    g_u8UserSelectPixel = 1;
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




