#include <stdint.h>
#include "sram.h"
#include "debuglog.h"
#include "usbd_def.h"


volatile uint32_t               sramReady0;
volatile uint32_t               sramReady1;
extern USBD_HandleTypeDef       USBD_Device;


void SRAM_Ready0IRQHandler(void)
{
    uint8_t         *buff;
    uint32_t         dataLen;

    buff            = SRAM_BUFF_0_ADDRESS;

    dataLen         = SRAM_DATA_VALID_LEN_0;
    dataLen         = (dataLen << 2);

    if (USBD_OK != USBD_HID_SendReport(&USBD_Device, buff, dataLen))
    {
        dlog_error("HID0 Send Error!\n");

        SRAM_Ready0Confirm();
    }

    sramReady0 = 1;
}


void SRAM_Ready1IRQHandler(void)
{
    uint8_t         *buff;
    uint32_t         dataLen;

    buff            = SRAM_BUFF_1_ADDRESS;

    dataLen         = SRAM_DATA_VALID_LEN_1;
    dataLen         = (dataLen << 2);

    if (USBD_OK != USBD_HID_SendReport(&USBD_Device, buff, dataLen))
    {
        dlog_error("HID1 Send Error!\n");

        SRAM_Ready1Confirm();
    }

    sramReady1 = 1;
}


void SRAM_Ready0Confirm(void)
{
    /* confirm to Baseband that the SRAM data has been processed, ready to receive new data */
    Reg_Write32(DMA_READY_0, 1);

    sramReady0 = 0;
}


void SRAM_Ready1Confirm(void)
{
    /* confirm to Baseband that the SRAM data has been processed, ready to receive new data */
    Reg_Write32(DMA_READY_1, 1);

    sramReady1 = 0;
}


void SRAM_GROUND_BypassVideoConfig(void)
{
    uint8_t      temp;

    /* Base Band bypass data, directly write to SRAM */
    #if 0
    temp    = BB_SPI_ReadByte(PAGE1, 0x8d);
    temp   |= 0x40;
    BB_SPI_WriteByte(PAGE1, 0x8d, temp);

    BB_SPIWriteByte(PAGE2, 0x56, 0x06);
    #endif
    /* Threshold of usb_fifo in BaseBand */

    /* Set the start address of sram for bb bypass channel 0*/
    Reg_Write32(SRAM_WR_ADDR_OFFSET_0, SRAM_BB_BYPASS_OFFSET_0);

    /* Set the max num of SRAM_READY interrupt trigger signal */
    Reg_Write32(SRAM_WR_MAX_LEN_0, SRAM_DMA_READY_LEN);

    /* Set the start address of sram for bb bypass channel 1*/
    Reg_Write32(SRAM_WR_ADDR_OFFSET_1, SRAM_BB_BYPASS_OFFSET_1);

    /* Set the max num of SRAM_READY interrupt trigger signal */
    Reg_Write32(SRAM_WR_MAX_LEN_1, SRAM_DMA_READY_LEN);

}


void SRAM_SKY_BypassVideoConfig(uint32_t channel)
{
    if (0 == channel)
    {
        Reg_Write32(SRAM_VIEW0_ENABLE_ADDR, 1);
    }
    else
    {
        Reg_Write32(SRAM_VIEW1_ENABLE_ADDR, 4);
    }

    Reg_Write32(SRAM_SKY_MASTER_ID_ADDR, SRAM_SKY_MASTER_ID_VALUE);

    Reg_Write32(SRAM_SKY_MASTER_ID_MASK_ADDR, SRAM_SKY_MASTER_ID_MASK_VALUE);
}


