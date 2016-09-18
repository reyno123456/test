#ifndef __SYS_TICKS_
#define __SYS_TICKS_

#include <stdint.h>

void Inc_sysTicks(void);

uint32_t Get_sysTick(void);

void Delay_sysTick(uint32_t Delay);

#endif
