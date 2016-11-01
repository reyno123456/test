#include <stdint.h>
#include <string.h>
#include "systicks.h"
#include "reg_rw.h"
#include "spi.h"
#include "debuglog.h"


/* Private typedef -----------------------------------------------------------*/
/* Private define ------------------------------------------------------------*/

/* Private macro -------------------------------------------------------------*/

#define     SPI_BASE_0	(0x40100000)	//ssi_0 
#define     SPI_BASE_1 	(0x40120000)	//ssi_1 
#define     SPI_BASE_2	(0x40140000)	//ssi_2 
#define     SPI_BASE_3	(0x40160000)	//ssi_3 
#define     SPI_BASE_4	(0x40180000)	//ssi_4 
#define     SPI_BASE_5	(0x401a0000)	//ssi_5 
#define     SPI_BASE_6	(0x401c0000)	//ssi_6
#define     SPI_BASE_7	(0xa0040000)	//ssi_7


//=== SPI as Master Register Mapping ===//
#define		CTRLR0		0x00    //Control Register0
#define		CTRLR1		0x04    //Control Register1
#define		SSIENR		0x08    //SSI Enable Reg
#define		MWCR		0x0C    //Microwire Control Reg
#define		SLAVE_EN	0x10    //Slave Enable Reg
#define		BAUDR		0x14    //Buad Rate Select
#define		TXFTLR		0x18    //Transmit FIFO Threshold Level
#define		RXFTLR		0x1c    //Receive FIFO Threshold Level
#define     TXFLR       0x20    //Transmit FIFO Level Register
#define     RXFLR       0x24    //Receive FIFO Level Register
#define     SR          0x28    //Status Register
#define     IMR         0x2C    //Interrupt Mask Register
#define     ISR         0x30    //Interrupt Status Register
#define     RISR        0x34    //Raw Interrupt Status Register
#define     TXOICR      0x38    //Transmit FIFO Overflow Interrupt Clear Register
#define     RXOICR      0x3c    //Receive FIFO OVerflow Interrupt Clear Register
#define     RXUICR      0x40    //Receive FIFO Underflow interrupt Clear register
#define     MSTICR      0x44    //Multi-master Interrupt Clear Register
#define     ICR         0x48    //Interrupt Clear Register
#define     DMACR       0x4C    //DMA Control Register
#define     DMATDLR     0x50    //DMA Transmit Data Level 
#define     DMARDLR     0x54    //DMA Receive Data Level 
#define     IDR         0x58    //Identification Register
#define     SSI_COMP_VERSION 0x5C //
#define		DR		    0x60    //Data Register


#define SPI_BASE_CLK_MHZ        (166)
#define SPI_DEF_TIMEOUT_TICKS   (50)

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
    Reg_Write32( (SPI_BaseList[en_id]+SSIENR),	0x00);			//disable ssi
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
    Reg_Write32( (SPI_BaseList[en_id]+SSIENR),	0x00);  		//disable ssi

    Reg_Write32( (SPI_BaseList[en_id]+CTRLR0),	st_settings->ctrl0);
    Reg_Write32( (SPI_BaseList[en_id]+BAUDR),	divider);
    Reg_Write32( (SPI_BaseList[en_id]+TXFTLR),	st_settings->Tx_Fthr);
    Reg_Write32( (SPI_BaseList[en_id]+RXFTLR),	st_settings->Rx_Ftlr);
    Reg_Write32( (SPI_BaseList[en_id]+SLAVE_EN),st_settings->SER);

    Reg_Write32( SPI_BaseList[en_id]+SSIENR,	0x01);          //enable ssi
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
			Reg_Write32(SPI_BaseList[en_id]+DR,	ptr_wbuf[i]); //write data
		}

		busy = 0;
		if(u32_wsize > 0)
		{
			uint32_t start = SysTicks_GetTickCount();
			do{
				state = Reg_Read32(SPI_BaseList[en_id]+SR);
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
			ptr_rbuf[i] = Reg_Read32(SPI_BaseList[en_id]+DR);
		}
	}
    
    SPI_disable(en_id);
    return 0;
}


void SPI_master_Deinit(ENUM_SPI_COMPONENT en_id)
{
    /*
     * Todo
    */
}