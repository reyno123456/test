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
    BOOT_CommandInit(uart_num);
}

static void uinit(void)
{
    SysTicks_UnInit();
    INTR_NVIC_DisableIRQ(UART_INTR0_VECTOR_NUM);
}

int main(void)
{

    Boot_Info st_bootInfo;
    memset(&st_bootInfo,0xff,sizeof(st_bootInfo));
    QUAD_SPI_ReadBlockByByte(INFO_BASE,(uint8_t *)(&st_bootInfo),sizeof(st_bootInfo));
    console_init(0,360000);
    uart_puts(0,"boot stag 1\r\n");
    QUAD_SPI_SetSpeed();

    SysTicks_Init(64000);
    ssleep(1);
   
    if(1 == g_u8BootMode)
    {
        ssleep(1);
        if((0 == st_bootInfo.present_boot))
        {
        	uart_puts(0,"boot BOOT0\r\n");
            uinit();
            BOOT_StartBoot(0);        
        }
        else if((1 == st_bootInfo.present_boot))
        {   
        	uart_puts(0,"boot BOOT1\r\n");
            uinit();
            BOOT_StartBoot(1);
        }
        else
        {
            uart_puts(0,"boot info error\r\n"); 
        }
    }

    uart_puts(0,"boot app\r\n");    
    uinit();
    BOOT_CopyFromNorToITCM();
    BOOT_BootApp();
} 

