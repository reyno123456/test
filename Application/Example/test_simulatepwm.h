#ifndef TEST_SIMULATEPWM_H
#define TEST_SIMULATEPWM_H

typedef struct pwm_handle
{
    uint32_t count[2];
    uint32_t overflow;
    uint32_t reload;
    uint32_t polarity;
    uint32_t data;
    uint32_t data1[2];
    uint32_t base_time;
    void (*function)(uint32_t,uint32_t);
    
} pwm_handle_st;

void command_TestSimulatePwm(void);

#endif