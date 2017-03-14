#include <stdint.h>
#include <string.h>
#include "debuglog.h"
#include "upgrade_command.h"
#include "systicks.h"
#include "serial.h"
#include "quad_spi_ctrl.h"
#include "upgrade_core.h"
#include "pll_ctrl.h"
#include "bb_spi.h"

static void console_init(uint32_t uart_num, uint32_t baut_rate)
{
    serial_init(uart_num, baut_rate);
    dlog_init(UPGRADE_CommandRun, DLOG_SERVER_PROCESSOR);
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    uint8_t tmp=0;

    BB_SPI_init();
    PLLCTRL_SetCoreClk(CPU0_CPU1_CORE_PLL_CLK, ENUM_CPU0_ID);

    SFR_TRX_MODE_SEL = 0x01;
    tmp = SFR_TRX_MODE_SEL;

    INTR_NVIC_SetPriorityGrouping(NVIC_PRIORITYGROUP_5);
    
    /* initialize the uart */
    console_init(0, 115200); //115200 in 200M CPU clock
    dlog_info("upgrade0 start!!! %d !!!\n",tmp);
    dlog_output(100);
    
    QUAD_SPI_SetSpeed(QUAD_SPI_SPEED_50M);

    // Default system tick: 1ms.
    uint32_t u32_priorityGroup = 0x00;
    SysTicks_Init(2000000);
    u32_priorityGroup = INTR_NVIC_GetPriorityGrouping();
    INTR_NVIC_SetIRQPriority(SYSTICK_VECTOR_NUM, INTR_NVIC_EncodePriority(u32_priorityGroup, 0x1f, 0));

    for( ;; )
    {
        DLOG_Process(NULL);
        SysTicks_DelayMS(20);
    }

} 

