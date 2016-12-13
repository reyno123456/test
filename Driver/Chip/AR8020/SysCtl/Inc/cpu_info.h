#ifndef CPU_ID_H
#define CPU_ID_H

#define CPU_ID_INFO_ADDRESS 0x0000018C

typedef enum
{
    ENUM_CPU0_ID = 0,
    ENUM_CPU1_ID = 1,
    ENUM_CPU2_ID = 2,
}ENUM_CPU_ID;

ENUM_CPU_ID CPUINFO_GetLocalCpuId(void);

#endif

