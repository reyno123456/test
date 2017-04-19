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


#define  RF8003S_RF_CLOCKRATE    (1)    //1MHz clockrate

static int RF8003s_SPI_WriteReg_internal(uint8_t u8_addr, uint8_t u8_data)
{
    int ret = 0;
    uint8_t wdata[] = {0x80, (u8_addr <<1), u8_data};   //RF_8003S_SPI: wr: 0x80 ; 

    //SPI_master_init(BB_SPI_BASE_IDX, &init);
    SPI_write_read(BB_SPI_BASE_IDX, wdata, sizeof(wdata), 0, 0); 
    ret =  SPI_WaitIdle(BB_SPI_BASE_IDX, BB_SPI_MAX_DELAY);

    return ret;
}


static int RF8003s_SPI_ReadReg_internal(uint8_t u8_addr)
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

    SPI_master_init(BB_SPI_BASE_IDX, &init);
 
    SPI_write_read(BB_SPI_BASE_IDX, wdata, sizeof(wdata), &rdata, 1);
    SPI_WaitIdle(BB_SPI_BASE_IDX, BB_SPI_MAX_DELAY);

    BB_SPI_init();
    SPI_master_init(BB_SPI_BASE_IDX, &init);

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
    return RF8003s_SPI_WriteReg_internal(u8_addr, u8_data);
}

/**
  * @brief : Read 8003 RF register by SPI 
  * @param : addr: 8003 SPI address
  * @retval  0: sucess   1: FAIL
  */
int RF8003s_SPI_ReadReg(uint8_t u8_addr, uint8_t *pu8_rxValue)
{
    *pu8_rxValue = RF8003s_SPI_ReadReg_internal(u8_addr);
    return 0;
}

/**
  * @brief : init RF8003s register
  * @param : addr: 8003 SPI address
  * @retval  None
  */
void RF8003s_init(uint8_t *pu8_regs1, uint8_t *pu8_regs2, STRU_BoardCfg *boardCfg, ENUM_BB_MODE en_mode)
{
    uint8_t idx;
    uint8_t cnt;
    uint8_t num;
    STRU_RF_REG * pstru_rfReg = NULL;

    //RF 1
    {
        BB_SPI_curPageWriteByte(0x01,0x01);             //bypass: SPI change into 8003
        
        if ( boardCfg != NULL )
        {        
            if ( en_mode == BB_SKY_MODE )               //sky mode register replace
            {
                num = boardCfg->u8_rf1SkyRegsCnt;
                pstru_rfReg = (STRU_RF_REG * )boardCfg->pstru_rf1SkyRegs;
            }
            else                                        //ground mode register replace
            {
                num = boardCfg->u8_rf1GrdRegsCnt;
                pstru_rfReg = (STRU_RF_REG * )boardCfg->pstru_rf1GrdRegs;
            }
        
            for (cnt = 0; (pstru_rfReg != NULL) && (cnt < num); cnt++)
            {
                pu8_regs1[pstru_rfReg[cnt].addr] = pstru_rfReg[cnt].value;
            } 
        }
        
        for(idx = 0; idx < 128; idx++)
        {
            RF8003s_SPI_WriteReg_internal( idx, pu8_regs1[idx]);
        }
        
        {
            //add patch, reset 8003
            RF8003s_SPI_WriteReg_internal(0x15, 0x51);
            RF8003s_SPI_WriteReg_internal(0x15, 0x50);
        }
        
        BB_SPI_curPageWriteByte(0x01,0x02);             //SPI change into 8020
    }

    //RF 2 only used in ground   
    if (boardCfg != NULL && boardCfg->u8_rf8003Cnt > 1 && en_mode == BB_GRD_MODE )
    {
        num = boardCfg->u8_rf2GrdRegsCnt;
        pstru_rfReg = (STRU_RF_REG * )boardCfg->pstru_rf2GrdRegs;
        
        BB_SPI_curPageWriteByte(0x01,0x03);             //bypass: SPI change into 2rd 8003s
        
        for (cnt = 0; (pstru_rfReg != NULL) && (cnt < num); cnt++)
        {
            pu8_regs2[pstru_rfReg[cnt].addr] = pstru_rfReg[cnt].value;
        }
        for(idx = 0; idx < 128; idx++)
        {
            RF8003s_SPI_WriteReg_internal( idx, pu8_regs2[idx]);
        }

        {
            //add patch, reset 8003
            RF8003s_SPI_WriteReg_internal(0x15, 0x51);
            RF8003s_SPI_WriteReg_internal(0x15, 0x50);
        } 
        
        BB_SPI_curPageWriteByte(0x01,0x02);             //SPI change into 8020
    }
}

void RF8003s_afterCali(ENUM_BB_MODE en_mode, STRU_BoardCfg *boardCfg)
{
    STRU_RF_REG * rf1_regs, * rf2_regs;
    uint8_t cnt;
    uint8_t rf_regcnt1, rf_regcnt2;

    if( NULL == boardCfg)
    {
        return;
    }

    if (en_mode == BB_SKY_MODE)
    {
        rf_regcnt1 = boardCfg->u8_rf1SkyRegsCntAfterCali;
        rf_regcnt2 = 0;

        rf1_regs   = (STRU_RF_REG * )boardCfg->pstru_rf1SkyRegsAfterCali;
        rf2_regs   = NULL;
    }
    else
    {
        rf_regcnt1 = boardCfg->u8_rf1GrdRegsCntAfterCali;
        rf_regcnt2 = boardCfg->u8_rf2GrdRegsCntAfterCali;

        rf1_regs   = (STRU_RF_REG * )boardCfg->pstru_rf1GrdRegsAfterCali;
        rf2_regs   = (STRU_RF_REG * )boardCfg->pstru_rf2GrdRegsAfterCali;
    }

    if ( rf_regcnt1 > 0 && rf1_regs != NULL)
    {
        BB_SPI_curPageWriteByte(0x01,0x01);             //bypass: SPI change into 1st 8003s
        
        for(cnt = 0; cnt < rf_regcnt1; cnt++)
        {
            RF8003s_SPI_WriteReg_internal( rf1_regs[cnt].addr, rf1_regs[cnt].value);
        }

        {
            //add patch, reset 8003
            uint16_t delay = 0;
            RF8003s_SPI_WriteReg_internal(0x15, 0x51);
            while(delay ++ < 1000);
            RF8003s_SPI_WriteReg_internal(0x15, 0x50);
        } 
        
        BB_SPI_curPageWriteByte(0x01,0x02);             //SPI change into 8020    
    }

    if (boardCfg->u8_rf8003Cnt > 1 && rf_regcnt2 > 0 && rf2_regs != NULL)
    {
        BB_SPI_curPageWriteByte(0x01,0x03);             //bypass: SPI change into 2rd 8003s
        
        for(cnt = 0; cnt < rf_regcnt2; cnt++)
        {
            dlog_info("%x %x \n", rf2_regs[cnt].addr, rf2_regs[cnt].value);
            RF8003s_SPI_WriteReg_internal( rf2_regs[cnt].addr, rf2_regs[cnt].value);
        }

        {
            //add patch, reset 8003
            uint16_t delay = 0;
            RF8003s_SPI_WriteReg_internal(0x15, 0x51);
            while(delay ++ < 1000);            
            RF8003s_SPI_WriteReg_internal(0x15, 0x50);
        } 
        
        BB_SPI_curPageWriteByte(0x01,0x02);             //SPI change into 8020    
    }    
}
