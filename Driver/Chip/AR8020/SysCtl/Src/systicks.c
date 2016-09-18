#include "systicks.h"

static volatile uint32_t u32_Tick = 0;

/**
  * @brief This function is called to increment a global variable "u32_Tick"
  *        used as application time base.
  * @note In the default implementation, this variable is incremented each in Systick ISR.
  * @retval None
  */
void Inc_sysTicks(void)
{
    u32_Tick++;
}

/**
  * @brief Provides a tick value
  * @note: Call xTaskGetTickCount instead if the FreeRTOS is running.  
  * @retval Tick value
  */
uint32_t Get_sysTick(void)
{
    return u32_Tick;
}

/**
  * @brief This function provides delay based on variable incremented.
  * @note In the default implementation , SysTick timer is the source of time base.
  *       It is used to generate interrupts at regular time intervals where u32_Tick
  *       is incremented.
  *       call vTaskDelay instead if the FreeRTOS is running.  
  * @param Delay: specifies the delay time length, in milliseconds.
  * @retval None
  */
void Delay_sysTick(uint32_t Delay)
{
    uint32_t tickstart = 0;
    tickstart = Get_sysTick();
    while((Get_sysTick() - tickstart) < Delay)
    {}
}