#include <stdint.h>
#include <string.h>
#include "systicks.h"
#include "reg_rw.h"
#include "spi.h"
#include "debuglog.h"
#include "interrupt.h"
#include "pll_ctrl.h"


/* Private variables ---------------------------------------------------------*/
static uint32_t SPI_BaseList[]= {
    SPI_BASE_0,
    SPI_BASE_1,
    SPI_BASE_2,
    SPI_BASE_3,
    SPI_BASE_4,
    SPI_BASE_5,
    SPI_BASE_6,
    SPI_BASE_7,
};
    
static STRU_SPI_InitTypes spi_inits[]= {
    {0},
    {0},
    {0},
    {0},
    {0},
    {0},
    {0},
    { //default value for BB SPI.
        .ctrl0   = SPI_CTRL0_DEF_VALUE,
        .clk_Mhz = 9,
        .Tx_Fthr = SPI_TXFTLR_DEF_VALUE,
        .Rx_Ftlr = SPI_RXFTLR_DEF_VALUE,
        .SER     = SPI_SSIENR_DEF_VALUE,
    },
};

static STRU_SPI_INT_DATA s_st_spiIntData[SPI_MAX_CHANNEL];


static void SPI_SetReadDataLen(ENUM_SPI_COMPONENT en_id, uint16_t u16_len);

static void SPI_disable(ENUM_SPI_COMPONENT en_id);

static int32_t SPI_SetIntData(ENUM_SPI_COMPONENT en_id,
                              uint8_t *ptr_wbuf, uint32_t u32_wsize,
                              uint8_t *ptr_rbuf, uint32_t u32_rsize);

static int32_t SPI_SetMode(ENUM_SPI_COMPONENT en_id, uint32_t spi_mode);

static int32_t SPI_EnableInt(ENUM_SPI_COMPONENT en_id, uint32_t data);

static int32_t SPI_DisEnableInt(ENUM_SPI_COMPONENT en_id, uint32_t data);

static uint32_t SPI_GetInputClockByIndex(ENUM_SPI_COMPONENT en_id);

static uint32_t SPI_CalcClkDiv(ENUM_SPI_COMPONENT en_id, uint32_t u32_rateMhz);




/* Private function prototypes -----------------------------------------------*/
static void SPI_disable(ENUM_SPI_COMPONENT en_id)
{
    Reg_Write32( (SPI_BaseList[en_id]+SPI_SLAVE_EN), 0 );
    Reg_Write32( (SPI_BaseList[en_id]+SPI_SSIENR),  0x00);          //disable ssi
}


/* Exported functions ---------------------------------------------------------*/

/**
  * @brief  init SPI in master 
  * @param  en_id ref@ENUM_SPI_COMPONENT
  * @param  settings: the SPI register setting for SPI master
  * @retval None
  */
void SPI_master_init(ENUM_SPI_COMPONENT en_id, STRU_SPI_InitTypes *st_settings)
{
    uint32_t divider = SPI_CalcClkDiv(en_id, st_settings->clk_Mhz);

    SPI_DisEnableInt(en_id, SPI_IMR_MASK);

    Reg_Write32( (SPI_BaseList[en_id]+SPI_SLAVE_EN), 0 );
    Reg_Write32( (SPI_BaseList[en_id]+SPI_SSIENR),  0x00);          //disable ssi

    Reg_Write32( (SPI_BaseList[en_id]+SPI_CTRLR0),  st_settings->ctrl0);
    Reg_Write32( (SPI_BaseList[en_id]+SPI_BAUDR),   divider);
    Reg_Write32( (SPI_BaseList[en_id]+SPI_TXFTLR),  st_settings->Tx_Fthr);
    Reg_Write32( (SPI_BaseList[en_id]+SPI_RXFTLR),  st_settings->Rx_Ftlr);
    Reg_Read32(SPI_BaseList[en_id]+SPI_ICR);
    Reg_Write32( SPI_BaseList[en_id]+SPI_SSIENR,    0x01);          //enable ssi
    
    memset((uint8_t *)(&s_st_spiIntData[en_id]), 0x00, sizeof(STRU_SPI_INT_DATA));
    memcpy( (void *)(spi_inits+en_id), (void *)st_settings, sizeof(STRU_SPI_InitTypes)); 
}

static void SPI_SetReadDataLen(ENUM_SPI_COMPONENT en_id, uint16_t u16_len)
{    
    Reg_Write32( (SPI_BaseList[en_id]+SPI_SSIENR),  0x00);          //disable ssi

    Reg_Write32( (SPI_BaseList[en_id]+SPI_CTRLR1),  u16_len);

    Reg_Write32( SPI_BaseList[en_id]+SPI_SSIENR,    0x01);          //enable ssi
}

int32_t SPI_write_read(ENUM_SPI_COMPONENT en_id,
                       uint8_t *ptr_wbuf, uint32_t u32_wsize,
                       uint8_t *ptr_rbuf, uint32_t u32_rsize)                       
{
    volatile uint32_t state, busy;
    uint32_t i = 0;
    uint32_t u32_rxCnt = 0;
    uint32_t u32_tmpLen = 0;
    uint32_t tfe;
    uint32_t start;

    u32_tmpLen = ((u32_wsize < SPI_TXFLR_MAX)? u32_wsize:(SPI_TXFLR_MAX));
    
    SPI_master_init(en_id, spi_inits+en_id);
    SPI_SetIntData(en_id, ptr_wbuf + u32_tmpLen, u32_wsize-u32_tmpLen, ptr_rbuf, u32_rsize);

    if(u32_wsize > 0)
    {
        if (u32_rsize > 0)
        {
            SPI_SetReadDataLen(en_id, u32_rsize-1);
            
            SPI_SetMode(en_id, SPI_CTRLR0_TMOD_EE);
            
            if (u32_rsize <= SPI_RXFTLR_DEF_VALUE)
            {
                Reg_Write32( (SPI_BaseList[en_id]+SPI_RXFTLR), 0);
            }

            SPI_EnableInt(en_id, SPI_IMR_RXFIM);
        }
        else
        {
            SPI_SetMode(en_id, SPI_CTRLR0_TMOD_TO);
        }
        
        while(i<u32_tmpLen)
        {
            Reg_Write32(SPI_BaseList[en_id]+SPI_DR, ptr_wbuf[i++]); //write data
        }
        Reg_Write32( (SPI_BaseList[en_id]+SPI_SLAVE_EN),  spi_inits[en_id].SER );
        
        if (u32_wsize > SPI_TXFLR_MAX)
        {
            SPI_EnableInt(en_id, SPI_IMR_TXEIM);
        }
    }
    
    return 0;
}

void SPI_IntrSrvc(uint32_t u32_vectorNum)
{
    uint8_t u8_spiCh;
    uint8_t u8_spiIsr;
    uint8_t u8_cnt = 0;
    uint8_t tmpLen;
    static uint32_t u32_cnt = 0;

    if (VIDEO_SPI_INTR_BB_VECTOR_NUM == u32_vectorNum)
    {
        u8_spiCh = 7;
    }
    else
    {
        u8_spiCh = u32_vectorNum - SSI_INTR_N0_VECTOR_NUM;
    }
    
   
    u8_spiIsr = Reg_Read32(SPI_BaseList[u8_spiCh]+SPI_ISR) & SPI_ISR_MASK;

    u8_cnt = 0;
    if (u8_spiIsr & SPI_ISR_TXEIS) // TX INT
    {
        tmpLen = Reg_Read32(SPI_BaseList[u8_spiCh]+SPI_TXFLR);
        tmpLen = SPI_TXFLR_MAX - tmpLen;
        while ((s_st_spiIntData[u8_spiCh].txAlrLen) < (s_st_spiIntData[u8_spiCh].txLen))
        {
            Reg_Write32(SPI_BaseList[u8_spiCh]+SPI_DR, 
            s_st_spiIntData[u8_spiCh].txBuf[s_st_spiIntData[u8_spiCh].txAlrLen]); //write data
            s_st_spiIntData[u8_spiCh].txAlrLen += 1;
            u8_cnt += 1;
            if (u8_cnt >= tmpLen)
            {
                break;
            }
        }
        if((s_st_spiIntData[u8_spiCh].txAlrLen) >= (s_st_spiIntData[u8_spiCh].txLen))
        {
            SPI_DisEnableInt(u8_spiCh, SPI_IMR_TXEIM);
        }
    }

    u8_cnt = 0;
    if (u8_spiIsr & SPI_ISR_RXFIS) // RX INT
    {
        tmpLen = Reg_Read32(SPI_BaseList[u8_spiCh]+SPI_RXFLR);
        while ((s_st_spiIntData[u8_spiCh].rxAlrLen) < (s_st_spiIntData[u8_spiCh].rxLen))
        {
            s_st_spiIntData[u8_spiCh].rxBuf[s_st_spiIntData[u8_spiCh].rxAlrLen] = Reg_Read32(SPI_BaseList[u8_spiCh]+SPI_DR);
            s_st_spiIntData[u8_spiCh].rxAlrLen += 1;
            u8_cnt += 1;
            if (u8_cnt >= tmpLen)
            {
                break;
            }
        }
        
        if((s_st_spiIntData[u8_spiCh].rxAlrLen) >= (s_st_spiIntData[u8_spiCh].rxLen))
        {
            SPI_DisEnableInt(u8_spiCh, SPI_IMR_RXFIM);
        }
        
        if (((s_st_spiIntData[u8_spiCh].rxLen) - (s_st_spiIntData[u8_spiCh].rxAlrLen)) <= SPI_RXFTLR_DEF_VALUE)
        {
            Reg_Write32( (SPI_BaseList[u8_spiCh]+SPI_RXFTLR), 0);
        }
    }

    if (u8_spiIsr & (~(SPI_ISR_TXEIS | SPI_ISR_RXFIS))) // some error happpened.
    {
        SPI_DisEnableInt(u8_spiCh, SPI_IMR_MASK);
    }
}

static int32_t SPI_SetIntData(ENUM_SPI_COMPONENT en_id,
                              uint8_t *ptr_wbuf, uint32_t u32_wsize,
                              uint8_t *ptr_rbuf, uint32_t u32_rsize) 
{

    if (en_id < SPI_MAX_CHANNEL)
    {
        s_st_spiIntData[en_id].txLen = u32_wsize;     
        s_st_spiIntData[en_id].txAlrLen = 0;  
        s_st_spiIntData[en_id].txBuf = ptr_wbuf;     
        s_st_spiIntData[en_id].rxLen = u32_rsize;     
        s_st_spiIntData[en_id].rxAlrLen = 0;  
        s_st_spiIntData[en_id].rxBuf = ptr_rbuf;  

        return 0;
    }
    else
    {
        return -1;
    }
}

static int32_t SPI_SetMode(ENUM_SPI_COMPONENT en_id, uint32_t spi_mode) 
{
    uint32_t u32_data;
    
    if (en_id < SPI_MAX_CHANNEL)
    {
        Reg_Write32( (SPI_BaseList[en_id]+SPI_SSIENR),  0x00);          //disable ssi

        u32_data = Reg_Read32(SPI_BaseList[en_id]+SPI_CTRLR0) & (~(SPI_CTRLR0_TMOD_MASK));
        u32_data = u32_data | spi_mode;
        Reg_Write32( (SPI_BaseList[en_id]+SPI_CTRLR0),  u32_data);

        Reg_Write32( SPI_BaseList[en_id]+SPI_SSIENR,    0x01);          //enable ssi

        return 0;
    }
    else
    {
        return -1;
    }
}


static int32_t SPI_EnableInt(ENUM_SPI_COMPONENT en_id, uint32_t data) 
{
    uint32_t u32_data;
    
    if (en_id < SPI_MAX_CHANNEL)
    {
        u32_data = Reg_Read32(SPI_BaseList[en_id]+SPI_IMR) | data;
    
        Reg_Write32( (SPI_BaseList[en_id]+SPI_IMR),  u32_data); 

        return 0;
    }
    else
    {
        return -1;
    }
}

static int32_t SPI_DisEnableInt(ENUM_SPI_COMPONENT en_id, uint32_t data) 
{
    uint32_t u32_data;
    
    if (en_id < SPI_MAX_CHANNEL)
    {
        u32_data = Reg_Read32(SPI_BaseList[en_id]+SPI_IMR) & (~data);
    
        Reg_Write32( (SPI_BaseList[en_id]+SPI_IMR),  u32_data); 

        return 0;
    }
    else
    {
        return -1;
    }
}

static uint32_t SPI_GetInputClockByIndex(ENUM_SPI_COMPONENT en_id)
{
    uint16_t u16_pllClk = 64;

    switch (en_id)
    {
    case SPI_0:
    case SPI_1:
    case SPI_2:
    case SPI_3:
    case SPI_4:
    case SPI_5:
    case SPI_6:
        PLLCTRL_GetCoreClk(&u16_pllClk, ENUM_CPU0_ID);
        u16_pllClk = u16_pllClk >> 1;
        break;
    case SPI_7:
        PLLCTRL_GetCoreClk(&u16_pllClk, ENUM_CPU2_ID);
        break;
    default:
        break;
    }

    return (uint32_t)u16_pllClk;
}

static uint32_t SPI_CalcClkDiv(ENUM_SPI_COMPONENT en_id, uint32_t u32_rateMhz)
{
    uint32_t u32_clk = SPI_GetInputClockByIndex(en_id); // M -> K

    if ((u32_rateMhz >= 1) && (u32_rateMhz <= u32_clk))
    {
        return (u32_clk / u32_rateMhz);
    }
    else
    {
        return (u32_clk);
    }
}

int SPI_GetBusyStatus(ENUM_SPI_COMPONENT en_id)
{
    int32_t data;
    
    data = Reg_Read32(SPI_BaseList[en_id]+SPI_SR);

    if ( data & SPI_SR_TFE )
    {
        return (data & SPI_SR_BUSY);
    }
    else
    {
        return 1;
    }
}

int SPI_WaitIdle(ENUM_SPI_COMPONENT en_id, uint32_t timeOut)
{
    uint32_t start;
    
    if (0 != timeOut)
    {
        start = SysTicks_GetTickCount();
        while (SPI_GetBusyStatus(en_id))
        {
  
            if ((SysTicks_GetDiff(start, SysTicks_GetTickCount())) >= timeOut)
            {
            
                return -1;
 
            }
    
        }

 
    }

    return 0;
}

