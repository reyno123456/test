#include "sram.h"
#include "debuglog.h"




void SRAM_Ready0IRQHandler(void)
{
    /* DATA is in SRAM, here to trigger corresponding task to process  */
    dlog_info("receive DATA in SRAM 0\n");

    SRAM_Ready0Confirm();
}


void SRAM_Ready1IRQHandler(void)
{
    /* DATA is in SRAM, here to trigger corresponding task to process  */
    dlog_info("receive DATA in SRAM 1\n");

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



