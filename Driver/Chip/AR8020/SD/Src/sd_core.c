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
#include <stddef.h>
#include <stdint.h>
#include "reg_rw.h"
#include "sd_core.h"
#include "stm32f746xx.h"
#include "FreeRTOSConfig.h"
#include "memory_config.h"

/**
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

