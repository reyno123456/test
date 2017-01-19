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

/** 
 * @brief   Start the DMA Transfer
 * @param   u32_srcAddress: The source memory Buffer address
 * @param   u32_dstAddress: The destination memory Buffer address
 * @param   u32_dataLength: The length of data to be transferred from source to destination
 * @return  none
 */
void HAL_DMA_Start(uint32_t u32_srcAddress, uint32_t u32_dstAddress, uint32_t u32_dataLength, ENUM_Chan u8_channel, ENUM_TransferType e_transType)
{
    int32_t u32_ret = 0;
    u32_ret = DMA_Init(u8_channel,7);
    if (u32_ret < 0)
    {
    	dlog_info("No enough Channel for transfer!\n");
    }
    DMA_transfer(u32_srcAddress, u32_dstAddress, u32_dataLength, u32_ret, e_transType);
}


