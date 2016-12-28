#ifndef _SPI_
#define _SPI_

#include <stdint.h>

#define     SPI_BASE_0	(0x40100000)	//ssi_0 
#define     SPI_BASE_1 	(0x40120000)	//ssi_1 
#define     SPI_BASE_2	(0x40140000)	//ssi_2 
#define     SPI_BASE_3	(0x40160000)	//ssi_3 
#define     SPI_BASE_4	(0x40180000)	//ssi_4 
#define     SPI_BASE_5	(0x401a0000)	//ssi_5 
#define     SPI_BASE_6	(0x401c0000)	//ssi_6
#define     SPI_BASE_7	(0xa0040000)	//ssi_7


//=== SPI as Master Register Mapping ===//
#define	    CTRLR0	        (0x00)    //Control Register0
#define	    CTRLR1	        (0x04)    //Control Register1
#define	    SSIENR	        (0x08)    //SSI Enable Reg
#define	    MWCR	        (0x0C)    //Microwire Control Reg
#define	    SLAVE_EN	        (0x10)    //Slave Enable Reg
#define	    BAUDR	        (0x14)    //Buad Rate Select
#define	    TXFTLR	        (0x18)    //Transmit FIFO Threshold Level
#define	    RXFTLR	        (0x1c)    //Receive FIFO Threshold Level
#define     TXFLR               (0x20)    //Transmit FIFO Level Register
#define     RXFLR               (0x24)    //Receive FIFO Level Register
#define     SR                  (0x28)    //Status Register
#define     IMR                 (0x2C)    //Interrupt Mask Register
#define     ISR                 (0x30)    //Interrupt Status Register
#define     RISR                (0x34)    //Raw Interrupt Status Register
#define     TXOICR              (0x38)    //Transmit FIFO Overflow Interrupt Clear Register
#define     RXOICR              (0x3c)    //Receive FIFO OVerflow Interrupt Clear Register
#define     RXUICR              (0x40)    //Receive FIFO Underflow interrupt Clear register
#define     MSTICR              (0x44)    //Multi-master Interrupt Clear Register
#define     ICR                 (0x48)    //Interrupt Clear Register
#define     DMACR               (0x4C)    //DMA Control Register
#define     DMATDLR             (0x50)    //DMA Transmit Data Level 
#define     DMARDLR             (0x54)    //DMA Receive Data Level 
#define     IDR                 (0x58)    //Identification Register
#define     SSI_COMP_VERSION    (0x5C)    //
#define	    DR		        (0x60)    //Data Register


#define SPI_BASE_CLK_MHZ        (166)
#define SPI_DEF_TIMEOUT_TICKS   (50)

typedef enum
{
	SPI_0 = 0,
	SPI_1,
	SPI_2,
	SPI_3,
	SPI_4,
	SPI_5,
	SPI_6,
	SPI_7
} ENUM_SPI_COMPONENT;

typedef struct
{
    uint32_t ctrl0;     //
    uint32_t ctrl2;     //
    uint32_t clk_Mhz;   //clockrate (MHz)
    uint32_t Tx_Fthr;   //TX FiFO threshhold
    uint32_t Rx_Ftlr;   //
    uint32_t SER;       //
} STRU_SPI_InitTypes;


void SPI_master_init(ENUM_SPI_COMPONENT en_id, STRU_SPI_InitTypes *st_settings);

int32_t SPI_write_read(ENUM_SPI_COMPONENT en_id,
                       uint8_t *ptr_wbuf, uint32_t u32_wsize,
                       uint8_t *ptr_rbuf, uint32_t u32_rsize);

#endif
