#include "pll_ctrl.h"
#include "BB_spi.h"
#include "debuglog.h"




void PLLCTRL_SetCoreClk(uint32_t pllClk)
{
    uint8_t clk_low;
    uint8_t clk_high;

    dlog_info("pll setcoreclk\n");

    BB_SPI_init();

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

    BB_SPI_WriteByte(PAGE1, 0xA1, clk_low);
    BB_SPI_WriteByte(PAGE1, 0xA2, clk_high);

    SPI_UART_SEL    = 0x0;
}

