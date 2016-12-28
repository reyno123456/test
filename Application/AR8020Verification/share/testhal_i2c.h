
#ifndef __TEST_HAL_I2C__
#define __TEST_HAL_I2C__

#define TAR_24C256_ADDR (0x51)

void commandhal_Test24C256Write(char* i2c_num_str,char* i2c_value);
void commandhal_Test24C256Read(char* i2c_num_str);

#endif