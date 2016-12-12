#include "test_usbh.h"
#include "debuglog.h"
#include "interrupt.h"
#include "cmsis_os.h"
#include "sram.h"
#include "quad_spi_ctrl.h"
#include "nor_flash.h"
#include "upgrade.h"
#include "serial.h"
#include "test_usbh.h"
#include "systicks.h"
#include "bb_ctrl_proxy.h"
static uint8_t g_u8arrayRecData[RDWR_SECTOR_SIZE]={0};
USBH_HandleTypeDef              hUSBHost;
USBH_BypassVideoCtrl            g_usbhBypassVideoCtrl;
USBH_AppCtrl                    g_usbhAppCtrl;


static void USBH_UserProcess(USBH_HandleTypeDef *phost, uint8_t id)
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

static void UPGRADE_HApplicationInit(void)
{
    reg_IrqHandle(OTG_INTR0_VECTOR_NUM, USB_LL_OTG0_IRQHandler);

    USBH_Init(&hUSBHost, USBH_UserProcess, 0);

    USBH_RegisterClass(&hUSBHost, USBH_MSC_CLASS);

    USBH_Start(&hUSBHost);
    printf("usb host init done!\n");
    return;
}

void UPGRADE_Upgrade(void const *argument)
{

    FRESULT    fileResult;
    FIL        MyFile;
    uint32_t   u32_bytesRead= RDWR_SECTOR_SIZE;
    uint32_t   u32_recDataSum = 0;
    uint32_t   u32_norAddr = 0x20000;

    if(SFR_TRX_MODE_GROUND == BB_GetBoardMODE())
    {
        UPGRADE_HApplicationInit();
        USBH_MountUSBDisk();
    }
    

    printf("Nor flash init start ... \n");
    NOR_FLASH_Init();
    printf("Nor flash init end   ...\n");
    dlog_output(100);
    SysTicks_DelayMS(500);
    while(APPLICATION_READY != g_usbhAppCtrl.usbhAppState)
    {
        printf("find mass storage\n");    
    }
    dlog_output(100);
    if (APPLICATION_READY == g_usbhAppCtrl.usbhAppState)
    {
        fileResult = f_open(&MyFile,argument , FA_READ);
        if (FR_OK != fileResult)
        {
            printf("open or create file error: %d\n", fileResult);
            
            while(1);
        }          
        while(RDWR_SECTOR_SIZE == u32_bytesRead)
        {            
            memset(g_u8arrayRecData,0,RDWR_SECTOR_SIZE);
            fileResult = f_read(&MyFile, (void *)g_u8arrayRecData, RDWR_SECTOR_SIZE, (void *)&u32_bytesRead);
            if((fileResult != FR_OK))
            {
                printf("Cannot Read from the file \n");
                f_close(&MyFile);
            }
            NOR_FLASH_EraseSector(u32_norAddr);
            NOR_FLASH_WriteByteBuffer(u32_norAddr,g_u8arrayRecData,RDWR_SECTOR_SIZE); 
            printf("f_read success %d %x \n",u32_bytesRead,u32_norAddr);           
            dlog_output(100);
            u32_recDataSum+=u32_bytesRead;
            u32_norAddr += RDWR_SECTOR_SIZE;             
            //u8_upgradeFlage++;               
        }
         
        f_close(&MyFile);
        printf("upgrade ok %x\n",u32_recDataSum);
    }
    else
    {
        printf("don't find usb\n");
    }
    
    dlog_output(100);
    vTaskDelete(NULL);
}

