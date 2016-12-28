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


/* add for dma address and dtcm address convert */
#define DTCM_START_ADDR                 0x20000000
#define DTCM_END_ADDR                   0x20080000
#define DTCM_CPU0_DMA_ADDR_OFFSET       0x24080000
#define DTCM_CPU1_DMA_ADDR_OFFSET       0x24180000


#endif

