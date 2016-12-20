/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_sys_ctl.h
Description: The external HAL APIs to use the I2C controller.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/12/20
History: 
        0.0.1    2016/12/20    The initial version of hal_sys_ctl.h
*****************************************************************************/

#ifndef __HAL_SYS_CTL_H__
#define __HAL_SYS_CTL_H__

#include <stdint.h>
#include "hal_ret_type.h"

/**
* @brief  The CPU clock initialization function.
* @param  u16_cpu0Clk                   CPU0 clock in MHz.
*         u16_cpu1Clk                   CPU1 clock in MHz.
*         u16_cpu2Clk                   CPU2 clock in MHz.
* @retval HAL_OK                        means the CPU clock initializtion is well done.
*         HAL_SYS_CTL_ERR_SET_CPU_CLK   means some error happens in the CPU clock initializtion.
* @note   This API need be called only once by CPU0.
*/

HAL_RET_T HAL_SYS_CTL_InitCpuClk(uint16_t u16_cpu0Clk, uint16_t u16_cpu1Clk, uint16_t u16_cpu2Clk);

/**
* @brief  The FPU enable function.
* @param  u8_fpuEnable                0 - disable FPU access.
*                                     1 - enable FPU access.
* @retval HAL_OK                      means the FPU is enabled or disabled well.
*         HAL_SYS_CTL_ERR_FPU_ENABLE  means some error happens in the initializtion.
* @note   If not call this API, the default FPU is disabled.
*/

HAL_RET_T HAL_SYS_CTL_FpuEnable(uint8_t u8_fpuEnable);

/**
* @brief  The system controller initial function.
* @param  None                
* @retval HAL_OK                means the system controller is initialized well.
*         HAL_SYS_CTL_ERR_INIT  means some error happens in the system controller init.
* @note   This function must be called when the system starts.
*/

HAL_RET_T HAL_SYS_CTL_Init(void);

#endif

