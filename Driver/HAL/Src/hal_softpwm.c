/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_softpwm.c
Description: this module contains the helper fucntions necessary to control the general
             purpose softpwm block.softpwm use a timer to toggle gpio create pwm.
Author: Artosy Software Team
Version: 0.0.1
Date: 2016/12/19
History:
         0.0.1    2016/12/19    The initial version of hal_softpwm.c
*****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "hal_ret_type.h"
#include "hal_gpio.h"
#include "hal_timer.h"
#include "hal_softpwm.h"

STRU_SoftPwmHandle g_stPwmQueue[MAXSOFTPWM];
static ENUM_HAL_TIMER_Num g_etimerNum;

static uint32_t g_u32TimCount = 0;
uint32_t g_ACount[MAXSOFTPWM][2];

/**
* @brief    timer IRQhandler
* @param    e_timerNum: pwm number
            u32_lowus: timer load count of low
            u32_highus: timer load count of high
* @retval   none 
* @note     none
*/
static void SOFTPWM_TimerIRQHandler(uint32_t u32_vectorNum)
{
    g_u32TimCount++;
}

/**
* @brief    register tiemr and start timer
* @param    e_timerNum: pwm number
* @retval   none 
* @note     none
*/
void HAL_SOFTPWM_SetTimer(ENUM_HAL_TIMER_Num e_timerNum)
{
    g_etimerNum = e_timerNum;
    HAL_TIMER_RegisterTimer(e_timerNum, MIXMODIFYTIMEUM, SOFTPWM_TimerIRQHandler);
}

/**
* @brief    find availble position of array
* @param    e_timerNum: pwm number
* @retval   Availble Position 
* @note     none
*/
static uint8_t SOFTPWM_FindAvailblePosition(void)
{
    uint8_t i = 0;
    
    for (i=0; i<MAXSOFTPWM; i++)
    {
        if ((0 == g_stPwmQueue[i].u32_countArray[0]) && (0 == g_stPwmQueue[i].u32_countArray[1]))
        {
            return i;
        }
    }
    return -1;

}

/**
* @brief    add a simulatepwm
* @param    e_timerNum: pwm number
* @retval   -1 fail 
* @note     none
*/
uint8_t HAL_SOFTPWM_AddPwm(STRU_SoftPwmHandle *tmp)
{
    uint8_t u8_tmpNum = 0;
    uint32_t u32_tmpTime = 0;

    if (NULL == tmp->function)
    {                
        return -1; 
    }

    u8_tmpNum = SOFTPWM_FindAvailblePosition();
    if (-1 == u8_tmpNum)
    {
        return -1;
    }

    memset(&(g_stPwmQueue[u8_tmpNum]), 0, sizeof(STRU_SoftPwmHandle));
    memcpy(&(g_stPwmQueue[u8_tmpNum]),tmp,sizeof(STRU_SoftPwmHandle)); 

    g_stPwmQueue[u8_tmpNum].function = HAL_GPIO_SetPin;

    g_stPwmQueue[u8_tmpNum].u8_polarity = 1;
    g_stPwmQueue[u8_tmpNum].u32_baseTime = g_u32TimCount;
    u32_tmpTime = g_stPwmQueue[u8_tmpNum].u32_baseTime + tmp->u32_countArray[tmp->u8_polarity];
    if (u32_tmpTime < g_stPwmQueue[u8_tmpNum].u32_baseTime)
    {
        g_stPwmQueue[u8_tmpNum].u32_overFlow = 0xffffffffUL - g_stPwmQueue[u8_tmpNum].u32_baseTime;
    }   

    (*(g_stPwmQueue[u8_tmpNum].function))(g_stPwmQueue[u8_tmpNum].u8_pin,g_stPwmQueue[u8_tmpNum].u8_polarity);
    
    
    return 0;
}

/**
* @brief    while toggle pin to creat soft pwm
* @param    none
* @retval   none 
* @note     none
*/
void HAL_SOFTPWM_RunPwm(void)
{
    uint32_t tmp_time = 0;
    uint32_t time = g_u32TimCount;
    uint32_t i = 0;
        
    for (i=0; i<MAXSOFTPWM; i++)
    {
        
        if (g_stPwmQueue[i].u32_baseTime > time)
        {
            g_stPwmQueue[i].u32_baseTime = time;
        }

        time +=g_stPwmQueue[i].u32_overFlow;
        if ((g_stPwmQueue[i].u32_countArray[g_stPwmQueue[i].u8_polarity] + g_stPwmQueue[i].u32_baseTime <= time) && 
            (0 != g_stPwmQueue[i].u32_countArray[g_stPwmQueue[i].u8_polarity]))
        {            
            (*(g_stPwmQueue[i].function))(g_stPwmQueue[i].u8_pin,g_stPwmQueue[i].u8_polarity);

            if ((g_stPwmQueue[i].u8_polarity ==0) && (g_ACount[i][0] != 0))
            {
                g_stPwmQueue[i].u32_countArray[0]=g_ACount[i][0];
                g_stPwmQueue[i].u32_countArray[1]=g_ACount[i][1];
                g_ACount[i][0] = 0;
                g_ACount[i][1] = 0;
            }
            
            g_stPwmQueue[i].u32_baseTime = time;

            g_stPwmQueue[i].u8_polarity = (g_stPwmQueue[i].u8_polarity == 1)?0:1;
            tmp_time = g_stPwmQueue[i].u32_baseTime + g_stPwmQueue[i].u32_countArray[g_stPwmQueue[i].u8_polarity]; 
            if (tmp_time < time)
            {   
                g_stPwmQueue[i].u32_overFlow = 0xffffffffUL - g_stPwmQueue[i].u32_baseTime;
            }
            else
            {
                g_stPwmQueue[i].u32_overFlow = 0;           
            }
            

        }
    }
}
