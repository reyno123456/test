/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_timer.c
Description: this module contains the helper fucntions necessary to control the general
             purpose timer block
Author: Artosy Software Team
Version: 0.0.1
Date: 2016/12/19
History:
         0.0.1    2016/12/19    The initial version of hal_timer.c
*****************************************************************************/


#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "interrupt.h"
#include "hal_ret_type.h"
#include "timer.h"
#include "hal_timer.h"
#include "debuglog.h"


static void HAL_TIMER_VectorFunctionN0(void);
static void HAL_TIMER_VectorFunctionN1(void);
static void HAL_TIMER_VectorFunctionN2(void);
static void HAL_TIMER_VectorDefault(void);

static void (*g_pv_TiemrVectorNumArray[3])(void)={  HAL_TIMER_VectorFunctionN0,
                                                    HAL_TIMER_VectorFunctionN1,
                                                    HAL_TIMER_VectorFunctionN2};


static void (*g_pv_TimerVectorListArray[3][8])(void)= {{HAL_TIMER_VectorDefault,HAL_TIMER_VectorDefault,HAL_TIMER_VectorDefault,HAL_TIMER_VectorDefault,HAL_TIMER_VectorDefault,HAL_TIMER_VectorDefault,HAL_TIMER_VectorDefault},
                                                        {HAL_TIMER_VectorDefault,HAL_TIMER_VectorDefault,HAL_TIMER_VectorDefault,HAL_TIMER_VectorDefault,HAL_TIMER_VectorDefault,HAL_TIMER_VectorDefault,HAL_TIMER_VectorDefault},
                                                        {HAL_TIMER_VectorDefault,HAL_TIMER_VectorDefault,HAL_TIMER_VectorDefault,HAL_TIMER_VectorDefault,HAL_TIMER_VectorDefault,HAL_TIMER_VectorDefault,HAL_TIMER_VectorDefault}};
/**
* @brief    register timer
* @param    e_timerNum: timer number, the right number should be 0-23.
            u32_timeus: timer load count
            *fun_callBack: interrput callback
* @retval   HAL_OK                 means the registeration tiemr is well done.
*           HAL_TIMER_ERR_UNKNOWN  means the timer number error. 
* @note     none
*/
HAL_RET_T HAL_TIMER_RegisterTimer(ENUM_HAL_TIMER_Num e_timerNum, uint32_t u32_timeus, void *fun_callBack)
{
    if (e_timerNum > HAL_TIMER_NUM23)
    {
        return HAL_TIMER_ERR_UNKNOWN;
    }

    init_timer_st st_timer;
    memset(&st_timer,0,sizeof(init_timer_st));

    st_timer.base_time_group = e_timerNum/8;
    st_timer.time_num = e_timerNum%8;
    st_timer.ctrl |= TIME_ENABLE | USER_DEFINED;

    TIM_RegisterTimer(st_timer,u32_timeus);

    dlog_info(" group %d time_num %d ",st_timer.base_time_group,st_timer.time_num);   
    g_pv_TimerVectorListArray[st_timer.base_time_group][st_timer.time_num] = fun_callBack;
    reg_IrqHandle(TIMER_INTR00_VECTOR_NUM + e_timerNum, g_pv_TiemrVectorNumArray[st_timer.base_time_group]);
    //reg_IrqHandle(TIMER_INTR00_VECTOR_NUM + e_timerNum, fun_callBack);
    INTR_NVIC_EnableIRQ(TIMER_INTR00_VECTOR_NUM + e_timerNum);
    TIM_StartTimer(st_timer);
    dlog_output(100);
    return HAL_OK;
}

/**
* @brief    stop timer
* @param    e_timerNum: timer number, the right number should be 0-23.
* @retval   HAL_OK                 means the stop tiemr success.
*           HAL_TIMER_ERR_UNKNOWN  means the timer number error. 
* @note     none
*/
HAL_RET_T HAL_TIMER_Stop(ENUM_HAL_TIMER_Num e_timerNum)
{
    if (e_timerNum > HAL_TIMER_NUM23)
    {
        return HAL_TIMER_ERR_UNKNOWN;
    }

    init_timer_st st_timer;
    memset(&st_timer,0,sizeof(init_timer_st));

    st_timer.base_time_group = e_timerNum/8;
    st_timer.time_num = e_timerNum%8;
    st_timer.ctrl |= TIME_ENABLE | USER_DEFINED;

    INTR_NVIC_DisableIRQ(TIMER_INTR00_VECTOR_NUM + e_timerNum);

    INTR_NVIC_ClearPendingIRQ(TIMER_INTR00_VECTOR_NUM + e_timerNum);

    TIM_StopTimer(st_timer);
    
    return HAL_OK;
}

/**
* @brief    start timer
* @param    e_timerNum: timer number, the right number should be 0-23.
* @retval   HAL_OK                 means the start tiemr success.
*           HAL_TIMER_ERR_UNKNOWN  means the timer number error. 
* @note     none
*/
HAL_RET_T HAL_TIMER_Start(ENUM_HAL_TIMER_Num e_timerNum)
{
    if (e_timerNum > HAL_TIMER_NUM23)
    {
        return HAL_TIMER_ERR_UNKNOWN;
    }

    init_timer_st st_timer;
    memset(&st_timer,0,sizeof(init_timer_st));

    st_timer.base_time_group = e_timerNum/8;
    st_timer.time_num = e_timerNum%8;
    st_timer.ctrl |= TIME_ENABLE | USER_DEFINED;

    INTR_NVIC_EnableIRQ(TIMER_INTR00_VECTOR_NUM + e_timerNum);

    TIM_StartTimer(st_timer);
    
    return HAL_OK;
}

/**
* @brief    clear timer interrupt
* @param    e_timerNum: timer number, the right number should be 0-23.
* @retval   HAL_OK                 means the clear tiemr interrupt success.
*           HAL_TIMER_ERR_UNKNOWN  means the timer number error. 
* @note     none
*/
HAL_RET_T HAL_TIMER_ClearNvic(ENUM_HAL_TIMER_Num e_timerNum)
{
    if (e_timerNum > HAL_TIMER_NUM23)
    {
        return HAL_TIMER_ERR_UNKNOWN;
    }

    init_timer_st st_timer;
    memset(&st_timer,0,sizeof(init_timer_st));

    st_timer.base_time_group = e_timerNum/8;
    st_timer.time_num = e_timerNum%8;
    st_timer.ctrl |= TIME_ENABLE | USER_DEFINED;
    TIM_ClearNvic(st_timer);
    
    return HAL_OK;

}

uint8_t HAL_TIMER_GetIntrStatus(ENUM_HAL_TIMER_Num e_timerNum)
{
    /*if (e_timerNum > HAL_TIMER_NUM23)
    {
        return HAL_TIMER_ERR_UNKNOWN;
    }*/

    init_timer_st st_timer;
    memset(&st_timer,0,sizeof(init_timer_st));

    st_timer.base_time_group = e_timerNum/8;
    st_timer.time_num = e_timerNum%8;
    st_timer.ctrl |= TIME_ENABLE | USER_DEFINED;
    
    return TIM_IntrGetIntrStatus(st_timer);

}

static void HAL_TIMER_VectorFunctionN0(void)
{
    uint32_t u32_tmpvalue = 0;
    uint32_t i = 0;
    for(i=0; i<8; i++)
    {
        
        u32_tmpvalue = HAL_TIMER_GetIntrStatus(i);
        if(0 !=u32_tmpvalue)
        {
            //dlog_info(" %d HAL_TIMER_GetIntrStatus(%d) ",i,u32_tmpvalue);  
            HAL_TIMER_ClearNvic(i);
            (*(g_pv_TimerVectorListArray[0][i]))();   
        }        
    }

}

static void HAL_TIMER_VectorFunctionN1(void)
{
    uint32_t u32_tmpvalue = 0;
    uint32_t i = 0;
    for(i=0; i<8; i++)
    {
        
        u32_tmpvalue = HAL_TIMER_GetIntrStatus(i+8);  
        if(0 !=u32_tmpvalue)
        {
            HAL_TIMER_ClearNvic(i+8);
            (*(g_pv_TimerVectorListArray[1][i]))();   
        }
    }
}


static void HAL_TIMER_VectorFunctionN2(void)
{
    uint32_t u32_tmpvalue = 0;
    uint32_t i = 0;
    for(i=0; i<8; i++)
    {
        
        u32_tmpvalue = HAL_TIMER_GetIntrStatus(i+16);   
        if(0 !=u32_tmpvalue)
        {
            HAL_TIMER_ClearNvic(i+16);
            (*(g_pv_TimerVectorListArray[2][i]))();  
        }
    }
}


static void HAL_TIMER_VectorDefault(void)
{
    dlog_info(" default ");
    dlog_output(100);
}