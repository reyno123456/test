#define TIM_CLC 50 //50MHz
#include "stdio.h"
#include <stdlib.h>
#include <math.h>
#include "config_functions_sel.h"
#include "debuglog.h"
#include "BB_ctrl.h"


#ifdef BASEBAND_GRD
#include "grd_controller.h"
#endif

#ifdef BASEBAND_SKY
#include "sky_controller.h"
#endif


int test_bbctrl(void);


void test_BB_SKY(void)
{
    STRU_BB_initType initSkyType = {
        .en_mode = BB_SKY_MODE,
    };
 
    BB_uart10_spi_sel(0x00000003);
    BB_init(&initSkyType);
    //BB_uart10_spi_sel(0x00000000);
    test_bbctrl();
    //BB_uart10_spi_sel(0x00000000);
    printf("test_BB_SKY Done \r\n");
}


void test_BB_Grd(void)
{
    STRU_BB_initType initGrdType = {
        .en_mode = BB_GRD_MODE,
    };

    BB_uart10_spi_sel(0x00000003);
    BB_init( &initGrdType);
    test_bbctrl();
    //BB_uart10_spi_sel(0x00000000);
    
    dlog_info("test_BB_Grd Done 1\n");
}



int test_bbctrl(void)
{
	//Timer2_Init();
	#ifdef BASEBAND_GRD
	//Grd_Timer3_Init();
	Grd_Parm_Initial();
	Grd_Id_Initial();
	#endif

	#ifdef BASEBAND_SKY
    
    printf("BASEBAND_SKY \n");
	Sky_Parm_Initial();
//	Sky_Id_Initial();
	//Sky_Timer3_Init();
	#endif
	
	Sys_Parm_Init();
	//PC_Parm_Init();
}
