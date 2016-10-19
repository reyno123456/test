#include <stdint.h>
#include <stdio.h>

#include "debuglog.h"
#include "timer.h"
#include "test_timer.h"
#include "interrupt.h"


init_timer_st g_stTimer;
init_timer_st g_stPwm;
uint32_t g_u32TimCount = 0;

void TIM_IRQHandler(void)
{
    TIM_ClearNvic(g_stTimer);  
    g_u32TimCount ++;
}

void Test_TimerInit(uint32_t timer_group, uint32_t timer_num, uint32_t timer_count)
{
    dlog_info("Timer init \n");
    memset(&g_stTimer, 0, sizeof(init_timer_st));
    
    g_stTimer.base_time_group = timer_group;
    g_stTimer.time_num = timer_num;
    g_stTimer.ctrl |= TIME_ENABLE | USER_DEFINED;
                     
    g_u32TimCount = 0;
    
    TIM_RegisterTimer(g_stTimer, timer_count*1000);  
    reg_IrqHandle(TIMER_INTR00_VECTOR_NUM + timer_group*8 + timer_num, TIM_IRQHandler);
    INTR_NVIC_EnableIRQ(TIMER_INTR00_VECTOR_NUM + timer_group*8 + timer_num);
}

void command_TestTim(uint8_t *timer_group, uint8_t *timer_num, uint8_t *timer_count)
{
    uint32_t u32_TimGroup = strtoul(timer_group, NULL, 0);
    uint32_t u32_TimNum = strtoul(timer_num, NULL, 0);
    uint32_t u32_TimCount = strtoul(timer_count, NULL, 0);
    
    Test_TimerInit(u32_TimGroup, u32_TimNum, u32_TimCount);
    TIM_StartTimer(g_stTimer);
    
    while(g_u32TimCount < 5000)
    {
        if((g_u32TimCount)%1000 == 0)
            dlog_info("timer count %d \n", g_u32TimCount);
    }
   
    TIM_StopTimer(g_stTimer); 
    INTR_NVIC_DisableIRQ(TIMER_INTR00_VECTOR_NUM + u32_TimGroup*8 + u32_TimNum);
}
                                                              
void command_TestTimUsed(void)
{
    uint32_t i = 0;
    
    for(i = 0;i <8; i++)
    {
        Test_TimerInit(0, i, 1);
        TIM_StartTimer(g_stTimer);
    
        while(g_u32TimCount < 5000)
        {
            if((g_u32TimCount)%1000 == 0)
                dlog_info("timer count %d \n", g_u32TimCount);
        }   
   
        TIM_StopTimer(g_stTimer); 
        INTR_NVIC_DisableIRQ(TIMER_INTR00_VECTOR_NUM + i);
        g_u32TimCount = 0;
    }
    
    for(i = 0;i <2; i++)
    {
        Test_TimerInit(1, i, 1);
        TIM_StartTimer(g_stTimer);
    
        while(g_u32TimCount < 5000)
        {
            if((g_u32TimCount)%1000 == 0)
                dlog_info("timer count %d \n", g_u32TimCount);
        }   
   
        TIM_StopTimer(g_stTimer); 
        INTR_NVIC_DisableIRQ(TIMER_INTR00_VECTOR_NUM + 8 + i);
        g_u32TimCount = 0;
    }
}

void command_TestTimAll(void)
{
    uint32_t i = 0;
    uint32_t j = 0;
    
    for(j = 0;j <3; j++)
    {
        for(i = 0;i <8; i++)
        {   
            Test_TimerInit(0, i, 1);
            TIM_StartTimer(g_stTimer);
    
            while(g_u32TimCount < 3000)
            {
                if((g_u32TimCount)%1000 == 0)
                    dlog_info("timer count %d \n", g_u32TimCount);
            }   
   
            TIM_StopTimer(g_stTimer); 
            INTR_NVIC_DisableIRQ(TIMER_INTR00_VECTOR_NUM + i);
        }
    }
}

void Test_PwmInit(uint32_t timer_group, uint32_t timer_num, uint32_t low, uint32_t high)
{
    dlog_info("Timer init \n");
    
    memset(&g_stPwm, 0, sizeof(init_timer_st));
    
    g_stPwm.base_time_group = timer_group;
    g_stPwm.time_num = timer_num;
    g_stPwm.ctrl |= TIME_ENABLE | USER_DEFINED |TIME_PWM_ENABLE;
                     
    TIM_RegisterPwm(g_stPwm, low*1000, high*1000);  

}

void command_TestPwm(uint8_t *timer_group, uint8_t *timer_num, uint8_t *low, uint8_t *high)
{
    uint32_t u32_PwmGroup = strtoul(timer_group, NULL, 0);
    uint32_t u32_PwmNum = strtoul(timer_num, NULL, 0);
    uint32_t u32_PwmHigh = strtoul(high, NULL, 0);
    uint32_t u32_PwmLow = strtoul(low, NULL, 0);
    
    Test_PwmInit(u32_PwmGroup, u32_PwmNum, u32_PwmLow, u32_PwmHigh);
    TIM_StartPwm(g_stPwm);
    dlog_info("start pwm  group %d  num %d \n",u32_PwmGroup, u32_PwmNum);
}
                                                              
void command_TestPwmAll(void)
{
    uint32_t i = 0;
    for(i = 0;i <8; i++)
    {
        Test_PwmInit(0, i, 1, 1);
        TIM_StartPwm(g_stPwm);
    }
    
    for(i = 0;i <2; i++)
    {
        Test_PwmInit(1, i, 1, 1);
        TIM_StartPwm(g_stPwm);
    }
}
