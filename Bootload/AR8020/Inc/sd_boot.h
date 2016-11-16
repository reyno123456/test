#ifndef __SD_BOOT__H
#define __SD_BOOT__H

#include "ff.h"
#include "ff_gen_drv.h"
#include "sd_host.h"


extern Diskio_drvTypeDef  SD_Driver;

void BOOTLOAD_UpdataFromSDToNor(void);
void BOOTLOAD_BootFromSD(void);
void BOOTLOAD_UpdataBootloaderFromSD(void);

#endif