/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_i2c.c
Description: The internal APIs to control the RF 8003s
Author: Artosy Software Team
Version: 0.0.1
Date: 2016/12/20
History: 
        0.0.1    2016/12/20    The initial version of rf8003s.c
*****************************************************************************/
#include "debuglog.h"
#include "spi.h"
#include "bb_spi.h"
#include "rf8003s.h"

#define  RF8003S_RF_CLOCKRATE    (5)    //5MHz clockrate

static int RF8003s_SPI_WriteReg_internal(uint8_t u8_addr, uint8_t u8_data, uint8_t u8_flag)
{
    int ret = 0;
    uint8_t wdata[] = {0x80, u8_addr, u8_data};   //RF_8003S_SPI: wr: 0x80 ; 
    
    //use low speed for the RF8003 read, from test, read fail if use the same clockrate as baseband
    STRU_SPI_InitTypes init = {
        .ctrl0   = 0x47,
        .clk_Mhz = RF8003S_RF_CLOCKRATE,    
        .Tx_Fthr = 0x03,
        .Rx_Ftlr = 0x6,
        .SER     = 0x01
    };

    if(u8_flag)
    {
        BB_SPI_curPageWriteByte(0x01,0x01);     //SPI change into 8003
    }

    SPI_master_init(BB_SPI_BASE_IDX, &init);
    ret = SPI_write_read(BB_SPI_BASE_IDX, wdata, sizeof(wdata), 0, 0); 

    if(u8_flag)
    {
        BB_SPI_curPageWriteByte(0x01,0x02);     //SPI change into 8003
    }

    return ret;
}


static int RF8003s_SPI_ReadReg_internal(uint8_t u8_addr, uint8_t u8_flag)
{
    uint8_t wdata[3] = {0x00, u8_addr, u8_addr};      //RF_8003S_SPI:  rd: 0x00
    uint8_t rdata[3] = {0};
    
    //use low speed for the RF8003 read, from test, read fail if use the same clockrate as baseband
    STRU_SPI_InitTypes init = {
        .ctrl0   = 0x47,
        .clk_Mhz = RF8003S_RF_CLOCKRATE,
        .Tx_Fthr = 0x03,
        .Rx_Ftlr = 0x6,
        .SER     = 0x01
    };
    if(u8_flag)
    {
        BB_SPI_curPageWriteByte(0x01,0x01);     //SPI change into 8003
    }

    SPI_master_init(BB_SPI_BASE_IDX, &init);

    SPI_write_read(BB_SPI_BASE_IDX, wdata, 3 /*sizeof(wdata)*/, rdata, 3); 

    BB_SPI_init();
    SPI_master_init(BB_SPI_BASE_IDX, &init);

    if(u8_flag)
    {
        BB_SPI_curPageWriteByte(0x01,0x02);     //SPI change into 8003
    }
    dlog_info("read out %x %x %x \r\n", rdata[0], rdata[1], rdata[2]);
    return rdata[2];
}

/**
  * @brief : Write 8003 RF register by SPI 
  * @param : addr: 8003 SPI address
  * @param : data: data for 8003
  * @retval  0: sucess   1: FAIL
  */
int RF8003s_SPI_WriteReg(uint8_t u8_addr, uint8_t u8_data)
{
    return RF8003s_SPI_WriteReg_internal(u8_addr, u8_data, 1);
}

/**
  * @brief : Read 8003 RF register by SPI 
  * @param : addr: 8003 SPI address
  * @retval  0: sucess   1: FAIL
  */
int RF8003s_SPI_ReadReg(uint8_t u8_addr, uint8_t *pu8_rxValue)
{
    *pu8_rxValue = RF8003s_SPI_ReadReg_internal(u8_addr, 1);
    return 0;
}

/**
  * @brief : init RF8003s register
  * @param : addr: 8003 SPI address
  * @retval  None
  */
void RF8003s_init(uint8_t *pu8_regs)
{
    uint8_t idx;

    BB_SPI_curPageWriteByte(0x01,0x01);             //bypass: SPI change into 8003

    for(idx = 0; idx < 128; idx++)
    {
        RF8003s_SPI_WriteReg_internal( (idx << 1), pu8_regs[idx], 0);

        #if (RF_SPI_TEST ==1)
        uint8_t data = SPI_Read8003(idx*2, 0);
        dlog_info("%d %d \n", idx, data);
        #endif
    }

    {
        //add patch, reset 8003
        RF8003s_SPI_WriteReg_internal(0x15 *2, 0x51, 0);
        RF8003s_SPI_WriteReg_internal(0x15 *2, 0x50, 0);
    }

    BB_SPI_curPageWriteByte(0x01,0x02);             //SPI change into 8020
}
