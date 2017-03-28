#include <stdint.h>
#include "cpu_info.h"
#include "stm32f746xx.h"

ENUM_CPU_ID CPUINFO_GetLocalCpuId(void) 
{
    return *((ENUM_CPU_ID*)CPU_ID_INFO_ADDRESS);
}

void CPUINFO_ICacheEnable(uint8_t u8_icacheEnable)
{
    if (u8_icacheEnable != 0)
    {
        /* Enable I-Cache */
        SCB_EnableICache();
    }
    else
    {
        /* Disable I-Cache */
        SCB_DisableICache();
    }
}

void CPUINFO_DCacheEnable(uint8_t u8_dcacheEnable)
{
    if (u8_dcacheEnable != 0)
    {
        /* Enable D-Cache */
        SCB_EnableDCache();
    }
    else
    {
        /* Disable D-Cache */
        SCB_DisableDCache();
    }
}

