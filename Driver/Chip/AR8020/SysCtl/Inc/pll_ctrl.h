#ifndef __PLL_CTRL_H
#define __PLL_CTRL_H

#include <stdint.h>

#define CPU0_CPU1_CORE_PLL_CLK       200                                       /* unit: MHz */
#define CPU2_CORE_PLL_CLK            166                                       /* unit: MHz */
#define SPI_UART_SEL                 (*(volatile unsigned int *)0xA003009C)


typedef enum
{
    CPU0_ID = 0,
    CPU1_ID = 1,
    CPU2_ID = 2,
}CPU_ID_ENUM;



void PLLCTRL_SetCoreClk(uint32_t pllClk, CPU_ID_ENUM cpuId);

#endif

