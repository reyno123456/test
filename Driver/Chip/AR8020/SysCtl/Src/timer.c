#include "timer.h"
#include <string.h>
#include "interrupt.h"
#include "reg_rw.h"
#include "debuglog.h"
static timer_handle_st timer_queue[10];
static volatile uint32_t g_timer_us;

void run_timer(void)
{

    uint32_t tmp_time = 0;
    uint32_t time = g_timer_us;
    uint32_t i = 0;
    
    
    for(i=1; i<9; i++)
    {
        if(timer_queue[i].base_time > time)
        {
            timer_queue[i].base_time = time;
        }

        time +=timer_queue[i].overflow;
 
 /*   	if(_timer_us%100 == 0)
            dlog_info("run_timer %d\n",i);
   */ 
        if((timer_queue[i].count + timer_queue[i].base_time < time) && (0 != timer_queue[i].count))
        {
	    if(NULL == timer_queue[i].function)
	    {
                dlog_info("run_fuction\n");
                memset(&(timer_queue[i]), 0, sizeof(timer_handle_st));
	        continue; 
	    }
            (*(timer_queue[i].function))();
            
	    if(0 == timer_queue[i].reload)
            {
                memset(&(timer_queue[i]), 0, sizeof(timer_handle_st)); 
            }
            else
            {
                timer_queue[i].base_time = time;
                tmp_time = timer_queue[i].base_time + timer_queue[i].count; 
                if(tmp_time < time)
                {   
                    timer_queue[i].overflow = 0xffffffffUL - timer_queue[i].base_time;
                }
                else
                    timer_queue[i].overflow = 0;           
            }
        }
    }
}
/*
 * timer isr
 */
void timer0_isr(void)
{
    uint32_t tmp = 0;
    uint32_t i = 0;
    for(i=0; i<7; i++)
    {
        
        tmp = Reg_Read32(BASE_ADDR_TIMER0 + TIMER0_TNT_STATUS+(i*0x14));  
        if(0 !=tmp)
        {
            g_timer_us++;
            tmp = Reg_Read32(BASE_ADDR_TIMER0 + TMRNEOI_0+(i*0x14));  
        }
    }
    	if(g_timer_us%100 == 0)
            dlog_info("run_timer %d\n",g_timer_us);
}
void timer1_isr(void)
{
    uint32_t tmp = 0;
    uint32_t i = 0;
    for(i=0; i<7; i++)
    {
        
        tmp = Reg_Read32(BASE_ADDR_TIMER1 + TIMER1_TNT_STATUS+(i*0x14));  
        if(0 !=tmp)
        {
            g_timer_us++;
            tmp = Reg_Read32(BASE_ADDR_TIMER1 + TMRNEOI_0+(i*0x14));  
        }        
    }
}
void timer2_isr(void)
{
    uint32_t tmp = 0;
    uint32_t i = 0;
    for(i=0; i<7; i++)
    {
        
        tmp = Reg_Read32(BASE_ADDR_TIMER2 + TIMER2_TNT_STATUS+(i*0x14));  
        if(0 !=tmp)
        {
            g_timer_us++;
            tmp = Reg_Read32(BASE_ADDR_TIMER2 + TMRNEOI_0+(i*0x14));  
        }        
    }
}
/*
 * timer process
 */
void timer_config(
                uint32_t addr,          // timer base addr
                uint32_t ctrl,          // ctrl is the value to set Control register
                uint32_t ctrl_offset,   // Control register addr offset
                uint32_t cnt1,          // count1 value
                uint32_t cnt1_offset,   // Count register1 addr offset
                uint32_t cnt2,          // count2 value
                uint32_t cnt2_offset    // Count register2 addr offset
                ){  
    
    Reg_Write32(addr + ctrl_offset,  ctrl & 0xfffffffe);  // set PWM, Mask and Timer Mode; disable Timer
 
    Reg_Write32(addr + cnt1_offset,  cnt1);               // load the count1
    
    Reg_Write32(addr + cnt2_offset,  cnt2);               // load the count2
    
//    Reg_Write32(addr + ctrl_offset,  ctrl);               // enable TimerN 
}
/*
 * find null timer struct
*/
static uint32_t find_null_timer(void)
{
    uint32_t i = 0;
    
    for(i=1; i<9; i++)
    {
        if(0 == timer_queue[i].count)
        {
            return i;
        }
    }
    return TIMER_FULL;

}
uint32_t stop_timer(init_timer_st time_st)
{
    uint32_t timer_num = time_st.time_num;
//    printf("stop_timer %d!!\n",timer_num);
    if(timer_num > 7 || time_st.base_time_group > 2)
    {
        return TIMER_NOEXISTENT;

    }
    if(time_st.base_time_group == 0)
    {
        Reg_Write32(BASE_ADDR_TIMER0 + CTRL_0+(timer_num*0x14), 0x02); 
    }
    if(time_st.base_time_group == 1)
    {
        Reg_Write32(BASE_ADDR_TIMER1 + CTRL_0+(timer_num*0x14), 0x02); 
    }

    if(time_st.base_time_group == 2)
    {
        Reg_Write32(BASE_ADDR_TIMER2 + CTRL_0+(timer_num*0x14), 0x02); 
    }
}
uint32_t start_timer(init_timer_st time_st)
{
    uint32_t timer_num = time_st.time_num;
    uint32_t data_tmp = 0;
    if(timer_num > 7 || time_st.base_time_group > 2)
    {
        return TIMER_NOEXISTENT;

    }
    
    if(time_st.base_time_group == 0)
    {
        Reg_Write32(BASE_ADDR_TIMER0 + CTRL_0+(timer_num*0x14), 0x03); 
    }
    if(time_st.base_time_group == 1)
    {
        Reg_Write32(BASE_ADDR_TIMER1 + CTRL_0+(timer_num*0x14), time_st.ctrl); 
    }

    if(time_st.base_time_group == 2)
    {
        Reg_Write32(BASE_ADDR_TIMER2 + CTRL_0+(timer_num*0x14), time_st.ctrl); 
    }
}
/*
 *add timer us
*/
uint32_t add_timer(uint32_t timer_us, void *call_back, uint32_t reload)
{
    uint32_t tmp_num = 0;
    uint32_t tmp_time = 0;
    tmp_num = find_null_timer();
    if(TIMER_FULL == tmp_num)
    {
        return TIMER_FULL;
    }
    memset(&(timer_queue[tmp_num]), 0, sizeof(timer_handle_st)); 
    timer_queue[tmp_num].count = timer_us;
    timer_queue[tmp_num].base_time = g_timer_us;
    timer_queue[tmp_num].reload = reload;
    timer_queue[tmp_num].function = call_back;
   
    tmp_time = timer_queue[tmp_num].base_time + timer_us; 
   
    if(tmp_time < timer_queue[tmp_num].base_time)
        timer_queue[tmp_num].overflow = 0xffffffffUL - timer_queue[tmp_num].base_time;
    
    (*(timer_queue[tmp_num].function))();
    dlog_info("add_timer %d %d %d\n",tmp_num,g_timer_us,timer_us);
    return TIMER_SUCCESS;
}
/*
 * delete timer
*/
uint32_t del_timer(uint32_t timer_us)
{
    uint32_t i = 0;
    for(i=1; i<9; i++)
    {
        if(timer_queue[i].count == timer_us)
        {
            memset(&(timer_queue[i]), 0, sizeof(timer_handle_st));
            return TIMER_SUCCESS; 
        }
    }
    return  TIMER_NOT_TIME;
}
    
uint32_t delay_us(uint32_t us)
{
   
    uint32_t tmp_time = g_timer_us ;

    memset(&(timer_queue[0]), 0, sizeof(timer_handle_st)); 
    
    timer_queue[0].count = us;
    timer_queue[0].base_time = tmp_time;
    timer_queue[0].reload = 0;
    timer_queue[0].function = NULL;
    
    tmp_time = timer_queue[0].base_time + us; 
   
     if(tmp_time < timer_queue[0].base_time)
        timer_queue[0].overflow = 0xffffffffUL - timer_queue[0].base_time;
   
     while(1)
    {
         
        if(0 == timer_queue[0].overflow)
        {
            if(timer_queue[0].count + timer_queue[0].base_time < tmp_time)
            {
                break;
            }
        }
        else
        {
            if(timer_queue[0].base_time >= tmp_time)
            {
               
                timer_queue[0].count = timer_queue[0].count - timer_queue[0].overflow ;
                timer_queue[0].base_time = tmp_time;
                timer_queue[0].overflow = 0; 

            }    
        }
    }

}
uint32_t timer_delay_ms(uint32_t ms)
{
    uint32_t count = ms;
    while(count--)
    {
        delay_us(1000);
    }

}
/*
 *initialization timer
 *
*/
//uint32_t ctrl |=  TIME_ENABLE | USER_DEFINED
// 1us =0x7D 1.25ms = 0x7D*1250 = time_us 
uint32_t init_timer(init_timer_st time_st, uint32_t time) 
{
    uint32_t ctrl = time_st.ctrl;
    uint32_t timer_num = time_st.time_num;
    uint32_t isr_tmp = 0;
    memset((&timer_queue), 0, sizeof(timer_handle_st)*10);
    dlog_info("init_timer %d %d %d\n",time_st.base_time_group,timer_num,ctrl);
    if(timer_num > 7 || time_st.base_time_group > 2)
    {
        return TIMER_NOEXISTENT;

    }
    if(time_st.base_time_group == 0)
    {
        reg_IrqHandle(TIMER_INTR00_VECTOR_NUM, timer0_isr);
        timer_config(BASE_ADDR_TIMER0, 0x03, CTRL_0+(timer_num*0x14),
                     time, CNT1_0+(timer_num*0x14),
                     0x00, CNT2_0+(timer_num*0x04));
    }
    if(time_st.base_time_group == 1)
    {
        reg_IrqHandle(TIMER_INTR00_VECTOR_NUM, timer1_isr);
        timer_config(BASE_ADDR_TIMER1, ctrl, CTRL_0+(timer_num*0x14),
                     time, CNT1_0+(timer_num*0x14),
                     0x00, CNT2_0+(timer_num*0x04));
    }

    if(time_st.base_time_group == 2)
    {
        reg_IrqHandle(TIMER_INTR00_VECTOR_NUM, timer2_isr);
        timer_config(BASE_ADDR_TIMER2, ctrl, CTRL_0+(timer_num*0x14),
                     time, CNT1_0+(timer_num*0x14),
                     0x00, CNT2_0+(timer_num*0x04));
    }
}


uint32_t register_timer(init_timer_st time_st, uint32_t time_us) 
{
    uint32_t ctrl = time_st.ctrl;
    uint32_t timer_num = time_st.time_num;
    uint32_t isr_tmp = 0;
    uint32_t time = TIM_CLC_MHZ * time_us;
    memset((&timer_queue), 0, sizeof(timer_handle_st)*10);
    
    dlog_info("init_timer %d %d %d\n",time_st.base_time_group,timer_num,ctrl);
    if(timer_num > 7 || time_st.base_time_group > 2)
    {
        return TIMER_NOEXISTENT;

    }
    if(time_st.base_time_group == 0)
    {
        
        timer_config(BASE_ADDR_TIMER0, 0x03, CTRL_0+(timer_num*0x14),
                     time, CNT1_0+(timer_num*0x14),
                     0x00, CNT2_0+(timer_num*0x04));
    }
    if(time_st.base_time_group == 1)
    {
        timer_config(BASE_ADDR_TIMER1, ctrl, CTRL_0+(timer_num*0x14),
                     time, CNT1_0+(timer_num*0x14),
                     0x00, CNT2_0+(timer_num*0x04));
    }

    if(time_st.base_time_group == 2)
    {
   //     reg_IrqHandle(TIMER2_0_VECTOR_NUM + timer_num, timer2_isr);
        timer_config(BASE_ADDR_TIMER2, ctrl, CTRL_0+(timer_num*0x14),
                     time, CNT1_0+(timer_num*0x14),
                     0x00, CNT2_0+(timer_num*0x04));
    }
}


