
#include "stdio.h"
#include <stdlib.h>
#include <math.h>
#include "config_functions_sel.h"
#include "sys_peripheral_init.h"
#include "sys_peripheral_communication.h"

#ifdef BASEBAND_SKY
#include "sky_controller.h"
#endif
#ifdef BASEBAND_GRD
#include "grd_controller.h"
#endif

extern PC_FlagTypeDef PCState;

int main(void)
{

    /**
      * @brief Initializes the MCU system.
      * @illustrate
      *
         (+) Initial Hardware.
         (+) Initial Systemclk.
         (+) Set SysTick_IRQn Irq Priority
         (+) Initial Timer,includes Timer2 and Timer3.
         (+) Initial Cy7c68013 with MCU IO for control and debug.
         (+) Initial MCU with Baseband IO include SPI/Reset/Irq/Usart.
    */

	HAL_Init();
	SystemClock_Config();
	HAL_NVIC_SetPriority(SysTick_IRQn, 1, 0);
	Baseband_Reset_Initial();
	Baseband_Reset(20);

	Sky_Grd_Sel_Initial();
	#ifdef BASEBAND_GRD
		Sky_Grd_Sel(SEL_GRD);
	#endif
	
	#ifdef BASEBAND_SKY
		Sky_Grd_Sel(SEL_SKY);
	#endif
	
	Baseband_Spi_Initial();
	Baseband_Power_Up();
	Baseband_Load();
	Rf_Load();
	
	//calibration
	Baseband_Soft_Reset();
	Baseband_Calibration_Load();
	Rf_Calibration_Load();
	Rf_PA_NO_CURRENT_During_TX_Calibration();

	Cy7c68013_Interface_Initial();
	Cy7c68013_Reset(20);
	Cy7c68013_State_Init();

	Timer2_Init();
	
	#ifdef BASEBAND_GRD
		Grd_Timer3_Init();
		Grd_Parm_Initial();
		Grd_Id_Initial();
	#endif
	
	#ifdef BASEBAND_SKY
		Sky_Parm_Initial();
		// Initial the Id_filter for find channel frequency to select the Telecontroller of itself
		// write 0XFF to Id filter register,from address 0x97 to 0x9b in the baseband register table in page2
		Sky_Id_Initial();
		Sky_Timer3_Init();
	#endif
	
	Sys_Parm_Init();
	PC_Parm_Init();
	Tx_Rx_Irq_Initial();

	while(1)              //循环等待数据
	{
		Rx_Cy7c_Msg();
		Tx_Cy7c_Msg();
	}
}
