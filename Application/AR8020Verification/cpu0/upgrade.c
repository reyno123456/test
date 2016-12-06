#include "test_usbh.h"
#include "debuglog.h"
#include "interrupt.h"
#include "cmsis_os.h"
#include "sram.h"
#include "quad_spi_ctrl.h"
#include "nor_flash.h"
#include "upgrade.h"

void BOOTLOAD_Upgrade(void const *argument)
{

    FRESULT    fileResult;
    FIL        MyFile;
    uint32_t   u32_bytesRead= RDWR_SECTOR_SIZE;
    uint8_t    *u8_arrayRecData = (uint8_t *)0x21004000;
    uint32_t   u32_recDataSum = 0;
    uint32_t   u32_norAddr = (0x20000);
    dlog_info("Nor flash init start ...");
    NOR_FLASH_Init();
    dlog_info("Nor flash init end   ...");
    dlog_output(100);
    if (APPLICATION_READY == g_usbhAppCtrl.usbhAppState)
    {
        fileResult = f_open(&MyFile, "app.bin", FA_READ);
        if (FR_OK != fileResult)
        {
            dlog_info("open or create file error: %d\n", fileResult);
            return;
        }                    
        while(RDWR_SECTOR_SIZE == u32_bytesRead)
        {
            
            memset(u8_arrayRecData,0,RDWR_SECTOR_SIZE);
            fileResult = f_read(&MyFile, (void *)u8_arrayRecData, RDWR_SECTOR_SIZE, (void *)&u32_bytesRead);
            if((fileResult != FR_OK))
            {
                dlog_info("Cannot Read from the file \n");
                f_close(&MyFile);
            }
            NOR_FLASH_EraseSector(u32_norAddr);
            NOR_FLASH_WriteByteBuffer(u32_norAddr,u8_arrayRecData,RDWR_SECTOR_SIZE);
            u32_recDataSum+=u32_bytesRead;
            u32_norAddr += RDWR_SECTOR_SIZE; 
            dlog_info("f_read success %d!",u32_bytesRead);
            dlog_output(100);               
        }
        f_close(&MyFile);
    }
    else
    {
        dlog_info("Appli_state\n");
    }
    dlog_info("upgrade ok\n");
    dlog_output(100);
}
