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

#define UPGRADE_UART_DEBUGE

#ifdef  UPGRADE_UART_DEBUGE
#define UART_UPGRADE
#define DLOG_INFO(...) dlog_info(__VA_ARGS__)
#else
#define DLOG_INFO(...)
#endif


static volatile uint8_t g_u8UartFinish;
static volatile uint32_t g_u32RecCount;
static char *g_pDst = (char *)RECEIVE_ADDR;

static void UPGRADE_IRQHandler(void)
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
            if((0x67 == *(g_pDst +g_u32RecCount) ) && (0x45 == *(g_pDst +g_u32RecCount-1)) && (0x34 == *(g_pDst +g_u32RecCount-2)))
            {
                g_u8UartFinish=0;
            }           
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
    //sdram init Done
    while(!(SDRAM_INIT_DONE & 0x01))
    {
        ;
    }
    reg_IrqHandle(UART_INTR0_VECTOR_NUM, UPGRADE_IRQHandler);
    while(g_u8UartFinish)
    {
        if((0 !=g_u32RecCount) && (0 == g_u32RecCount%10000))
            DLOG_INFO("receive data %d\n",g_u32RecCount);
    }
    dlog_output(100);
    DLOG_INFO("receive finish %d\n",g_u32RecCount);
    UPGRADE_CommandInit(0);
}

static uint32_t UPGRADE_ModifyBootInfo(void)
{
    uint32_t addr=0;
    uint8_t i=0;
    Boot_Info st_bootInfo;
    memset(&st_bootInfo,0,sizeof(st_bootInfo)); 
    NOR_FLASH_ReadByteBuffer(0x1000,(uint8_t *)(&st_bootInfo),sizeof(st_bootInfo));
    NOR_FLASH_EraseSector(0x1000);
    if(0==st_bootInfo.present_boot)
    {
        addr=0x11000;
        st_bootInfo.present_boot=1;
    }
    else
    {
        addr=0x2000;
        st_bootInfo.present_boot=0;                
    }
    NOR_FLASH_WriteByteBuffer(0x1000,(uint8_t *)(&st_bootInfo),sizeof(st_bootInfo)); 
    return  addr;
}

static void UPGRADE_EraseWriteFlash(uint32_t u32_addr)
{
    uint32_t i=0;
    for(i=0;i<(g_u32RecCount/RDWR_SECTOR_SIZE);i++)
    {
        DLOG_INFO("upgrade  %p %d%%\n",u32_addr+RDWR_SECTOR_SIZE*i,(i*RDWR_SECTOR_SIZE)*100/g_u32RecCount);
        NOR_FLASH_EraseSector(u32_addr+RDWR_SECTOR_SIZE*i);
        NOR_FLASH_WriteByteBuffer((u32_addr+RDWR_SECTOR_SIZE*i),(g_pDst+RDWR_SECTOR_SIZE*i),RDWR_SECTOR_SIZE);
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
    g_u8UartFinish = 1;
    g_u32RecCount = 0;
    
    UPGRADE_UartReceive();
    UPGRADE_EraseWriteFlash(APP_ADDR_OFFSET);
    
}

void UPGRADE_BootloadFromUart(void)
{
    g_u8UartFinish = 1;
    g_u32RecCount = 0;
    uint32_t u32_addr=0;
    UPGRADE_UartReceive();
    g_u32RecCount -=3;
    u32_addr = UPGRADE_ModifyBootInfo();
    UPGRADE_EraseWriteFlash(u32_addr);
    
}