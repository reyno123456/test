#include <stdint.h>
#include "cpu_info.h"

ENUM_CPU_ID CPUINFO_GetLocalCpuId(void) 
{
    return *((ENUM_CPU_ID*)CPU_ID_INFO_ADDRESS);
}

