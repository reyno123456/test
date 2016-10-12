#ifndef TEST_I2C_ADV7611_H
#define TEST_I2C_ADV7611_H

void command_initADV7611(void);
void command_dumpADV7611Settings(void);
void command_readSDV7611(char *slvAddr, char *regAddr);
void command_writeSDV7611(char *slvAddr, char *regAddr, char *regVal);
void command_readADV7611VideoFormat(uint32_t* widthPtr, uint32_t* hightPtr, uint32_t* framteratePtr);
void command_Test24C256Write(char* i2c_num_str);
void command_Test24C256Read(char* i2c_num_str);

#endif
