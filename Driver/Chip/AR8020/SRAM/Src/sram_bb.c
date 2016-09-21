#include "sram_bb.h"
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
    BB_SPI_WriteByte(PAGE2, 0x57, 0x20);
    BB_SPI_WriteByte(PAGE2, 0x58, 0x00);

    /* Set the start address of sram */
    Reg_Write32(SRAM_WR_ADDR_OFFSET_0, SRAM_BASE_ADDRESS);

    /* Set the max num of SRAM_READY interrupt trigger signal */
    Reg_Write32(SRAM_WR_MAX_LEN_0, SRAM_DMA_READY_LEN);
}



