#include "sram.h"
#include "debuglog.h"




void SRAM_Ready0IRQHandler(void)
{
    uint32_t     dataLen;

    dataLen    = *((uint32_t *)SRAM_DATA_VALID_LEN_0);

    /* DATA is in SRAM, here to trigger corresponding task to process  */
    dlog_info("receive DATA in SRAM 0: 0x%08x\n", dataLen);

    SRAM_Ready0Confirm();
}


void SRAM_Ready1IRQHandler(void)
{
    uint32_t     dataLen;

    dataLen    = *((uint32_t *)SRAM_DATA_VALID_LEN_1);

    /* DATA is in SRAM, here to trigger corresponding task to process  */
    dlog_info("receive DATA in SRAM 1: 0x%08x\n", dataLen);

    SRAM_Ready1Confirm();
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



