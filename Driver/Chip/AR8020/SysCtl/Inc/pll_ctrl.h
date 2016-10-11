#ifndef __PLL_CTRL_H
#define __PLL_CTRL_H

#include <stdint.h>

#define CORE_PLL_CLK            250                                       /* unit: MHz */
#define SPI_UART_SEL            (*(volatile unsigned int *)0xA003009C)


void PLLCTRL_SetCoreClk(uint32_t pllClk);


#endif

