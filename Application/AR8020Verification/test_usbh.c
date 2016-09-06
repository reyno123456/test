#include "test_usbh.h"
#include "debuglog.h"
#include "ff.h"
#include "usbh_conf.h"
#include "usbh_def.h"
#include "usbh_core.h"
#include "usbh_msc.h"
#include "ff_gen_drv.h"
#include "usbh_diskio.h"
#include "interrupt.h"
#include "interrupt_internal.h"

/* USB Host Global Variables */
USBH_HandleTypeDef   hUSBHost;
FATFS                USBH_fatfs;
FIL                  MyFile;
uint8_t              rtext[200];
uint8_t              wtext[] = "USB Host Library : Mass Stroage Example";


FILE_OPERATION_CONTEXT fileOperation;

void test_OperateFile(void)
{
    FRESULT           fileResult;
    uint32_t          bytesread;
    uint32_t          bytesWritten;

    fileResult = f_open(&MyFile, "0:USBHost.txt", FA_CREATE_ALWAYS | FA_WRITE);

    if (FR_OK != fileResult)
    {
        dlog_error("open or create file error: %d\n", fileResult);

        return;
    }

    fileResult = f_write(&MyFile, wtext, sizeof(wtext), (void *)&bytesWritten);

    f_close(&MyFile);

    if((bytesWritten == 0) || (fileResult != FR_OK))
    {
        dlog_error("write file error: %d!\n", fileResult);

        return;
    }

    fileResult = f_open(&MyFile, "0:USBHost.txt", FA_READ);

    if(f_open(&MyFile, "0:USBHost.txt", FA_READ) != FR_OK)
    {
        dlog_error("open read file error: %d");

        return;
    }

    fileResult = f_read(&MyFile, rtext, sizeof(rtext), (void *)&bytesread);

    if((bytesread == 0) || (fileResult != FR_OK))
    {
        dlog_error("Cannot Read from the file \n");

        f_close(&MyFile);

        return;
    }

    f_close(&MyFile);

    if (bytesread == bytesWritten)
    {
        dlog_info("FatFs data compare SUCCES!\n");
    }
    else
    {
        dlog_error("FatFs data compare ERROR!\n");
    }

    return;
}


void test_DisplayFile(char *path, uint8_t recuLevel)
{
    FRESULT        fileResult = FR_OK;
    FILINFO        fno;
    DIR            dir;
    char          *fn;
    char           tmp[14];
    uint8_t        lineIndex = 0;

#if _USE_LFN
    static char    lfn[_MAX_LFN + 1];
    fno.lfname     = lfn;
    fno.lfsize     = sizeof(lfn);
#endif

    fileResult     = f_opendir(&dir, path);

    if (FR_OK != fileResult)
    {
        dlog_error("open dir fail\n");

        return;
    }
    else
    {
        while (USBH_MSC_IsReady(&hUSBHost))
        {
            fileResult = f_readdir(&dir, &fno);

            if ((FR_OK != fileResult) || (0 == fno.fname[0]) )
            {
                break;
            }
            if ('.' == fno.fname[0])
            {
                continue;
            }

#if _USE_LFN
            fn = fno.lfname;
            memcpy(tmp, fn, fno.lfsize);
#else
            fn = fno.fname;
            memcpy(tmp, fn, 13);
#endif
            lineIndex++;

            if (lineIndex > 0)
            {
                lineIndex = 0;
            }

            if(recuLevel == 1)
            {
                dlog_info("   |__");
            }
            else if(recuLevel == 2)
            {
                dlog_info("   |   |__");
            }
            if((fno.fattrib & AM_MASK) == AM_DIR)
            {
                strcat(tmp, "\n"); 
                dlog_info("%s", tmp);
            }
            else
            {
                strcat(tmp, "\n"); 
                dlog_info("%s", tmp);
            }
      
            if(((fno.fattrib & AM_MASK) == AM_DIR)&&(recuLevel == 2))
            {
                test_DisplayFile(fn, 2);
            }
        }
        f_closedir(&dir);
    }

    return;
}


void test_ProcessOperation(void)
{
    switch (fileOperation.state)
    {
    case FILE_OPERATION_START:
        if (APPLICATION_READY == Appli_state)
        {
            fileOperation.state = FILE_OPERATION_BUSY;
        }

        break;

    case FILE_OPERATION_BUSY:
        if (0 == fileOperation.operated)
        {
            if (APPLICATION_READY == Appli_state)
            {
                test_OperateFile();

                fileOperation.state    = FILE_OPERATION_DISPLAY;
                fileOperation.operated = 1;
            }
        }

        break;

    case FILE_OPERATION_DISPLAY:
        if (APPLICATION_READY == Appli_state)
        {
            test_DisplayFile("0:/", 1);

            fileOperation.state = FILE_OPERATION_START;
        }

        break;

    default:
        break;
    }

    if (APPLICATION_DISCONNECT == Appli_state)
    {
        Appli_state            = APPLICATION_IDLE;

        fileOperation.state    = FILE_OPERATION_START;
        fileOperation.operated = 0;
    }
}


void USBH_UserPorcess(USBH_HandleTypeDef *phost, uint8_t id)
{
    switch(id)
    {
    case HOST_USER_SELECT_CONFIGURATION:
        break;

    case HOST_USER_DISCONNECTION:
        Appli_state = APPLICATION_DISCONNECT;
        break;

    case HOST_USER_CLASS_ACTIVE:
        Appli_state = APPLICATION_READY;
        break;

    case HOST_USER_CONNECTION:
        break;

    default:
        break;
    }
}


void test_usbh(void)
{
    FRESULT   fileResult;

    dlog_info("start test_usbh\n");

    reg_IrqHandle(USB_OTG0_VECTOR_NUM, USB_OTG0_IRQHandler);

    USBH_Init(&hUSBHost, USBH_UserPorcess, 0);

    USBH_RegisterClass(&hUSBHost, USBH_MSC_CLASS);

    USBH_Start(&hUSBHost);

    FATFS_LinkDriver(&USBH_Driver, "0:/");

    fileResult = f_mount(&USBH_fatfs, "0:/", 0);

    if (fileResult != FR_OK)
    {
        dlog_error("mount fatfs error: %d\n", fileResult);

        return;
    }

    dlog_info("start to operate file\n");

    fileOperation.state     = FILE_OPERATION_START;
    fileOperation.operated  = 0;

    while (1)
    {
        USBH_Process(&hUSBHost);

        test_ProcessOperation();
    }
}
