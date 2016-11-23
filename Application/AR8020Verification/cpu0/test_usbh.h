#ifndef __TEST__USBH__H
#define __TEST__USBH__H

#include <stdio.h>
#include "usbh_def.h"
#include "cmsis_os.h"


typedef enum
{
    USBH_APP_START_BYPASS_VIDEO = 0,
    USBH_APP_STOP_BYPASS_VIDEO,
    USBH_APP_CREATE_FILE,
    USBH_APP_READ_FILE,
    USBH_APP_CREATE_FOLDER,
    USBH_APP_SHOW_DIR,
    USBH_UPGRADE
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


void USBH_UserPorcess(USBH_HandleTypeDef *phost, uint8_t id);
void USBH_MainTask(void);
void USBH_ApplicationInit(void);
void USBH_MountUSBDisk(void);
void USBH_BypassVideo(void);

extern osMessageQId             USBH_AppEvent;
extern USBH_BypassVideoCtrl     g_usbhBypassVideoCtrl;


#endif

