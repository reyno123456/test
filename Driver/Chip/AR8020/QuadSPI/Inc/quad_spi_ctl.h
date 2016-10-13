#ifndef QUAD_SPI_CONTROLLER_H
#define QUAD_SPI_CONTROLLER_H

//--------------------------------------------------------------------
// memory address space define here
//--------------------------------------------------------------------
#define HP_SPI_BASE_ADDR  0x40C00000

#define HP_SPI_CMD_BASE         ( 0   )*4
#define HP_SPI_AW_MEM_BASE      ( 32  )*4
#define HP_SPI_CODE_MEM_BASE    ( 48  )*4

#define HP_SPI_CONFIG_REG       ( 112 )*4
#define HP_SPI_INIT_REG         ( 113 )*4
#define HP_SPI_RD_HW_REG        ( 114 )*4
#define HP_SPI_RD_LW_REG        ( 115 )*4
#define HP_SPI_WR_HW_REG        ( 116 )*4
#define HP_SPI_WR_LW_REG        ( 117 )*4
#define HP_SPI_UPDATE_RD_REG    ( 118 )*4
#define HP_SPI_UPDATE_WR_REG    ( 119 )*4

//-------------------------------------------------------------------
// spansion instruction address mapping
//-------------------------------------------------------------------
#define INSTRUCT_05_ADDR  ( 0 )*4
#define INSTRUCT_35_ADDR  ( 1 )*4
#define INSTRUCT_33_ADDR  ( 2 )*4
#define INSTRUCT_04_ADDR  ( 3 )*4
#define INSTRUCT_01_ADDR  ( 4 )*4
#define INSTRUCT_ab_ADDR  ( 5 )*4
#define INSTRUCT_90_ADDR  ( 6 )*4
#define INSTRUCT_9f_ADDR  ( 7 )*4
#define INSTRUCT_77_ADDR  ( 8 )*4
#define INSTRUCT_C7_ADDR  ( 9 )*4
#define INSTRUCT_75_ADDR  ( 10 )*4
#define INSTRUCT_7a_ADDR  ( 11 )*4
#define INSTRUCT_66_ADDR  ( 12 )*4
#define INSTRUCT_99_ADDR  ( 13 )*4
#define INSTRUCT_ff_ADDR  ( 14 )*4
#define INSTRUCT_b9_ADDR  ( 15 )*4
#define INSTRUCT_06_ADDR  ( 16 )*4
#define INSTRUCT_50_ADDR  ( 17 )*4
#define INSTRUCT_18_ADDR  ( 18 )*4
#define INSTRUCT_19_ADDR  ( 19 )*4
#define INSTRUCT_20_ADDR  ( 20 )*4
#define INSTRUCT_d8_ADDR  ( 21 )*4
#define INSTRUCT_5a_ADDR  ( 22 )*4
#define INSTRUCT_48_ADDR  ( 23 )*4
#define INSTRUCT_44_ADDR  ( 24 )*4
#define INSTRUCT_42_ADDR  ( 25 )*4

#define INSTRUCT_MEM_OFFSET  ( 16 )*4

typedef enum
{
    QUAD_SPI_SPEED_25M = 0,
    QUAD_SPI_SPEED_50M,
    QUAD_SPI_SPEED_100M,
    QUAD_SPI_SPEED_25M_ENCRYPT,
    QUAD_SPI_SPEED_50M_ENCRYPT,
    QUAD_SPI_SPEED_100M_ENCRYPT,
    QUAD_SPI_SPEED_UNKNOWN,
} ENUM_QUAD_SPI_SPEED;

uint8_t QUAD_SPI_BlockErase(uint32_t flash_blk_st_addr);
uint8_t QUAD_SPI_SectorErase(uint32_t flash_sect_st_addr);
uint8_t QUAD_SPI_ChipErase(void);
uint8_t QUAD_SPI_WriteByte(uint32_t flash_addr, uint8_t value);
uint8_t QUAD_SPI_WriteHalfWord(uint32_t flash_addr, uint16_t value);
uint8_t QUAD_SPI_WriteWord(uint32_t flash_addr, uint32_t value);
uint8_t QUAD_SPI_WriteBlockByWord(uint32_t flash_blk_st_addr, uint32_t* blk_val_table);
uint8_t QUAD_SPI_SetSpeed(ENUM_QUAD_SPI_SPEED speed);


#endif

