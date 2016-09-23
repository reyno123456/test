#include "debuglog.h"


int globalvalue = 1;
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    serial_init(1);
    int localvalue = 1;
    dlog_info("cpu1 start!!! \n");


    /* We should never get here as control is now taken by the scheduler */
    for( ;; );
} 

