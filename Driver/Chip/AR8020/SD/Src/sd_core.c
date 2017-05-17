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
#include "debuglog.h"
#include "systicks.h"

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
  //dlog_info("CMD%d = 0x%x\n", Command->CmdIndex, tmpreg);
  //dlog_info("CMD%dARG = 0x%x\n", Command->CmdIndex, Command->Argument);
  return SDMMC_OK;
}

SDMMC_Status Core_SDMMC_WaiteCmdDone(SDMMC_REG *SDMMCx)
{
  uint32_t get_val, cmd_done;
  
  uint32_t start;
  start = SysTicks_GetTickCount();

  do {
    get_val = Core_SDMMC_GetRINTSTS(SDMMCx);
    cmd_done = (get_val & SDMMC_RINTSTS_CMD_DONE); 
    // dlog_info("cmd_done?\n");
    if((SysTicks_GetDiff(start, SysTicks_GetTickCount())) > 1000)
    {
        dlog_error("time out");
    }
    // dlog_info("CMD_DONE RINTSTS = %x", get_val);
  } while (!cmd_done);
  return SDMMC_OK;
}

SDMMC_Status Core_SDMMC_WaiteDataOver(SDMMC_REG *SDMMCx)
{
  uint32_t get_val, data_over;
  uint32_t start;
  start = SysTicks_GetTickCount();

  do {
    get_val = Core_SDMMC_GetRINTSTS(SDMMCx);
    data_over = (get_val & SDMMC_RINTSTS_DATA_OVER);
    // dlog_info("data_over?\n");
    // dlog_info("DATA_OVER RINTSTS = %x", get_val);
    if((SysTicks_GetDiff(start, SysTicks_GetTickCount())) > 1000)
    {
        dlog_error("time out");
    }
  } while (!data_over);
  return SDMMC_OK;
}

SDMMC_Status Core_SDMMC_WaiteCardBusy(SDMMC_REG *SDMMCx)
{
  uint32_t get_val, card_busy;
  uint32_t start;
  start = SysTicks_GetTickCount();

  do {
    get_val = Core_SDMMC_GetSTATUS(SDMMCx);
    card_busy = (get_val & SDMMC_STATUS_DATA_BUSY); 
    // dlog_info("card_busy?\n");
    // dlog_info("CARD_BUSY STATUS = %x", get_val);
    if((SysTicks_GetDiff(start, SysTicks_GetTickCount())) > 100)
    {
        dlog_error("time out");
    }
  } while (card_busy);
  return SDMMC_OK;
}

SDMMC_Status Core_SDMMC_WaiteCmdStart(SDMMC_REG *SDMMCx)
{
  uint32_t get_val, cmd_start;
  uint32_t start;
  start = SysTicks_GetTickCount();

  do {
    get_val = Core_SDMMC_GetCMD(SDMMCx); 
    cmd_start = (get_val & SDMMC_CMD_START_CMD);
    // dlog_info("cmd_start?\n");
    // dlog_info("CMD_START CMD = %x", get_val);
    if((SysTicks_GetDiff(start, SysTicks_GetTickCount())) > 100)
    {
        dlog_error("time out");
        return SDMMC_BUSY;
    }
  } while (cmd_start);
  return SDMMC_OK;
}

SDMMC_Status Core_SDMMC_WaiteVoltSwitchInt(SDMMC_REG *SDMMCx) 
{
  uint32_t get_val, volt_switch_int;
  uint32_t start;

  start = SysTicks_GetTickCount();
  do {
    get_val = Core_SDMMC_GetRINTSTS(SDMMCx);
    volt_switch_int = (get_val & SDMMC_RINTSTS_HTO);
    // dlog_info("volt_switch_int\n");
    // dlog_output(50);
    if((SysTicks_GetDiff(start, SysTicks_GetTickCount())) > 100)
    {
        dlog_error("time out");
    }
  } while (!volt_switch_int);
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

