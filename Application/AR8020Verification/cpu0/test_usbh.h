#ifndef __TEST__USBH__H
#define __TEST__USBH__H

#include <stdio.h>
#include "usbh_def.h"
#include "cmsis_os.h"


typedef enum
{
    USBH_APP_BYPASS_VIDEO = 0,
    USBH_APP_CREATE_FILE,
    USBH_APP_READ_FILE,
    USBH_APP_CREATE_FOLDER,
    USBH_APP_SHOW_DIR
}USBH_APP_TYPE_DEF;


void USBH_UserPorcess(USBH_HandleTypeDef *phost, uint8_t id);
void USBH_MainTask(void);
void USBH_ApplicationInit(void);
void USBH_MountUSBDisk(void);
void USBH_BypassVideo(void);

extern osMessageQId         USBH_AppEvent;


#endif

