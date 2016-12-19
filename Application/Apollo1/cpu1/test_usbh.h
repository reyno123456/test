#ifndef __TEST__USBH__H
#define __TEST__USBH__H

#include <stdio.h>
#include "usbh_def.h"
#include "ff.h"
#include "usbh_conf.h"
#include "usbh_core.h"
#include "usbh_msc.h"
#include "usbh_uvc.h"
#include "ff_gen_drv.h"
#include "usbh_diskio.h"



typedef struct
{
    ApplicationStateDef     usbhAppState;
    FIL                     usbhAppFile;
    FATFS                   usbhAppFatFs;
} USBH_AppCtrl;


void USBH_UserPorcess(USBH_HandleTypeDef *phost, uint8_t id);
void USBH_ApplicationInit(void);
void USBH_MountUSBDisk(void);
void USBH_USBHostStatus(void);
void USBH_ProcUVC(void);


#endif

