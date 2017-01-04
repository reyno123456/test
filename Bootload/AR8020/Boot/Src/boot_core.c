#include <stdint.h>
#include <string.h>
#include "boot_core.h"
#include "boot_serial.h"

/****************************************
boot App
*****************************************/


char  BOOT_HexToASCII(unsigned char  data_hex)
{ 
    char  ASCII_Data;
    ASCII_Data=data_hex & 0x0F;
    if(ASCII_Data<10) 
        ASCII_Data=ASCII_Data+0x30; //‘0--9’
    else  
        ASCII_Data=ASCII_Data+0x37; //‘A--F’
    return ASCII_Data;
}

void BOOT_HexGroupToString(unsigned int addr, unsigned int HexLength)
{
    
    unsigned int i=0;
    char OutStrBuffer[3];
    for(i=0;i<HexLength;i++)
    {
        
        OutStrBuffer[0]=BOOT_HexToASCII(((*((uint8_t*)(addr+i)))>>4)&0x0F);
        OutStrBuffer[1]=BOOT_HexToASCII((*((uint8_t*)(addr+i)))&0x0F);
        OutStrBuffer[2]='\0';
        uart_puts(0,OutStrBuffer);
    }    
}

void BOOT_PrintInfo(uint32_t u32_addr)
{
    uart_puts(0,"Created:");
    BOOT_HexGroupToString(u32_addr-DATEY_OFFSET, 2);
    uart_puts(0," ");
    BOOT_HexGroupToString(u32_addr-DATEm_OFFSET, 1);
    uart_puts(0," ");
    BOOT_HexGroupToString(u32_addr-DATEd_OFFSET, 1);
    uart_puts(0," ");
    BOOT_HexGroupToString(u32_addr-DATEH_OFFSET, 1);
    uart_puts(0,":");
    BOOT_HexGroupToString(u32_addr-DATEM_OFFSET, 1);
    uart_puts(0,":");
    BOOT_HexGroupToString(u32_addr-DATES_OFFSET, 1);
    uart_puts(0,"\r\n");
    uart_puts(0,"Load address:0x");
    BOOT_HexGroupToString(u32_addr-LOCALADDR_OFFSET, 4);
    uart_puts(0,"\r\n");
    uart_puts(0,"version V");
    BOOT_HexGroupToString(u32_addr-VERSION_MAJOR_OFFSET, 1);
    uart_puts(0,".");
    BOOT_HexGroupToString(u32_addr-VERSION_MINOR_OFFSET, 1);
    uart_puts(0,"\r\n");
    uart_puts(0,"Data size:0x");
    BOOT_HexGroupToString(u32_addr-SIZE1_OFFSET, 1);
    BOOT_HexGroupToString(u32_addr-SIZE2_OFFSET, 1);
    BOOT_HexGroupToString(u32_addr-SIZE3_OFFSET, 1);
    BOOT_HexGroupToString(u32_addr-SIZE4_OFFSET, 1);
    uart_puts(0,"\r\n");
}

void BOOT_BootApp(void)
{

    *((volatile uint32_t*)MCU1_CPU_WAIT) = 0;

    *((volatile uint32_t*)MCU2_CPU_WAIT) = 0;

    *((uint32_t*)MCU0_VECTOR_TABLE_REG) = 0;
    (*((void(*)())((*((uint32_t*)(ITCM0_START+4))))))();
}

void BOOT_StartBoot(uint8_t index,unsigned int address)
{

/*    uart_puts(0,"BOOT_StartBoot\r\n");
    char OutStrBuffer[3];
    OutStrBuffer[0]=BOOT_HexToASCII(((((address>>24)&0xff))>>4)&0x0F);
    OutStrBuffer[1]=BOOT_HexToASCII((((address>>24)&0xff))&0x0F);
    OutStrBuffer[3]='\0';
    uart_puts(0,OutStrBuffer);

    OutStrBuffer[0]=BOOT_HexToASCII(((((address>>16)&0xff))>>4)&0x0F);
    OutStrBuffer[1]=BOOT_HexToASCII((((address>>16)&0xff))&0x0F);
    OutStrBuffer[3]='\0';
    uart_puts(0,OutStrBuffer);

    OutStrBuffer[0]=BOOT_HexToASCII(((((address>>8)&0xff))>>4)&0x0F);
    OutStrBuffer[1]=BOOT_HexToASCII((((address>>8)&0xff))&0x0F);
    OutStrBuffer[3]='\0';
    uart_puts(0,OutStrBuffer);

    OutStrBuffer[0]=BOOT_HexToASCII(((((address)&0xff))>>4)&0x0F);
    OutStrBuffer[1]=BOOT_HexToASCII((((address)&0xff))&0x0F);
    OutStrBuffer[3]='\0';
    uart_puts(0,OutStrBuffer);
    uart_puts(0,"\r\n");
    uart_puts(0,"index\r\n");
    OutStrBuffer[0]=BOOT_HexToASCII(((((index)&0xff))>>4)&0x0F);
    OutStrBuffer[1]=BOOT_HexToASCII((((index)&0xff))&0x0F);
    OutStrBuffer[3]='\0';
    uart_puts(0,OutStrBuffer);
    uart_puts(0,"\r\n");
*/
    if((index !=0) && (index !=1))
    {
         uart_puts(0,"boot info error\r\n");
         return;
    }
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wnonnull"
    
    memcpy((void*)ITCM0_START, (void*)address, BOOT_SIZE);         
    #pragma GCC diagnostic pop
    *((volatile uint32_t*)(MCU0_VECTOR_TABLE_REG)) = ITCM0_START;
    (*((void(*)())((*((uint32_t*)(ITCM0_START+4))))))();
    
}

void BOOT_CopyFromNorToITCM(unsigned int address)
{

    /*uart_puts(0,"BOOT_CopyFromNorToITCM\r\n");
    char OutStrBuffer[3];
    OutStrBuffer[0]=BOOT_HexToASCII(((((address>>24)&0xff))>>4)&0x0F);
    OutStrBuffer[1]=BOOT_HexToASCII((((address>>24)&0xff))&0x0F);
    OutStrBuffer[3]='\0';
    uart_puts(0,OutStrBuffer);

    OutStrBuffer[0]=BOOT_HexToASCII(((((address>>16)&0xff))>>4)&0x0F);
    OutStrBuffer[1]=BOOT_HexToASCII((((address>>16)&0xff))&0x0F);
    OutStrBuffer[3]='\0';
    uart_puts(0,OutStrBuffer);

    OutStrBuffer[0]=BOOT_HexToASCII(((((address>>8)&0xff))>>4)&0x0F);
    OutStrBuffer[1]=BOOT_HexToASCII((((address>>8)&0xff))&0x0F);
    OutStrBuffer[3]='\0';
    uart_puts(0,OutStrBuffer);

    OutStrBuffer[0]=BOOT_HexToASCII(((((address)&0xff))>>4)&0x0F);
    OutStrBuffer[1]=BOOT_HexToASCII((((address)&0xff))&0x0F);
    OutStrBuffer[3]='\0';
    uart_puts(0,OutStrBuffer);
    uart_puts(0,"\r\n");*/

    uint8_t* cpu0_app_size_addr = (uint8_t*)address;
    uint32_t cpu0_app_size = GET_WORD_FROM_ANY_ADDR(cpu0_app_size_addr);
    uint32_t cpu0_app_start_addr = address + 4;

    uint8_t* cpu1_app_size_addr = (uint8_t*)(cpu0_app_start_addr + cpu0_app_size);
    uint32_t cpu1_app_size = GET_WORD_FROM_ANY_ADDR(cpu1_app_size_addr);
    uint32_t cpu1_app_start_addr = cpu0_app_start_addr + cpu0_app_size + 4;

    uint8_t* cpu2_app_size_addr = (uint8_t*)(cpu1_app_start_addr + cpu1_app_size);
    uint32_t cpu2_app_size = GET_WORD_FROM_ANY_ADDR(cpu2_app_size_addr);
    uint32_t cpu2_app_start_addr = cpu1_app_start_addr + cpu1_app_size + 4;
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wnonnull"
    memcpy((void*)ITCM0_START, (void*)cpu0_app_start_addr, cpu0_app_size);
    #pragma GCC diagnostic pop
    memcpy((void*)ITCM1_START, (void*)cpu1_app_start_addr, cpu1_app_size);
    memcpy((void*)ITCM2_START, (void*)cpu2_app_start_addr, cpu2_app_size);
}