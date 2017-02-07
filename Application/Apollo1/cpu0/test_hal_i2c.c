#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "debuglog.h"
#include "hal_ret_type.h"
#include "hal_i2c.h"
#include "test_hal_i2c.h"

static ENUM_HAL_I2C_COMPONENT e_i2cComponent;
static uint16_t u16_i2cAddr;
//static uint8_t u8_i2cAddr[2];

void command_TestHalI2cInit(uint8_t *ch, uint8_t *i2c_addr, uint8_t *speed)
{
    uint32_t u32_ch = strtoul(ch, NULL, 0);
    uint32_t u32_addr = (uint16_t)strtoul(i2c_addr, NULL, 0);
    uint32_t u32_speed = strtoul(speed, NULL, 0);

    e_i2cComponent = (ENUM_HAL_I2C_COMPONENT)u32_ch;
    u16_i2cAddr = (uint16_t)u32_addr;

    HAL_I2C_MasterInit(e_i2cComponent,  u16_i2cAddr, (ENUM_HAL_I2C_SPEED)u32_speed);

    dlog_info("e_i2cComponent:%d i2cAddr:0x%x speed:%d",
               e_i2cComponent,  u16_i2cAddr, u32_speed);
}

void command_TestHalI2cWrite(uint8_t *addr_data)
{
    uint8_t u8_addr_data[3];
    uint32_t u32_data = strtoul(addr_data, NULL, 0);
    
    u8_addr_data[0] = (u32_data >> 16)  & 0xFF; 
    u8_addr_data[1] = (u32_data >> 8)  & 0xFF; 
    u8_addr_data[2] = (u32_data >> 0)  & 0xFF; 

    HAL_I2C_MasterWriteData(e_i2cComponent, 
                            u16_i2cAddr,
                            u8_addr_data,
                            3);

    dlog_info("e_i2cComponent:%d i2cAddr:0x%x subAddr:0x%x%x w_data:0x%x",
               e_i2cComponent,  u16_i2cAddr, u8_addr_data[0], u8_addr_data[1], u8_addr_data[2]);
}

void command_TestHalI2cRead(uint8_t *sub_addr)
{
    uint8_t u8_subAddr[2];
    uint8_t u8_rdata;
    uint32_t u32_data = strtoul(sub_addr, NULL, 0);

    u8_subAddr[1] = (u32_data >> 8) & 0xFF;
    u8_subAddr[0] = (u32_data >> 0) & 0xFF;

    HAL_I2C_MasterReadData(e_i2cComponent, 
                           u16_i2cAddr,
                           u8_subAddr,
                           2,
                           &u8_rdata,
                           1);


    dlog_info("e_i2cComponent:%d i2cAddr:0x%x subAddr:0x%x%x r_data:0x%x",
               e_i2cComponent,  u16_i2cAddr, u8_subAddr[0], u8_subAddr[1], u8_rdata);

} 
