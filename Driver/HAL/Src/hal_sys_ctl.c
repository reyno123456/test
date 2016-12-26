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

#include <string.h>
#include "fpu.h"
#include "cpu_info.h"
#include "pll_ctrl.h"
#include "bb_spi.h"
#include "inter_core.h"
#include "systicks.h"
#include "debuglog.h"
#include "hal_sys_ctl.h"
#include "hal_ret_type.h"

static STRU_HAL_SYS_CTL_CONFIG s_st_defHalSysCtlCfg =
{
    .u16_cpu0cpu1Clk       = CPU0_CPU1_CORE_PLL_CLK,
    .u16_cpu2Clk           = CPU2_CORE_PLL_CLK,
    .u8_fpuEnable          = 1,
    .u16_sysTickIntervalUs = 1000,
};

/**
* @brief  The CPU clock set function.
* @param  u16_cpu0cpu1Clk               CPU0 and CPU1 clock in MHz.
*         u16_cpu2Clk                   CPU2 clock in MHz.
* @retval HAL_OK                        means the CPU clock initializtion is well done.
*         HAL_SYS_CTL_ERR_SET_CPU_CLK   means some error happens in the CPU clock initializtion.
* @note   This API need be called only once by CPU0.
*/

HAL_RET_T HAL_SYS_CTL_SetCpuClk(uint16_t u16_cpu0cpu1Clk, uint16_t u16_cpu2Clk)
{
    if (CPUINFO_GetLocalCpuId() == ENUM_CPU0_ID)
    {
        BB_SPI_init();
        PLLCTRL_SetCoreClk(u16_cpu0cpu1Clk, ENUM_CPU0_ID);
        PLLCTRL_SetCoreClk(u16_cpu2Clk, ENUM_CPU2_ID);
        return HAL_OK;
    }
    else
    {
        return HAL_SYS_CTL_ERR_SET_CPU_CLK;
    }
}

/**
* @brief  The FPU enable function.
* @param  u8_fpuEnable                0 - disable FPU access.
*                                     1 - enable FPU access.
* @retval HAL_OK                      means the FPU is enabled.
*         HAL_SYS_CTL_ERR_FPU_ENABLE  means some error happens in the init period.
* @note   The default FPU is enabled by HAL_SYS_CTL_Init. 
*/

HAL_RET_T HAL_SYS_CTL_FpuEnable(uint8_t u8_fpuEnable)
{
    if (u8_fpuEnable == 1)
    {
        FPU_AccessEnable();
    }
    else if (u8_fpuEnable == 0)
    {
        FPU_AccessDisable();
    }
    
    return HAL_OK;
}

/**
* @brief  The system tick init function.
* @param  u32_sysTick                System tick count to create tick interrupt.
* @retval HAL_OK                         means the FPU is enabled.
*         HAL_SYS_CTL_ERR_SYS_TICK_INIT  means some error happens in the init period.
* @note   The tick interrupt interval is set to 1ms by HAL_SYS_CTL_Init.
*/

HAL_RET_T HAL_SYS_CTL_SysTickInit(uint32_t u32_sysTickCount)
{
    SysTicks_Init(u32_sysTickCount);
    
    return HAL_OK;
}

/**
* @brief  The system controller configure get function.
* @param  None                
* @retval HAL_OK    means the system controller configure pointer can be used by application.
* @note   The right way to use this function:
*         1. Call the function HAL_SYS_CTL_GetConfig to get the current system controller configuration.
*         2. Change some paramter if you want.
*         3. Call the function HAL_SYS_CTL_Init to do system controller init.
*/

HAL_RET_T HAL_SYS_CTL_GetConfig(STRU_HAL_SYS_CTL_CONFIG *pst_halSysCtlCfg)
{
    if (pst_halSysCtlCfg != NULL)
    {
        memcpy(pst_halSysCtlCfg, &s_st_defHalSysCtlCfg, sizeof(STRU_HAL_SYS_CTL_CONFIG));
    }
    
    return HAL_OK;
}

/**
* @brief  The system controller initial function.
* @param  None                
* @retval HAL_OK                means the system controller is initialized well.
*         HAL_SYS_CTL_ERR_INIT  means some error happens in the system controller init.
* @note   This function must be called when the system starts.
*         The right way to use this function:
*         1. Call the function HAL_SYS_CTL_GetConfig to get the current system controller configuration.
*         2. Change some paramter if you want.
*         3. Call the function HAL_SYS_CTL_Init to do system controller init.
*/

HAL_RET_T HAL_SYS_CTL_Init(STRU_HAL_SYS_CTL_CONFIG *pst_usrHalSysCtlCfg)
{
    uint16_t u16_pllClk = 0;
    uint32_t u32_tickCnt = 0;

    STRU_HAL_SYS_CTL_CONFIG *pst_halSysCtlCfg = NULL;

    if (pst_usrHalSysCtlCfg == NULL)
    {
        pst_halSysCtlCfg = &s_st_defHalSysCtlCfg;
    }
    else
    {
        pst_halSysCtlCfg = pst_usrHalSysCtlCfg;
    }

    // Inter core SRAM init
    InterCore_Init();

    // Default clock: CPU0 and CPU1 200M; CPU2 166M.
    HAL_SYS_CTL_SetCpuClk(pst_halSysCtlCfg->u16_cpu0cpu1Clk, pst_halSysCtlCfg->u16_cpu2Clk);

    // Wait till the PLL ready
    while (PLLCTRL_CheckCoreClkReady() == 0) {;}

    // Enable FPU access
    HAL_SYS_CTL_FpuEnable(pst_halSysCtlCfg->u8_fpuEnable);

    // Default system tick: 1ms.
    PLLCTRL_GetCoreClk(&u16_pllClk, CPUINFO_GetLocalCpuId());
    u32_tickCnt = ((uint32_t)u16_pllClk) * 1000 * 1000 / pst_halSysCtlCfg->u16_sysTickIntervalUs;
    HAL_SYS_CTL_SysTickInit(u32_tickCnt);

    // Delay to PLL stable
    SysTicks_DelayMS(10);

    dlog_info("HAL System controller init done \n");
    
    return HAL_OK;
}


