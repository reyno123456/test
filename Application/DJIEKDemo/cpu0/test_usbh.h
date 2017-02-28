#ifndef __TEST__USBH__H
#define __TEST__USBH__H

#include <stdio.h>
#include "cmsis_os.h"
#include "ff.h"
#include "ff_gen_drv.h"
#include "usbh_diskio.h"


typedef enum
{
    USBH_APP_START_BYPASS_VIDEO = 0,
    USBH_APP_STOP_BYPASS_VIDEO,
}USBH_APP_EVENT_DEF;


typedef struct
{
    volatile uint8_t        taskActivate;
    volatile uint8_t        taskExist;
    volatile uint8_t        fileOpened;
    uint8_t                 reserved;
    osThreadId              threadID;
    osSemaphoreId           semID;
} USBH_BypassVideoCtrl;


typedef struct
{
    osMessageQId            usbhAppEvent;
    FIL                     usbhAppFile;
    FATFS                   usbhAppFatFs;
} USBH_AppCtrl;


void USBH_USBHostStatus(void const *argument);
void USBH_BypassVideo(void const *argument);
void USBH_MountUSBDisk(void);
void USB_MainTask(void const *argument);


extern USBH_AppCtrl             g_usbhAppCtrl;
extern USBH_BypassVideoCtrl     g_usbhBypassVideoCtrl;


#endif

