#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#include "interrupt.h"
#include "test_gpio.h"
#include "debuglog.h"
#include "gpio.h"
#include "test_timer.h"

uint32_t volatile g_u32GpioCount = 0;
uint8_t  volatile g_u8GpioNum = 0;
uint8_t  volatile g_u8GpioType = 0;

void GPIO_IRQHandler(void)
{
    if(1 == g_u8GpioType)
    	GPIO_Intr_ClearIntr(g_u8GpioNum);
    
    g_u32GpioCount ++;
}

void command_TestGpioNormal(uint8_t *gpionum, uint8_t *highorlow)
{
	uint8_t u8_GpioNum = strtoul(gpionum, NULL, 0);
    uint8_t u8_GpioValue = strtoul(highorlow, NULL, 0);
    GPIO_SetMode(u8_GpioNum, GPIO_MODE_1);
    GPIO_SetPinDirect(u8_GpioNum, GPIO_DATA_DIRECT_OUTPUT);
	GPIO_SetPinCtrl(u8_GpioNum, GPIO_CTRL_SOFTWARE);
    GPIO_SetPin(u8_GpioNum,u8_GpioValue);	
}

void command_TestGpioNormalRange(uint8_t *gpionum1, uint8_t *gpionum2, uint8_t *highorlow)
{
	uint8_t u8_GpioNum1 = strtoul(gpionum1, NULL, 0);
	uint8_t u8_GpioNum2 = strtoul(gpionum2, NULL, 0);
    uint8_t u8_GpioValue = strtoul(highorlow, NULL, 0);
    GPIO_ModeRange(u8_GpioNum1, u8_GpioNum2, GPIO_MODE_1);
    GPIO_SetPinRange(u8_GpioNum1, u8_GpioNum2, u8_GpioValue);	
}

void command_TestGpioInterrupt(uint8_t *gpionum, uint8_t *inttype, uint8_t *polarity)
{
	uint8_t u8_GpioNum = strtoul(gpionum, NULL, 0);
	uint8_t u8_GpioIntType = strtoul(inttype, NULL, 0);
    uint8_t u8_GpioPolarity = strtoul(polarity, NULL, 0);

	g_u8GpioNum = u8_GpioNum;
	g_u8GpioType = u8_GpioIntType;
	GPIO_SetPinDirect(u8_GpioNum, GPIO_DATA_DIRECT_INPUT);
	GPIO_SetPinCtrl(u8_GpioNum, GPIO_CTRL_SOFTWARE);
	GPIO_SetMode(u8_GpioNum, GPIO_MODE_1);
	GPIO_Intr_SetPinIntrEn(u8_GpioNum, GPIO_INTEN_INTERRUPT);
	GPIO_Intr_SetPinIntrMask(u8_GpioNum, GPIO_MASK_MASK);
	GPIO_Intr_SetPinIntrType(u8_GpioNum, u8_GpioIntType);
	GPIO_Intr_SetPinIntrPol(u8_GpioNum, u8_GpioPolarity);	
	GPIO_SetPinDebounce(u8_GpioNum, GPIO_DEBOUNCE_ON);
	
	reg_IrqHandle(GPIO_INTR_N0_VECTOR_NUM + (u8_GpioNum>>5), GPIO_IRQHandler);
    INTR_NVIC_EnableIRQ(GPIO_INTR_N0_VECTOR_NUM + (u8_GpioNum>>5));
    
    while(1)
    {
    	if(0 != g_u32GpioCount)
    	{
    		dlog_info("\n gpio interrupt \n");
    		g_u32GpioCount = 0;
    	}
    }
	

}
