/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal.c
Description: The external HAL APIs for common driver functions.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/1/4
History: 
        0.0.1    2016/1/4    The initial version of hal_dma.c
*****************************************************************************/

#include <stdint.h>
#include "systicks.h"
#include "hal_ret_type.h"
#include "hal.h"
#include "dma.h"
#include "debuglog.h"
#include "cmsis_os.h"
#include "hal_dma.h"

/** 
 * @brief   Start the DMA Transfer
 * @param   u32_srcAddress: The source memory Buffer address
 * @param   u32_dstAddress: The destination memory Buffer address
 * @param   u32_dataLength: The length of data to be transferred from source to destination
 * @return  none
 */
HAL_RET_T HAL_DMA_Start(uint32_t u32_srcAddress, uint32_t u32_dstAddress, uint32_t u32_dataLength, 
					ENUM_DMA_chan u8_channel, ENUM_DMA_TransferType e_transType)
{
	int32_t channel = 0;
	channel = DMA_Init(u8_channel,7);
	if (channel < 0)
	{
		dlog_info("No enough Channel for transfer!\n");
		return HAL_FALSE;
	}  
	else 
	{
		dlog_info("channel = %d\n", channel);
	}

	DMA_transfer(u32_srcAddress, u32_dstAddress, u32_dataLength, channel, e_transType);

	// while( DMA_getStatus(channel)  ==  0)
	// {
	//	HAL_Delay(1);
	// }
	return HAL_TRUE;
}

HAL_RET_T HAL_DMA_init(void)
{
	DMA_initIRQ();
	
	return HAL_TRUE;
}

HAL_RET_T HAL_DMA_forDriverTest(uint32_t u32_srcAddress, uint32_t u32_dstAddress, uint32_t u32_dataLength)
{
	DMA_forDriverTransfer(u32_srcAddress, u32_dstAddress, u32_dataLength, DMA_blockTimer, 10);
	
	return HAL_TRUE;
}
