/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal.h
Description: The external HAL APIs for common functions.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/12/20
History: 
        0.0.1    2016/12/20    The initial version of hal.h
*****************************************************************************/

#ifndef __HAL_H__
#define __HAL_H__

#include "hal_ret_type.h"

/**
* @brief  The hal delay function.
* @param  u32_ms    the delay time value in millisecond unit.               
* @retval HAL_OK    means the delay function is well done.
* @note   This function must be called when the system starts.
*/

HAL_RET_T HAL_Delay(uint32_t u32_ms);

#endif

