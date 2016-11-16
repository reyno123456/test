#ifndef __TEST__USBH__H
#define __TEST__USBH__H

#include <stdio.h>
#include "usbh_def.h"

typedef enum
{
    FILE_OPERATION_IDLE = 0,
    FILE_OPERATION_START,
    FILE_OPERATION_BUSY,
    FILE_OPERATION_DISPLAY,
}FileOperationState;


typedef struct _FileOperationContext
{
    volatile FileOperationState state;
    volatile uint16_t            operated;
}FILE_OPERATION_CONTEXT;


void USBH_UserPorcess(USBH_HandleTypeDef *phost, uint8_t id);
void usbh(void);
void USBH_ApplicationInit(void);
void USBH_MountUSBDisk(void);




#endif

