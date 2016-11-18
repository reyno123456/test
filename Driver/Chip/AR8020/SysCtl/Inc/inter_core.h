#ifndef __INTER_CORE_H
#define __INTER_CORE_H

#include <stdint.h>
#include "sys_event.h"

//#define INTER_CORE_DEBUG_LOG_ENABLE

/* need to modify after debug ok */
#define INTER_CORE_MSG_SHARE_MEMORY_BASE_ADDR     0x21004000
#define INTER_CORE_MSG_SHARE_MEMORY_NUMBER        5
#define INTER_CORE_MSG_SHARE_MEMORY_DATA_LENGTH   16

#define INTER_CORE_TRIGGER_REG_ADDR               0xA003004C
#define INTER_CORE_TRIGGER_IRQ0_BITMAP            0x00000001
#define INTER_CORE_TRIGGER_IRQ1_BITMAP            0x00000002

typedef struct
{
    INTER_CORE_CPU_ID    enSrcCpuID;
    INTER_CORE_CPU_ID    enDstCpuID;
    uint32_t             dataAccessed;
    uint32_t             lock;
    INTER_CORE_MSG_ID    enMsgID;
    uint8_t              data[INTER_CORE_MSG_SHARE_MEMORY_DATA_LENGTH];
}INTER_CORE_MSG_TYPE;

// SRAM DCache disable
#define SRAM_MEMORY_MPU_REGION_NUMBER  1
#define SRAM_MEMORY_MPU_REGION_ST_ADDR_0 0x21004000
#define SRAM_MEMORY_MPU_REGION_ATTR_0    (0  << 28) | \
                                         (3  << 24) | \
                                         (1  << 19) | \
                                         (0  << 18) | \
                                         (0  << 17) | \
                                         (0  << 16) | \
                                         (0  <<  8) | \
                                         (11 <<  1) | \
                                         (1  <<  0)

void SRAM_SKY_BypassVideoConfig(uint32_t channel);

void SRAM_DCacheDisable(uint8_t type);


void InterCore_Init(void);
uint8_t InterCore_SendMsg(INTER_CORE_CPU_ID dst, INTER_CORE_MSG_ID msg, uint8_t* buf, uint32_t length);
uint8_t InterCore_GetMsg(INTER_CORE_MSG_ID* msg_p, uint8_t* buf, uint32_t max_length);

#endif
