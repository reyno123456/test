#ifndef __TEST__USBH__H
#define __TEST__USBH__H

#include <stdio.h>
#include "cmsis_os.h"
#include "ff.h"
#include "ff_gen_drv.h"
#include "usbh_diskio.h"


#define USB_VIDEO_BYPASS_SIZE_ONCE              (8192)
#define USB_VIDEO_BYPASS_CHANNEL_0_DEST_ADDR    (0xB1000000)
#define USB_VIDEO_BYPASS_CHANNEL_1_DEST_ADDR    (0xB1800000)


typedef enum
{
    USBH_APP_START_BYPASS_VIDEO = 0,
    USBH_APP_STOP_BYPASS_VIDEO,
}USBH_APP_EVENT_DEF;


typedef enum
{
    USBH_UVC_TASK_IDLE                  = 0,
    USBH_UVC_TASK_START                 = 1,
    USBH_UVC_TASK_GET_FRAME             = 2,
    USBH_UVC_TASK_CHECK_FRAME_READY     = 3,
    USBH_UVC_TASK_DISCONNECT            = 4,
} USBH_UVC_TASK_STATE;


typedef enum
{
    USBH_VIDEO_BYPASS_TASK_IDLE         = 0,
    USBH_VIDEO_BYPASS_TASK_START        = 1,
    USBH_VIDEO_BYPASS_TASK_TRANS        = 2,
    USBH_VIDEO_BYPASS_TASK_STOP         = 3,
} USBH_VIDEO_BYPASS_TASK_STATE;


typedef struct
{
    volatile uint8_t        taskActivate;
    volatile uint8_t        taskExist;
    volatile uint8_t        fileOpened;
    volatile uint8_t        bypassChannel;
    osThreadId              threadID;
    osSemaphoreId           semID;
    USBH_VIDEO_BYPASS_TASK_STATE taskState;
} USBH_BypassVideoCtrl;


typedef struct
{
    osMessageQId            usbhAppEvent;
    FATFS                   usbhAppFatFs;
} USBH_AppCtrl;



void USBH_USBHostStatus(void const *argument);
void USBH_BypassVideo(void const *argument);
void USBH_MountUSBDisk(void);
void USB_MainTask(void const *argument);
void command_startBypassVideo(uint8_t *bypassChannel);
void command_stopBypassVideo(void);
void USBH_ProcUVC(void);
void command_ViewUVC(void);
void USBH_UVCTask(void const *argument);
void command_startUVC(char *width, char *height);
void command_saveUVC(void);
void command_stopSaveUVC(void);


extern USBH_AppCtrl             g_usbhAppCtrl;
extern USBH_BypassVideoCtrl     g_usbhBypassVideoCtrl;


#endif

