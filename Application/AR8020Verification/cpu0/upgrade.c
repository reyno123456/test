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
#include "md5.h"
#include "systicks.h"
#include "bb_ctrl_proxy.h"

static uint8_t g_u8arrayRecData[RDWR_SECTOR_SIZE]={0};

USBH_HandleTypeDef              hUSBHost;
USBH_BypassVideoCtrl            g_usbhBypassVideoCtrl;
USBH_AppCtrl                    g_usbhAppCtrl;

static uint8_t  g_u8upgradeFlage;
static uint8_t  g_u8Amd5Sum[16];
static uint32_t g_u32address=0;
static uint32_t   g_u32recDataSum = 0;

#define READ_DATA_SIZE  1024*4  
#define MD5_SIZE        16  

static uint8_t UPGRADE_MD5SUM(void)
{
    uint32_t i=0;
    uint32_t u32_RecCountTmp = g_u32recDataSum-34;
    uint32_t u32_AddressTmp  = g_u32address+34;
    uint32_t u32_Count=0;
    uint8_t  md5_value[MD5_SIZE];
//    uint8_t    *p8_data = (uint8_t *)(g_u32address+34);
    MD5_CTX md5;
    MD5Init(&md5);

    for(i=0;i<((u32_RecCountTmp)/RDWR_SECTOR_SIZE);i++)
    {
        NOR_FLASH_ReadByteBuffer((u32_AddressTmp+RDWR_SECTOR_SIZE*i),g_u8arrayRecData,RDWR_SECTOR_SIZE);
        MD5Update(&md5, g_u8arrayRecData, RDWR_SECTOR_SIZE);        
        u32_Count+=RDWR_SECTOR_SIZE;
    }
    if(0 != ((u32_RecCountTmp)%RDWR_SECTOR_SIZE))
    {
        NOR_FLASH_ReadByteBuffer((u32_AddressTmp+RDWR_SECTOR_SIZE*i),g_u8arrayRecData,(u32_RecCountTmp%RDWR_SECTOR_SIZE));
        MD5Update(&md5, g_u8arrayRecData, (u32_RecCountTmp%RDWR_SECTOR_SIZE));
        u32_Count+=(u32_RecCountTmp%RDWR_SECTOR_SIZE);
    }
    MD5Final(&md5, md5_value);
    for(i=0;i<16;i++)
    {
        if(md5_value[i] != g_u8Amd5Sum[i])
        {
            dlog_info("nor flash checksum .........fail\n");
            return -1;
            vTaskDelete(NULL);
        }
    }
    dlog_info("nor flash checksum .........ok\n"); 
    return 0;
}

static void BOOT_PrintInfo(uint32_t u32_addr)
{
    uint8_t* p8_infoAddr = (uint8_t*)(u32_addr);
    uint8_t i=0;
    dlog_info("Created:%02x%02x %02x %02x %02x:%02x:%02x\n",*(p8_infoAddr+1),*(p8_infoAddr+2),*(p8_infoAddr+3),*(p8_infoAddr+4)\
                                                        ,*(p8_infoAddr+5),*(p8_infoAddr+6),*(p8_infoAddr+7));
    dlog_info("load address:0x%02x%02x%02x%02x\n",*(p8_infoAddr+8),*(p8_infoAddr+9),*(p8_infoAddr+10),*(p8_infoAddr+11));
    dlog_info("Version:%02x.%02x\n",*(p8_infoAddr+12),*(p8_infoAddr+13));
    dlog_info("Data size:%x\n",GET_WORD_FROM_ANY_ADDR(p8_infoAddr+14));
    for(i=0;i<16;i++)
    {
        g_u8Amd5Sum[i]=*(p8_infoAddr+18+i);
    }
}



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
    dlog_info("usb host init done!\n");
    return;
}

static void UPGRADE_ModifyBootInfo()
{
    uint8_t i=0;
    Boot_Info st_bootInfo;
    memset(&st_bootInfo,0xff,sizeof(st_bootInfo)); 
    NOR_FLASH_ReadByteBuffer(0x1000,(uint8_t *)(&st_bootInfo),sizeof(st_bootInfo));
    NOR_FLASH_EraseSector(0x1000);
    
   
    st_bootInfo.apploadaddress=g_u32address + 0x10000000;
    
    NOR_FLASH_WriteByteBuffer(0x1000,(uint8_t *)(&st_bootInfo),sizeof(st_bootInfo)); 
}

void UPGRADE_Upgrade(void const *argument)
{

    FRESULT    fileResult;
    FIL        MyFile;
    uint32_t   u32_bytesRead= RDWR_SECTOR_SIZE;
    uint32_t   u32_recDataSumTmp =0;
    uint32_t   u32_norAddr = 0x20000;
    uint32_t   i;  
    uint8_t    md5_value[MD5_SIZE];  
    MD5_CTX md5;
    g_u8upgradeFlage =0;

    
    if(SFR_TRX_MODE_GROUND == BB_GetBoardMODE())
    {
        UPGRADE_HApplicationInit();
        USBH_MountUSBDisk();
    }

    dlog_info("Nor flash init start ... \n");
    NOR_FLASH_Init();
    dlog_info("Nor flash init end   ...\n");
    dlog_output(100);
    SysTicks_DelayMS(500);
    while(APPLICATION_READY != g_usbhAppCtrl.usbhAppState)
    {
        dlog_info("finding mass storage\n");
        SysTicks_DelayMS(500);    
    }
    dlog_output(100); 
    #if 1
    MD5Init(&md5);
    u32_bytesRead = RDWR_SECTOR_SIZE;
    fileResult = f_open(&MyFile,argument , FA_READ);
    if (FR_OK != fileResult)
    {
        dlog_info("open or create file error: %d\n", fileResult);
        
        vTaskDelete(NULL);
    }          
    while(RDWR_SECTOR_SIZE == u32_bytesRead)
    {            
        memset(g_u8arrayRecData,0,RDWR_SECTOR_SIZE);
        fileResult = f_read(&MyFile, (void *)g_u8arrayRecData, RDWR_SECTOR_SIZE, (void *)&u32_bytesRead);
        if((fileResult != FR_OK))
        {
            dlog_info("Cannot Read from the file \n");
            f_close(&MyFile);
            vTaskDelete(NULL);
        }
        if(g_u8upgradeFlage!=0)
        {               
           MD5Update(&md5, g_u8arrayRecData, u32_bytesRead);
        }
        else
        {                
            memset(g_u8Amd5Sum,0,16);
            BOOT_PrintInfo((uint32_t)(&g_u8arrayRecData));
            u32_norAddr = GET_WORD_BOOT_INOF(&(g_u8arrayRecData[8]));
            u32_norAddr-=0x10000000;
            g_u32address = u32_norAddr;
            MD5Update(&md5, &(g_u8arrayRecData[34]), (u32_bytesRead-34));
            dlog_info("address %x\n",u32_norAddr);  
        }
        dlog_info("checkings file\n");
        dlog_output(100); 
        SysTicks_DelayMS(1);
        g_u32recDataSum+=u32_bytesRead;   
        g_u8upgradeFlage++;           
    }
    MD5Final(&md5, md5_value); 
    f_close(&MyFile);    
    for(i=0;i<16;i++)
    {
        
        if(md5_value[i] != g_u8Amd5Sum[i])
        {
            dlog_info("checksum .........fail\n");
            vTaskDelete(NULL);
        }
    }
    dlog_info("file checksum .........ok\n");
    dlog_output(100); 
    #endif
 
    u32_bytesRead = RDWR_SECTOR_SIZE;
    fileResult = f_open(&MyFile,argument , FA_READ);
    if (FR_OK != fileResult)
    {
        dlog_info("open or create file error: %d\n", fileResult);
        vTaskDelete(NULL);
    }          
    while(RDWR_SECTOR_SIZE == u32_bytesRead)
    {            
        memset(g_u8arrayRecData,0,RDWR_SECTOR_SIZE);
        fileResult = f_read(&MyFile, (void *)g_u8arrayRecData, RDWR_SECTOR_SIZE, (void *)&u32_bytesRead);
        if((fileResult != FR_OK))
        {
            dlog_info("Cannot Read from the file \n");
            f_close(&MyFile);
        }
        dlog_info("fread %d\n",u32_bytesRead);
        NOR_FLASH_EraseSector(u32_norAddr);
        NOR_FLASH_WriteByteBuffer(u32_norAddr,g_u8arrayRecData,RDWR_SECTOR_SIZE); 
        dlog_info("write flash %d%%\n",(u32_recDataSumTmp*100/g_u32recDataSum));
        dlog_output(100);          
        u32_recDataSumTmp+=u32_bytesRead; 
        
        u32_norAddr += RDWR_SECTOR_SIZE;                      
    }    
         
    f_close(&MyFile);
    dlog_info("upgrade ok %x\n",g_u32recDataSum);
    dlog_info("start checksum nor_flash .......\n");
    dlog_output(100);
    if(-1 != UPGRADE_MD5SUM())
    {
        UPGRADE_ModifyBootInfo();
    }
    
    vTaskDelete(NULL);

}
