/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_dma.h
Description: The external HAL APIs to use the I2C controller.
Author: Artosy Software Team
Version: 0.0.1
Date: 2017/1/4
History: 
        0.0.1    2016/12/20    The initial version of hal_dma.h
*****************************************************************************/


#ifndef __HAL_DMA___
#define __HAL_DMA___


#include <stdint.h>
#include "hal_ret_type.h"


/** 
 * @brief   Start the DMA Transfer
 * @param   u32_srcAddress: The source memory Buffer address
 * @param   u32_dstAddress: The destination memory Buffer address
 * @param   u32_dataLength: The length of data to be transferred from source to destination
 * @return  none
 */
void HAL_DMA_Start(uint32_t u32_srcAddress, uint32_t u32_dstAddress, uint32_t u32_dataLength);


#endif
