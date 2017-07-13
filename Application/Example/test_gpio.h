
#ifndef _TEST_GPIO_
#define _TEST_GPIO_

void command_TestGpioNormal(uint8_t *gpionum, uint8_t *highorlow);
void command_TestGpioNormal2(uint8_t u8_GpioNum, uint8_t u8_GpioValue);
void command_TestGpioNormalRange(uint8_t *gpionum1, uint8_t *gpionum2, uint8_t *highorlow);
void command_TestGpioInterrupt(uint8_t *gpionum, uint8_t *inttype, uint8_t *polarity);

#endif
