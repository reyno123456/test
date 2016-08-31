#ifndef __SYS_PERIPHERAL_COMMUNICATION_H
#define __SYS_PERIPHERAL_COMMUNICATION_H

#include "config_functions_sel.h"

typedef struct
{
   uint8_t SkyIrq;              /*!< Specifies the sky 14ms irq had delayed.
                                 This parameter can be a value of @ref <enable or disable>   */
   uint8_t SkySpecialIrq;       /*!< Specifies the 560ms irq from the timer.
                                 This parameter can be a value of @ref <enable or disable> */
   uint8_t GrdIrq;              /*!< Specifies the ground 14ms irq had delayed.
                                 This parameter can be a value of @ref <enable or disable>   */
   uint8_t GrdGetSnrA;          /*!< Specifies the flag of getting snr 8 times in every 14ms.
                                 This parameter can be a value of @ref <enable or disable>   */
   uint8_t GrdGetSnrB;
   uint8_t GrdGetSnrC;
   uint8_t GrdGetSnrD;
   uint8_t GrdGetSnrE;
   uint8_t GrdGetSnrF;
   uint8_t GrdGetSnrG;
   uint8_t GrdGetSnrH;

   uint8_t TxcyregOnly;       /*!< Specifies the flag that plot reg value to cy7c and ensure run one time in every 14ms irq.
                                 This parameter can be a value of @ref <enable,disable>  */
   uint8_t TxcyosdOnly;       /*!< Specifies the flag that plot osd msg to cy7c and ensure run one time in every 14ms irq.
                                 This parameter can be a value of @ref <enable,disable>  */
}Sys_FlagTypeDef;
typedef struct
{
  uint8_t CmdsearchID;        /*!< Specifies command of searching ID from PC.
                                 This parameter can be a value of @ref <enable or disable> */
  uint8_t CmdEXTI;            /*!< Specifies command of enable/disable exteral irq from PC.
                                 This parameter can be a value of @ref <enable or disable> */
}PC_FlagTypeDef;
//=======================================Stm32 system=================================================
void Sys_Parm_Init(void);
void PC_Parm_Init(void);

//=======================================Stm32 & Baseband=================================================

void Spi_Cs_Reset(void);
void Spi_Cs_Set(void);
void Spi_Sclk_Reset(void);
void Spi_Sclk_Set(void);
void Spi_Mosi_Reset(void);
void Spi_Mosi_Set(void);
uint8_t Spi_Baseband_ReadWrite( uint8_t spiRW, uint8_t spiAdrress, uint8_t spiData);
void Baseband_Config(const uint32_t * pConfig , uint32_t iLength);
void Baseband_Load(void);
void Baseband_Soft_Reset(void);
uint8_t Spi_Rf_ReadWrite( uint8_t spiRW, uint8_t spiAdrress, uint8_t spiData);
uint8_t Rf_Handler(uint8_t iClass, uint8_t iOrder, uint8_t iAddr, uint8_t iData);
uint8_t Rf_Config(const uint32_t * pConfig, uint32_t iLength);
void Rf_Load(void);
uint8_t Read_Calibration_AR8001(uint8_t ADDRESS1,uint8_t ibit);
void Write_Back_AR8003(uint8_t Set_Reset_write, uint8_t ADDRESS2,uint8_t jbit);
void Calculate_Txa_Sin_Ofs(void);
void Calculate_Txb_Sin_Ofs(void);
void Baseband_Calibration_Load(void);
void Rf_Calibration_Load(void);
void Rf_PA_NO_CURRENT_During_TX_Calibration(void);
void Baseband_Power_Up(void);

void Sky_Grd_Sel(uint8_t SEL);
void Baseband_Reset(__IO uint32_t TIME);  //hardware reset

void USART1_IRQHandler(void);

void USART3_IRQHandler(void);


void Sky_Grd_USART(void);


//***********************************************************************************
//=======================================Stm32 & Cy7c68013 =================================================

void Mcu_Write_Reset(void);
void Mcu_Write_Set(void);
void Mcu_Clk_Reset(void);
void Mcu_Clk_Set(void);
void Cy7c68013_Reset(__IO uint32_t TIME);
void Bus_Set_Input(void);
void Bus_Set_Output(void);
void Bus_Set_Data(uint8_t xDATA);
void Temp_Delay(uint8_t d);
void Cy7c68013_State_Init(void);
void Read_Cy7c68013(void);
void Write_Cy7c68013(uint8_t* iDATA,uint8_t len);
void Rx_Cy7c_Msg(void);
void Hanlde_Cy7c_Msg(void);
void Tx_Cy7c_Msg(void);

#endif
