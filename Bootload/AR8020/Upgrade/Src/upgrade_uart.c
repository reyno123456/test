#include <stdint.h>
#include <string.h>
#include "upgrade_command.h"
#include "serial.h"
#include "debuglog.h"
#include "interrupt.h"
#include "quad_spi_ctrl.h"
#include "upgrade_core.h"
#include "upgrade_uart.h"
#include "nor_flash.h"
#include "systicks.h"
#include "upgrade_md5.h"

#define UPGRADE_UART_DEBUGE

#ifdef  UPGRADE_UART_DEBUGE
#define UART_UPGRADE
#define DLOG_INFO(...) dlog_info(__VA_ARGS__)
#else
#define DLOG_INFO(...)
#endif

static volatile uint32_t g_u32RecCount;
static volatile uint32_t g_u32LoadAddr;
static volatile uint32_t g_u32ImageSize;
static volatile uint8_t g_u32RecFlage;
static char *g_pDst = (char *)RECEIVE_ADDR;
static uint8_t g_u8Amd5Sum[16];

#define READ_DATA_SIZE  1024*4  
#define MD5_SIZE        16  

static void UPGRADE_InitParament(void)
{
    g_u32RecCount = 0;
    g_u32ImageSize=0x1000;
    g_u32RecFlage=1;
    g_u32LoadAddr =0;
    memset(g_u8Amd5Sum,0,16);
} 
static int8_t UPGRADE_MD5SUM(uint32_t u32_addr)
{
    uint32_t i=0;
    uint32_t j=0;
    uint32_t u32_RecCountTmp=g_u32RecCount-34;
    uint32_t u32_Count=0;
    uint8_t md5_value[MD5_SIZE];
    //uint8_t    *p8_data = (uint8_t *)(u32_addr+34);
    uint8_t    *p8_data = (uint8_t *)(u32_addr+34);
    MD5_CTX md5;
    MD5Init(&md5);
    DLOG_INFO("start checksum nor flash\n");
    for(i=0;i<((u32_RecCountTmp)/RDWR_SECTOR_SIZE);i++)
    {
        MD5Update(&md5, (p8_data+RDWR_SECTOR_SIZE*i), RDWR_SECTOR_SIZE);
        u32_Count+=RDWR_SECTOR_SIZE;
    }
    if(0 != ((u32_RecCountTmp)%RDWR_SECTOR_SIZE))
    {
        MD5Update(&md5, (p8_data+RDWR_SECTOR_SIZE*i), (u32_RecCountTmp%RDWR_SECTOR_SIZE));
        u32_Count+=(u32_RecCountTmp%RDWR_SECTOR_SIZE);
    }
    MD5Final(&md5, md5_value);
    for(i=0;i<16;i++)
    {
        if(md5_value[i] != g_u8Amd5Sum[i])
        {
            DLOG_INFO("checksum......fail\n");
            for(j=0;j<16;j++)
            {
                DLOG_INFO("cmp %02x %02x\n",md5_value[j],g_u8Amd5Sum[j]);
                return -1;
            }
            
        }
    }
    DLOG_INFO("checksum......ok\n");
    return 0; 
}
static void UPGRADE_IRQHandler(uint32_t vectorNum)
{
    uint32_t          u32_isrType;
    uint32_t          u32_status;
    volatile uart_type   *uart_regs =(uart_type *)UART0_BASE;
    u32_status     = uart_regs->LSR;
    u32_isrType    = uart_regs->IIR_FCR;

    if (UART_IIR_RECEIVEDATA == (u32_isrType & UART_IIR_RECEIVEDATA))
    {
        if ((u32_status & UART_LSR_DATAREADY) == UART_LSR_DATAREADY)
        {
            *(g_pDst +g_u32RecCount) = uart_regs->RBR_THR_DLL;        
            //g_pDst++;
            g_u32RecCount++;
        }
    }
}

static void UPGRADE_UartReceive(void)
{
    DLOG_INFO("Nor flash init start ...\n");
    NOR_FLASH_Init();
    DLOG_INFO("Nor flash end\n");
    dlog_output(100);
    uint32_t i=0;
    //sdram init Done
    while(!(SDRAM_INIT_DONE & 0x01))
    {
        ;
    }
    reg_IrqHandle(UART_INTR0_VECTOR_NUM, UPGRADE_IRQHandler, NULL);
    DLOG_INFO("interrupt\n");
    dlog_output(100);
    while((g_u32ImageSize!=g_u32RecCount))
    {
        if((1 == g_u32RecFlage)&&(g_u32RecCount>100))
        {
            uint8_t* p8_sizeAddr = (uint8_t*)(RECEIVE_ADDR+14);
            uint8_t* p8_loadAddr = (uint8_t*)(RECEIVE_ADDR+8);
            uint8_t* p8_md5Addr = (uint8_t*)(RECEIVE_ADDR+18);
            g_u32ImageSize = GET_WORD_FROM_ANY_ADDR(p8_sizeAddr);
            g_u32RecFlage =0;
            g_u32LoadAddr = GET_WORD_BOOT_INOF(p8_loadAddr);
            for(i=0;i<16;i++)
            {
                //DLOG_INFO("cmp %02x %02x\n",md5_value[i],g_u8Amd5Sum[i]);
                g_u8Amd5Sum[i]=*(p8_md5Addr+i);
            } 
            DLOG_INFO("image size %x load address %x\n",g_u32ImageSize,g_u32LoadAddr);
            dlog_output(100);
        }
        if((0 !=g_u32RecCount) && (0 == g_u32RecCount%10000))
        {
            DLOG_INFO("receive data %d\n",g_u32RecCount);
            dlog_output(100);
        }
    }    
    DLOG_INFO("receive finish %d\n",g_u32RecCount);
    dlog_output(100);
    UPGRADE_CommandInit(0);
}

static void UPGRADE_ModifyBootInfo(uint8_t index)
{
    uint8_t i=0;
    Boot_Info st_bootInfo;
    memset(&st_bootInfo,0xff,sizeof(st_bootInfo)); 
    NOR_FLASH_ReadByteBuffer(0x1000,(uint8_t *)(&st_bootInfo),sizeof(st_bootInfo));
    NOR_FLASH_EraseSector(0x1000);
    
    if(index == 0)
    {
        if(0==st_bootInfo.present_boot)
        {
            st_bootInfo.present_boot=1;        
        }
        else
        {
            st_bootInfo.present_boot=0;                
        }
        st_bootInfo.bootloadaddress = g_u32LoadAddr;
    }
    else
    {
        st_bootInfo.apploadaddress=g_u32LoadAddr;
    }
    NOR_FLASH_WriteByteBuffer(0x1000,(uint8_t *)(&st_bootInfo),sizeof(st_bootInfo)); 
}

static void UPGRADE_EraseWriteFlash(uint32_t u32_addr)
{
    uint32_t i=0;
    for(i=0;i<(g_u32RecCount/RDWR_SECTOR_SIZE);i++)
    {
        DLOG_INFO("upgrade  %p %d%%\n",u32_addr+RDWR_SECTOR_SIZE*i,(i*RDWR_SECTOR_SIZE)*100/g_u32RecCount);
        NOR_FLASH_EraseSector(u32_addr+RDWR_SECTOR_SIZE*i);
        NOR_FLASH_WriteByteBuffer((u32_addr+RDWR_SECTOR_SIZE*i),(g_pDst+RDWR_SECTOR_SIZE*i),RDWR_SECTOR_SIZE);
        dlog_output(100);
    }
    if(0 != g_u32RecCount%RDWR_SECTOR_SIZE)
    {
        NOR_FLASH_EraseSector(u32_addr+RDWR_SECTOR_SIZE*i);
        NOR_FLASH_WriteByteBuffer((u32_addr+RDWR_SECTOR_SIZE*i),(g_pDst+RDWR_SECTOR_SIZE*i),RDWR_SECTOR_SIZE);
    }
    dlog_output(100);
    DLOG_INFO("upgrade  finish\n");
}

void UPGRADE_APPFromUart(void)
{
    UPGRADE_InitParament();
    UPGRADE_UartReceive();    
    if(g_u32LoadAddr<APPLICATION_IMAGE_START)
    {
        DLOG_INFO("image upgrade address error\n");
        dlog_output(100);
        return;
    }
    
    if(-1 != UPGRADE_MD5SUM(RECEIVE_ADDR))
    {
        UPGRADE_EraseWriteFlash(0x20000);
        //UPGRADE_ModifyBootInfo(1);
    }
    
    
}

void UPGRADE_BootloadFromUart(void)
{
    UPGRADE_InitParament();
    UPGRADE_UartReceive();
    if((g_u32LoadAddr<BOOT_ADDR) || (g_u32LoadAddr>=APPLICATION_IMAGE_START) )
    {
        DLOG_INFO("image upgrade address error\n");
        dlog_output(100);
        return;
    }
    
     if(-1 != UPGRADE_MD5SUM(RECEIVE_ADDR))
    {
        UPGRADE_EraseWriteFlash(g_u32LoadAddr-0x10000000);
        UPGRADE_ModifyBootInfo(0);
    }
    
}