#include "inter_core.h"
#include "debuglog.h"
#include "interrupt.h"
#include "string.h"
#include "reg_map.h"
#include "sys_event.h"
#include "lock.h"
#include "reg_map.h"

static void InterCore_IRQ0Handler(void);
static void InterCore_IRQ1Handler(void);

static void InterCore_ResetIRQ0(void)
{
    *((volatile uint32_t *)(INTER_CORE_TRIGGER_REG_ADDR)) &= ~INTER_CORE_TRIGGER_IRQ0_BITMAP;
}

static void InterCore_ResetIRQ1(void)
{
    *((volatile uint32_t *)(INTER_CORE_TRIGGER_REG_ADDR)) &= ~INTER_CORE_TRIGGER_IRQ1_BITMAP;
}

static void InterCore_IRQ0Handler(void)
{
#ifdef INTER_CORE_DEBUG_LOG_ENABLE
    dlog_info("handler 0\n");
#endif

    InterCore_ResetIRQ0();

    INTER_CORE_MSG_ID msg = 0; 
    uint8_t buf[INTER_CORE_MSG_SHARE_MEMORY_DATA_LENGTH];
    uint32_t max_length = INTER_CORE_MSG_SHARE_MEMORY_DATA_LENGTH;

    // Get all the messages in the SRAM buffer
    uint8_t mem_cnt = INTER_CORE_MSG_SHARE_MEMORY_NUMBER; // Max count to avoid the endless loop risk
    while(mem_cnt--)
    {
        msg = 0;
        memset(buf, 0, sizeof(buf));
        InterCore_GetMsg(&msg, buf, sizeof(buf));

        // Message process
        if (msg != 0)
        {
            // Check whether this message is an inter-core system event
            if (msg & SYS_EVENT_INTER_CORE_MASK)
            {
                // Remove the inter-core mask to avoid the loop notification
                uint32_t event = msg & ~SYS_EVENT_INTER_CORE_MASK;

                // Notify the message as a system event to the local CPU
                SYS_EVENT_Notify_From_ISR(event, (void*)buf);
#ifdef INTER_CORE_DEBUG_LOG_ENABLE
                dlog_info("Notify event 0x%x by inter core msg", event);
#endif
            }
        }
        else
        {
            break;
        }
    }
}

static void InterCore_IRQ1Handler(void)
{
    InterCore_ResetIRQ1();
#ifdef INTER_CORE_DEBUG_LOG_ENABLE
    dlog_info("handler 1\n");
#endif
}

static void InterCore_TriggerIRQ0(void)
{
    *((volatile uint32_t *)(INTER_CORE_TRIGGER_REG_ADDR)) |= INTER_CORE_TRIGGER_IRQ0_BITMAP;
}

static void InterCore_TriggerIRQ1(void)
{
    *((volatile uint32_t *)(INTER_CORE_TRIGGER_REG_ADDR)) |= INTER_CORE_TRIGGER_IRQ1_BITMAP;
}

static void InterCore_SRAMDCacheDisable(uint8_t type)
{
    if (type == 0)
    {
        MPU->RNR  = SRAM_MEMORY_MPU_REGION_NUMBER;
        MPU->RBAR = SRAM_MEMORY_MPU_REGION_ST_ADDR_0 | (1 << 4) | (SRAM_MEMORY_MPU_REGION_NUMBER << 0);
        MPU->RASR = SRAM_MEMORY_MPU_REGION_ATTR_0;
        MPU->CTRL = MPU_CTRL_PRIVDEFENA_Msk | MPU_CTRL_HFNMIENA_Msk | MPU_CTRL_ENABLE_Msk;
        __asm volatile ("dsb");
        __asm volatile ("isb");
    }
}

void InterCore_Init(void)
{
    // Init the SRAM data share buffer
    InterCore_SRAMDCacheDisable(0);
    volatile INTER_CORE_MSG_TYPE* msgPtr = (INTER_CORE_MSG_TYPE*)INTER_CORE_MSG_SHARE_MEMORY_BASE_ADDR;
    memset(msgPtr, 0, sizeof(INTER_CORE_MSG_TYPE)*INTER_CORE_MSG_SHARE_MEMORY_NUMBER);

    // Interrupt enable
    reg_IrqHandle(VIDEO_GLOBAL2_INTR_RES_VSOC0_VECTOR_NUM, InterCore_IRQ0Handler);
    INTR_NVIC_EnableIRQ(VIDEO_GLOBAL2_INTR_RES_VSOC0_VECTOR_NUM);
    /*
    reg_IrqHandle(VIDEO_GLOBAL2_INTR_RES_VSOC1_VECTOR_NUM, InterCore_IRQ1Handler);
    INTR_NVIC_EnableIRQ(VIDEO_GLOBAL2_INTR_RES_VSOC1_VECTOR_NUM);
    */
}

uint8_t InterCore_SendMsg(INTER_CORE_CPU_ID dst, INTER_CORE_MSG_ID msg, uint8_t* buf, uint32_t length)
{
    uint8_t i = 0;
    volatile INTER_CORE_MSG_TYPE* msgPtr = (INTER_CORE_MSG_TYPE*)INTER_CORE_MSG_SHARE_MEMORY_BASE_ADDR;

    // Parse the SRAM data buffer to find the section can be used
    for(i = 0 ; i < INTER_CORE_MSG_SHARE_MEMORY_NUMBER; i++)
    {
        if(msgPtr[i].dataAccessed == msgPtr[i].enDstCpuID)
        {
            break;
        }
    }

    // Check whether the right section is found
    if (i == INTER_CORE_MSG_SHARE_MEMORY_NUMBER)
    {
        dlog_error("SRAM inter CPU share memory buffer is full!");
        return 0;
    }

    // Set the other parameters 
#if defined CPU0_DRV
    msgPtr[i].enSrcCpuID = INTER_CORE_CPU0_ID;
#elif defined CPU1_DRV
    msgPtr[i].enSrcCpuID = INTER_CORE_CPU1_ID;
#elif defined CPU2_DRV
    msgPtr[i].enSrcCpuID = INTER_CORE_CPU2_ID;
#else
    dlog_error("Error CPU number!");
    return 0;
#endif
    msgPtr[i].enDstCpuID = (dst & (~(msgPtr[i].enSrcCpuID)));    // Unmask the local CPU ID
    msgPtr[i].dataAccessed = 0;
    msgPtr[i].enMsgID = msg;
    memcpy(msgPtr[i].data, buf, (length <= sizeof(msgPtr[i].data)) ? length : sizeof(msgPtr[i].data));

    // Trigger the interrupt
    InterCore_TriggerIRQ0();
    
    return 1;
}

uint8_t InterCore_GetMsg(INTER_CORE_MSG_ID* msg_p, uint8_t* buf, uint32_t max_length)
{
    uint8_t i = 0;
    INTER_CORE_CPU_ID dst_filter;
    volatile INTER_CORE_MSG_TYPE* msgPtr = (INTER_CORE_MSG_TYPE*)INTER_CORE_MSG_SHARE_MEMORY_BASE_ADDR;

    // Check the input pointers
    if ((msg_p == NULL) || (buf == NULL))
    {
        dlog_error("Null Pointer!");
        return 0;
    }

    // Filter to filter out other CPU's data
#if defined CPU0_DRV
    dst_filter = INTER_CORE_CPU0_ID;
#elif defined CPU1_DRV
    dst_filter = INTER_CORE_CPU1_ID;
#elif defined CPU2_DRV
    dst_filter = INTER_CORE_CPU2_ID;
#else
    dlog_error("Error CPU number!");
    return 0;
#endif

    // Check the data buffer to find the current CPU's data
    for(i = 0 ; i < INTER_CORE_MSG_SHARE_MEMORY_NUMBER; i++)
    {
#ifdef INTER_CORE_DEBUG_LOG_ENABLE
        dlog_info("msgPtr[%d].dataAccessed 0x%x, msgPtr[%d].enDstCpuID 0x%x", i, msgPtr[i].dataAccessed, i, msgPtr[i].enDstCpuID);
#endif
        if(((msgPtr[i].dataAccessed & dst_filter) == 0) && ((msgPtr[i].enDstCpuID & dst_filter) != 0))
        {
            // Set the data accessed flag of the current CPU
            // Add lock to avoid multi CPU conflict
            Lock(&msgPtr[i].lock);
            msgPtr[i].dataAccessed |= dst_filter;
            __asm volatile ("dsb"); // Sync to SRAM and make sure other CPUs can access the latest data in SRAM
            UnLock(&msgPtr[i].lock);
            break;
        }
    }

    // Check whether some message is found
    if (i == INTER_CORE_MSG_SHARE_MEMORY_NUMBER)
    {
#ifdef INTER_CORE_DEBUG_LOG_ENABLE
        dlog_info("No specified inter CPU message!");
#endif
        return 0;
    }
    else
    {
        dlog_info("Matched inter CPU message 0x%x!", msgPtr[i].enMsgID);
    }

    // Retrieve the message data
    *msg_p = msgPtr[i].enMsgID;
    memcpy(buf, msgPtr[i].data, (max_length <= sizeof(msgPtr[i].data)) ?  max_length : sizeof(msgPtr[i].data));

    return 1;
}

