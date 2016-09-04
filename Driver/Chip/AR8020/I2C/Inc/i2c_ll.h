#ifndef I2C_LL_H
#define I2C_LL_H

#include <stddef.h>
#include <stdint.h>
#include "i2c.h"

#define MAX_I2C_CONTOLLER_NUMBER 5

#define BASE_ADDR_I2C0 0x40200000
#define BASE_ADDR_I2C1 0x40240000
#define BASE_ADDR_I2C2 0x40280000
#define BASE_ADDR_I2C3 0x402c0000
#define BASE_ADDR_I2C4 0x40900000

#define I2C_CMD_ID_START 0x10000

typedef enum
{
    I2C_CMD_SET_MODE = I2C_CMD_ID_START,
    I2C_CMD_SET_M_SPEED,
    I2C_CMD_SET_M_TARGET_ADDRESS,
    I2C_CMD_SET_S_SLAVE_ADDRESS,
    I2C_CMD_SET_M_WRITE_DATA,
    I2C_CMD_GET_M_READ_DATA,
    I2C_CMD_GET_M_IDLE,
    I2C_CMD_GET_M_TXFIFO_EMPTY,
} ENUM_I2C_CMD_ID;

typedef struct
{
    unsigned int    IC_CON      ;       // 0x00
    unsigned int    IC_TAR      ;       // 0x04
    unsigned int    IC_SAR      ;       // 0x08
    unsigned int    IC_HS_MADDR ;       // 0x0c
    unsigned int    IC_DATA_CMD ;       // 0x10
    unsigned int    IC_SS_SCL_HCNT;     // 0x14
    unsigned int    IC_SS_SCL_LCNT;     // 0x18
    unsigned int    IC_FS_SCL_HCNT;     // 0x1c
    unsigned int    IC_FS_SCL_LCNT;     // 0x20
    unsigned int    IC_HS_SCL_HCNT;     // 0x24
    unsigned int    IC_HS_SCL_LCNT;     // 0x28
    unsigned int    IC_INTR_STAT;       // 0x2c
    unsigned int    IC_INTR_MASK;       // 0x30
    unsigned int    IC_RAW_INTR_STAT;   // 0x34
    unsigned int    IC_RX_TL    ;       // 0x38
    unsigned int    IC_TX_TL    ;       // 0x3c
    unsigned int    IC_CLR_INTR ;       // 0x40
    unsigned int    IC_CLR_RX_UNDER;    // 0x44
    unsigned int    IC_CLR_RX_OVER;     // 0x48
    unsigned int    IC_CLR_TX_OVER;     // 0x4c
    unsigned int    IC_CLR_RD_REQ;      // 0x50
    unsigned int    IC_CLR_TX_ABRT;     // 0x54
    unsigned int    IC_CLR_RX_DONE;     // 0x58
    unsigned int    IC_CLR_ACTIVITY;    // 0x5c
    unsigned int    IC_CLR_STOP_DET;    // 0x60
    unsigned int    IC_CLR_START_DET;   // 0x64
    unsigned int    IC_CLR_GEN_GALL;    // 0x68
    unsigned int    IC_ENABLE   ;       // 0x6c
    unsigned int    IC_STATUS   ;       // 0x70
    unsigned int    IC_TXFLR    ;       // 0x74
    unsigned int    IC_RXFLR    ;       // 0x78
    unsigned int    IC_SDA_HOLD ;       // 0x7c
    unsigned int    IC_TX_ABRT_SOURCE;  // 0x80
    unsigned int    IC_SLV_DATA_NACK_ONLY;// 0x84
    unsigned int    IC_DMA_CR   ;       // 0x88
    unsigned int    IC_DMA_TDLR ;       // 0x8c
    unsigned int    IC_DMA_RDLR ;       // 0x90
    unsigned int    IC_SDA_SETUP;       // 0x94
    unsigned int    IC_ACK_GENERAL_CALL;// 0x98
    unsigned int    IC_ENABLE_STATUS;   // 0x9c
    unsigned int    IC_FS_SPKLEN;       // 0xa0
    unsigned int    IC_HS_SPKLEN;       // 0xa4
    unsigned int    reserved[19];       // 0xa8 ~ 0xf0
    unsigned int    IC_COMP_PARAM_1;    // 0xf4
    unsigned int    IC_COMP_VERSION;    // 0xf8
    unsigned int    IC_COMP_TYPE;       // 0xfc
} STRU_I2C_Type;

typedef struct
{
    const uint32_t u32_i2cRegBaseAddr;
    ENUM_I2C_Mode  en_i2cMode;
    union
    {
        struct
        {
            uint16_t addr;
            ENUM_I2C_Speed  speed;
        } master;
        struct
        {
            uint16_t addr;
        } slave;
    } parameter;
} STRU_I2C_Controller;

void I2C_LL_Delay(unsigned int delay);
STRU_I2C_Controller* I2C_LL_GetI2CController(EN_I2C_COMPONENT en_i2cComponent);
uint8_t I2C_LL_IOCtl(STRU_I2C_Controller* ptr_i2cController, ENUM_I2C_CMD_ID en_CommandID, uint32_t* ptr_CommandVal);

#endif

