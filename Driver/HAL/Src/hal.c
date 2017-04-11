/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal.c
Description: The external HAL APIs for common driver functions.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/12/20
History: 
        0.0.1    2016/12/20    The initial version of hal.c
*****************************************************************************/

#include <stdint.h>
#include "systicks.h"
#include "hal_ret_type.h"
#include "hal.h"
#include "dma.h"

/**
* @brief  The hal delay function.
* @param  u32_ms    the delay time value in millisecond unit.               
* @retval HAL_OK    means the delay function is well done.
* @note   This function must be called when the system starts.
*/

HAL_RET_T HAL_Delay(uint32_t u32_ms)
{
    ar_osDelay(u32_ms);
}

__attribute__((weak)) void ar_osDelay(uint32_t u32_ms)
{
    SysTicks_DelayMS(u32_ms);
}


