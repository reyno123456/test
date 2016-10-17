#include "debuglog.h"
#include "config_functions_sel.h"
#include "sys_peripheral_init.h"

#include <timer.h>
#include "interrupt.h"
#include <string.h>

init_timer_st init_timer0_0;
init_timer_st init_timer0_1;

/*********************Initial Systemclk*********************/
void TIM0_IRQHandler(void);
void TIM1_IRQHandler(void);


void Timer0_Init(void)
{
	init_timer0_0.base_time_group = 0;
	init_timer0_0.time_num = 0;
    init_timer0_0.ctrl = 0;
	init_timer0_0.ctrl |= TIME_ENABLE | USER_DEFINED;
    
	#ifdef BASEBAND_SKY
	register_timer(init_timer0_0, 8800); 
	#endif
    
	#ifdef BASEBAND_GRD
	register_timer(init_timer0_0, 5000); 
	#endif
    
	reg_IrqHandle(TIMER_INTR00_VECTOR_NUM, TIM0_IRQHandler);
}

void Sky_Timer1_Init(void)
{
  init_timer0_1.base_time_group = 0;
  init_timer0_1.time_num = 1;
  init_timer0_1.ctrl = 0;
  init_timer0_1.ctrl |= TIME_ENABLE | USER_DEFINED;
  register_timer(init_timer0_1, 1000);
  reg_IrqHandle(TIMER_INTR01_VECTOR_NUM, TIM1_IRQHandler);
}

void Grd_Timer1_Init(void)
{
  init_timer0_1.base_time_group = 0;
  init_timer0_1.time_num = 1;
  init_timer0_1.ctrl = 0;
  init_timer0_1.ctrl |= TIME_ENABLE | USER_DEFINED;
  register_timer(init_timer0_1, 1250);
  reg_IrqHandle(TIMER_INTR01_VECTOR_NUM, TIM1_IRQHandler);
}