#include "../cpu0/test_usbh.h"
#include "debuglog.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "systicks.h"
#include "hal_usb_host.h"

void writeAudioPcm(void)
{
    FRESULT    fileResult;
    FIL        MyFile;
    uint32_t byteswritten=0;
    char test[10]="asdfadsg";
    dlog_info("start !!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    USBH_MountUSBDisk();
    while (HAL_USB_HOST_STATE_READY != HAL_USB_GetHostAppState())
    {
        dlog_info("finding mass storage\n");
        SysTicks_DelayMS(500);
    }

    fileResult = f_open(&MyFile,"audio.pcm" , FA_CREATE_ALWAYS | FA_WRITE);
    if (FR_OK != fileResult)
    {
        dlog_info("open or create file error: %d\n", fileResult);
        
        vTaskDelete(NULL);
    }
    fileResult = f_write(&MyFile, (uint8_t *)(0x81F00000), 1024*1024, (void *)&byteswritten);
    //fileResult = f_write(&MyFile, test, sizeof(test), (void *)&byteswritten);

    if((fileResult != FR_OK) || (byteswritten==0))
    {
        dlog_info("f_write error! \n");
        f_close(&MyFile);
        vTaskDelete(NULL);
    }
    f_close(&MyFile);

    fileResult = f_open(&MyFile,"audio.mp3" , FA_CREATE_ALWAYS | FA_WRITE);
    if (FR_OK != fileResult)
    {
        dlog_info("open or create file error: %d\n", fileResult);
        
        vTaskDelete(NULL);
    }
    fileResult = f_write(&MyFile, (uint8_t *)(0x81E00000), 43392, (void *)&byteswritten);
    //fileResult = f_write(&MyFile, test, sizeof(test), (void *)&byteswritten);

    if((fileResult != FR_OK) || (byteswritten==0))
    {
        dlog_info("f_write error! \n");
        f_close(&MyFile);
        vTaskDelete(NULL);
    }
    f_close(&MyFile);
    dlog_info("f_write %x !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n",byteswritten);

}

void test_StorageData(char *filename,char* startaddr,char* length)
{
    
    FRESULT    fileResult;
    FIL        MyFile;
    uint32_t byteswritten=0;
    uint32_t u32_startData = strtoul(startaddr, NULL, 0);
    uint32_t u32_lenght = strtoul(length, NULL, 0);

    dlog_info("start !!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n");
    USBH_MountUSBDisk();
    while (HAL_USB_HOST_STATE_READY != HAL_USB_GetHostAppState())
    {
        dlog_info("finding mass storage\n");
        SysTicks_DelayMS(500);
    }

    fileResult = f_open(&MyFile,filename , FA_CREATE_ALWAYS | FA_WRITE);
    if (FR_OK != fileResult)
    {
        dlog_info("open or create file error: %d\n", fileResult);
        
        vTaskDelete(NULL);
    }
    fileResult = f_write(&MyFile, (uint8_t *)(u32_startData), u32_lenght, (void *)&byteswritten);
    //fileResult = f_write(&MyFile, test, sizeof(test), (void *)&byteswritten);

    if((fileResult != FR_OK) || (byteswritten==0))
    {
        dlog_info("f_write error! \n");
        f_close(&MyFile);
        vTaskDelete(NULL);
    }
    f_close(&MyFile);
    dlog_info("f_write %x !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!\n",byteswritten);

}