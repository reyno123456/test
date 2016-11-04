#ifndef __BB_CTRL_
#define __BB_CTRL_

#include <stdint.h>
#include "BB_spi.h"

typedef enum
{
    BB_SKY_MODE     = 0,
    BB_GRD_MODE     = 1,
    BB_MODE_UNKNOWN = 0xFF
} ENUM_BB_MODE;


typedef enum
{
    BB_RESET_TX     = 0x00,
    BB_RESET_RX     = 0x01,
    BB_RESET_TXRX   = 0x02,
    BB_RESET_UNKNOWN= 0xFF
} ENUM_RST_MODE;


typedef enum
{
    SKY_VID_PATH_NONE   = 0x00,
    SKY_VID_PATH_0      = 0x40,
    SKY_VID_PATH_1      = 0x80,
    SKY_VID_PATH_BOTH   = 0xC0,
    SKY_VID_PATH_UNKNOWN= 0xFF
} ENUM_VID_PATH;

typedef enum
{
    IT_ONLY_MODE = 0,       //iamge transmit only
    IT_RC_MODE = 1,         //iamge transmit and RC mode
    IT_MAX_TRX_CTRL = 0xff
} ENUM_TRX_CTRL;


typedef enum
{
    GRD_RCV_DISABLE     = 0x00,     //disable ground receive 
    GRD_RCV_ENABLE      = 0x01,     //enable ground receive 
    GRD_MAX_RCV         = 0xff
} ENUM_RCV_enable;


typedef struct
{
    ENUM_BB_MODE en_mode;
    ENUM_VID_PATH en_vidPath;
    ENUM_RCV_enable en_rcvEnable;
}STRU_BB_initType;


void BB_init(STRU_BB_initType *ptr_initType);

void BB_uart10_spi_sel(uint32_t sel_dat);

int BB_softReset(ENUM_RST_MODE en_mode);

uint8_t BB_set_Rcfrq(uint8_t ch);
uint8_t BB_set_ITfrq(uint8_t ch);
uint8_t BB_set_sweepfrq(uint8_t ch);
uint8_t BB_ReadReg(ENUM_REG_PAGES page, uint8_t addr);
uint8_t BB_WriteReg(ENUM_REG_PAGES page, uint8_t addr, uint8_t data);

#endif
