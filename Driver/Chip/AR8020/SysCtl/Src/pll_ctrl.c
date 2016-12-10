#include "pll_ctrl.h"
#include "bb_spi.h"
#include "debuglog.h"




void PLLCTRL_SetCoreClk(uint32_t pllClk, CPU_ID_ENUM cpuId)
{
    uint8_t clk_low;
    uint8_t clk_high;

//    dlog_info("pll setcoreclk: %dMHz\n", pllclk);

    if(pllClk > 255)
    {
        clk_high    = 1;
        clk_low     = pllClk - 256;
    }
    else
    {
        clk_high    = 0;
        clk_low     = (uint8_t)(pllClk & 0xff);
    }

    SPI_UART_SEL    = 0x03;

    if ((CPU0_ID == cpuId) || (CPU1_ID == cpuId))
    {
        BB_SPI_WriteByte(PAGE1, 0xA1, clk_low);
        BB_SPI_WriteByte(PAGE1, 0xA2, clk_high);
    }
    else
    {
        BB_SPI_WriteByte(PAGE1, 0xA4, clk_low);
        BB_SPI_WriteByte(PAGE1, 0xA5, clk_high);
    }

    SPI_UART_SEL    = 0x0;
}

