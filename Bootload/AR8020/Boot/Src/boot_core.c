#include <stdint.h>
#include <string.h>
#include "boot_core.h"
#include "boot_serial.h"

/****************************************
boot App
*****************************************/

void BOOT_BootApp(void)
{

    *((volatile uint32_t*)MCU1_CPU_WAIT) = 0;

    *((volatile uint32_t*)MCU2_CPU_WAIT) = 0;

    *((uint32_t*)MCU0_VECTOR_TABLE_REG) = 0;
    (*((void(*)())((*((uint32_t*)(ITCM0_START+4))))))();
}

void BOOT_StartBoot(uint8_t index)
{
    #pragma GCC diagnostic push
    #pragma GCC diagnostic ignored "-Wnonnull"
    if(0==index)
    {
        memcpy((void*)ITCM0_START, (void*)BOOT_ADDR0, BOOT_SIZE);       
    }
    else if(1==index)
    {
        memcpy((void*)ITCM0_START, (void*)BOOT_ADDR1, BOOT_SIZE);    
    }
    else
    {
        return;
    }
    #pragma GCC diagnostic pop
    *((volatile uint32_t*)(MCU0_VECTOR_TABLE_REG)) = ITCM0_START;
    (*((void(*)())((*((uint32_t*)(ITCM0_START+4))))))();
    
}

void BOOT_CopyFromNorToITCM(void)
{

    uint8_t* cpu0_app_size_addr = (uint8_t*)APPLICATION_IMAGE_START;
    uint32_t cpu0_app_size = GET_WORD_FROM_ANY_ADDR(cpu0_app_size_addr);
    uint32_t cpu0_app_start_addr = APPLICATION_IMAGE_START + 4;

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