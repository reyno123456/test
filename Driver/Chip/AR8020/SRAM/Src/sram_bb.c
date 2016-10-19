#include "sram_bb.h"
#include "sram_common.h"
#include "BB_ctrl.h"


void SRAM_BB_BypassVideoConfig(void)
{
    uint8_t      temp;

    /* Base Band bypass data, directly write to SRAM */
    temp    = BB_SPI_ReadByte(PAGE1, 0x8d);
    temp   |= 0x40;
    BB_SPI_WriteByte(PAGE1, 0x8d, temp);

    BB_SPI_WriteByte(PAGE2, 0x56, 0x06);

    /* Threshold of usb_fifo in BaseBand */
//    BB_SPI_WriteByte(PAGE2, 0x57, 0x20);
//    BB_SPI_WriteByte(PAGE2, 0x58, 0x00);

    /* Set the start address of sram for bb bypass channel 0*/
    Reg_Write32(SRAM_WR_ADDR_OFFSET_0, SRAM_BB_BYPASS_OFFSET_0);

    /* Set the max num of SRAM_READY interrupt trigger signal */
    Reg_Write32(SRAM_WR_MAX_LEN_0, SRAM_DMA_READY_LEN);

    /* Set the start address of sram for bb bypass channel 1*/
    Reg_Write32(SRAM_WR_ADDR_OFFSET_1, SRAM_BB_BYPASS_OFFSET_1);

    /* Set the max num of SRAM_READY interrupt trigger signal */
    Reg_Write32(SRAM_WR_MAX_LEN_1, SRAM_DMA_READY_LEN);

}



