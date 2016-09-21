#ifndef __INTER_CORE_H
#define __INTER_CORE_H

#include <stdint.h>

#define INTER_CORE_CPU_NUM                3

/* need to modify after debug ok */
#define INTER_CORE_MSG_CPU0_BASE_ADDR     0x21000000
#define INTER_CORE_MSG_CPU1_BASE_ADDR     0x21000000
#define INTER_CORE_MSG_CPU2_BASE_ADDR     0x21000000
#define INTER_CORE_MSG_POOL_MAX           10


#define INTER_CORE_TRIGGER_REG_ADDR       0xA003004C
#define INTER_CORE_TRIGGER_IRQ0_BITMAP    0x00000001
#define INTER_CORE_TRIGGER_IRQ1_BITMAP    0x00000002




typedef enum
{
    INTER_CORE_CPU0_ID = 0,
    INTER_CORE_CPU1_ID = 1,
    INTER_CORE_CPU2_ID = 2,
}INTER_CORE_CPU_ID;


typedef enum
{
    /* CPU0 -------------> CPU2 */

    /* CPU2 -------------> CPU0 */
    INTER_CORE_CPU2_CPU0_USB_VIDEO_DATA_REQ = 0x20000001,

}INTER_CORE_MSG_ID;


typedef struct
{
    INTER_CORE_MSG_ID    enMsgID;
    INTER_CORE_CPU_ID    enSrcCpuID;
    INTER_CORE_CPU_ID    enDstCpuID;
    void                *pData;
    uint8_t              msgLength;
}INTER_CORE_MSG_TYPE;


typedef struct
{
    uint8_t              firstMsgPos;
    uint8_t              nextMsgPos;
    INTER_CORE_MSG_TYPE  stInterCoreMsg[INTER_CORE_MSG_POOL_MAX];
}INTER_CORE_MSG_POOL_TYPE;


void                 InterCore_Init(void);
void                 InterCore_SendMsg(INTER_CORE_MSG_TYPE *pstInterCoreMsg);
INTER_CORE_MSG_TYPE* InterCore_GetMsg(INTER_CORE_CPU_ID enInterCoreCpuID);
extern void          InterCore_IRQ0Handler(void);
extern void          InterCore_IRQ1Handler(void);

void                 InterCore_TriggerIRQ0(void);
void                 InterCore_TriggerIRQ1(void);



#endif
