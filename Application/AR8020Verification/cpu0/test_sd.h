#ifndef __TEST__SD__H
#define __TEST__SD__H

#include "ff.h"
#include "ff_gen_drv.h"
#include "hal_sd.h"

FATFS SDFatFs;  /* File system object for SD card logical drive */
FIL MyFile, MyFileIn;     /* File object */
char SDPath[4]; /* SD card logical drive path */
extern Diskio_drvTypeDef  SD_Driver;

void TestWR(void);
void TestFatFs(void);
void TestFatFs2();
void TestFatFs1();
void TestSDIRQ(void);
void command_initSdcard();
void command_SdcardFatFs(char *argc);
void TestRawWR();
void OS_TestRawWR_Handler(void const * argument);
void OS_TestRawWR();
void OS_TestSD_Erase_Handler(void const * argument);
void OS_TestSD_Erase();

#endif