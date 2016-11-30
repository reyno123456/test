#include <stdint.h>
#include <stddef.h>
#include <string.h>

#include "debuglog.h"
#include "systicks.h"
#include "boot.h"
#include "sd_boot.h"

//#define UPGRADE_SD_DEBUGE

#ifdef  UPGRADE_SD_DEBUGE
#define UPDATA_SD
#define SD_DLOG_INFO(...) dlog_info(__VA_ARGS__)
#else
#define SD_DLOG_INFO(...)
#endif


void BOOTLOAD_UpdataFromSDToNor(void)
{    
    FRESULT fileResult;
    FIL     MyFile;
    FATFS   SDFatFs; 
    uint8_t SDPath[4];    
    SD_DLOG_INFO("BOOTLOAD_UpdataFromSDToNor\n");
    if (FATFS_LinkDriver(&SD_Driver, SDPath) != 0)
    {
        SD_DLOG_INFO("FATFS_LinkDriver error \n");
        return ;
    }    
    if ((fileResult = f_mount(&SDFatFs, (TCHAR const*)SDPath, 1)) != FR_OK)
    {

        SD_DLOG_INFO("f_mount = %d\n", fileResult);
        SD_DLOG_INFO("f_mount error!\n");
        return ;
    }
    SD_DLOG_INFO("SD mount ok \n");
    fileResult = f_open(&MyFile, "ar8020.bin", FA_READ);
    if (FR_OK != fileResult)
    {
        SD_DLOG_INFO("open or create file error: %d\n", fileResult);
        return ;
    }
    if(BOOTLOAD_CopyDataToNor(MyFile,APP_ADDR_OFFSET) != 0)
    {
        return;     
    }   

    f_close(&MyFile);
    fileResult = FATFS_UnLinkDriver(SDPath);
    return;

}
void BOOTLOAD_BootFromSD(void)
{
    FRESULT fileResult;
    FIL     MyFile;
    FATFS   SDFatFs;  
    uint8_t SDPath[4];  
    SD_DLOG_INFO("BOOTLOAD_SDBoot\n");    
    if (FATFS_LinkDriver(&SD_Driver, SDPath) != 0)
    {
        SD_DLOG_INFO("FATFS_LinkDriver error \n");
        return ;
    }    
    if ((fileResult = f_mount(&SDFatFs, (TCHAR const*)SDPath, 1)) != FR_OK)
    {

        SD_DLOG_INFO("f_mount = %d\n", fileResult);
        SD_DLOG_INFO("f_mount error!\n");
        return ;
    }
    SD_DLOG_INFO("SD mount ok \n");
    fileResult = f_open(&MyFile, "ar8020.bin", FA_READ);
    if (FR_OK != fileResult)
    {
        SD_DLOG_INFO("open or create file error: %d\n", fileResult);
        return ;
    }   
    if(BOOTLOAD_CopyDataToITCM(MyFile,ITCM0_START) != 0)
    {
        return;     
    }    
   /* if(BOOTLOAD_CopyDataToTCM(ITCM1_START) != 0)
    {
        return;     
    }
    if(BOOTLOAD_CopyDataToTCM(ITCM2_START) != 0)
    {
        return;     
    }*/
    f_close(&MyFile);
    fileResult = FATFS_UnLinkDriver(SDPath);
    
    BOOTLOAD_BootApp();  

    return;
}
void BOOTLOAD_UpdataBootloaderFromSD(void)
{
    FRESULT fileResult;
    FIL     MyFile;
    FATFS   SDFatFs;
    uint8_t SDPath[4];    
    SD_DLOG_INFO("BOOTLOAD_UpdataBootloaderFromSD\n");
    if (FATFS_LinkDriver(&SD_Driver, SDPath) != 0)
    {
        SD_DLOG_INFO("FATFS_LinkDriver error \n");
        return ;
    }    
    if ((fileResult = f_mount(&SDFatFs, (TCHAR const*)SDPath, 1)) != FR_OK)
    {

        SD_DLOG_INFO("f_mount = %d\n", fileResult);
        SD_DLOG_INFO("f_mount error!\n");
        return ;
    }
    SD_DLOG_INFO("SD mount ok \n");
    fileResult = f_open(&MyFile, "ar8020.bin", FA_READ);
    if (FR_OK != fileResult)
    {
        SD_DLOG_INFO("open or create file error: %d\n", fileResult);
        return ;
    }
    if(BOOTLOAD_CopyDataToNor(MyFile,0) != 0)
    {
        return;     
    }   

    f_close(&MyFile);
    fileResult = FATFS_UnLinkDriver(SDPath);
    return;

}