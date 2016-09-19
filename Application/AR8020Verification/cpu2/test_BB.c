#include "BB_ctrl.h"
           
void test_BB_SKY(void)
{
    STRU_BB_initType initSkyType = {
        .en_mode = BB_SKY_MODE,
    };
 
    BB_uart10_spi_sel(0x00000003);
    BB_init(&initSkyType);
    BB_uart10_spi_sel(0x00000000);
}


void test_BB_Grd(void)
{
    STRU_BB_initType initGrdType = {
        .en_mode = BB_GRD_MODE,
    };

    BB_init( &initGrdType);
}