#include "test_usbh.h"
#include "debuglog.h"
#include "interrupt.h"
#include "cmsis_os.h"
#include "dma.h"
#include "sram.h"


#define USB_VIDEO_BYPASS_SIZE_ONCE      (8192)
#define USB_VIDEO_BYPASS_DEST_ADDR      (0xB1000000)


/* USB Host Global Variables */
USBH_HandleTypeDef              hUSBHost;
USBH_BypassVideoCtrl            g_usbhBypassVideoCtrl;
USBH_AppCtrl                    g_usbhAppCtrl;

#define RDWR_SECTOR_SIZE (1024*4)


void BOOTLOAD_Upgrade(void)
{

    FRESULT    fileResult;
    FIL        MyFile;
    uint32_t   u32_bytesRead= RDWR_SECTOR_SIZE;
    uint8_t    *u8_arrayRecData = (uint8_t *)0x21004000;
    uint32_t   u32_recDataSum = 0;
    uint32_t   u32_norAddr = (0x10000+0x10000000);
    dlog_info("Nor flash init start ...");
    NOR_FLASH_Init();
    dlog_info("Nor flash init end   ...");
    if (APPLICATION_READY == g_usbhAppCtrl.usbhAppState)
    {
        fileResult = f_open(&MyFile, "ar8020.bin", FA_READ);
        if (FR_OK != fileResult)
        {
            dlog_info("open or create file error: %d\n", fileResult);
            return;
        }                    
        while(RDWR_SECTOR_SIZE == u32_bytesRead)
        {
            
           // memset(u8_arrayRecData,0,RDWR_SECTOR_SIZE);
            NOR_FLASH_EraseSector(u32_norAddr);
            QUAD_SPI_WriteEnable();
            QUAD_SPI_CheckBusy();
            fileResult = f_read(&MyFile, u32_norAddr, RDWR_SECTOR_SIZE, (void *)&u32_bytesRead);
            if((fileResult != FR_OK))
            {
                dlog_info("Cannot Read from the file \n");
                f_close(&MyFile);
            }
            QUAD_SPI_CheckBusy();
            u32_recDataSum+=u32_bytesRead;
            u32_norAddr += RDWR_SECTOR_SIZE; 
           // dlog_info("f_read success %d!",u32_bytesRead);
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

void USBH_BypassVideo(void)
{
    FRESULT             fileResult;
    uint32_t            bytesread;
    uint8_t            *videoBuff;

    fileResult          = FR_OK;
    bytesread           = 0;
    videoBuff           = SRAM_BUFF_0_ADDRESS;

    dlog_info("enter USBH_BypassVideo Task!\n");

    while (1)
    {
        if (osOK == osSemaphoreWait(g_usbhBypassVideoCtrl.semID, osWaitForever))
        {
            while (1)
            {
                if ((APPLICATION_READY == g_usbhAppCtrl.usbhAppState)
                  &&(1 == g_usbhBypassVideoCtrl.taskActivate))
                {
                    if (g_usbhBypassVideoCtrl.fileOpened == 0)
                    {
                        g_usbhBypassVideoCtrl.fileOpened = 1;

                        fileResult = f_open(&(g_usbhAppCtrl.usbhAppFile), "0:usbtest.264", FA_READ);

                        if(fileResult != FR_OK)
                        {
                            dlog_error("open file error: %d\n", (uint32_t)fileResult);

                            break;
                        }
                    }

                    fileResult = f_read(&(g_usbhAppCtrl.usbhAppFile), videoBuff, USB_VIDEO_BYPASS_SIZE_ONCE, (void *)&bytesread);

                    if(fileResult != FR_OK)
                    {
                        g_usbhBypassVideoCtrl.fileOpened    = 0;
                        f_close(&(g_usbhAppCtrl.usbhAppFile));

                        dlog_error("Cannot Read from the file \n");

                        continue;
                    }

                    dma_transfer((uint32_t *)videoBuff, (uint32_t *)SRAM_SKY_BYPASS_ENCODER_BUFF, 8192);

                    osDelay(100);

                    if (bytesread < USB_VIDEO_BYPASS_SIZE_ONCE)
                    {
                        dlog_info("a new round!\n");
                        g_usbhBypassVideoCtrl.fileOpened    = 0;
                        f_close(&(g_usbhAppCtrl.usbhAppFile));
                    }
                }
                else
                {
                    if (1 == g_usbhBypassVideoCtrl.fileOpened)
                    {
                        g_usbhBypassVideoCtrl.fileOpened    = 0;
                        f_close(&(g_usbhAppCtrl.usbhAppFile));
                    }

                    break;
                }

            }

            g_usbhBypassVideoCtrl.taskActivate  = 0;
        }
    }
}


void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id)
{
    switch(id)
    {
    case HOST_USER_SELECT_CONFIGURATION:
        break;

    case HOST_USER_DISCONNECTION:
        g_usbhAppCtrl.usbhAppState  = APPLICATION_DISCONNECT;
        break;

    case HOST_USER_CLASS_ACTIVE:
        g_usbhAppCtrl.usbhAppState  = APPLICATION_READY;
        break;

    case HOST_USER_CONNECTION:
        break;

    default:
        break;
    }
}


void USBH_ApplicationInit(void)
{
    reg_IrqHandle(OTG_INTR0_VECTOR_NUM, USB_LL_OTG0_IRQHandler);

    USBH_Init(&hUSBHost, USBH_UserProcess, 0);

    USBH_RegisterClass(&hUSBHost, USBH_MSC_CLASS);

    USBH_Start(&hUSBHost);
}


void USBH_MountUSBDisk(void)
{
    FRESULT   fileResult;

    FATFS_LinkDriver(&USBH_Driver, "0:/");

    fileResult = f_mount(&(g_usbhAppCtrl.usbhAppFatFs), "0:/", 0);

    if (fileResult != FR_OK)
    {
        dlog_error("mount fatfs error: %d\n", fileResult);

        return;
    }
}


void USBH_MainTask(void)
{
    osEvent event;

    USBH_ApplicationInit();

    USBH_MountUSBDisk();

    dlog_info("usb host main task\n");

    while (1)
    {
        event = osMessageGet(g_usbhAppCtrl.usbhAppEvent, osWaitForever);

        if (event.status == osEventMessage)
        {
            switch (event.value.v)
            {
            case USBH_APP_START_BYPASS_VIDEO:
                {
                    /* Need to start a new task */
                    if (0 == g_usbhBypassVideoCtrl.taskExist)
                    {
                        osThreadDef(BypassTask, USBH_BypassVideo, osPriorityIdle, 0, 4 * 128);
                        g_usbhBypassVideoCtrl.threadID  = osThreadCreate(osThread(BypassTask), NULL);

                        if (NULL == g_usbhBypassVideoCtrl.threadID)
                        {
                            dlog_error("create Video Bypass Task error!\n");

                            break;
                        }

                        g_usbhBypassVideoCtrl.taskExist = 1;

                        osSemaphoreDef(bypassVideoSem);
                        g_usbhBypassVideoCtrl.semID     = osSemaphoreCreate(osSemaphore(bypassVideoSem), 1);

                        if (NULL == g_usbhBypassVideoCtrl.semID)
                        {
                            osThreadTerminate(g_usbhBypassVideoCtrl.threadID);
                            g_usbhBypassVideoCtrl.taskExist = 0;

                            dlog_error("create Video Bypass Semaphore error!\n");

                            break;
                        }
                    }

                    /* activate the task */
                    SRAM_SKY_EnableBypassVideoConfig(0);

                    osSemaphoreRelease(g_usbhBypassVideoCtrl.semID);

                    break;
                }

            case USBH_APP_STOP_BYPASS_VIDEO:
                {
                    dlog_info("stop bypassvideo task!\n");

                    SRAM_SKY_DisableBypassVideoConfig(0);
                }
                break;
            case USBH_UPGRADE:
                {
                    osThreadDef(UsbUpgrade, BOOTLOAD_Upgrade, osPriorityIdle, 0, 10 * 128);
                    osThreadCreate(osThread(UsbUpgrade), NULL);
                }
                break;
            case USBH_APP_CREATE_FILE:
                break;

            case USBH_APP_READ_FILE:
                break;

            case USBH_APP_CREATE_FOLDER:
                break;

            case USBH_APP_SHOW_DIR:
                break;

            default:
                break;
            }
        }
    }
}


