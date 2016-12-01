#include <stdint.h>
#include <string.h>

#include "debuglog.h"
#include "upgrade_command.h"
#include "systicks.h"
#include "serial.h"
#include "quad_spi_ctrl.h"
#include "upgrade_core.h"
#include "pll_ctrl.h"
#include "BB_spi.h"

static void console_init(uint32_t uart_num, uint32_t baut_rate)
{
    serial_init(uart_num, baut_rate);
    dlog_init(uart_num);
    UPGRADE_CommandInit(uart_num);
}
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    BB_SPI_init();

    PLLCTRL_SetCoreClk(CPU0_CPU1_CORE_PLL_CLK, CPU0_ID);
    /* initialize the uart */
    console_init(0, 115200); //115200 in 200M CPU clock
    dlog_info("bootload0 start!!! \n");
    dlog_output(100);

    SysTicks_Init(200000);
    QUAD_SPI_SetSpeed(QUAD_SPI_SPEED_50M);
    dlog_output(100);

    for( ;; )
    {
        if (UPGRADE_CommandGetEnterStatus() == 1)
        {
            UPGRADE_CommandFulfill();
        }

        dlog_output(100);
        SysTicks_DelayMS(20);
    }

} 

