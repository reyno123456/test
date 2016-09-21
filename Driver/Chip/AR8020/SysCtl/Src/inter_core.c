#include "inter_core.h"
#include "debuglog.h"
#include "string.h"
#include "reg_map.h"


INTER_CORE_MSG_POOL_TYPE  g_stInterCoreMsgPool[3];


void InterCore_Init(void)
{
    uint32_t          index;
    uint32_t          prioritygroup;

    for (index = 0; index < INTER_CORE_CPU_NUM; index++)
    {
        g_stInterCoreMsgPool[index].firstMsgPos = 0;
        g_stInterCoreMsgPool[index].nextMsgPos  = 0;
    }

    NVIC_ISER->ISER2 |= 0x00000020;
    NVIC_ISER->ISER2 |= 0x00000040;
}



void InterCore_SendMsg(INTER_CORE_MSG_TYPE *pstInterCoreMsg)
{
    INTER_CORE_MSG_POOL_TYPE  *pstDestCpuMsgPool;
    uint32_t                  *triggerAddr;

    /* put the message into the dest cpu pool */
    pstDestCpuMsgPool = &g_stInterCoreMsgPool[pstInterCoreMsg->enDstCpuID];

    memcpy(&pstDestCpuMsgPool->stInterCoreMsg[pstDestCpuMsgPool->nextMsgPos], pstInterCoreMsg, sizeof(INTER_CORE_MSG_TYPE));

    if (pstDestCpuMsgPool->nextMsgPos < (INTER_CORE_MSG_POOL_MAX - 1))
    {
        pstDestCpuMsgPool->nextMsgPos++;
    }
    else
    {
        pstDestCpuMsgPool->nextMsgPos = 0;
    }

    if (pstDestCpuMsgPool->firstMsgPos == pstDestCpuMsgPool->nextMsgPos)
    {
        dlog_error("dest cpu message pool is full\n");
    }

    triggerAddr        = INTER_CORE_TRIGGER_REG_ADDR;
    /* trigger the interrupt to inform the dest cpu */
    if (INTER_CORE_CPU0_ID == pstInterCoreMsg->enDstCpuID)
    {
        *triggerAddr  |= 0x1;
       // NVIC_ISPR->ISPR2 |= 0x00000060;
       // NVIC_SetPendingIRQ(INTER_CORE_CPU2_CPU0_TRIGGER);
    }
    else if (INTER_CORE_CPU2_ID == pstInterCoreMsg->enDstCpuID)
    {
        *triggerAddr  |= 0x2;
       // NVIC_ISPR->ISPR2 |= 0x00000060;
       // NVIC_SetPendingIRQ(INTER_CORE_CPU0_CPU2_TRIGGER);
    }
}


INTER_CORE_MSG_TYPE* InterCore_GetMsg(INTER_CORE_CPU_ID enInterCoreCpuID)
{
    INTER_CORE_MSG_POOL_TYPE  *pstMsgPool;
    INTER_CORE_MSG_TYPE       *pstMsg;

    pstMsgPool = &g_stInterCoreMsgPool[enInterCoreCpuID];

    /* judge the message pool is not empty */
    if ( (pstMsgPool->nextMsgPos > pstMsgPool->firstMsgPos)
      && ( (pstMsgPool->nextMsgPos - pstMsgPool->firstMsgPos) > 1) )
    {
        pstMsg = &(pstMsgPool->stInterCoreMsg[pstMsgPool->firstMsgPos]);

        pstMsgPool->firstMsgPos++;
    }
    else if ( pstMsgPool->firstMsgPos > pstMsgPool->nextMsgPos )
    {
        if ( (pstMsgPool->nextMsgPos + INTER_CORE_MSG_POOL_MAX) > (pstMsgPool->firstMsgPos + 1) )
        {
            pstMsg = &(pstMsgPool->stInterCoreMsg[pstMsgPool->firstMsgPos]);

            pstMsgPool->firstMsgPos++;

            if (pstMsgPool->firstMsgPos == INTER_CORE_MSG_POOL_MAX)
            {
                pstMsgPool->firstMsgPos = 0;
            }
        }
    }
    else
    {
        dlog_error("message pool is empty\n");
    }

    return pstMsg;
}


void InterCore_IRQ0Handler(void)
{
    dlog_info("handler 0\n");
}


void InterCore_IRQ1Handler(void)
{
    dlog_info("handler 1\n");
}


void InterCore_TriggerIRQ0(void)
{
    *((uint32_t *)(INTER_CORE_TRIGGER_REG_ADDR)) |= INTER_CORE_TRIGGER_IRQ0_BITMAP;
}


void InterCore_TriggerIRQ1(void)
{
    *((uint32_t *)(INTER_CORE_TRIGGER_REG_ADDR)) |= INTER_CORE_TRIGGER_IRQ1_BITMAP;
}


