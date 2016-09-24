#include "debuglog.h"

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    serial_init(1, 115200);
    dlog_info("cpu1 start!!! \n");


    /* We should never get here as control is now taken by the scheduler */
    for( ;; );
} 

