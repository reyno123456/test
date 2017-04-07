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

void CPUINFO_ICacheInvalidate(void)
{
    SCB_InvalidateICache();
}

void CPUINFO_DCacheInvalidate(void)
{
    SCB_InvalidateDCache();
}

void CPUINFO_DCacheClean(void)
{
    SCB_CleanDCache();
}

void CPUINFO_DCacheCleanInvalidate(void)
{
    SCB_CleanInvalidateDCache();
}

void CPUINFO_DCacheInvalidateByAddr(uint32_t *addr, int32_t dsize)
{
    SCB_InvalidateDCache_by_Addr(addr, dsize);
}

void CPUINFO_DCacheCleanByAddr(uint32_t *addr, int32_t dsize)
{
    SCB_CleanDCache_by_Addr(addr, dsize);
}

void CPUINFO_DCacheCleanInvalidateByAddr(uint32_t *addr, int32_t dsize)
{
    SCB_CleanInvalidateDCache_by_Addr(addr, dsize);
}



