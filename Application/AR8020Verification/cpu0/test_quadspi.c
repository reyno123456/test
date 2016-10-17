#include <stdlib.h>
#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include "quad_spi_ctrl.h"
#include "nor_flash.h"
#include "test_quadspi.h"
#include "debuglog.h"
#include "stm32f746xx.h"

#define TEST_FLASH_START_ADDR 0x10000000

void command_setQuadSPISpeed(char* speed_str)
{
    ENUM_QUAD_SPI_SPEED speed = (ENUM_QUAD_SPI_SPEED)strtoul(speed_str, NULL, 0);
    QUAD_SPI_SetSpeed(speed);
    dlog_info("Set QUAD SPI speed to %d\n", speed);
}

void command_testQuadSPISpeedData(void)
{
    uint8_t data_buf_ref[1024*4];
    uint8_t data_buf[1024*4];

    uint8_t* check_ptr;
    uint16_t max_sect_cnt = 64;
    uint16_t cur_sect_num = 0;

    for(cur_sect_num = 0; cur_sect_num < max_sect_cnt; cur_sect_num++)
    {
        uint8_t* check_ptr_org = (uint8_t*)(TEST_FLASH_START_ADDR + (cur_sect_num * sizeof(data_buf)));

        uint32_t i = 0;
        uint32_t j = 0;

        QUAD_SPI_SetSpeed(QUAD_SPI_SPEED_25M);
        check_ptr = check_ptr_org;
        for (i = 0; i < sizeof(data_buf); i++)
        {
            data_buf_ref[i] = *check_ptr++;        
        }

        dlog_info("25M quad spi flash read ready, 0x%x!", check_ptr_org);

        QUAD_SPI_SetSpeed(QUAD_SPI_SPEED_50M);
        check_ptr = check_ptr_org;
        for (i = 0; i < sizeof(data_buf); i++)
        {
            data_buf[i] = *check_ptr++;        
        }

        if (memcmp(data_buf, data_buf_ref, sizeof(data_buf)) == 0)
        {
            dlog_info("50M quad spi flash read pass!");
        }
        else
        {
            dlog_info("50M quad spi flash read fail!");
        }
/*
        QUAD_SPI_SetSpeed(QUAD_SPI_SPEED_100M);
        check_ptr = check_ptr_org;
        for (i = 0; i < sizeof(data_buf); i++)
        {
            data_buf[i] = *check_ptr++;        
        }

        if (memcmp(data_buf, data_buf_ref, sizeof(data_buf)) == 0)
        {
            dlog_info("100M quad spi flash read pass!");
        }
        else
        {
            dlog_info("100M quad spi flash read fail!");
        }
*/
    }
    
    dlog_info("QUAD SPI data test finished\n");
}

void command_initWinbondNorFlash(void)
{
    //05h
    QUAD_SPI_UpdateInstruct(QUAD_SPI_INSTR_0, 0x001c14, 0x700);
    //35h
    QUAD_SPI_UpdateInstruct(QUAD_SPI_INSTR_1, 0x001cd4, 0x700);
    //15h
    QUAD_SPI_UpdateInstruct(QUAD_SPI_INSTR_2, 0x001c54, 0x700);
    //04h
    QUAD_SPI_UpdateInstruct(QUAD_SPI_INSTR_3, 0x609c10, 0x0);
    //01h
    QUAD_SPI_UpdateInstruct(QUAD_SPI_INSTR_4, 0x609c04, 0x700);
    //31h
    QUAD_SPI_UpdateInstruct(QUAD_SPI_INSTR_5, 0x609cc4, 0x700);
    //11h
    QUAD_SPI_UpdateInstruct(QUAD_SPI_INSTR_6, 0x609c44, 0x700);
    //C7h
    QUAD_SPI_UpdateInstruct(QUAD_SPI_INSTR_9, 0x409f1c, 0x0);
    //06h
    QUAD_SPI_UpdateInstruct(QUAD_SPI_INSTR_16, 0x609c18, 0x0);
    //50h
    QUAD_SPI_UpdateInstruct(QUAD_SPI_INSTR_17, 0x609d40, 0x0);
    //20h
    QUAD_SPI_UpdateInstruct(QUAD_SPI_INSTR_20, 0x609c80, 0x17);
    //D8h
    QUAD_SPI_UpdateInstruct(QUAD_SPI_INSTR_21, 0x609f60, 0x17);
}

void command_eraseWinbondNorFlash(char* type_str, char* start_addr_str)
{
    unsigned char type = strtoul(type_str, NULL, 0);
    uint32_t start_addr = strtoul(start_addr_str, NULL, 0);

    
    switch(type)
    {
    case 0:
        QUAD_SPI_WriteEnable();
        QUAD_SPI_CheckBusy();
        QUAD_SPI_SectorErase(start_addr);
        QUAD_SPI_CheckBusy();
        break;
    case 1:
        QUAD_SPI_WriteEnable();
        QUAD_SPI_CheckBusy();
        QUAD_SPI_BlockErase(start_addr);
        QUAD_SPI_CheckBusy();
        break;
    case 2:
        QUAD_SPI_WriteEnable();
        QUAD_SPI_CheckBusy();
        QUAD_SPI_ChipErase();
        QUAD_SPI_CheckBusy();
        break;
    default:
        break;
    }

    dlog_info("Erase finished!");
}

void command_writeWinbondNorFlash(char* start_addr_str, char* size_str, char* val_str)
{
    unsigned char val = strtoul(val_str, NULL, 0);
    uint32_t start_addr = strtoul(start_addr_str, NULL, 0);
    uint32_t size = strtoul(size_str, NULL, 0);
    
    uint32_t i;

    QUAD_SPI_WriteEnable();
    QUAD_SPI_CheckBusy();

    for (i = 0; i < size; i++)
    {
        QUAD_SPI_WriteByte(start_addr++, val++);
    }

    QUAD_SPI_CheckBusy();

    dlog_info("Write finished!");
}

void command_readWinbondNorFlash(char* start_addr_str, char* size_str)
{
    uint32_t start_addr = strtoul(start_addr_str, NULL, 0);
    uint32_t size = strtoul(size_str, NULL, 0);
    
    uint32_t i;
    uint8_t val;

    SCB_DisableDCache();

    for (i = 0; i < size; i++)
    {
        QUAD_SPI_ReadByte(start_addr, &val);
        dlog_info("%x, %x", val, start_addr);
        start_addr++;
    }

    SCB_EnableDCache();

    dlog_info("Read finished!");
}

void command_testAllNorFlashOperations(char* start_addr_str, char* size_str, char* val_str)
{
    unsigned char val = strtoul(val_str, NULL, 0);
    uint32_t start_addr = strtoul(start_addr_str, NULL, 0);
    uint32_t size = strtoul(size_str, NULL, 0);

    uint8_t* buf = malloc(size);
    if (buf)
    {
        uint32_t i = 0;
        for(i = 0; i < size; i++)
        {
            buf[i] = val + i;
        }
    }
    else
    {
        dlog_error("malloc error!");
    }

    dlog_info("Nor flash init start ...");
    NOR_FLASH_Init();
    dlog_info("Nor flash init end");

    dlog_info("Nor flash sector erase start ...");
    NOR_FLASH_EraseSector(start_addr);
    dlog_info("Nor flash sector erase end");

    dlog_info("Nor flash read start ...");
    command_readWinbondNorFlash(start_addr_str, size_str);
    dlog_info("Nor flash read end");

    dlog_info("Nor flash write start ...");
    if (buf)
    {
        NOR_FLASH_WriteByteBuffer(start_addr, buf, size);
    }
    dlog_info("Nor flash write end");

    dlog_info("Nor flash read start ...");
    command_readWinbondNorFlash(start_addr_str, size_str);
    dlog_info("Nor flash read end");    

    dlog_info("Nor flash sector erase start ...");
    NOR_FLASH_EraseSector(start_addr);
    dlog_info("Nor flash sector erase end");

    dlog_info("Nor flash read start ...");
    command_readWinbondNorFlash(start_addr_str, size_str);
    dlog_info("Nor flash read end");

    dlog_info("Nor flash write start ...");
    if (buf)
    {
        NOR_FLASH_WriteByteBuffer(start_addr, buf, size);
    }
    dlog_info("Nor flash write end");

    dlog_info("Nor flash read start ...");
    command_readWinbondNorFlash(start_addr_str, size_str);
    dlog_info("Nor flash read end");

    dlog_info("Nor flash block erase start ...");
    NOR_FLASH_EraseBlock(start_addr);
    dlog_info("Nor flash block erase end");    

    dlog_info("Nor flash read start ...");
    command_readWinbondNorFlash(start_addr_str, size_str);
    dlog_info("Nor flash read end");

    dlog_info("Nor flash write start ...");
    if (buf)
    {
        NOR_FLASH_WriteByteBuffer(start_addr, buf, size);
    }
    dlog_info("Nor flash write end");

    dlog_info("Nor flash read start ...");
    command_readWinbondNorFlash(start_addr_str, size_str);
    dlog_info("Nor flash read end");

#if 0
    dlog_info("Nor flash chip erase start ...");
    NOR_FLASH_EraseChip();
    dlog_info("Nor flash chip erase end");

    dlog_info("Nor flash read start ...");
    command_readWinbondNorFlash(start_addr_str, size_str);
    dlog_info("Nor flash read end");
#endif

    if (buf)
    {
        free(buf);
        buf = 0;
    }

    dlog_info("test finished!");
}


