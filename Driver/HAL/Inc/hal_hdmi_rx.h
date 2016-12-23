/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_hdmi_rx.h
Description: The external HAL APIs to use the HDMI RX.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/12/20
History: 
        0.0.1    2016/12/20    The initial version of hal_hdmi_rx.h
*****************************************************************************/

#ifndef __HAL_HDMI_RX_H__
#define __HAL_HDMI_RX_H__

#include <stdint.h>
#include "hal_ret_type.h"

typedef struct
{
    uint16_t u16_width;
    uint16_t u16_hight;
    uint8_t  u8_framerate;
} STRU_HDMI_RX_OUTPUT_FORMAT;

typedef struct
{
    uint8_t u8_devEnable;
    STRU_HDMI_RX_OUTPUT_FORMAT st_videoFormat;
} STRU_HDMI_RX_STATUS;

typedef enum
{
    HAL_HDMI_RX_0 = 0,
    HAL_HDMI_RX_1,
    HAL_HDMI_RX_MAX
} ENUM_HAL_HDMI_RX;

#define HDMI_RX_FORMAT_NOT_SUPPORT_COUNT_MAX 50

/**
* @brief  The HDMI RX init function.
* @param  e_hdmiIndex       The HDMI RX index number, the right number should be 0-1 and totally
*                           2 HDMI RX can be supported.
* @retval HAL_OK            means the HDMI RX init is well done.
*         HAL_I2C_ERR_INIT  means some error happens in the HDMI RX init.
* @note   None.
*/

HAL_RET_T HAL_HDMI_RX_Init(ENUM_HAL_HDMI_RX e_hdmiIndex);

/**
* @brief  The HDMI RX init function.
* @param  e_hdmiIndex         The HDMI RX index number, the right number should be 0-1 and totally
*                             2 HDMI RX can be supported.
* @param  pu16_width          The pointer to the video width value.
* @param  pu16_hight          The pointer to the video hight value.
* @param  pu8_framterate      The pointer to the video framerate value.
* @retval HAL_OK                            means the HDMI RX init is well done.
*         HAL_HDMI_RX_ERR_GET_VIDEO_FORMAT  means some error happens in the HDMI RX init.
* @note   None.
*/

HAL_RET_T HAL_HDMI_RX_GetVideoFormat(ENUM_HAL_HDMI_RX e_hdmiIndex, 
                                     uint16_t *pu16_width, 
                                     uint16_t *pu16_hight, 
                                     uint8_t  *pu8_framterate);

#endif

