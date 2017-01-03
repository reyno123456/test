#ifndef __TEST__SD__H
#define __TEST__SD__H

#include "ff.h"
#include "ff_gen_drv.h"
#include "hal_sd.h"

FATFS SDFatFs;  /* File system object for SD card logical drive */
FIL MyFile;     /* File object */
char SDPath[4]; /* SD card logical drive path */
extern Diskio_drvTypeDef  SD_Driver;

void TestWR(void);
void TestFatFs(void);
void TestSDIRQ(void);
void command_SdcardFatFs(void);
void command_initSdcard();

#endif