#include <stdint.h>
#include <string.h>
#include "debuglog.h"
#include "command.h"
#include "systicks.h"
#include "serial.h"
#include "quad_spi_ctrl.h"
#include "ff.h"
#include "boot.h"


uint8_t g_bootloadmode = 0;

static void console_init(uint32_t uart_num, uint32_t baut_rate)
{
    serial_init(uart_num, baut_rate);
    dlog_init(uart_num);
    command_init(uart_num);
}
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    /* initialize the uart */
    console_init(0, 360000); //115200 in 200M CPU clock
    dlog_info("bootload start!!! \n");
    dlog_output(100);

    SysTicks_Init(64000);
    QUAD_SPI_SetSpeed(QUAD_SPI_SPEED_50M);

    SysTicks_DelayMS(100);

    if (g_bootloadmode == 0)
    {
        BOOTLOAD_CopyFromNorToITCM();
        BOOTLOAD_BootApp();
    }

    dlog_info("Running in bootload mode");
    dlog_output(100);

    for( ;; )
    {
        if (command_getEnterStatus() == 1)
        {
            command_fulfill();
        }

        dlog_output(100);
        SysTicks_DelayMS(20);
    }

} 

