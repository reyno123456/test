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
#include "rf_8003s.h"


#define  RF8003S_RF_CLOCKRATE    (1)    //2MHz clockrate

static int RF8003s_SPI_WriteReg_internal(uint8_t u8_addr, uint8_t u8_data, uint8_t u8_flag)
{
    int ret = 0;
    uint8_t wdata[] = {0x80, (u8_addr <<1), u8_data};   //RF_8003S_SPI: wr: 0x80 ; 
    
    //use low speed for the RF8003 read, from test, read fail if use the same clockrate as baseband
    /*
    STRU_SPI_InitTypes init = {
        .ctrl0   = 0x47,
        .clk_Mhz = RF8003S_RF_CLOCKRATE,    
        .Tx_Fthr = 0x03,
        .Rx_Ftlr = 0x6,
        .SER     = 0x01
    };
    */

    if(u8_flag)
    {
        BB_SPI_curPageWriteByte(0x01,0x01);     //SPI change into 8003
    }

    //SPI_master_init(BB_SPI_BASE_IDX, &init);
    SPI_write_read(BB_SPI_BASE_IDX, wdata, sizeof(wdata), 0, 0); 
    ret =  SPI_WaitIdle(BB_SPI_BASE_IDX, BB_SPI_MAX_DELAY);

    if(u8_flag)
    {
        BB_SPI_curPageWriteByte(0x01,0x02);     //SPI change into 8003
    }

    return ret;
}


static int RF8003s_SPI_ReadReg_internal(uint8_t u8_addr, uint8_t u8_flag)
{
    uint8_t wdata[2] = {0x00, (u8_addr<<1)};      //RF_8003S_SPI:  rd: 0x00
    uint8_t rdata;
    
    //use low speed for the RF8003 read, from test, read fail if use the same clockrate as baseband
    STRU_SPI_InitTypes init = {
        .ctrl0   = SPI_CTRL0_DEF_VALUE,
        .clk_Mhz = RF8003S_RF_CLOCKRATE,
        .Tx_Fthr = SPI_TXFTLR_DEF_VALUE,
        .Rx_Ftlr = SPI_RXFTLR_DEF_VALUE,
        .SER     = SPI_SSIENR_DEF_VALUE
    };
    if(u8_flag)
    {
        BB_SPI_curPageWriteByte(0x01,0x01);     //SPI change into 8003
    }

    SPI_master_init(BB_SPI_BASE_IDX, &init);
 
    SPI_write_read(BB_SPI_BASE_IDX, wdata, sizeof(wdata), &rdata, 1);
    SPI_WaitIdle(BB_SPI_BASE_IDX, BB_SPI_MAX_DELAY);

    BB_SPI_init();
    SPI_master_init(BB_SPI_BASE_IDX, &init);

    if(u8_flag)
    {
        BB_SPI_curPageWriteByte(0x01,0x02);     //SPI change into 8003
    }
    
    return rdata;
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
void RF8003s_init(uint8_t *pu8_regs1, uint8_t *pu8_regs2, STRU_BoardCfg *boardCfg)
{
    uint8_t idx;
    uint8_t cnt;

    BB_SPI_curPageWriteByte(0x01,0x01);             //bypass: SPI change into 8003

    if ( boardCfg != NULL )
    {
        for (cnt = 0; cnt < boardCfg->u8_rf1RegsCnt; cnt++)
        {
            STRU_RF_REG rfReg1 = boardCfg->pstru_rf1Regs[cnt];
            pu8_regs1[rfReg1.addr] = rfReg1.value;
        }    
    }

    for(idx = 0; idx < 128; idx++)
    {
        RF8003s_SPI_WriteReg_internal( idx, pu8_regs1[idx], 0);
    }

    {
        //add patch, reset 8003
        RF8003s_SPI_WriteReg_internal(0x15, 0x51, 0);
        RF8003s_SPI_WriteReg_internal(0x15, 0x50, 0);
    }

    BB_SPI_curPageWriteByte(0x01,0x02);             //SPI change into 8020
    
    if (boardCfg != NULL && boardCfg->u8_rf8003Cnt > 1)
    {
        BB_SPI_curPageWriteByte(0x01,0x03);             //bypass: SPI change into 2rd 8003s
        
        for (cnt = 0; cnt < boardCfg->u8_rf2RegsCnt; cnt++)
        {
            STRU_RF_REG rfReg2 = boardCfg->pstru_rf2Regs[cnt];
            pu8_regs2[rfReg2.addr] = rfReg2.value;
        }
        for(idx = 0; idx < 128; idx++)
        {
            RF8003s_SPI_WriteReg_internal( idx, pu8_regs2[idx], 0);
        }

        {
            //add patch, reset 8003
            RF8003s_SPI_WriteReg_internal(0x15, 0x51, 0);
            RF8003s_SPI_WriteReg_internal(0x15, 0x50, 0);
        } 
        
        BB_SPI_curPageWriteByte(0x01,0x02);             //SPI change into 8020
    }
}

/**
  * @brief :  power consumption for RF8003s
  * @param : 
  * @retval  
  */
void RF8003s_Set(ENUM_BB_MODE en_mode)
{
    uint32_t u32_delay = 1000;

    BB_SPI_curPageWriteByte(0x01,0x01);             //bypass: SPI change into 8003

    RF8003s_SPI_WriteReg_internal(0x35, 0x70, 0);
    RF8003s_SPI_WriteReg_internal(0x45, 0x87, 0);
    RF8003s_SPI_WriteReg_internal(0x15, 0x51, 0);
    while(u32_delay--);
    RF8003s_SPI_WriteReg_internal(0x15, 0x50, 0);

    if(en_mode == BB_GRD_MODE)
    {
        RF8003s_SPI_WriteReg_internal(0x00, 0x74, 0);
        RF8003s_SPI_WriteReg_internal(0x2D, 0xF6, 0);
        RF8003s_SPI_WriteReg_internal(0x37, 0xE0, 0);
    }

    BB_SPI_curPageWriteByte(0x01,0x02);             //SPI change into 8020
}
