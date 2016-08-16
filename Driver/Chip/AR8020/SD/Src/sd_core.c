/**
  * @file    sd_core.c
  * @author  Minzhao
  * @version V1.0.0
  * @date    7-7-2016
  * @brief   source file of sd core.
  *          This file contains:
  *          Initialization/de-initialization functions
  *            I/O operation functions
  *            Peripheral Control functions
  *            Peripheral State functions
  */
#include "system_config.h"
#include "sd_core.h"
#include "serial.h"
/**TODO
  * @brief  Initializes the SDMMC according to the specified
  *         parameters in the SDMMC_InitTypeDef and create the associated handle.
  * @param  SDMMCx: Pointer to SDMMC register base
  * @param  Init: SDMMC initialization structure
  * @retval SDMMC status
  */
// SDMMC_Status SDMMC_Init(SDMMC_REG *SDMMCx, SDMMC_Init Init)
// {
//  uint32_t tmpreg = 0;

//  // /*/* Check the parameters */
//  // assert_param(IS_SDMMC_ALL_INSTANCE(SDMMCx));
//  // assert_param(IS_SDMMC_CLOCK_EDGE(Init.ClockEdge));
//  // assert_param(IS_SDMMC_CLOCK_BYPASS(Init.ClockBypass));
//  // assert_param(IS_SDMMC_CLOCK_POWER_SAVE(Init.ClockPowerSave));
//  // assert_param(IS_SDMMC_BUS_WIDE(Init.BusWide));
//  // assert_param(IS_SDMMC_HARDWARE_FLOW_CONTROL(Init.HardwareFlowControl));
//  // assert_param(IS_SDMMC_CLKDIV(Init.ClockDiv));*/

//  /* Set SDMMC configuration parameters */
//  tmpreg |= (Init.ClockEdge           | \
//             Init.ClockBypass         | \
//             Init.ClockPowerSave      | \
//             Init.BusWide             | \
//             Init.HardwareFlowControl | \
//             Init.ClockDiv
//            );

//  /* Write to SDMMC CLKCR */
//  MODIFY_REG(SDMMCx->CLKCR, CLKCR_CLEAR_MASK, tmpreg);

//  return SDMMC_OK;
// }

/**TODO
  * @brief  Read data (word) from Rx FIFO in blocking mode (polling)
  * @param  SDMMCx: Pointer to SDMMC register base
  * @retval HAL status
  */
// uint32_t Core_SDMMC_ReadFIFO(SDMMC_REG *SDMMCx)
// {
//  /* Read data from Rx FIFO */
//  return (SDMMCx->FIFO);
// }

/**TODO
  * @brief  Write data (word) to Tx FIFO in blocking mode (polling)
  * @param  SDMMCx: Pointer to SDMMC register base
  * @param  pWriteData: pointer to data to write
  * @retval HAL status
  */
// SDMMC_Status Core_SDMMC_WriteFIFO(SDMMC_REG *SDMMCx, uint32_t *pWriteData)
// {
//  /* Write data to FIFO */
//  SDMMCx->FIFO = *pWriteData;

//  return HAL_OK;
// }

/**
  * @brief  Set SDMMC Power state to ON.
  * @param  SDMMCx: Pointer to SDMMC register base
  * @retval HAL status
  */
// SDMMC_Status Core_SDMMC_PowerState_ON(SDMMC_REG *SDMMCx)
// {
//   /* Set power state to ON */
//   SDMMCx->PWREN = (SDMMC_PWREN_0 | \
//                    SDMMC_PWREN_1 | \
//                    SDMMC_PWREN_2 | \
//                    SDMMC_PWREN_3 );

//   return SDMMC_OK;
// }

/**
  * @brief  Set SDMMC Power state to OFF.
  * @param  SDMMCx: Pointer to SDMMC register base
  * @retval HAL status
  */
// SDMMC_Status Core_SDMMC_PowerState_OFF(SDMMC_REG *SDMMCx)
// {
//   /* Set power state to OFF */
//   SDMMCx->PWREN = (uint32_t)0x00000000;

//   return SDMMC_OK;
// }


/**FIXME
  * @brief  Get SDMMC Power state.
  * @param  SDMMCx: Pointer to SDMMC register base
  * @retval Power status of the controller. The returned value can be one of the
  *         following values:
  *            - 0x00: Power OFF
  *            - 0x01: Power ON
  */
uint32_t Core_SDMMC_GetPowerState(SDMMC_REG *SDMMCx)
{
  return (SDMMCx->PWREN & SDMMC_PWREN_0); //for card one
}

/**
  * @brief  Configure the SDMMC command path according to the specified parameters in
  *         SDMMC_CmdInit structure and send the command
  * @param  SDMMCx: Pointer to SDMMC register base
  * @param  Command: pointer to a SDMMC_CmdInitTypeDef structure that contains
  *         the configuration information for the SDMMC command
  * @retval HAL status
  */
SDMMC_Status Core_SDMMC_SendCommand(SDMMC_REG *SDMMCx, SDMMC_CmdInitTypeDef *Command)
{
  uint32_t tmpreg = 0;
  /* Set the SDMMC Argument value */
  Core_SDMMC_SetCMDARG(SDMMCx, Command->Argument);
  /* Set SDMMC command parameters */
  tmpreg |= (uint32_t)(Command->CmdIndex | Command->Attribute);
  /* Write to SDMMC CMD register */
  Core_SDMMC_SetCMD(SDMMCx, tmpreg);
  return SDMMC_OK;
}

/**
  * @brief  Return the response received from the card for the last command
  * @param  SDMMCx: Pointer to SDMMC register base
  * @param  Response: Specifies the SDMMC response register.
  *          This parameter can be one of the following values:
  *            @arg SDMMC_RESP0: Response Register 0
  *            @arg SDMMC_RESP1: Response Register 1
  *            @arg SDMMC_RESP2: Response Register 2
  *            @arg SDMMC_RESP3: Response Register 3
  * @retval The Corresponding response register value
  */
uint32_t Core_SDMMC_GetResponse(SDMMC_REG *SDMMCx, uint32_t Response)
{
  __IO uint32_t tmp = 0;
  switch (Response) {
  case SDMMC_RESP0:
    tmp = Core_SDMMC_GetRESP0(SDMMCx);
    break;
  case SDMMC_RESP1:
    tmp = Core_SDMMC_GetRESP1(SDMMCx);
    break;
  case SDMMC_RESP2:
    tmp = Core_SDMMC_GetRESP2(SDMMCx);
    break;
  case SDMMC_RESP3:
    tmp = Core_SDMMC_GetRESP3(SDMMCx);
    break;
  }
  return tmp;
}


/**TODO
  * @brief  Configure the SDMMC data path according to the specified
  *         parameters in the SDMMC_DataInitTypeDef.
  * @param  SDMMCx: Pointer to SDMMC register base
  * @param  Data : pointer to a SDMMC_DataInitTypeDef structure
  *         that contains the configuration information for the SDMMC data.
  * @retval HAL status
  */
// SDMMC_Status Core_SDMMC_DataConfig(SDMMC_REG *SDMMCx, SDMMC_DataInit *Data)
// {
//   uint32_t tmpreg = 0;

//   /* Check the parameters */
//   // assert_param(IS_SDMMC_DATA_LENGTH(Data->DataLength));
//   // assert_param(IS_SDMMC_BLOCK_SIZE(Data->DataBlockSize));
//   // assert_param(IS_SDMMC_TRANSFER_DIR(Data->TransferDir));
//   // assert_param(IS_SDMMC_TRANSFER_MODE(Data->TransferMode));
//   // assert_param(IS_SDMMC_DPSM(Data->DPSM));

//   /* Set the SDMMC Data TimeOut value */
//   SDMMCx->DTIMER = Data->DataTimeOut;

//   /* Set the SDMMC DataLength value */
//   SDMMCx->DLEN = Data->DataLength;

//   /* Set the SDMMC data configuration parameters */
//   tmpreg |= (uint32_t)(Data->DataBlockSize | \
//                        Data->TransferDir   | \
//                        Data->TransferMode  | \
//                        Data->DPSM);

//   /* Write to SDMMC DCTRL */
//   MODIFY_REG(SDMMCx->DCTRL, DCTRL_CLEAR_MASK, tmpreg);

//   return HAL_OK;
// }

/** TODO
  * @brief  Returns number of remaining data bytes to be transferred.
  * @param  SDMMCx: Pointer to SDMMC register base
  * @retval Number of remaining data bytes to be transferred
  */
// uint32_t Core_SDMMC_GetDataCounter(SDMMC_REG *SDMMCx)
// {
//   return (SDMMCx->DCOUNT);
// }

/** TODO
  * @brief  Get the FIFO data
  * @param  SDMMCx: Pointer to SDMMC register base
  * @retval Data received
  */
// uint32_t Core_SDMMC_GetFIFOCount(SDMMC_REG *SDMMCx)
// {
//   return (SDMMCx->FIFO);
// }


/** TODO
  * @brief  Sets one of the two options of inserting read wait interval.
  * @param  SDMMCx: Pointer to SDMMC register base
  * @param  SDMMC_ReadWaitMode: SDMMC Read Wait operation mode.
  *          This parameter can be:
  *            @arg SDMMC_READ_WAIT_MODE_CLK: Read Wait control by stopping SDMMCCLK
  *            @arg SDMMC_READ_WAIT_MODE_DATA2: Read Wait control using SDMMC_DATA2
  * @retval None
  */
// SDMMC_Status Core_SDMMC_SetSDMMCReadWaitMode(SDMMC_REG *SDMMCx, uint32_t SDMMC_ReadWaitMode)
// {
//   /* Check the parameters */
//   assert_param(IS_SDMMC_READWAIT_MODE(SDMMC_ReadWaitMode));

//   /* Set SDMMC read wait mode */
//   SDMMCx->DCTRL |= SDMMC_ReadWaitMode;

//   return HAL_OK;
// }
