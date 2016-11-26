#ifndef __TEST__USBD__H
#define __TEST__USBD__H

#include "usbd_core.h"
#include "cmsis_os.h"


typedef enum
{
    USBD_APP_SEND_CTRL      = 0,
    USBD_APP_SEND_VIDEO
}USBD_APP_EVENT_DEF;


void USBD_ApplicationInit(void);
void USBD_MainTask(void);


extern osMessageQId     USBD_AppEvent;


#endif
