#ifndef __TEST__USBD__H
#define __TEST__USBD__H

#include "usbd_core.h"
#include "cmsis_os.h"


typedef enum
{
    USBD_TASK_SRAM_0_READY = 0,
    USBD_TASK_SRAM_1_READY,
    USBD_TASK_DEV_DISCONN
}USBD_TASK_EVENT_DEF;


void USBD_ApplicationInit(void);
void USBD_MainTask(void);


extern osMessageQId     USBD_AppEvent;


#endif
