#ifndef TEST_I2C_ADV7611_H
#define TEST_I2C_ADV7611_H

void test_24c256(void);
void command_initADV7611(void);
void command_readSDV7611(char *slvAddr, char *regAddr);
void command_writeSDV7611(char *slvAddr, char *regAddr, char *regVal);
void command_readADV7611VideoFormat(uint32_t* widthPtr, uint32_t* hightPtr, uint32_t* framteratePtr);

#endif
