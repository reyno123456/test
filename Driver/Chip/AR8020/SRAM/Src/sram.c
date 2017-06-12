#include <stdint.h>
#include "sram.h"
#include "debuglog.h"
#include "usbd_def.h"
#include "usbd_hid.h"
#include "reg_rw.h"
#include "systicks.h"

volatile uint32_t               sramReady0;
volatile uint32_t               sramReady1;
extern USBD_HandleTypeDef       USBD_Device[USBD_PORT_NUM];
volatile uint8_t                g_u8DataPathReverse = 0;
uint32_t                        g_TickRecord[2];
STRU_CHANNEL_PORT_CONFIG        g_stChannelPortConfig[SRAM_CHANNEL_NUM];


void SRAM_Ready0IRQHandler(uint32_t u32_vectorNum)
{
    uint8_t                *buff;
    uint32_t                dataLen;
    uint8_t                 u8_usbPortId;
    USBD_HandleTypeDef     *pdev;
    uint8_t                 u8_endPoint;

    u8_usbPortId            = g_stChannelPortConfig[0].u8_usbPort;
    pdev                    = &USBD_Device[u8_usbPortId];

    if (pdev->u8_videoDisplay == 0)
    {
        SRAM_Ready0Confirm();

        return;
    }

    buff                    = (uint8_t *)SRAM_BUFF_0_ADDRESS;

    dataLen                 = SRAM_DATA_VALID_LEN_0;
    dataLen                 = (dataLen << 2);

    u8_endPoint             = g_stChannelPortConfig[0].u8_usbEp;

    if (USBD_OK != USBD_HID_SendReport(pdev, buff, dataLen, u8_endPoint))
    {
        dlog_error("HID0 Send Error!\n");

        SRAM_Ready0Confirm();
    }
    else
    {
        if (pdev->u8_connState != 2)
        {
            SRAM_Ready0Confirm();
        }
        else
        {
            g_TickRecord[0] = SysTicks_GetTickCount();
            sramReady0 = 1;
        }
    }
}


void SRAM_Ready1IRQHandler(uint32_t u32_vectorNum)
{
    uint8_t                *buff;
    uint32_t                dataLen;
    uint8_t                 u8_usbPortId;
    USBD_HandleTypeDef     *pdev;
    uint8_t                 u8_endPoint;

    u8_usbPortId            = g_stChannelPortConfig[1].u8_usbPort;
    pdev                    = &USBD_Device[u8_usbPortId];

    if (pdev->u8_videoDisplay == 0)
    {
        SRAM_Ready1Confirm();

        return;
    }

    buff                    = (uint8_t *)SRAM_BUFF_1_ADDRESS;

    dataLen                 = SRAM_DATA_VALID_LEN_1;
    dataLen                 = (dataLen << 2);

    u8_endPoint             = g_stChannelPortConfig[1].u8_usbEp;

    if (USBD_OK != USBD_HID_SendReport(pdev, buff, dataLen, u8_endPoint))
    {
        dlog_error("HID1 Send Error!\n");

        SRAM_Ready1Confirm();
    }
    else
    {
        if (pdev->u8_connState != 2)
        {
            SRAM_Ready1Confirm();
        }
        else
        {
            g_TickRecord[1] = SysTicks_GetTickCount();
            sramReady1 = 1;
        }
    }
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


void SRAM_GROUND_ReceiveVideoConfig(void)
{
    //uint8_t      temp;

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


void SRAM_SKY_EnableBypassVideoConfig(uint32_t channel)
{
    if (0 == channel)
    {
        Reg_Write32(SRAM_VIEW0_ENABLE_ADDR, SRAM_VIEW0_ENABLE);
    }
    else
    {
        Reg_Write32(SRAM_VIEW1_ENABLE_ADDR, SRAM_VIEW1_ENABLE);
    }

    Reg_Write32(SRAM_SKY_MASTER_ID_ADDR, SRAM_SKY_MASTER_ID_VALUE);

    Reg_Write32(SRAM_SKY_MASTER_ID_MASK_ADDR, SRAM_SKY_MASTER_ID_MASK_VALUE);
}


void SRAM_SKY_DisableBypassVideoConfig(uint32_t channel)
{
    uint32_t        regValue;

    if (0 == channel)
    {
        regValue    = Reg_Read32(SRAM_VIEW0_ENABLE_ADDR);
        regValue   &= ~SRAM_VIEW0_ENABLE;

        Reg_Write32(SRAM_VIEW0_ENABLE_ADDR, regValue);
    }
    else
    {
        regValue    = Reg_Read32(SRAM_VIEW1_ENABLE_ADDR);
        regValue   &= ~SRAM_VIEW1_ENABLE;

        Reg_Write32(SRAM_VIEW1_ENABLE_ADDR, regValue);
    }
}


void SRAM_CheckTimeout(void)
{
    if (sramReady0)
    {
        if ((SysTicks_GetDiff(g_TickRecord[0], SysTicks_GetTickCount())) >= SRAM_TIMEOUT_THRESHOLD)
        {
            dlog_error("channel 0 timeout");

            SRAM_Ready0Confirm();
        }
    }

    if (sramReady1)
    {
        if ((SysTicks_GetDiff(g_TickRecord[1], SysTicks_GetTickCount())) >= SRAM_TIMEOUT_THRESHOLD)
        {
            dlog_error("channel 1 timeout");

            SRAM_Ready1Confirm();
        }
    }
}


