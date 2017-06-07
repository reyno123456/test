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

#ifdef __cplusplus
extern "C"
{
#endif

#include <stdint.h>
#include "hal_ret_type.h"


typedef enum
{
    ENUM_HAL_SRAM_DATA_PATH_NORMAL    = 0,
    ENUM_HAL_SRAM_DATA_PATH_REVERSE   = 1,
} ENUM_HAL_SRAM_DATA_PATH;


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
void HAL_SRAM_ReceiveVideoConfig(ENUM_HAL_SRAM_DATA_PATH e_dataPathReverse);

/**
* @brief  Enable Sky Video Bypass Function
* @param  e_sramVideoCh                                     indicate which video channel to use
* @retval   HAL_OK                                               reset buffer success
*               HAL_SRAM_ERR_CHANNEL_INVALID      the input channel number is not correct
* @note  
*/
HAL_RET_T HAL_SRAM_EnableSkyBypassVideo(ENUM_HAL_SRAM_VIDEO_CHANNEL e_sramVideoCh);

/**
* @brief  Disable Sky Video Bypass Function
* @param  e_sramVideoCh                                     indicate which video channel to use
* @retval   HAL_OK                                               reset buffer success
*               HAL_SRAM_ERR_CHANNEL_INVALID      the input channel number is not correct
* @note  
*/
HAL_RET_T HAL_SRAM_DisableSkyBypassVideo(ENUM_HAL_SRAM_VIDEO_CHANNEL e_sramVideoCh);


/**
* @brief  open the video output to PC or PAD.
* @param  void
* @retval   void
* @note  
*/
void HAL_SRAM_OpenVideo(void);


/**
* @brief  close the video output to PC or PAD.
* @param  void
* @retval   void
* @note  
*/
void HAL_SRAM_CloseVideo(void);

void HAL_SRAM_CheckChannelTimeout(void);

#ifdef __cplusplus
}
#endif

#endif


