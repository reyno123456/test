#ifndef __SYS_PERIPHERAL_INIT_H
#define __SYS_PERIPHERAL_INIT_H

#include "config_functions_sel.h"

typedef enum 
{
    DISABLE = 0, 
    ENABLE = !DISABLE
} FunctionalState;


void Timer0_Init(void);
void Sky_Timer1_Init(void);
void Grd_Timer1_Init(void);

#endif
