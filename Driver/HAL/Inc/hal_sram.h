/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_sram.h
Description: The external HAL APIs to use the SRAM.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/12/21
History: 
        0.0.1    2016/12/21    The initial version of hal_sram.h
*****************************************************************************/

#ifndef __HAL_SRAM_H__
#define __HAL_SRAM_H__

#include <stdint.h>
#include "hal_ret_type.h"


typedef enum
{
    HAL_SRAM_VIDEO_CHANNEL_0 = 0,
    HAL_SRAM_VIDEO_CHANNEL_1,
} ENUM_HAL_SRAM_VIDEO_CHANNEL;



/**
* @brief  Config the Buffer in SRAM to Receive Video Data from SKY.
* @param  void
* @retval   void
* @note  
*/
void HAL_SRAM_ReceiveVideoConfig(void);

/**
* @brief  Reset the SRAM Buffer to receive the next package video data.
* @param  void
* @retval   void
* @note  
*/
HAL_RET_T HAL_SRAM_ResetBuffer(ENUM_HAL_SRAM_VIDEO_CHANNEL e_sramVideoCh);

/**
* @brief  Reset the SRAM Buffer1 to receive the next package video data.
* @param  void
* @retval   void
* @note  
*/
HAL_RET_T HAL_SRAM_EnableSkyBypassVideo(ENUM_HAL_SRAM_VIDEO_CHANNEL e_sramVideoCh);

/**
* @brief  Reset the SRAM Buffer1 to receive the next package video data.
* @param  void
* @retval   void
* @note  
*/
HAL_RET_T HAL_SRAM_DisableSkyBypassVideo(ENUM_HAL_SRAM_VIDEO_CHANNEL e_sramVideoCh);


#endif


