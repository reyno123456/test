#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include "interrupt.h"
#include "debuglog.h"
#include "gpio.h"
#include "debuglog.h"
#include "timer.h"
#include "test_simulatepwm.h"
static init_timer_st g_stSimulateTimer;
static pwm_handle_st timer_queue[20];

static uint32_t g_u32TimCount = 0;
static uint32_t g_u32Index = 0;
static uint32_t g_ACount[5][2];

void TIMPWM_IRQHandler(void)
{
    TIM_ClearNvic(g_stSimulateTimer);
    g_u32TimCount++;
    /*if(0 == g_u32TimCount%2)
    {
        GPIO_SetPin(64,1);
    }
    else
    {
        GPIO_SetPin(64,0);
    }   */ 
}

void command_TestSimulategpio(uint8_t gpionum)
{
    uint8_t u8_GpioNum =gpionum;
    GPIO_SetMode(u8_GpioNum, GPIO_MODE_1);
    GPIO_SetPinDirect(u8_GpioNum, GPIO_DATA_DIRECT_OUTPUT);
    GPIO_SetPinCtrl(u8_GpioNum, GPIO_CTRL_SOFTWARE);    
}

void command_testSimulateTimer(void)
{    
    dlog_info("command_testSimulateTimer");
    memset((void *)&g_stSimulateTimer, 0, sizeof(init_timer_st));

    g_stSimulateTimer.base_time_group = 0;
    g_stSimulateTimer.time_num = 3;
    g_stSimulateTimer.ctrl |= TIME_ENABLE | USER_DEFINED;

    TIM_RegisterTimer(g_stSimulateTimer, 10);  
    reg_IrqHandle(TIMER_INTR00_VECTOR_NUM + g_stSimulateTimer.base_time_group*8 + g_stSimulateTimer.time_num, TIMPWM_IRQHandler);
    
    INTR_NVIC_EnableIRQ(TIMER_INTR00_VECTOR_NUM + g_stSimulateTimer.base_time_group*8 + g_stSimulateTimer.time_num);
    TIM_StartTimer(g_stSimulateTimer);
}

static uint32_t find_null_timer(void)
{
    uint32_t i = 0;
    
    for(i=0; i<20; i++)
    {
        if((0 == timer_queue[i].count[0]) && (0 == timer_queue[i].count[1]))
        {
            return i;
        }
    }
    return -1;

}

uint32_t add_timer(pwm_handle_st *tmp)
{
    dlog_info("add_timer");
    uint32_t tmp_num = 0;
    uint32_t tmp_time = 0;
    if(NULL == tmp->function)
    {                
        dlog_info("NULL == tmp->function");
        return -1; 
    }
    tmp_num = find_null_timer();
    if(-1 == tmp_num)
    {
        dlog_info("TIMER_FULL");
        dlog_output(100);
        return TIMER_FULL;
    }

    memset(&(timer_queue[tmp_num]), 0, sizeof(pwm_handle_st));
    memcpy(&(timer_queue[tmp_num]),tmp,sizeof(pwm_handle_st)); 
    
    timer_queue[tmp_num].base_time = g_u32TimCount;   
    tmp_time = timer_queue[tmp_num].base_time + tmp->count[tmp->polarity]; 
   
    if(tmp_time < timer_queue[tmp_num].base_time)
        timer_queue[tmp_num].overflow = 0xffffffffUL - timer_queue[tmp_num].base_time;
    dlog_info("add_timer parament %d %d %d\n",tmp_num,tmp->base_time,tmp->count[tmp->polarity]);
    return TIMER_SUCCESS;
}
/*
uint32_t del_timer(uint32_t timer_us)
{
    uint32_t i = 0;
    for(i=1; i<9; i++)
    {
        if(timer_queue[i].count == timer_us)
        {
            memset(&(timer_queue[i]), 0, sizeof(pwm_handle_st));
            return TIMER_SUCCESS; 
        }
    }
    return  TIMER_NOT_TIME;
}
 */
void run_timer(void)
{

    uint32_t tmp_time = 0;
    uint32_t time = g_u32TimCount;
    uint32_t i = 0;
        
    for(i=0; i<20; i++)
    {
        if(timer_queue[i].base_time > time)
        {
            timer_queue[i].base_time = time;
        }

        time +=timer_queue[i].overflow;
        if((timer_queue[i].count[timer_queue[i].polarity] + timer_queue[i].base_time < time) && (0 != timer_queue[i].count[timer_queue[i].polarity]))
        {            
            /*if(timer_queue[i].polarity ==1)
            {
                g_u32Index++;
                timer_queue[i].count[0]=g_ACount[g_u32Index%5][0];
                timer_queue[i].count[1]=g_ACount[g_u32Index%5][1];
            }*/
            /*if(g_u32Index>0xfffffff)
            {
                g_u32Index =0;
            }*/

            timer_queue[i].base_time = time;
            timer_queue[i].polarity = timer_queue[i].polarity%2;
            timer_queue[i].polarity = (timer_queue[i].polarity == 1)?0:1;
            tmp_time = timer_queue[i].base_time + timer_queue[i].count[timer_queue[i].polarity]; 
            if(tmp_time < time)
            {   
                timer_queue[i].overflow = 0xffffffffUL - timer_queue[i].base_time;
            }
            else
            {
                timer_queue[i].overflow = 0;           
            }
            (*(timer_queue[i].function))(timer_queue[i].data,timer_queue[i].data1[timer_queue[i].polarity]);
            
            
           
        }
    }
}
void command_TestSimulatePwm(void)
{
    pwm_handle_st tmp;
    uint32_t i = 0;
    memset(&timer_queue,0,sizeof(pwm_handle_st)*10);
    
    command_testSimulateTimer();

    memset(&tmp,0,sizeof(pwm_handle_st));
    
    tmp.count[0] =10;
    tmp.count[1] =20;
    tmp.data = 64;
    tmp.data1[0] = 1;
    tmp.data1[2] = 0;
    tmp.reload =1;
    tmp.function = GPIO_SetPin;
    g_u32Index =0;
    command_TestSimulategpio(64);
    add_timer(&tmp);
/*
    g_ACount[0][0] = 50;
    g_ACount[0][1] = 200;
    g_ACount[1][0] = 80;
    g_ACount[1][1] = 170;
    g_ACount[2][0] = 100;
    g_ACount[2][1] = 150;
    g_ACount[3][0] = 120;
    g_ACount[3][1] = 130;
    g_ACount[4][0] = 150;
    g_ACount[4][1] = 100;
    for(i=90;i<100;i++)
    {
        tmp.data = i;
        command_TestSimulategpio(i);
        add_timer(&tmp);
    }
    */


    while(1)
    {
       run_timer(); 
       dlog_output(100);
    }
}