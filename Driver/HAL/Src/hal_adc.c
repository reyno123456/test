/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_adc.h
Description: The external HAL APIs to use the I2C controller.
Author: Artosy Software Team
Version: 0.0.1
Date: 2017/1/23
History: 
        0.0.1    2017/1/23    The initial version of hal_adc.h
*****************************************************************************/


#include <stdint.h>
#include "reg_rw.h"
#include "hal_adc.h"

/** 
 * @brief   read the ADC value from the input
 * @param   channel:         select the channel x input into AR8020
 * sample rate = 25MHz/12 = 2.0833MHz;
 * Sample interval = 0.48us;
 * @return  the digital value of AD
 */
uint32_t HAL_ADC_Read(uint8_t channel)
{
	uint32_t u32_SarAddr = 0x40B000EC;
	uint32_t u32_dateAddr = 0x40B000F4;
	Reg_Write32(u32_SarAddr, channel << 2);
	return Reg_Read32(u32_dateAddr);
}
