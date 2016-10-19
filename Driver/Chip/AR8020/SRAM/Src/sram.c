#include <stdint.h>
#include "sram.h"
#include "sram_common.h"
#include "debuglog.h"



extern volatile uint32_t  sramReady0;
extern volatile uint32_t  sramReady1;

void SRAM_Ready0IRQHandler(void)
{
    uint32_t     dataLen;

    dataLen    = *((uint32_t *)SRAM_DATA_VALID_LEN_0);

    /* DATA is in SRAM, here to trigger corresponding task to process  */
    dlog_info("receive DATA in SRAM 0: 0x%08x\n", dataLen);

   // dlog_info("dataLen0: 0x%08x\n", dataLen);

   // SRAM_Ready0Confirm();
   //
   sramReady0 = 1;
}

void SRAM_Ready1IRQHandler(void)
{
    uint32_t     dataLen;
    uint32_t     index;
    uint8_t      *buff;
    uint32_t     packetCount;
    uint32_t     lastPacketLen;

    dataLen    = *((uint32_t *)SRAM_DATA_VALID_LEN_1);

    /* DATA is in SRAM, here to trigger corresponding task to process  */
    dlog_info("receive DATA in SRAM 1: 0x%08x\n", dataLen);
#if 0
    packetCount    = dataLen + 127;
    packetCount    >>= 7;

    lastPacketLen  = dataLen - ((packetCount - 1)  << 7);

    buff           = SRAM_BASE_ADDRESS + (SRAM_BB_BYPASS_OFFSET_1 << 2);

    for (index = 0; index < (packetCount - 1); index++)
    {
        USBD_HID_SendReport(&USBD_Device, (uint8_t *)(buff + ( index << 9 )), 0x200);
    }
    
    USBD_HID_SendReport(&USBD_Device, (uint8_t *)(buff + ( index << 9 )), (lastPacketLen << 2));
#endif
//    volatile uint32_t  *regCount = 0x40b00048;
//    *regCount = 1;
   sramReady1 = 1;



#if 0
    dlog_info("0x21000000: 0x%08x\n", *((uint32_t *)0x21000000));
    dlog_info("0x21000004: 0x%08x\n", *((uint32_t *)0x21000004));
    dlog_info("0x21000008: 0x%08x\n", *((uint32_t *)0x21000008));
    dlog_info("0x2100000C: 0x%08x\n", *((uint32_t *)0x2100000C));
    dlog_info("0x21000010: 0x%08x\n", *((uint32_t *)0x21000010));
    dlog_info("0x21000014: 0x%08x\n", *((uint32_t *)0x21000014));
    dlog_info("0x21000018: 0x%08x\n", *((uint32_t *)0x21000018));
    dlog_info("0x2100001C: 0x%08x\n", *((uint32_t *)0x2100001C));
#endif
//    SRAM_Ready1Confirm();
}


void SRAM_Ready0Confirm(void)
{
    /* confirm to Baseband that the SRAM data has been processed, ready to receive new data */
    Reg_Write32(DMA_READY_0, 1);
}


void SRAM_Ready1Confirm(void)
{
    /* confirm to Baseband that the SRAM data has been processed, ready to receive new data */
    Reg_Write32(DMA_READY_1, 1);
}



