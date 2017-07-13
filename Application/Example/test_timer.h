#ifndef _TEST_TIM_
#define _TEST_TIM_

void command_TestTim(uint8_t *timer_group, uint8_t *timer_num, uint8_t *timer_count);
void command_TestTimUsed(void);
void command_TestTimAll(void);

void command_TestPwm(uint8_t *timer_group, uint8_t *timer_num, uint8_t *high, uint8_t *low);
void command_TestPwmAll(void);
#endif
