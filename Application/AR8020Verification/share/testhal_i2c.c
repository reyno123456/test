
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "hal_ret_type.h"
#include "systicks.h"
#include "hal_i2c.h"
#include "testhal_i2c.h"
#include "debuglog.h"

void testhal_24c256_write(unsigned char i2c_num,unsigned char value);
void testhal_24c256_read(unsigned char i2c_num);

void commandhal_Test24C256Write(char* i2c_num_str,char* i2c_value)
{
    testhal_24c256_write(strtoul(i2c_num_str, NULL, 0),strtoul(i2c_value, NULL, 0));
}

void commandhal_Test24C256Read(char* i2c_num_str)
{
    dlog_info("tt %s", i2c_num_str);
    testhal_24c256_read(strtoul(i2c_num_str, NULL, 0));
}

void testhal_24c256_write(unsigned char i2c_num,unsigned char value)
{
    HAL_I2C_MasterInit(i2c_num, TAR_24C256_ADDR, HAL_I2C_FAST_SPEED);
    
    dlog_info("i2c_num %d I2C_Initial finished!\n", i2c_num);

    unsigned char data_src1[6] = {0x00,0x00,61, 62, 63, 64};
    unsigned char data_src2[6] = {0x00,0x04,65, 66, 67, 68};

    HAL_I2C_MasterWriteData(i2c_num, TAR_24C256_ADDR, data_src1, 6);
    SysTicks_DelayMS(20);
    HAL_I2C_MasterWriteData(i2c_num, TAR_24C256_ADDR, data_src2, 6);

    dlog_info("I2C_Master_Write_Data finished!\n");
}

void testhal_24c256_read(unsigned char i2c_num)
{
    unsigned short rd_start_addr = 0;
    unsigned int size = 8;
    
    HAL_I2C_MasterInit(i2c_num, TAR_24C256_ADDR, HAL_I2C_FAST_SPEED);
    
    dlog_info("i2c_num %d I2C_Initial finished!\n", i2c_num);

    int i;
    unsigned char data_chk[8];

    memset((void *)data_chk, 0, sizeof(data_chk));
    HAL_I2C_MasterReadData(i2c_num, TAR_24C256_ADDR, (uint8_t*)&rd_start_addr, 2, data_chk, size);
    for(i = 0; i < size; i++)
    {
        dlog_info("%d, data_chk = %d\n", rd_start_addr+i, data_chk[i]);
        dlog_output(100);
    }
    dlog_info("I2C_Master_Read_Data finished!\n");
}