#ifndef _SPI_
#define _SPI_

#include <stdint.h>

#ifndef NULL
#define NULL  0
#endif

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
