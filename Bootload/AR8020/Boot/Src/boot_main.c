#include <string.h>
#include <stdint.h>
#include "boot_serial.h"
#include "boot_norflash.h"
#include "boot_core.h"
#include "boot_interrupt.h"
#include "boot_command.h"
#include "boot_systicks.h"

uint8_t g_u8BootMode = 0;

static void console_init(uint32_t uart_num, uint32_t baut_rate)
{
    uart_init(uart_num, baut_rate);
}

static void uinit(void)
{
    SysTicks_UnInit();
}

int main(void)
{

    Boot_Info st_bootInfo;    
    console_init(0,115200);
    uart_puts(0,"boot stag 1\r\n");
    QUAD_SPI_SetSpeed();

    SysTicks_Init(64000);
    SysTicks_DelayMS(200);
    memset(&st_bootInfo,0xff,sizeof(st_bootInfo));
    QUAD_SPI_ReadBlockByByte(INFO_BASE,(uint8_t *)(&st_bootInfo),sizeof(st_bootInfo));

    if((st_bootInfo.bootloadaddress == 0xffffffff) || (st_bootInfo.bootloadaddress == 0x0) )
    {
        st_bootInfo.bootloadaddress=BOOT_ADDR0;
        uart_puts(0,"bootloadaddress default value\r\n");
    }
    if((st_bootInfo.apploadaddress == 0xffffffff) || (st_bootInfo.apploadaddress == 0x0))
    {
        st_bootInfo.apploadaddress=APPLICATION_IMAGE_START;
        uart_puts(0,"apploadaddress default value\r\n");
    }

    if('t' == uart_getc(0))
    {
        uart_puts(0,"boot bootload\r\n");
        ssleep(1);
        if('t' == uart_getc(0))
        {            
            
            if((0 == st_bootInfo.present_boot))
            {
                uart_puts(0,"boot BOOT0\r\n");       
            }
            else if((1 == st_bootInfo.present_boot))
            {   
                uart_puts(0,"boot BOOT1\r\n");
            }
            else
            {
                uart_puts(0,"boot info error\r\n"); 
            }
            BOOT_PrintInfo(st_bootInfo.bootloadaddress+IMAGE_HAER_OFSET);
            SysTicks_DelayMS(200);
            uinit();
            BOOT_StartBoot(st_bootInfo.present_boot,st_bootInfo.bootloadaddress+IMAGE_HAER_OFSET);
        }
        
    }
    uart_puts(0,"boot app\r\n");
    BOOT_PrintInfo(st_bootInfo.apploadaddress+IMAGE_HAER_OFSET);
    SysTicks_DelayMS(200);    
    uinit();
    BOOT_CopyFromNorToITCM(st_bootInfo.apploadaddress+IMAGE_HAER_OFSET);
    BOOT_BootApp();
} 

