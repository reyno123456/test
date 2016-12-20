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

typedef enum
{
    HAL_HDMI_RX_0 = 0,
    HAL_HDMI_RX_1,
} ENUM_HAL_HDMI_RX;

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
* @param  pu32_width          The pointer to the video width value.
* @param  pu32_hight          The pointer to the video hight value.
* @param  pu32_framterate     The pointer to the video framerate value.
* @retval HAL_OK                            means the HDMI RX init is well done.
*         HAL_HDMI_RX_ERR_GET_VIDEO_FORMAT  means some error happens in the HDMI RX init.
* @note   None.
*/

HAL_RET_T HAL_HDMI_RX_GetVideoFormat(ENUM_HAL_HDMI_RX e_hdmiIndex, 
                                     uint32_t *pu32_width, 
                                     uint32_t *pu32_hight, 
                                     uint32_t *pu32_framterate);

#endif

