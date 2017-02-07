#ifndef __TEST_HAL_I2C__
#define __TEST_HAL_I2C__


void command_TestHalI2cInit(uint8_t *ch, uint8_t *i2c_addr, uint8_t *speed);
void command_TestHalI2cWrite(uint8_t *addr_data);
void command_TestHalI2cRead(uint8_t *sub_addr);

#endif
