#include <stddef.h>
#include <stdint.h>
#include "quad_spi_ctl.h"
#include "test_quadspi.h"
#include "debuglog.h"

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
    }
    
    dlog_info("QUAD SPI data test finished\n");
}

