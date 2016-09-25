#ifndef __TEST__SD__H
#define __TEST__SD__H

#include "ff.h"
#include "sd_host.h"
#include "ff_gen_drv.h"

// extern int _flash_data_start;
// extern int _flash_data_end;
// extern int _flash_data_load;

FATFS SDFatFs;  /* File system object for SD card logical drive */
FIL MyFile;     /* File object */
char SDPath[4]; /* SD card logical drive path */
extern Diskio_drvTypeDef  SD_Driver;

void TestWR(void);
void TestFatFs(void);

#endif