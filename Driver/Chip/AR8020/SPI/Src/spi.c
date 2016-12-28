#include <stdint.h>
#include <string.h>
#include "systicks.h"
#include "reg_rw.h"
#include "spi.h"
#include "debuglog.h"


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
        .ctrl0   = 0x47,
        .clk_Mhz = 9,
        .Tx_Fthr = 3,
        .Rx_Ftlr = 6,
        .SER     = 1,
    },
};


/* Private function prototypes -----------------------------------------------*/
static void SPI_disable(ENUM_SPI_COMPONENT en_id)
{
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
    uint32_t divider = (SPI_BASE_CLK_MHZ + st_settings->clk_Mhz-1)/st_settings->clk_Mhz;
    Reg_Write32( (SPI_BaseList[en_id]+SPI_SSIENR),  0x00);          //disable ssi

    Reg_Write32( (SPI_BaseList[en_id]+SPI_CTRLR0),  st_settings->ctrl0);
    Reg_Write32( (SPI_BaseList[en_id]+SPI_BAUDR),   divider);
    Reg_Write32( (SPI_BaseList[en_id]+SPI_TXFTLR),  st_settings->Tx_Fthr);
    Reg_Write32( (SPI_BaseList[en_id]+SPI_RXFTLR),  st_settings->Rx_Ftlr);
    Reg_Write32( (SPI_BaseList[en_id]+SPI_SLAVE_EN),st_settings->SER);

    Reg_Write32( SPI_BaseList[en_id]+SPI_SSIENR,    0x01);          //enable ssi
    memcpy( (void *)(spi_inits+en_id), (void *)st_settings, sizeof(STRU_SPI_InitTypes));
}


/**
  * @brief  SPI write and read function. 
  * @param  en_id ref@ENUM_SPI_COMPONENT
  * @param  ptr_wbuf, u32_wsize: write data buffer and size
  * @param  ptr_rbuf, u32_rsize: read data buffer and size
  * @retval None
  */
int32_t SPI_write_read(ENUM_SPI_COMPONENT en_id,
                       uint8_t *ptr_wbuf, uint32_t u32_wsize,
                       uint8_t *ptr_rbuf, uint32_t u32_rsize)                       
{
    volatile uint32_t state, busy;
    uint32_t i = 0;

    SPI_master_init(en_id, spi_inits+en_id);
    if(u32_wsize > 0)
    {
        for(i =0; i < u32_wsize; i++)
        {
            Reg_Write32(SPI_BaseList[en_id]+SPI_DR, ptr_wbuf[i]); //write data
        }

        busy = 0;
        if(u32_wsize > 0)
        {
            uint32_t start = SysTicks_GetTickCount();
            do{
                state = Reg_Read32(SPI_BaseList[en_id]+SPI_SR);
                busy = state & 0x01;
            }while(busy && SysTicks_GetTickCount() - start < SPI_DEF_TIMEOUT_TICKS);
        }    
        if(busy)
        {
            dlog_error("SPI %d Timeout", (int)en_id);
            return 1;
        }
    }
    
    if(u32_rsize > 0)
    {
        for(i =0; i < u32_rsize; i++)
        {
            ptr_rbuf[i] = Reg_Read32(SPI_BaseList[en_id]+SPI_DR);
        }
    }
    
    SPI_disable(en_id);
    return 0;
}


