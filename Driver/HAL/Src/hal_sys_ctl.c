/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_sys_ctl.c
Description: The external HAL APIs to set system controller.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/12/20
History: 
        0.0.1    2016/12/20    The initial version of hal_sys_ctl.c
*****************************************************************************/

#include "fpu.h"
#include "hal_sys_ctl.h"
#include "hal_ret_type.h"

/**
* @brief  The CPU clock set function.
* @param  u16_cpu0Clk                   CPU0 clock in MHz.
*         u16_cpu1Clk                   CPU1 clock in MHz.
*         u16_cpu2Clk                   CPU2 clock in MHz.
* @retval HAL_OK                        means the CPU clock initializtion is well done.
*         HAL_SYS_CTL_ERR_SET_CPU_CLK   means some error happens in the CPU clock initializtion.
* @note   This API need be called only once by CPU0.
*/

HAL_RET_T HAL_SYS_CTL_SetCpuClk(uint16_t u16_cpu0Clk, uint16_t u16_cpu1Clk, uint16_t u16_cpu2Clk)
{
    return HAL_OK;
}

/**
* @brief  The FPU enable function.
* @param  u8_fpuEnable                0 - disable FPU access.
*                                     1 - enable FPU access.
* @retval HAL_OK                      means the FPU is enabled.
*         HAL_SYS_CTL_ERR_FPU_ENABLE  means some error happens in the init period.
* @note   If not call this API, the default FPU is disabled.
*/

HAL_RET_T HAL_SYS_CTL_FpuEnable(uint8_t u8_fpuEnable)
{
    return HAL_OK;
}

/**
* @brief  The system controller initial function.
* @param  None                
* @retval HAL_OK                means the system controller is initialized well.
*         HAL_SYS_CTL_ERR_INIT  means some error happens in the system controller init.
* @note   This function must be called when the system starts.
*/

HAL_RET_T HAL_SYS_CTL_Init(void)
{
    return HAL_OK;
}


