#include <stdint.h>
#include "cpu_info.h"
#include "stm32f746xx.h"

ENUM_CPU_ID CPUINFO_GetLocalCpuId(void) 
{
    return *((ENUM_CPU_ID*)CPU_ID_INFO_ADDRESS);
}

void CPUINFO_ICacheEnable(uint8_t u8_icacheEnable)
{
    static uint8_t s_u8_icacheEnable = 0;
    
    if (u8_icacheEnable != 0)
    {
        /* Enable I-Cache */
        SCB_EnableICache();
        s_u8_icacheEnable = 1;
    }
    else
    {
        if (s_u8_icacheEnable != 0)
        {
            /* Disable I-Cache */
            SCB_DisableICache();
            s_u8_icacheEnable = 0;
        }
    }
}

void CPUINFO_DCacheEnable(uint8_t u8_dcacheEnable)
{
    static uint8_t s_u8_dcacheEnable = 0;
    
    if (u8_dcacheEnable != 0)
    {
        /* Enable D-Cache */
        SCB_EnableDCache();
        s_u8_dcacheEnable = 1;
    }
    else
    {
        if (s_u8_dcacheEnable != 0)
        {
            /* Disable D-Cache */
            SCB_DisableDCache();
            s_u8_dcacheEnable = 0;
        }
    }
}

