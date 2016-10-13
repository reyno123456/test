#include <stdint.h>
#include "data_type.h"
#include "debuglog.h"
#include "quad_spi_ctl.h"
#include "reg_rw.h"

static uint8_t flash_program_instruct_enable = FALSE;

static uint8_t enableFlashProgramInstruct(void)
{
    Reg_Write32(HP_SPI_BASE_ADDR + HP_SPI_INIT_REG, BIT(0));
    flash_program_instruct_enable = TRUE;
}

uint8_t QUAD_SPI_BlockErase(uint32_t flash_blk_st_addr)
{
    uint8_t ret = TRUE;

    if (flash_program_instruct_enable == FALSE)
    {
        enableFlashProgramInstruct();
    }

    // 64KB Erase
    Reg_Write32(HP_SPI_BASE_ADDR + INSTRUCT_d8_ADDR + INSTRUCT_MEM_OFFSET, flash_blk_st_addr);
    Reg_Write32(HP_SPI_BASE_ADDR + INSTRUCT_d8_ADDR, 0);
    
    return ret;
}

uint8_t QUAD_SPI_SectorErase(uint32_t flash_sect_st_addr)
{
    uint8_t ret = TRUE;

    if (flash_program_instruct_enable == FALSE)
    {
        enableFlashProgramInstruct();
    }

    // 4K erase
    Reg_Write32( HP_SPI_BASE_ADDR + INSTRUCT_20_ADDR + INSTRUCT_MEM_OFFSET, flash_sect_st_addr);
    Reg_Write32( HP_SPI_BASE_ADDR + INSTRUCT_20_ADDR, 0);    
        
    return ret;
}

uint8_t QUAD_SPI_ChipErase(void)
{
    uint8_t ret = TRUE;

       
    return ret;
}

uint8_t QUAD_SPI_WriteByte(uint32_t flash_addr, uint8_t value)
{
    uint8_t ret = TRUE;
    
    *((uint8_t*)(flash_addr)) = value;
            
    return ret;
}

uint8_t QUAD_SPI_WriteHalfWord(uint32_t flash_addr, uint16_t value)
{
    uint8_t ret = TRUE;

    *((uint16_t*) (flash_addr)) = value;  

    return ret;
}

uint8_t QUAD_SPI_WriteWord(uint32_t flash_addr, uint32_t value)
{
    uint8_t ret = TRUE;

    *((uint32_t*) (flash_addr)) = value;     
            
    return ret;
}

uint8_t QUAD_SPI_WriteBlockByWord(uint32_t flash_blk_st_addr, uint32_t* blk_val_table)
{
    uint8_t ret = TRUE;
    
                
    return ret;
}

uint8_t QUAD_SPI_SetSpeed(ENUM_QUAD_SPI_SPEED speed)
{
    uint8_t ret = TRUE;
   
    uint32_t mask_val = 0;
    uint32_t mask_bit = BIT(2) | BIT(3) | BIT(7);

    switch(speed)
    {
    case QUAD_SPI_SPEED_25M:
        mask_val = 0;
        break;
    case QUAD_SPI_SPEED_50M:
        mask_val = BIT(2);
        break;
    case QUAD_SPI_SPEED_100M:
        mask_val = BIT(3);
        break;
    case QUAD_SPI_SPEED_25M_ENCRYPT:
        mask_val = BIT(7);
        break;
    case QUAD_SPI_SPEED_50M_ENCRYPT:
        mask_val = BIT(2) | BIT(7);
        break;
    case QUAD_SPI_SPEED_100M_ENCRYPT:
        mask_val = BIT(3) | BIT(7);
        break;
    default:
        ret = FALSE;
        dlog_error("The quad spi speed is not supported!\n");
        break;
     }

    Reg_Write32_Mask(HP_SPI_BASE_ADDR + HP_SPI_CONFIG_REG, mask_val, mask_bit);
    
    return ret;
}

