/**
  * @file    sd_card.c
  * @author  Minzhao
  * @version V1.0.0
  * @date    7-10-2016
  * @brief   Header file of sd card.
  *          This file contains:
  *           + Initialization and de-initialization functions
  *           + IO operation functions
  *           + Peripheral Control functions
  *           + Peripheral State functions
  */

#include <stddef.h>
#include <stdint.h>
#include "reg_rw.h"
#include "sd_core.h"
#include "sd_card.h"
#include "stm32f746xx.h"
#include "debuglog.h"

IDMAC_DescTypeDef Descriptor;
/** @defgroup SD_Private_Functions SD Private Functions
  * @{
  */
static SD_ErrorTypedef IdentificateCard(SD_HandleTypeDef *hsd,SD_CardInfoTypedef *pCardInfo);
//static SD_ErrorTypedef IdentificateCard(SD_HandleTypeDef *hsd);
static SD_ErrorTypedef PowerOnCard(SD_HandleTypeDef *hsd);
static SD_ErrorTypedef InitializeCard(SD_HandleTypeDef *hsd);
static SD_ErrorTypedef PowerOffCard(SD_HandleTypeDef *hsd);
static SD_ErrorTypedef SD_CmdError(SD_HandleTypeDef *hsd);
static SD_ErrorTypedef SD_CmdResp1Error(SD_HandleTypeDef *hsd, uint32_t SD_CMD);
static SD_ErrorTypedef SD_CmdResp7Error(SD_HandleTypeDef *hsd);
static SD_ErrorTypedef SD_CmdResp7(SD_HandleTypeDef *hsd);
static SD_ErrorTypedef SD_CmdResp3Error(SD_HandleTypeDef *hsd);
static SD_ErrorTypedef SD_CmdResp2Error(SD_HandleTypeDef *hsd);
static SD_ErrorTypedef SD_CmdResp6Error(SD_HandleTypeDef *hsd, uint8_t SD_CMD, uint16_t *pRCA);
static SD_ErrorTypedef SD_WideBus_Enable(SD_HandleTypeDef *hsd);
static SD_ErrorTypedef SD_WideBus_Disable(SD_HandleTypeDef *hsd);
static SD_ErrorTypedef SD_FindSCR(SD_HandleTypeDef *hsd, uint32_t *pSCR);
static SD_ErrorTypedef SD_DMAConfig(SD_HandleTypeDef *hsd, SDMMC_DMATransTypeDef *dma);
static SD_ErrorTypedef SD_ENUM(SD_HandleTypeDef *hsd);
static SD_ErrorTypedef SD_IsCardProgramming(SD_HandleTypeDef *hsd, uint8_t *status);



SD_ErrorTypedef Card_SD_Init(SD_HandleTypeDef *hsd, SD_CardInfoTypedef *SDCardInfo)
{
  __IO SD_ErrorTypedef errorstate = SD_OK;
  uint32_t get_val;
  /* To identify if card is powered on */
  get_val = Core_SDMMC_GetPWREN(hsd->Instance);
  if (!get_val)
  {
    Core_SDMMC_SetPWREN(hsd->Instance, SDMMC_PWREN_0 | \
                    SDMMC_PWREN_1 | \
                    SDMMC_PWREN_2 | \
                    SDMMC_PWREN_3);    
    delay_ms(1);
  }

  errorstate = PowerOnCard(hsd);
  if (errorstate != SD_OK)
  {
    dlog_info("Power on Card Error!\n");
    return errorstate;
  }

  /* Identify card operating voltage */
  errorstate = InitializeCard(hsd);
  if (errorstate != SD_OK)
  {
    if (errorstate == 42)
    {
      dlog_info("No SD Card! Quit Initialization!\n");
    }
    else
    {
      dlog_info("Initialize Card Error!\n");  
    }
    return errorstate;
  }

  /* Initialize the present SDMMC card(s) and put them in idle state */
  errorstate = IdentificateCard(hsd, SDCardInfo);
  if (errorstate != SD_OK)
  {
    return errorstate;
  }

  /* Read CSD/CID MSD registers */
  //errorstate = Card_SD_Get_CardInfo(hsd, SDCardInfo);
  return errorstate;
}



/**
  * @brief  De-Initializes the SD card.
  * @param  hsd: SD handle
  * @retval HAL status
  */
SD_ErrorTypedef Card_SD_DeInit(SD_HandleTypeDef *hsd)
{
  SD_ErrorTypedef errorstate = SD_OK;
  /* Set SD power state to off */
  errorstate = PowerOffCard(hsd);

  return errorstate;
}

// SD_ErrorTypedef Card_SD_ReadBlock_DMA(SD_HandleTypeDef *hsd, SDMMC_DMATransTypeDef *dma)
// {
//   SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
//   //SDMMC_DataInitTypeDef sdmmc_datainitstructure;
//   typedef uint32_t *uint32p;
//   SD_ErrorTypedef errorstate = SD_OK;
//   uint32_t BlockIndex, NextDesp, Dst_Addr, TotalBlocks;
//   uint32p DES0, DES1, DES2, DES3;
//   uint32_t get_val, cmd_done, data_over, card_busy;
//   uint32_t RestBytes = dma->SectorNum * dma->BlockSize;
//   /* Initialize handle flags */
//   hsd->SdTransferCplt  = 0;
//   hsd->SdTransferErr   = SD_OK;

//   if (hsd->CardType == HIGH_CAPACITY_SD_CARD)
//   {
//     dma->BlockSize = 512;
//   }

//   /* Configure the SD DPSM (Data Path State Machine) */
//   errorstate = SD_DMAConfig(hsd, dma);
//   if (errorstate != SD_OK) {
//     return errorstate;
//   }
  

//   for (BlockIndex = 0; BlockIndex < dma->SectorNum; BlockIndex++)
//   {

//     if (RestBytes < dma->BlockSize)
//     {
//       Core_SDMMC_SetBYCTNT(hsd->Instance, RestBytes);
//     }

//     Dst_Addr = dma->DstAddr + 512 * BlockIndex;
//     DES0 = (uint32_t *)(dma->ListBaseAddr + 0x00000010 * BlockIndex + 0x00000000);
//     DES1 = (uint32_t *)(dma->ListBaseAddr + 0x00000010 * BlockIndex + 0x00000004);
//     DES2 = (uint32_t *)(dma->ListBaseAddr + 0x00000010 * BlockIndex + 0x00000008);
//     DES3 = (uint32_t *)(dma->ListBaseAddr + 0x00000010 * BlockIndex + 0x0000000c);
//     NextDesp = dma->ListBaseAddr + 0x00000010 * (BlockIndex + 1);

//     write_reg32(DES0, SDMMC_DES0_OWN | \
//                 SDMMC_DES0_CH | \
//                 SDMMC_DES0_FS | \
//                 SDMMC_DES0_LD);
//     write_reg32(DES1, 0x00000200);
//     write_reg32(DES2, Dst_Addr);
//     write_reg32(DES3, 0x00000000);

//     /* send CMD17*/
//     sdmmc_cmdinitstructure.Argument         = dma->SrcAddr + BlockIndex;
//     sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_READ_SINGLE_BLOCK;
//     sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
//     sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
//         SDMMC_CMD_USE_HOLD_REG | \
//         SDMMC_CMD_PRV_DAT_WAIT | \
//         SDMMC_CMD_DAT_EXP      | \
//         SDMMC_CMD_RESP_CRC     | \
//         SDMMC_CMD_RESP_EXP;
//     Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
//     Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);

//     /* Check for error conditions */
//     Core_SDMMC_WaiteCmdDone(hsd->Instance);
//     Core_SDMMC_WaiteDataOver(hsd->Instance);
//     Core_SDMMC_WaiteCardBusy(hsd->Instance);

//     write_reg32(&(hsd->Instance->DBADDR), NextDesp);
//     RestBytes -= dma->BlockSize;
//   }

//   /* Update the SD transfer error in SD handle */
//   hsd->SdTransferErr = errorstate;
//   return errorstate;
// }

SD_ErrorTypedef Card_SD_ReadBlock_DMA(SD_HandleTypeDef *hsd, SDMMC_DMATransTypeDef *dma)
{
  SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
  //SDMMC_DataInitTypeDef sdmmc_datainitstructure;
  typedef uint32_t *uint32p;
  SD_ErrorTypedef errorstate = SD_OK;
  uint32_t BlockIndex, NextDesp, Dst_Addr, TotalBlocks;
  uint32p DES0, DES1, DES2, DES3;
  uint32_t get_val, cmd_done, data_over, card_busy;
  uint32_t RestBytes = dma->SectorNum * dma->BlockSize;
  /* Initialize handle flags */
  hsd->SdTransferCplt  = 0;
  hsd->SdTransferErr   = SD_OK;

  if (hsd->CardType == HIGH_CAPACITY_SD_CARD)
  {
    dma->BlockSize = 512;
  }

  /* Configure the SD DPSM (Data Path State Machine) */
  errorstate = SD_DMAConfig(hsd, dma);
  if (errorstate != SD_OK) {
    return errorstate;
  }
  //Core_SDMMC_SetBYCTNT(hsd->Instance, dma->BlockSize);

  for (BlockIndex = 0; BlockIndex < dma->SectorNum; BlockIndex++)
  {

    if (RestBytes < dma->BlockSize)
    {
      Core_SDMMC_SetBYCTNT(hsd->Instance, RestBytes);
    }

    Dst_Addr = dma->DstAddr + 512 * BlockIndex;
    DES0 = (uint32_t *)(dma->ListBaseAddr + 0x00000010 * BlockIndex + 0x00000000);
    DES1 = (uint32_t *)(dma->ListBaseAddr + 0x00000010 * BlockIndex + 0x00000004);
    DES2 = (uint32_t *)(dma->ListBaseAddr + 0x00000010 * BlockIndex + 0x00000008);
    DES3 = (uint32_t *)(dma->ListBaseAddr + 0x00000010 * BlockIndex + 0x0000000c);
    NextDesp = dma->ListBaseAddr + 0x00000010 * (BlockIndex + 1);

    write_reg32(DES0, SDMMC_DES0_OWN | \
                SDMMC_DES0_CH | \
                SDMMC_DES0_FS | \
                SDMMC_DES0_LD);
    write_reg32(DES1, 0x00000200);
    write_reg32(DES2, Dst_Addr);
    write_reg32(DES3, 0x00000000);

    /* send CMD17*/
    sdmmc_cmdinitstructure.Argument         = dma->SrcAddr + BlockIndex;
    sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_READ_SINGLE_BLOCK;
    sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
    sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
        SDMMC_CMD_USE_HOLD_REG | \
        SDMMC_CMD_PRV_DAT_WAIT | \
        SDMMC_CMD_DAT_EXP      | \
        SDMMC_CMD_RESP_CRC     | \
        SDMMC_CMD_RESP_EXP;
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);

    /* Check for error conditions */
    Core_SDMMC_WaiteCmdDone(hsd->Instance);
    Core_SDMMC_WaiteDataOver(hsd->Instance);
    Core_SDMMC_WaiteCardBusy(hsd->Instance);

    write_reg32(&(hsd->Instance->DBADDR), NextDesp);
    RestBytes -= dma->BlockSize;
  }

  /* Update the SD transfer error in SD handle */
  hsd->SdTransferErr = errorstate;
  return errorstate;
}

SD_ErrorTypedef Card_SD_ReadMultiBlocks_DMA(SD_HandleTypeDef *hsd, SDMMC_DMATransTypeDef *dma)
{
  SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
  typedef uint32_t *uint32p;
  SD_ErrorTypedef errorstate = SD_OK;
  uint32_t BlockIndex, NextDesp, Dst_Addr, TotalBlocks;
  uint32p DES0, DES1, DES2, DES3;
  uint32_t get_val, cmd_done, data_over, card_busy;
  /* Initialize handle flags */
  hsd->SdTransferCplt  = 0;
  hsd->SdTransferErr   = SD_OK;

  if (hsd->CardType == HIGH_CAPACITY_SD_CARD)
  {
    dma->BlockSize = 512;
  }

  /* Configure the SD DPSM (Data Path State Machine) */
  errorstate = SD_DMAConfig(hsd, dma);
  if (errorstate != SD_OK) {
    return errorstate;
  }

  for (BlockIndex = 0; BlockIndex < dma->SectorNum; BlockIndex++)
  {

    Dst_Addr = dma->DstAddr + 512 * BlockIndex;
    DES0 = (uint32_t *)(dma->ListBaseAddr + 0x00000010 * BlockIndex + 0x00000000);
    DES1 = (uint32_t *)(dma->ListBaseAddr + 0x00000010 * BlockIndex + 0x00000004);
    DES2 = (uint32_t *)(dma->ListBaseAddr + 0x00000010 * BlockIndex + 0x00000008);
    DES3 = (uint32_t *)(dma->ListBaseAddr + 0x00000010 * BlockIndex + 0x0000000c);
    NextDesp = dma->ListBaseAddr + 0x00000010 * (BlockIndex + 1);

    if (BlockIndex == 0)
    {
      write_reg32(DES0, SDMMC_DES0_OWN | \
                SDMMC_DES0_FS | \
                SDMMC_DES0_CH);
      write_reg32(DES1, 0x00000200);
      write_reg32(DES2, Dst_Addr);   //the buffer address
      write_reg32(DES3, NextDesp);
    }
    else if (BlockIndex == dma->SectorNum - 1)
    {
      write_reg32(DES0, SDMMC_DES0_OWN | \
                SDMMC_DES0_CH | \
                SDMMC_DES0_LD);
      write_reg32(DES1, 0x00000200);
      write_reg32(DES2, Dst_Addr);   //the buffer address
      write_reg32(DES3, 0x00000000); 
    }
    else
    {
      write_reg32(DES0, SDMMC_DES0_OWN | \
                SDMMC_DES0_CH);
      write_reg32(DES1, 0x00000200);
      write_reg32(DES2, Dst_Addr);   //the buffer address
      write_reg32(DES3, NextDesp);
    }
  }
  /* send CMD18 */
  sdmmc_cmdinitstructure.Argument         = dma->SrcAddr;
  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_READ_MULTIPLE_BLOCK;
  sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
  sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
      SDMMC_CMD_USE_HOLD_REG | \
      SDMMC_CMD_PRV_DAT_WAIT | \
      SDMMC_CMD_SEND_STOP    | \
      SDMMC_CMD_DAT_EXP      | \
      SDMMC_CMD_RESP_CRC     | \
      SDMMC_CMD_RESP_EXP;
  Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);

  /* Check for error conditions */
  Core_SDMMC_WaiteCmdDone(hsd->Instance);
  //Core_SDMMC_WaiteDataOver(hsd->Instance);
  Core_SDMMC_WaiteCardBusy(hsd->Instance);
  /* Update the SD transfer error in SD handle */
  hsd->SdTransferErr = errorstate;
  return errorstate;
}

/*
 *   The original writesingleblock SD function, works well;
 */
SD_ErrorTypedef Card_SD_WriteBlock_DMA(SD_HandleTypeDef *hsd, SDMMC_DMATransTypeDef *dma)
{
  SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
  SD_ErrorTypedef errorstate = SD_OK;
  typedef uint32_t *uint32p;
  uint32_t BlockIndex, NextDesp, SrcAddr;
  uint32p DES0, DES1, DES2, DES3;
  uint32_t get_val, cmd_done, data_over, card_busy;
  /* Initialize handle flags */
  hsd->SdTransferCplt  = 0;
  hsd->SdTransferErr   = SD_OK;
  /* Initialize SD Read operation */
  hsd->SdOperation = SD_WRITE_SINGLE_BLOCK;

  if (hsd->CardType == HIGH_CAPACITY_SD_CARD)
  {
    dma->BlockSize = 512;
  }

  /* Configure the SD DPSM (Data Path State Machine) */
  errorstate = SD_DMAConfig(hsd, dma);
  if (errorstate != SD_OK) {
    dlog_info("SD_DMAConfig Fail\n");
    return errorstate;
  }

  for (BlockIndex = 0; BlockIndex < dma->SectorNum; BlockIndex++)
  {
    SrcAddr = dma->SrcAddr + dma->BlockSize * BlockIndex;
    DES0 = (uint32_t *)(dma->ListBaseAddr + 0x00000010 * BlockIndex + 0x00000000);
    DES1 = (uint32_t *)(dma->ListBaseAddr + 0x00000010 * BlockIndex + 0x00000004);
    DES2 = (uint32_t *)(dma->ListBaseAddr + 0x00000010 * BlockIndex + 0x00000008);
    DES3 = (uint32_t *)(dma->ListBaseAddr + 0x00000010 * BlockIndex + 0x0000000c);
    NextDesp = dma->ListBaseAddr + 0x00000010 * (BlockIndex + 1);

    write_reg32(DES0, SDMMC_DES0_OWN | \
                SDMMC_DES0_FS | \
                SDMMC_DES0_CH | \
                SDMMC_DES0_LD);
    write_reg32(DES1, 0x00000200);
    write_reg32(DES2, SrcAddr);   //the buffer address
    write_reg32(DES3, 0x00000000);

    dlog_info("SrcAddr = 0x%x\n", SrcAddr);
    dlog_info("DstAddr = 0x%d\n", dma->DstAddr);
    dlog_info("DES0 Addr = 0x%x", DES0);

    /* send CMD24 */
    sdmmc_cmdinitstructure.Argument         = dma->DstAddr;
    sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_WRITE_SINGLE_BLOCK;
    sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
    sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
                                              SDMMC_CMD_USE_HOLD_REG | \
                                              SDMMC_CMD_PRV_DAT_WAIT | \
                                              SDMMC_CMD_DAT_READ_WRITE | \
                                              SDMMC_CMD_DAT_EXP      | \
                                              SDMMC_CMD_RESP_CRC     | \
                                              SDMMC_CMD_RESP_EXP;
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
     /* Check for error conditions */
    Core_SDMMC_WaiteCmdDone(hsd->Instance);
    Core_SDMMC_WaiteDataOver(hsd->Instance);
    Core_SDMMC_WaiteCardBusy(hsd->Instance); 
    write_reg32(&(hsd->Instance->DBADDR), NextDesp);
  }
  
  /* Update the SD transfer error in SD handle */
  hsd->SdTransferErr = errorstate;
  return errorstate;
}


SD_ErrorTypedef Card_SD_WriteMultiBlocks_DMA(SD_HandleTypeDef *hsd, SDMMC_DMATransTypeDef *dma)
{
  SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
  SD_ErrorTypedef errorstate = SD_OK;
  typedef uint32_t *uint32p;
  uint32_t BlockIndex, NextDesp, SrcAddr;
  uint32p DES0, DES1, DES2, DES3;
  uint32_t get_val, cmd_done, data_over, card_busy;

  /* Initialize handle flags */
  hsd->SdTransferCplt  = 0;
  hsd->SdTransferErr   = SD_OK;
  /* Initialize SD Read operation */
  hsd->SdOperation = SD_WRITE_MULTIPLE_BLOCK;

  if (hsd->CardType == HIGH_CAPACITY_SD_CARD)
  {
    dma->BlockSize = 512;
  }
  /* Configure the SD DPSM (Data Path State Machine) */
  errorstate = SD_DMAConfig(hsd, dma);
  if (errorstate != SD_OK) {
    dlog_info("SD_DMAConfig Fail\n");
    return errorstate;
  }

  for (BlockIndex = 0; BlockIndex < dma->SectorNum; BlockIndex++)
  {
    SrcAddr = dma->SrcAddr + dma->BlockSize * BlockIndex;
    DES0 = (uint32_t *)(dma->ListBaseAddr + 0x00000010 * BlockIndex + 0x00000000);
    DES1 = (uint32_t *)(dma->ListBaseAddr + 0x00000010 * BlockIndex + 0x00000004);
    DES2 = (uint32_t *)(dma->ListBaseAddr + 0x00000010 * BlockIndex + 0x00000008);
    DES3 = (uint32_t *)(dma->ListBaseAddr + 0x00000010 * BlockIndex + 0x0000000c);
    NextDesp = dma->ListBaseAddr + 0x00000010 * (BlockIndex + 1);

    if (BlockIndex == 0)
    {
      write_reg32(DES0, SDMMC_DES0_OWN | \
                SDMMC_DES0_FS | \
                SDMMC_DES0_CH);
      write_reg32(DES1, 0x00000200);
      write_reg32(DES2, SrcAddr);   //the buffer address
      write_reg32(DES3, NextDesp);
    }
    else if (BlockIndex == dma->SectorNum - 1)
    {
      write_reg32(DES0, SDMMC_DES0_OWN | \
                SDMMC_DES0_CH | \
                SDMMC_DES0_LD);
      write_reg32(DES1, 0x00000200);
      write_reg32(DES2, SrcAddr);   //the buffer address
      write_reg32(DES3, 0x00000000); 
    }
    else
    {
      write_reg32(DES0, SDMMC_DES0_OWN | \
                SDMMC_DES0_CH);
      write_reg32(DES1, 0x00000200);
      write_reg32(DES2, SrcAddr);   //the buffer address
      write_reg32(DES3, NextDesp);
    }
  }

  /* send CMD25 */
  sdmmc_cmdinitstructure.Argument         = dma->DstAddr;
  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_WRITE_MULTIPLE_BLOCK;
  sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
  sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
      SDMMC_CMD_USE_HOLD_REG | \
      SDMMC_CMD_PRV_DAT_WAIT | \
      SDMMC_CMD_SEND_STOP    | \
      SDMMC_CMD_DAT_READ_WRITE | \
      SDMMC_CMD_DAT_EXP      | \
      SDMMC_CMD_RESP_CRC     | \
      SDMMC_CMD_RESP_EXP;
  Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
  /* Check for error conditions */
  Core_SDMMC_WaiteCmdDone(hsd->Instance);
  Core_SDMMC_WaiteDataOver(hsd->Instance);
  Core_SDMMC_WaiteCardBusy(hsd->Instance);

  #ifdef ECHO
      dlog_info("Stop: send CMD12\n");
  #endif
  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_STOP_TRANSMISSION;
  sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
  sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
      SDMMC_CMD_USE_HOLD_REG | \
      SDMMC_CMD_PRV_DAT_WAIT | \
      SDMMC_CMD_RESP_CRC | \
      SDMMC_CMD_RESP_EXP;
  Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
  Core_SDMMC_WaiteCmdDone(hsd->Instance);
  /* Update the SD transfer error in SD handle */
  hsd->SdTransferErr = errorstate;
  return errorstate;
}

/**
  * @brief  Erases the specified memory area of the given SD card.
  * @param  hsd: SD handle
  * @param  startaddr: Start byte address
  * @param  blocknum: End byte address
  * @retval SD Card error state
  */
SD_ErrorTypedef Card_SD_Erase(SD_HandleTypeDef *hsd, uint32_t startaddr, uint32_t blocknum)
{
  SD_ErrorTypedef errorstate = SD_OK;
  SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
  uint32_t get_val, cmd_done, data_over, card_busy;
  uint8_t cardstate = 0;

  /* According to sd-card spec 3.0 ERASE_GROUP_START (CMD32) and erase_group_end(CMD33) */
  if ((hsd->CardType == STD_CAPACITY_SD_CARD)||(hsd->CardType == HIGH_CAPACITY_SD_CARD))
  {
    /* Send CMD32 SD_ERASE_GRP_START with argument as addr  */
    sdmmc_cmdinitstructure.Argument         = (uint32_t)startaddr;
    sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SD_ERASE_WR_BLK_START;
    sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
    sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
        SDMMC_CMD_USE_HOLD_REG | \
        SDMMC_CMD_PRV_DAT_WAIT | \
        SDMMC_CMD_RESP_CRC     | \
        SDMMC_CMD_RESP_EXP;
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
    /* Check for error conditions */
    Core_SDMMC_WaiteCmdDone(hsd->Instance);
    Core_SDMMC_WaiteCardBusy(hsd->Instance);


    /* Send CMD33 SD_ERASE_GRP_END with argument as addr  */
    sdmmc_cmdinitstructure.Argument         = (uint32_t)(startaddr + blocknum - 1);
    sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SD_ERASE_WR_BLK_END;
    sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
    sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
        SDMMC_CMD_USE_HOLD_REG | \
        SDMMC_CMD_PRV_DAT_WAIT | \
        SDMMC_CMD_RESP_CRC     | \
        SDMMC_CMD_RESP_EXP;
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
    /* Check for error conditions */
    Core_SDMMC_WaiteCmdDone(hsd->Instance);
    Core_SDMMC_WaiteCardBusy(hsd->Instance);

  }

  /* Send CMD38 ERASE */
  sdmmc_cmdinitstructure.Argument         = 0;
  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_ERASE;
  sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
  sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
      SDMMC_CMD_USE_HOLD_REG | \
      SDMMC_CMD_PRV_DAT_WAIT | \
      SDMMC_CMD_RESP_CRC     | \
      SDMMC_CMD_RESP_EXP;
  Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);


//   /* Wait until the card is in programming state */
//   errorstate = SD_IsCardProgramming(hsd, &cardstate);
//   while ((errorstate == SD_OK) && ((cardstate == SD_CARD_PROGRAMMING) || (cardstate == SD_CARD_RECEIVE)))
//   {
//     errorstate = SD_IsCardProgramming(hsd, &cardstate);
#ifdef ECHO
//     dlog_info("sd is in programming\n");
#endif
//   }
  return errorstate;
}

/** 
  * @brief  Returns information about specific card.
  * @param  hsd: SD handle
  * @param  pCardInfo: Pointer to a SD_CardInfoTypedef structure that
  *         contains all SD cardinformation
  * @retval SD Card error state
  */
SD_ErrorTypedef Card_SD_Get_CardInfo(SD_HandleTypeDef *hsd, SD_CardInfoTypedef *pCardInfo)
{
  SD_ErrorTypedef errorstate = SD_OK;

  uint32_t tmp = 0;
  pCardInfo->CardType = (uint32_t)(hsd->CardType);
  pCardInfo->RCA      = (uint32_t)(hsd->RCA);

  /* Byte 0 */
  tmp = (hsd->CSD[3] & 0xFF000000) >> 24;
  pCardInfo->SD_csd.CSD_STRUCTURE      = (uint8_t)((tmp & 0xC0) >> 6);
  //dlog_info("CSD_STRUCTURE = %d\n", pCardInfo->SD_csd.CSD_STRUCTURE);
  pCardInfo->SD_csd.SysSpecVersion = (uint8_t)((tmp & 0x3C) >> 2);
  pCardInfo->SD_csd.Reserved1      = tmp & 0x03;
  /* Byte 1 */
  tmp = (hsd->CSD[3] & 0x00FF0000) >> 16;
  pCardInfo->SD_csd.TAAC = (uint8_t)tmp;
  //dlog_info("TAAC = %d\n", pCardInfo->SD_csd.TAAC);
  /* Byte 2 */
  tmp = (hsd->CSD[3] & 0x0000FF00) >> 8;
  pCardInfo->SD_csd.NSAC = (uint8_t)tmp;
  /* Byte 3 */
  //hsd->CardType = HIGH_CAPACITY_SD_CARD;
  tmp = hsd->CSD[3] & 0x000000FF;
  pCardInfo->SD_csd.TRAN_SPEED = (uint8_t)tmp;
  //dlog_info("TRAN_SPEED = %d\n", pCardInfo->SD_csd.TRAN_SPEED);
  /* Byte 4 */
  tmp = (hsd->CSD[1] & 0xFF000000) >> 24;
  pCardInfo->SD_csd.CCC = (uint16_t)(tmp << 4);

  /* Byte 5 */
  tmp = (hsd->CSD[2] & 0x00FF0000) >> 16;
  pCardInfo->SD_csd.CCC |= (uint16_t)((tmp & 0xF0) >> 4);
  //dlog_info("CCC = %d\n", pCardInfo->SD_csd.CCC);

  pCardInfo->SD_csd.READ_BL_LEN       = (uint8_t)(tmp & 0x0F);
  //dlog_info("READ_BL_LEN = %d\n", pCardInfo->SD_csd.READ_BL_LEN);

  /* Byte 6 */
  tmp = (hsd->CSD[2] & 0x0000FF00) >> 8;
  pCardInfo->SD_csd.READ_BL_PARTIAL   = (uint8_t)((tmp & 0x80) >> 7);
  //dlog_info("READ_BL_PARTIAL = %d\n", pCardInfo->SD_csd.READ_BL_PARTIAL);

  pCardInfo->SD_csd.WRITE_BLK_MISALIGN = (uint8_t)((tmp & 0x40) >> 6);
  //dlog_info("WRITE_BLK_MISALIGN = %d\n", pCardInfo->SD_csd.WRITE_BLK_MISALIGN);  

  pCardInfo->SD_csd.READ_BLK_MISALIGN = (uint8_t)((tmp & 0x20) >> 5);
  //dlog_info("READ_BLK_MISALIGN = %d\n", pCardInfo->SD_csd.READ_BLK_MISALIGN);  

  pCardInfo->SD_csd.DSP_IMP         = (uint8_t)((tmp & 0x10) >> 4);
  pCardInfo->SD_csd.Reserved2       = 0; /*!< Reserved */

  if (hsd->CardType == STD_CAPACITY_SD_CARD)
  {
    /* Byte 7 */
    pCardInfo->SD_csd.C_SIZE = (tmp & 0x03) << 10;
    tmp = (uint8_t)(hsd->CSD[2] & 0x000000FF);
    pCardInfo->SD_csd.C_SIZE |= (tmp) << 2;

    /* Byte 8 */
    tmp = (uint8_t)((hsd->CSD[1] & 0xFF000000) >> 24);
    pCardInfo->SD_csd.C_SIZE |= (tmp & 0xC0) >> 6;
    //dlog_info("C_SIZE = %d\n", pCardInfo->SD_csd.C_SIZE);  

    pCardInfo->SD_csd.MaxRdCurrentVDDMin = (tmp & 0x38) >> 3;
    pCardInfo->SD_csd.MaxRdCurrentVDDMax = (tmp & 0x07);

    /* Byte 9 */
    tmp = (uint8_t)((hsd->CSD[1] & 0x00FF0000) >> 16);
    pCardInfo->SD_csd.MaxWrCurrentVDDMin = (tmp & 0xE0) >> 5;
    pCardInfo->SD_csd.MaxWrCurrentVDDMax = (tmp & 0x1C) >> 2;
    pCardInfo->SD_csd.C_SIZEMul      = (tmp & 0x03) << 1;
    
    /* Byte 10 */
    tmp = (uint8_t)((hsd->CSD[1] & 0x0000FF00) >> 8);
    pCardInfo->SD_csd.C_SIZEMul |= (tmp & 0x80) >> 7;
    //dlog_info("C_SIZEMul = %d\n", pCardInfo->SD_csd.C_SIZEMul);  

    pCardInfo->CardCapacity  = (pCardInfo->SD_csd.C_SIZE + 1) ;
    pCardInfo->CardCapacity *= (1 << (pCardInfo->SD_csd.C_SIZEMul + 2));
    pCardInfo->CardBlockSize = 1 << (pCardInfo->SD_csd.READ_BL_LEN);
    pCardInfo->CardCapacity *= pCardInfo->CardBlockSize;
    //dlog_info("CardCapacity = %d\n", pCardInfo->CardCapacity);
  }
  else if (hsd->CardType == HIGH_CAPACITY_SD_CARD)
  {
    /* Byte 7 */
    tmp = (uint8_t)(hsd->CSD[2] & 0x000000FF);
    pCardInfo->SD_csd.C_SIZE = (tmp & 0x3F) << 16;
    /* Byte 8 */
    tmp = (uint8_t)((hsd->CSD[1] & 0xFF000000) >> 24);
    pCardInfo->SD_csd.C_SIZE |= (tmp << 8);
    /* Byte 9 */
    tmp = (uint8_t)((hsd->CSD[1] & 0x00FF0000) >> 16);
    pCardInfo->SD_csd.C_SIZE |= (tmp);
    /* Byte 10 */
    tmp = (uint8_t)((hsd->CSD[1] & 0x0000FF00) >> 8);

    pCardInfo->CardCapacity  = ((pCardInfo->SD_csd.C_SIZE + 1)) * 1024;
    dlog_info("CardCapacity = %d BLKs\n", pCardInfo->CardCapacity);
    pCardInfo->CardBlockSize = 512;
  }
  else
  {
    dlog_info("Error: Not supported card type\n");
    /* Not supported card type */
    errorstate = SD_ERROR;
  }

  pCardInfo->SD_csd.ERASE_BLK_EN = (tmp & 0x40) >> 6;
  pCardInfo->SD_csd.SECTOR_SIZE  = (tmp & 0x3F) << 1;

  /* Byte 11 */
  tmp = (uint8_t)(hsd->CSD[1] & 0x000000FF);
  pCardInfo->SD_csd.SECTOR_SIZE     |= (tmp & 0x80) >> 7;
  pCardInfo->SD_csd.WP_GRP_SIZE = (tmp & 0x7F);
  //dlog_info("sector size = %d\n", pCardInfo->SD_csd.SECTOR_SIZE);
  /* Byte 12 */
  tmp = (uint8_t)((hsd->CSD[0] & 0xFF000000) >> 24);
  pCardInfo->SD_csd.WP_GRP_ENABLE = (tmp & 0x80) >> 7;
  pCardInfo->SD_csd.ManDeflECC        = (tmp & 0x60) >> 5;
  pCardInfo->SD_csd.R2W_FACTOR       = (tmp & 0x1C) >> 2;
  pCardInfo->SD_csd.WRITE_BL_LEN     = (tmp & 0x03) << 2;

  /* Byte 13 */
  tmp = (uint8_t)((hsd->CSD[0] & 0x00FF0000) >> 16);
  pCardInfo->SD_csd.WRITE_BL_LEN      |= (tmp & 0xC0) >> 6;
  pCardInfo->SD_csd.WRITE_BL_PARTIAL = (tmp & 0x20) >> 5;
  pCardInfo->SD_csd.Reserved3           = 0;

  /* Byte 14 */
  tmp = (uint8_t)((hsd->CSD[0] & 0x0000FF00) >> 8);
  pCardInfo->SD_csd.FILE_FORMAT_GRP = (tmp & 0x80) >> 7;
  pCardInfo->SD_csd.COPY = (tmp & 0x40) >> 6;
  pCardInfo->SD_csd.PERM_WRITE_PROTECT    = (tmp & 0x20) >> 5;
  pCardInfo->SD_csd.TMP_WRITE_PROTECT    = (tmp & 0x10) >> 4;
  pCardInfo->SD_csd.FILE_FORMAT       = (tmp & 0x0C) >> 2;

  /* Byte 15 */
  tmp = (uint8_t)(hsd->CSD[0] & 0x000000FF);
  pCardInfo->SD_csd.CSD_CRC = (tmp & 0xFE) >> 1;
  pCardInfo->SD_csd.Reserved4 = 1;

  /* Byte 0 */
  tmp = (uint8_t)((hsd->CID[0] & 0xFF000000) >> 24);
  pCardInfo->SD_cid.ManufacturerID = tmp;
  /* Byte 1 */
  tmp = (uint8_t)((hsd->CID[0] & 0x00FF0000) >> 16);
  pCardInfo->SD_cid.OEM_AppliID = tmp << 8;

  /* Byte 2 */
  tmp = (uint8_t)((hsd->CID[0] & 0x000000FF00) >> 8);
  pCardInfo->SD_cid.OEM_AppliID |= tmp;

  /* Byte 3 */
  tmp = (uint8_t)(hsd->CID[0] & 0x000000FF);
  pCardInfo->SD_cid.ProdName1 = tmp << 24;

  /* Byte 4 */
  tmp = (uint8_t)((hsd->CID[1] & 0xFF000000) >> 24);
  pCardInfo->SD_cid.ProdName1 |= tmp << 16;

  /* Byte 5 */
  tmp = (uint8_t)((hsd->CID[1] & 0x00FF0000) >> 16);
  pCardInfo->SD_cid.ProdName1 |= tmp << 8;

  /* Byte 6 */
  tmp = (uint8_t)((hsd->CID[1] & 0x0000FF00) >> 8);
  pCardInfo->SD_cid.ProdName1 |= tmp;

  /* Byte 7 */
  tmp = (uint8_t)(hsd->CID[1] & 0x000000FF);
  pCardInfo->SD_cid.ProdName2 = tmp;

  /* Byte 8 */
  tmp = (uint8_t)((hsd->CID[2] & 0xFF000000) >> 24);
  pCardInfo->SD_cid.ProdRev = tmp;

  /* Byte 9 */
  tmp = (uint8_t)((hsd->CID[2] & 0x00FF0000) >> 16);
  pCardInfo->SD_cid.ProdSN = tmp << 24;

  /* Byte 10 */
  tmp = (uint8_t)((hsd->CID[2] & 0x0000FF00) >> 8);
  pCardInfo->SD_cid.ProdSN |= tmp << 16;

  /* Byte 11 */
  tmp = (uint8_t)(hsd->CID[2] & 0x000000FF);
  pCardInfo->SD_cid.ProdSN |= tmp << 8;

  /* Byte 12 */
  tmp = (uint8_t)((hsd->CID[3] & 0xFF000000) >> 24);
  pCardInfo->SD_cid.ProdSN |= tmp;

  /* Byte 13 */
  tmp = (uint8_t)((hsd->CID[3] & 0x00FF0000) >> 16);
  pCardInfo->SD_cid.Reserved1   |= (tmp & 0xF0) >> 4;
  pCardInfo->SD_cid.ManufactDate = (tmp & 0x0F) << 8;

  /* Byte 14 */
  tmp = (uint8_t)((hsd->CID[3] & 0x0000FF00) >> 8);
  pCardInfo->SD_cid.ManufactDate |= tmp;

  /* Byte 15 */
  tmp = (uint8_t)(hsd->CID[3] & 0x000000FF);
  pCardInfo->SD_cid.CID_CRC   = (tmp & 0xFE) >> 1;
  pCardInfo->SD_cid.Reserved2 = 1;

  return errorstate;
}

/**
  * @brief  Initializes all cards or single card as the case may be Card(s) come
  *         into standby state.
  * @param  hsd: SD handle
  * @retval SD Card error state
  */
//static SD_ErrorTypedef IdentificateCard(SD_HandleTypeDef *hsd)
static SD_ErrorTypedef IdentificateCard(SD_HandleTypeDef *hsd,SD_CardInfoTypedef *SDCardInfo)
{
  SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
  SD_ErrorTypedef errorstate = SD_OK;
  uint16_t sd_rca = 1;
  uint32_t cmd_done, response, get_val;
  if (hsd->CardType != SECURE_DIGITAL_IO_CARD)
  {
#ifdef ECHO
    dlog_info("Send CMD2\n");
#endif
    /* Send CMD2 ALL_SEND_CID */
    sdmmc_cmdinitstructure.Argument         = 0;
    sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_ALL_SEND_CID;
    sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R2;
    sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
        SDMMC_CMD_USE_HOLD_REG | \
        SDMMC_CMD_PRV_DAT_WAIT | \
        SDMMC_CMD_RESP_CRC     | \
        SDMMC_CMD_RESP_LONG    | \
        SDMMC_CMD_RESP_EXP;
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
    Core_SDMMC_WaiteCmdDone(hsd->Instance);
    /* Check for error conditions */
    errorstate = SD_CmdError(hsd);
    if (errorstate != SD_OK)
    {
      return errorstate;
    }
    /* Get Card identification number data */
    hsd->CID[0] = Core_SDMMC_GetResponse(hsd->Instance, SDMMC_RESP0);
    hsd->CID[1] = Core_SDMMC_GetResponse(hsd->Instance, SDMMC_RESP1);
    hsd->CID[2] = Core_SDMMC_GetResponse(hsd->Instance, SDMMC_RESP2);
    hsd->CID[3] = Core_SDMMC_GetResponse(hsd->Instance, SDMMC_RESP3);
    get_val = Core_SDMMC_GetRESP0(hsd->Instance);
    // dlog_info("CMD2 RESP0 = %x\n", get_val);
    get_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
    // dlog_info("CMD2 RINTSTS = %x\n", get_val);
    delay_ms(1000);
  }

  if ((hsd->CardType == STD_CAPACITY_SD_CARD)    ||  \
      (hsd->CardType == SECURE_DIGITAL_IO_COMBO_CARD) || (hsd->CardType == HIGH_CAPACITY_SD_CARD))
  {
    /* Send CMD3 SET_REL_ADDR with argument 0 */
    /* SD Card publishes its RCA. */
#ifdef ECHO
    dlog_info("Send CMD3\n");
#endif
    sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SEND_RELATIVE_ADDR;
    sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R6;
    sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
        SDMMC_CMD_USE_HOLD_REG | \
        SDMMC_CMD_PRV_DAT_WAIT | \
        SDMMC_CMD_RESP_CRC | \
        SDMMC_CMD_RESP_EXP;
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
    Core_SDMMC_WaiteCmdDone(hsd->Instance);
    /* Check for error conditions */
    errorstate = SD_CmdError(hsd);
    if (errorstate != SD_OK)
    {
      return errorstate;
    }
    response = Core_SDMMC_GetResponse(hsd->Instance, SDMMC_RESP0);
    sd_rca = (uint32_t)(response >> 16) & 0x0000FFFF;

    dlog_info("Card Relative Address = 0x%x\n", sd_rca);
    hsd->RCA = sd_rca;
    get_val = Core_SDMMC_GetRESP0(hsd->Instance);
    // dlog_info("CMD3 RESP0 = %x\n", get_val);
    get_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
    // dlog_info("CMD3 RINTSTS = %x\n", get_val);
  }

  if (hsd->CardType != SECURE_DIGITAL_IO_CARD)
  {
    /* Get the SD card RCA */
    hsd->RCA = sd_rca;
#ifdef ECHO
    dlog_info("Send CMD10\n");
#endif
    sdmmc_cmdinitstructure.Argument         = (uint32_t)(hsd->RCA << 16);
    sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SEND_CID;
    sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R2;
    sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
        SDMMC_CMD_USE_HOLD_REG | \
        SDMMC_CMD_PRV_DAT_WAIT | \
        SDMMC_CMD_RESP_CRC     | \
        SDMMC_CMD_RESP_LONG    | \
        SDMMC_CMD_RESP_EXP;
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
    Core_SDMMC_WaiteCmdDone(hsd->Instance);
    /* Check for error conditions */
    errorstate = SD_CmdError(hsd);
    if (errorstate != SD_OK)
    {
      return errorstate;
    }
    /* Get Card Specific Data */
    hsd->CID[3] = Core_SDMMC_GetResponse(hsd->Instance, SDMMC_RESP0);
    hsd->CID[2] = Core_SDMMC_GetResponse(hsd->Instance, SDMMC_RESP1);
    hsd->CID[1] = Core_SDMMC_GetResponse(hsd->Instance, SDMMC_RESP2);
    hsd->CID[0] = Core_SDMMC_GetResponse(hsd->Instance, SDMMC_RESP3);
  }

#ifdef ECHO
    dlog_info("Send CMD9\n");
#endif
    sdmmc_cmdinitstructure.Argument         = (uint32_t)(hsd->RCA << 16);
    sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SEND_CSD;
    sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R2;
    sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
        SDMMC_CMD_USE_HOLD_REG | \
        SDMMC_CMD_PRV_DAT_WAIT | \
        SDMMC_CMD_RESP_CRC     | \
        SDMMC_CMD_RESP_LONG    | \
        SDMMC_CMD_RESP_EXP;
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
    Core_SDMMC_WaiteCmdDone(hsd->Instance);
    /* Check for error conditions */
    errorstate = SD_CmdError(hsd);
    if (errorstate != SD_OK)
    {
      return errorstate;
    }
    /* Get Card Specific Data */
    /* Send CMD9 SEND_CSD with argument as card's RCA */
    hsd->CSD[0] = Core_SDMMC_GetResponse(hsd->Instance, SDMMC_RESP0);
    hsd->CSD[1] = Core_SDMMC_GetResponse(hsd->Instance, SDMMC_RESP1);
    hsd->CSD[2] = Core_SDMMC_GetResponse(hsd->Instance, SDMMC_RESP2);
    hsd->CSD[3] = Core_SDMMC_GetResponse(hsd->Instance, SDMMC_RESP3);

    /* Send CMD7 SDMMC_SEL_DESEL_CARD */
    sdmmc_cmdinitstructure.Argument         = (uint32_t)(hsd->RCA << 16);
    sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SEL_DESEL_CARD;
    sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
    sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
        SDMMC_CMD_USE_HOLD_REG | \
        SDMMC_CMD_PRV_DAT_WAIT | \
        SDMMC_CMD_RESP_CRC    | \
        SDMMC_CMD_RESP_EXP;
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
    /* waite for command finish*/
    Core_SDMMC_WaiteCmdDone(hsd->Instance);

    get_val = Core_SDMMC_GetRESP0(hsd->Instance);
    // dlog_info("CMD7 RESP0 = %x\n", get_val);
    get_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
    // dlog_info("CMD7 RINTSTS = %x\n", get_val);

    /* Send CMD55*/
  #ifdef ECHO
    dlog_info("Send CMD55\n");
  #endif
    sdmmc_cmdinitstructure.Argument         = (uint32_t)(hsd->RCA << 16);
    sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_APP_CMD;
    sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
        SDMMC_CMD_USE_HOLD_REG | \
        SDMMC_CMD_PRV_DAT_WAIT | \
        SDMMC_CMD_RESP_CRC | \
        SDMMC_CMD_RESP_EXP;
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
    /* Check for error conditions */
    Core_SDMMC_WaiteCmdDone(hsd->Instance);
    get_val = Core_SDMMC_GetRESP0(hsd->Instance);
    // dlog_info("CMD55 RESP0 = %x\n", get_val);
    get_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
    // dlog_info("CMD55 RINTSTS = %x\n", get_val);

    /* Send ACMD6 */
  #ifdef ECHO
    dlog_info("Send ACMD6\n");
  #endif
    /* define the data bus width */
    sdmmc_cmdinitstructure.Argument         = SDMMC_BUS_WIDE_4B;
    sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_APP_SD_SET_BUSWIDTH;
    sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
        SDMMC_CMD_USE_HOLD_REG | \
        SDMMC_CMD_PRV_DAT_WAIT | \
        SDMMC_CMD_RESP_CRC | \
        SDMMC_CMD_RESP_EXP;
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
    /* Check for error conditions */
    Core_SDMMC_WaiteCmdDone(hsd->Instance);
    get_val = Core_SDMMC_GetRESP0(hsd->Instance);
    // dlog_info("ACMD6 RESP0 = %x\n", get_val);
    get_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
    // dlog_info("ACMD6 RINTSTS = %x\n", get_val);

    /* Send CMD16*/
  #ifdef ECHO
    dlog_info("Send CMD16\n");
  #endif
    /* set the block length*/
    Core_SDMMC_SetCTYPE(hsd->Instance, SDMMC_CTYPE_4BIT);
    Core_SDMMC_SetCTRL(hsd->Instance, SDMMC_CTRL_USE_INTERNAL_IDMAC | \
                       SDMMC_CTRL_INT_ENABLE | \
                       SDMMC_CTRL_FIFO_RESET);
    Core_SDMMC_SetBLKSIZ(hsd->Instance, DATA_BLOCK_LEN);
    Core_SDMMC_SetBYCTNT(hsd->Instance, DATA_BYTE_CNT);

    sdmmc_cmdinitstructure.Argument         = DATA_BLOCK_LEN;
    sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SET_BLOCKLEN;
    sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
        SDMMC_CMD_USE_HOLD_REG | \
        SDMMC_CMD_PRV_DAT_WAIT | \
        SDMMC_CMD_RESP_CRC | \
        SDMMC_CMD_RESP_EXP;
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
    /* Check for error conditions */
    Core_SDMMC_WaiteCmdDone(hsd->Instance);

    get_val = Core_SDMMC_GetRESP0(hsd->Instance);
    // dlog_info("CMD16 RESP0 = %x\n", get_val);
    get_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
    // dlog_info("CMD16 RINTSTS = %x\n", get_val);
    /* All cards are initialized */
    return errorstate;
}

/**
  * @brief  Enquires cards about their operating voltage and configures clock
  *         controls and stores SD information that will be needed in future
  *         in the SD handle.
  * @param  hsd: SD handle
  * @retval SD Card error state
  */
static SD_ErrorTypedef PowerOnCard(SD_HandleTypeDef *hsd)
{
  //Core_SDMMC_SetINTMASK(hsd->Instance, 0xFFFFFFFF);
  Core_SDMMC_SetINTMASK(hsd->Instance, SDMMC_INTMASK_CARD_DETECT);
  /* clear intreq status */
  Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
  Core_SDMMC_SetCTRL(hsd->Instance, SDMMC_CTRL_INT_ENABLE );
  //SD clock = 50MHz
  // Core_SDMMC_SetCLKDIV(hsd->Instance, SDMMC_CLKDIV_BIT1);
  //SD clock = 1MHz
  //Core_SDMMC_SetCLKDIV(hsd->Instance, SDMMC_CLKDIV_BIT2 | \
                       SDMMC_CLKDIV_BIT5 | \
                       SDMMC_CLKDIV_BIT6 );
  //SD clock = 25MHz
  Core_SDMMC_SetCLKDIV(hsd->Instance, SDMMC_CLKDIV_BIT2);
  Core_SDMMC_SetCLKSRC(hsd->Instance, SDMMC_CLKSRC_CLKDIV0);
  Core_SDMMC_SetCLKENA(hsd->Instance, 0x0000FFFF);

  return 0;
}

static SD_ErrorTypedef InitializeCard(SD_HandleTypeDef *hsd)
{

  SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
  __IO SD_ErrorTypedef errorstate = SD_OK;
  uint32_t get_val = 0, vs_busy = 0, loop = 0;
  uint32_t response;

  hsd->CardType = STD_CAPACITY_SD_CARD;

  get_val = Core_SDMMC_GetCDETECT(hsd->Instance);
  if (get_val != 0)
  {
    dlog_info("No Card Detected!\n");
    return SD_NOTCARD;
  }

  /* send CMD0 first*/
  sdmmc_cmdinitstructure.Argument         = 0;
  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_GO_IDLE_STATE;
  sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_NO;
  sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
      SDMMC_CMD_USE_HOLD_REG | \
      SDMMC_CMD_UPDATE_CLK | \
      SDMMC_CMD_PRV_DAT_WAIT;
  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
  Core_SDMMC_WaiteCmdStart(hsd->Instance);

  Core_SDMMC_SetTMOUT(hsd->Instance, SDMMC_TMOUT_DEFAULT);
  Core_SDMMC_SetCTYPE(hsd->Instance, SDMMC_CTYPE_1BIT);
  /* FIFO_DEPTH = 16 words */
  Core_SDMMC_SetFIFOTH(hsd->Instance, 0x00070008);
  Core_SDMMC_SetCTRL(hsd->Instance, SDMMC_CTRL_USE_INTERNAL_IDMAC | \
                     SDMMC_CTRL_INT_ENABLE);
  Core_SDMMC_SetBMOD(hsd->Instance, SDMMC_BMOD_ENABLE | \
                     SDMMC_BMOD_FB);
  /* close the FIFO RX and TX interrupt */
  //Core_SDMMC_SetINTMASK(hsd->Instance, (uint32_t)0x0000FFFF & \
                        ~(SDMMC_INTMASK_TXDR | SDMMC_INTMASK_RXDR));
  Core_SDMMC_SetDBADDR(hsd->Instance, 0x440B5000);
  Core_SDMMC_SetIDINTEN(hsd->Instance, 0xFFFFFFFF);


// #ifdef  ENUMERATE
//   dlog_info("enter the SD_ENUM\n");
//   errorstate = SD_ENUM(hsd);
//   if (errorstate != SD_OK)
//   {
//     dlog_info("SD_ENUM state = %d\n", errorstate);
//     return errorstate;
//   }
// #endif

  /* CMD0: GO_IDLE_STATE -----------------------------------------------------*/
  /* No CMD get_val required */
#ifdef ECHO
  dlog_info("send CMD0\n");
#endif
  sdmmc_cmdinitstructure.Argument         = 0;
  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_GO_IDLE_STATE;
  sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_NO;
  sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
      SDMMC_CMD_USE_HOLD_REG | \
      SDMMC_CMD_PRV_DAT_WAIT;
  /* clear intreq status */
  Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
  /* Check for error conditions */
  errorstate = SD_CmdError(hsd);
  if (errorstate != SD_OK)
  {
    /* CMD Response Timeout (wait for CMDSENT flag) */
    return errorstate;
  }

  get_val = Core_SDMMC_GetRESP0(hsd->Instance);
  // dlog_info("CMD0 RESP0 = %x\n", get_val);
  get_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
  // dlog_info("CMD0 RINTSTS = %x\n", get_val);


#ifdef ECHO
  dlog_info("send CMD8\n");
#endif

  sdmmc_cmdinitstructure.Argument         = SD_CHECK_PATTERN;
  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SEND_IF_COND;
  sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R7;
  sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
      SDMMC_CMD_USE_HOLD_REG | \
      SDMMC_CMD_PRV_DAT_WAIT | \
      SDMMC_CMD_RESP_CRC | \
      SDMMC_CMD_RESP_EXP;
  Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
  /* Check for error conditions */
  errorstate = SD_CmdError(hsd);
  if (errorstate != SD_OK)
  {
    /* CMD Response Timeout (wait for CMDSENT flag) */
    return errorstate;
  }
  errorstate = SD_CmdResp7(hsd);
  if (errorstate != SD_OK)
  {
    dlog_info("CMD8: SD_UNSUPPORTED_VOLTAGE\n");
  }
  get_val = Core_SDMMC_GetRESP0(hsd->Instance);
  // dlog_info("CMD8 RESP0 = %x\n", get_val);
  get_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
  // dlog_info("CMD8 RINTSTS = %x\n", get_val);

  /* Send CMD55 */
#ifdef ECHO
  dlog_info("Send CMD55\n");
#endif
  sdmmc_cmdinitstructure.Argument         = 0;
  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_APP_CMD;
  sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
      SDMMC_CMD_USE_HOLD_REG | \
      SDMMC_CMD_PRV_DAT_WAIT | \
      SDMMC_CMD_RESP_CRC | \
      SDMMC_CMD_RESP_EXP;
  Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
  /* Check for error conditions */
  errorstate = SD_CmdError(hsd);
  if (errorstate != SD_OK)
  {
    /* CMD Response Timeout (wait for CMDSENT flag) */
    return errorstate;
  }
  get_val = Core_SDMMC_GetRESP0(hsd->Instance);
  // dlog_info("CMD55 RESP0 = %x\n", get_val);
  get_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
  // dlog_info("CMD55 RINTSTS = %x\n", get_val);

  /* Send ACMD41 */
#ifdef ECHO
  dlog_info("Send ACMD41\n");
#endif
  //sdmmc_cmdinitstructure.Argument         = SD_ACMD41_HCS | \
                                            SD_ACMD41_XPC | \
                                            SD_ACMD41_S18R;
  sdmmc_cmdinitstructure.Argument         = 0x51FF8000;
  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_APP_SD_SEND_OP_COND;
  sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R3;
  sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
      SDMMC_CMD_USE_HOLD_REG | \
      SDMMC_CMD_PRV_DAT_WAIT | \
      SDMMC_CMD_RESP_EXP;
  Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
  /* Check for error conditions */
  errorstate = SD_CmdError(hsd);
  if (errorstate != SD_OK)
  {
    /* CMD Response Timeout (wait for CMDSENT flag) */
    return errorstate;
  }
  /* Get command get_val */
  response = Core_SDMMC_GetResponse(hsd->Instance, SDMMC_RESP0);
  get_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
  //dlog_info("ACMD41 RINTSTS = 0x%x\n", get_val);
  response = Core_SDMMC_GetRESP0(hsd->Instance);
  //dlog_info("ACMD41 RESP0 = 0x%x\n", response);
  vs_busy = (response & SD_ACMD41_BUSY);

  if (vs_busy == 0) {
#ifdef ECHO
    dlog_info("ACMD Loop Start:\n");
#endif
    do {

      loop++;
      dlog_info("LOOP: Send CMD55\n");
      /* Send CMD55 */
      sdmmc_cmdinitstructure.Argument         = 0;
      sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_APP_CMD;
      sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
          SDMMC_CMD_USE_HOLD_REG | \
          SDMMC_CMD_PRV_DAT_WAIT | \
          SDMMC_CMD_RESP_CRC | \
          SDMMC_CMD_RESP_EXP;
      Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
      Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
      /* Check for error conditions */
      errorstate = SD_CmdError(hsd);
      if (errorstate != SD_OK)
      {
        /* CMD Response Timeout (wait for CMDSENT flag) */
        return errorstate;
      }

      response = Core_SDMMC_GetRESP0(hsd->Instance);
      // dlog_info("CMD55 RESP0 = %x\n", response);
      get_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
      // dlog_info("CMD55 RINTSTS = %x\n", get_val);
      /* Send ACMD41 */
      sdmmc_cmdinitstructure.Argument         = 0x51FF8000;
      //sdmmc_cmdinitstructure.Argument         = SD_ACMD41_HCS | \
                                                SD_ACMD41_XPC | \
                                                SD_ACMD41_S18R |\
                                                0xFFF000;
      sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_APP_SD_SEND_OP_COND;
      sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R3;
      sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
          SDMMC_CMD_USE_HOLD_REG | \
          SDMMC_CMD_PRV_DAT_WAIT | \
          SDMMC_CMD_RESP_EXP;
      Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
      Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
      /* Get command get_val */
      response = Core_SDMMC_GetResponse(hsd->Instance, SDMMC_RESP0);
      vs_busy = (response & SD_ACMD41_BUSY);

      get_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
      // dlog_info("ACMD41 RINTSTS = %x\n", get_val);
      response = Core_SDMMC_GetRESP0(hsd->Instance);
      // dlog_info("ACMD41 RESP0 = %x\n", response);
      if (loop == 1000) 
      {
#ifdef ECHO
        dlog_info("ACMD41 Loop Fail\n");
#endif
      }
      vs_busy = (response & SD_ACMD41_BUSY);
    } while (vs_busy == 0);

    if (vs_busy != 0) 
    {
#ifdef ECHO
      dlog_info("ACMD41 Loop Done\n");
#endif
    }
  }

  if (response & SD_ACMD41_HCS) 
  {
    dlog_info("This is SDHC/SDXC card!\n");
    hsd->CardType = HIGH_CAPACITY_SD_CARD;
  }
  else
  {
    dlog_info("This is SDSC card!\n");
  }

 if (response & SD_ACMD41_S18R) 
  {
    dlog_info("1.8v Support\n");
#ifdef ECHO
    dlog_info("Send CMD11\n");
#endif
    /* send CMD11 to switch 1.8V bus signaling level */
    Core_SDMMC_SetCLKENA(hsd->Instance, 0x0000FFFF);
    sdmmc_cmdinitstructure.Argument  = 0x41ffffff;
    sdmmc_cmdinitstructure.CmdIndex  = SD_CMD_VOLTAGE_SWITCH;
    sdmmc_cmdinitstructure.Response  = SDMMC_RESPONSE_R1;
    sdmmc_cmdinitstructure.Attribute = SDMMC_CMD_START_CMD | \
                                       SDMMC_CMD_USE_HOLD_REG | \
                                       SDMMC_CMD_VOLT_SWITCH  | \
                                       SDMMC_CMD_PRV_DAT_WAIT | \
                                       SDMMC_CMD_RESP_EXP  | \
                                       SDMMC_CMD_RESP_CRC;
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
#ifdef ECHO
    dlog_info("1.8v Switch Start\n");
#endif
    Core_SDMMC_WaiteCmdStart(hsd->Instance);
    Core_SDMMC_WaiteVoltSwitchInt(hsd->Instance);
    dlog_info("CMD11 RESP 1.8v Switch Success\n");
    get_val = Core_SDMMC_GetRESP0(hsd->Instance);
    // dlog_info("CMD11 RESP0 = %x\n", get_val);
    get_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
    // dlog_info("CMD11 RINTSTS = %x\n", get_val);


    /* disable all the clock */
    Core_SDMMC_SetCLKENA(hsd->Instance, 0x00000000);
    Core_SDMMC_SetUHSREG(hsd->Instance, 0x0000FFFF);

    sdmmc_cmdinitstructure.Argument  = 0x00000000;
    sdmmc_cmdinitstructure.CmdIndex  = SD_CMD_GO_IDLE_STATE;
    sdmmc_cmdinitstructure.Response  = SDMMC_RESPONSE_NO;
    sdmmc_cmdinitstructure.Attribute = SDMMC_CMD_START_CMD | \
                                       SDMMC_CMD_USE_HOLD_REG | \
                                       SDMMC_CMD_VOLT_SWITCH  | \
                                       SDMMC_CMD_PRV_DAT_WAIT | \
                                       SDMMC_CMD_UPDATE_CLK;
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
    get_val = Core_SDMMC_GetRESP0(hsd->Instance);
    // dlog_info("CMD0 RESP0 = %x\n", get_val);
    get_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
    // dlog_info("CMD0 RINTSTS = %x\n", get_val);
    delay_ms(500);
#ifdef ECHO
    dlog_info("Host Supply 1.8v Clock\n");
#endif

    Core_SDMMC_SetCLKENA(hsd->Instance, 0x0000FFFF);

    sdmmc_cmdinitstructure.Argument  = 0x00000000;
    sdmmc_cmdinitstructure.CmdIndex  = SD_CMD_GO_IDLE_STATE;
    sdmmc_cmdinitstructure.Response  = SDMMC_RESPONSE_NO;
    sdmmc_cmdinitstructure.Attribute = SDMMC_CMD_START_CMD | \
                                       SDMMC_CMD_USE_HOLD_REG | \
                                       SDMMC_CMD_VOLT_SWITCH  | \
                                       SDMMC_CMD_PRV_DAT_WAIT | \
                                       SDMMC_CMD_UPDATE_CLK;
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
    Core_SDMMC_WaiteVoltSwitchInt(hsd->Instance);
    get_val = Core_SDMMC_GetRESP0(hsd->Instance);
    //dlog_info("CMD0 RESP0 = %x\n", get_val);
    get_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
    //dlog_info("CMD0 RINTSTS = %x\n", get_val);
    dlog_info("Voltage Switching Success! \n");
  }
  else {
    dlog_info("1.8v not support\n");
  }
  return errorstate;

}
/**
  * @brief  deselect the card.
  * @param  hsd: SD handle
  * @retval SD Card error state
  */
static SD_ErrorTypedef PowerOffCard(SD_HandleTypeDef *hsd)
{
  SD_ErrorTypedef errorstate = SD_OK;
  SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;

  /* Set Power State to OFF */
  sdmmc_cmdinitstructure.Argument         = 0xFFFFFFFF;
  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SEL_DESEL_CARD;
  sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
  sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
      SDMMC_CMD_USE_HOLD_REG | \
      SDMMC_CMD_PRV_DAT_WAIT | \
      SDMMC_CMD_RESP_CRC | \
      SDMMC_CMD_RESP_EXP;
  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
  /* Check for error conditions */
  errorstate = SD_CmdError(hsd);
  if (errorstate != SD_OK)
  {
    return errorstate;
  }
  /* disable all mmc interrupt first */
  // Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
  // Core_SDMMC_SetINTMASK(hsd->Instance, 0x0);
  // /* disable clock to CIU */
  // Core_SDMMC_SetCLKENA(hsd->Instance, 0x0);
  // Core_SDMMC_SetCLKSRC(hsd->Instance, 0x0);

  Core_SDMMC_SetBMOD(hsd->Instance, SDMMC_BMOD_SWRESET);
  Core_SDMMC_SetCTRL(hsd->Instance, SDMMC_CTRL_CONTROLLER_RESET |\
                                    SDMMC_CTRL_FIFO_RESET |\
                                    SDMMC_CTRL_DMA_RESET);
  Core_SDMMC_SetRST_N(hsd->Instance, 0x0);
  Core_SDMMC_SetPWREN(hsd->Instance, 0x0);
  delay_ms(1);
  // uint32_t get_val;
  // dlog_info("Following are the SD register status\n");
  // get_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
  // dlog_info("RINTSTS = 0x%08x\n", get_val);
  // get_val = Core_SDMMC_GetSTATUS(hsd->Instance);
  // dlog_info("STATUS = 0x%08x\n", get_val);
  // get_val = Core_SDMMC_GetDBADDR(hsd->Instance);
  // dlog_info("DBADDR = 0x%08x\n", get_val);
  // get_val = Core_SDMMC_GetCTRL(hsd->Instance);
  // dlog_info("CTRL = 0x%08x\n", get_val);
  // get_val = Core_SDMMC_GetPWREN(hsd->Instance);
  // dlog_info("PWREN = 0x%08x\n", get_val);
  // get_val = Core_SDMMC_GetINTMASK(hsd->Instance);
  // dlog_info("INTMASK = 0x%08x\n", get_val);
  // get_val = Core_SDMMC_GetMINTSTS(hsd->Instance);
  // dlog_info("MINTSTS = 0x%08x\n", get_val);
  //Core_SDMMC_SetPWREN(hsd->Instance, SDMMC_PWREN_0 | SDMMC_PWREN_1);
  Core_SDMMC_SetINTMASK(hsd->Instance, SDMMC_INTMASK_CARD_DETECT);
  /* clear intreq status */
  Core_SDMMC_SetCTRL(hsd->Instance, SDMMC_CTRL_INT_ENABLE );
  Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);

  return errorstate;
}

/**
  * @brief  Returns the current card's status.
  * @param  hsd: SD handle
  * @param  pCardStatus: pointer to the buffer that will contain the SD card
  *         status (Card Status register)
  * @retval SD Card error state
  */
SD_ErrorTypedef SD_GetState(SD_HandleTypeDef *hsd, uint32_t *CardStatus)
{
  SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
  SD_ErrorTypedef errorstate = SD_OK;

  /* Send Status command */
  sdmmc_cmdinitstructure.Argument         = (uint32_t)(hsd->RCA << 16);
  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SEND_STATUS;
  sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
  sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
      SDMMC_CMD_USE_HOLD_REG | \
      SDMMC_CMD_PRV_DAT_WAIT | \
      SDMMC_CMD_RESP_CRC | \
      SDMMC_CMD_RESP_EXP;
  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
  /* Check for error conditions */
  errorstate = SD_CmdError(hsd);
  if (errorstate != SD_OK)
  {
    return errorstate;
  }
  /* Get SD card status */
  *CardStatus = Core_SDMMC_GetRESP0(hsd->Instance);
  dlog_info("Card State = %x\n", *CardStatus);

  return errorstate;
}

/**TODO
  * @brief  Checks for error conditions for CMD0.
  * @param  hsd: SD handle
  * @retval SD Card error state
  */
static SD_ErrorTypedef SD_CmdError(SD_HandleTypeDef *hsd)
{
  SD_ErrorTypedef errorstate = SD_OK;
  uint32_t RINTSTS_val, cmd_done, endBitErr, startBitErr, hdLockErr, datStarvTo, resp_to;
  uint32_t datCrcErr, resp_crc_err, datTranOver, resp_err, errHappen;
  do {
    RINTSTS_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
    cmd_done     = (RINTSTS_val & SDMMC_RINTSTS_CMD_DONE);

    endBitErr    = (RINTSTS_val & SDMMC_RINTSTS_EBE); //[15]
    startBitErr  = (RINTSTS_val & SDMMC_RINTSTS_SBE); //[13]
    hdLockErr    = (RINTSTS_val & SDMMC_RINTSTS_HLE); //[12]
    datStarvTo   = (RINTSTS_val & SDMMC_RINTSTS_HTO); //[10]
    resp_to      = (RINTSTS_val & SDMMC_RINTSTS_RTO); //[8]
    datCrcErr    = (RINTSTS_val & SDMMC_RINTSTS_DCRC); //[7]
    resp_crc_err = (RINTSTS_val & SDMMC_RINTSTS_RCRC); //[6]
    datTranOver  = (RINTSTS_val & SDMMC_RINTSTS_DATA_OVER); //[3]
    resp_err     = (RINTSTS_val & SDMMC_RINTSTS_RESP_ERR); //[1]
  } while (!cmd_done);

  // errHappen = endBitErr | startBitErr | hdLockErr | resp_to | datCrcErr | resp_crc_err | resp_err;
  errHappen = endBitErr | startBitErr | hdLockErr | datCrcErr | resp_crc_err | resp_err;
  if (errHappen) {
#ifdef ECHO
    dlog_info("CMD ERROR\n");
#endif
    return SD_ERROR;
  }
  return errorstate;
}

/**
  * @brief  Checks for error conditions for R7 response.
  * @param  hsd: SD handle
  * @retval SD Card error state
  */
static SD_ErrorTypedef SD_CmdResp7(SD_HandleTypeDef *hsd)
{
  SD_ErrorTypedef errorstate = SD_OK;
  uint32_t RESP_val;
  RESP_val = Core_SDMMC_GetResponse(hsd->Instance, SDMMC_RESP0);
// #ifdef ECHO
//   dlog_info("CMD8 RESP = 0x%08x\n",RESP_val);
// #endif
  if (RESP_val == SD_CHECK_PATTERN) {
#ifdef ECHO
//     dlog_info("CMD8 Resp Right: v2\n");
#endif
  }
  else {
#ifdef ECHO
//     dlog_info("CMD8 Resp Error: v2\n");
#endif
    errorstate = SD_UNSUPPORTED_VOLTAGE;
  }
  return errorstate;
}

/**
  * @brief  Checks for error conditions for R1 response.
  * @param  hsd: SD handle
  * @param  SD_CMD: The sent command index
  * @retval SD Card error state
  */
static SD_ErrorTypedef SD_CmdResp1Error(SD_HandleTypeDef *hsd, uint32_t SD_CMD)
{
  SD_ErrorTypedef errorstate = SD_OK;
  uint32_t response_r1;

  /* We have received response, retrieve it for analysis  */
  response_r1 = Core_SDMMC_GetResponse(hsd->Instance, SDMMC_RESP0);


  if ((response_r1 & SDMMC_RESP1_OUT_OF_RANGE) == SDMMC_RESP1_OUT_OF_RANGE)
  {
    return (SD_OUT_OF_RANGE);
  }

  if ((response_r1 & SDMMC_RESP1_ADDRESS_ERROR) == SDMMC_RESP1_ADDRESS_ERROR)
  {
    return (SD_ADDRESS_ERROR);
  }

  if ((response_r1 & SDMMC_RESP1_BLOCK_LEN_ERROR) == SDMMC_RESP1_BLOCK_LEN_ERROR)
  {
    return (SD_BLOCK_LEN_ERROR);
  }

  if ((response_r1 & SDMMC_RESP1_ERASE_SEQ_ERROR) == SDMMC_RESP1_ERASE_SEQ_ERROR)
  {
    return (SD_ERASE_SEQ_ERROR);
  }

  if ((response_r1 & SDMMC_RESP1_ERASE_PARAM) == SDMMC_RESP1_ERASE_PARAM)
  {
    return (SD_ERASE_PARAM);
  }

  if ((response_r1 & SDMMC_RESP1_WP_VIOLATION) == SDMMC_RESP1_WP_VIOLATION)
  {
    return (SD_WP_VIOLATION);
  }

  if ((response_r1 & SDMMC_RESP1_CARD_IS_LOCKED) == SDMMC_RESP1_CARD_IS_LOCKED)
  {
    return (SD_CARD_IS_LOCKED);
  }

  if ((response_r1 & SDMMC_RESP1_LOCK_UNLOCK_FAILED) == SDMMC_RESP1_LOCK_UNLOCK_FAILED)
  {
    return (SD_LOCK_UNLOCK_FAILED);
  }

  if ((response_r1 & SDMMC_RESP1_COM_CRC_ERROR) == SDMMC_RESP1_COM_CRC_ERROR)
  {
    return (SD_COM_CRC_ERROR);
  }

  if ((response_r1 & SDMMC_RESP1_ILLEGAL_COMMAND) == SDMMC_RESP1_ILLEGAL_COMMAND)
  {
    return (SD_ILLEGAL_COMMAND);
  }

  if ((response_r1 & SDMMC_RESP1_CARD_ECC_FAILED) == SDMMC_RESP1_CARD_ECC_FAILED)
  {
    return (SD_CARD_ECC_FAILED);
  }

  if ((response_r1 & SDMMC_RESP1_CC_ERROR) == SDMMC_RESP1_CC_ERROR)
  {
    return (SD_CC_ERROR);
  }

  if ((response_r1 & SDMMC_RESP1_ERROR) == SDMMC_RESP1_ERROR)
  {
    return (SD_GENERAL_UNKNOWN_ERROR);
  }

  if ((response_r1 & SDMMC_RESP1_CSD_OVERWRITE) == SDMMC_RESP1_CSD_OVERWRITE)
  {
    return (SD_CSD_OVERWRITE);
  }

  if ((response_r1 & SDMMC_RESP1_WP_ERASE_SKIP) == SDMMC_RESP1_WP_ERASE_SKIP)
  {
    return (SD_WP_ERASE_SKIP);
  }

  if ((response_r1 & SDMMC_RESP1_CARD_ECC_DISABLED) == SDMMC_RESP1_CARD_ECC_DISABLED)
  {
    return (SD_CARD_ECC_DISABLED);
  }

  if ((response_r1 & SDMMC_RESP1_ERASE_RESET) == SDMMC_RESP1_ERASE_RESET)
  {
    return (SD_ERASE_RESET);
  }

  if ((response_r1 & SDMMC_RESP1_AKE_SEQ_ERROR) == SDMMC_RESP1_AKE_SEQ_ERROR)
  {
    return (SD_AKE_SEQ_ERROR);
  }

  return errorstate;
}

/**
  * @brief  Checks if the SD card is in programming state.
  * @param  hsd: SD handle
  * @param  pStatus: pointer to the variable that will contain the SD card state
  * @retval SD Card error state
  */
static SD_ErrorTypedef SD_IsCardProgramming(SD_HandleTypeDef * hsd, uint8_t *status)
{
  SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
  uint32_t responseR1 = 0;
  SD_ErrorTypedef errorstate = SD_OK;
  sdmmc_cmdinitstructure.Argument         = (uint32_t)(hsd->RCA << 16);
  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SEND_STATUS;
  sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
  sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
      SDMMC_CMD_USE_HOLD_REG | \
      SDMMC_CMD_PRV_DAT_WAIT | \
      SDMMC_CMD_RESP_CRC     | \
      SDMMC_CMD_RESP_EXP;
  Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);

  /* We have received response, retrieve it for analysis */
  responseR1 = Core_SDMMC_GetResponse(hsd->Instance, SDMMC_RESP0);

  if ((responseR1 & SDMMC_RESP1_OUT_OF_RANGE) == SDMMC_RESP1_OUT_OF_RANGE)
  {
    return (SD_OUT_OF_RANGE);
  }

  if ((responseR1 & SDMMC_RESP1_ADDRESS_ERROR) == SDMMC_RESP1_ADDRESS_ERROR)
  {
    return (SD_ADDRESS_ERROR);
  }

  if ((responseR1 & SDMMC_RESP1_CARD_IS_LOCKED) == SDMMC_RESP1_CARD_IS_LOCKED)
  {
    return (SD_CARD_IS_LOCKED);
  }

  if ((responseR1 & SDMMC_RESP1_LOCK_UNLOCK_FAILED) == SDMMC_RESP1_LOCK_UNLOCK_FAILED)
  {
    return (SD_LOCK_UNLOCK_FAILED);
  }

  if ((responseR1 & SDMMC_RESP1_COM_CRC_ERROR) == SDMMC_RESP1_COM_CRC_ERROR)
  {
    return (SD_COM_CRC_ERROR);
  }

  if ((responseR1 & SDMMC_RESP1_ILLEGAL_COMMAND) == SDMMC_RESP1_ILLEGAL_COMMAND)
  {
    return (SD_ILLEGAL_COMMAND);
  }

  if ((responseR1 & SDMMC_RESP1_CARD_ECC_FAILED) == SDMMC_RESP1_CARD_ECC_FAILED)
  {
    return (SD_CARD_ECC_FAILED);
  }

  if ((responseR1 & SDMMC_RESP1_CC_ERROR) == SDMMC_RESP1_CC_ERROR)
  {
    return (SD_CC_ERROR);
  }

  if ((responseR1 & SDMMC_RESP1_ERROR) == SDMMC_RESP1_ERROR)
  {
    return (SD_GENERAL_UNKNOWN_ERROR);
  }

  if ((responseR1 & SDMMC_RESP1_CARD_ECC_DISABLED) == SDMMC_RESP1_CARD_ECC_DISABLED)
  {
    return (SD_CARD_ECC_DISABLED);
  }

  if ((responseR1 & SDMMC_RESP1_AKE_SEQ_ERROR) == SDMMC_RESP1_AKE_SEQ_ERROR)
  {
    return (SD_AKE_SEQ_ERROR);
  }

  /* Find out card status */
  *status = responseR1 & SDMMC_RESP1_CURRENT_STATE;

  return errorstate;
}

/**
  * @brief: identify if it is a sd card
  */
static SD_ErrorTypedef SD_ENUM(SD_HandleTypeDef * hsd)
{
  SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
  __IO SD_ErrorTypedef errorstate = SD_OK;
  uint32_t cmd_done, RINTSTS_val, RINTSTS_to, RINTSTS_crc_err, RINTSTS_err, RINTSTS_fail, RINTSTS_rto;
  uint32_t RESP_val, cmd_illegal, sdio_mem, sd_mem;
  sdmmc_cmdinitstructure.Argument         = 0;
  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_IO_SEND_OP_COND;
  sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_NO;
  sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
      SDMMC_CMD_USE_HOLD_REG | \
      SDMMC_CMD_SEND_INIT | \
      SDMMC_CMD_PRV_DAT_WAIT | \
      SDMMC_CMD_RESP_CRC | \
      SDMMC_CMD_RESP_EXP;
  Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
  RINTSTS_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
#ifdef ECHO
  dlog_info("CMD5 RINTSTS_val = %x\n", RINTSTS_val);
#endif
  do {
    RINTSTS_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
    cmd_done = (RINTSTS_val & SDMMC_RINTSTS_CMD_DONE);
  } while (!cmd_done);

  RINTSTS_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
#ifdef ECHO
  dlog_info("SD_ENUM: RINTSTS = %x\n", RINTSTS_val);
#endif
  RINTSTS_rto = (RINTSTS_val & SDMMC_RINTSTS_RTO);
  RINTSTS_crc_err = (RINTSTS_val & SDMMC_RINTSTS_RCRC);
  RINTSTS_err = (RINTSTS_val & SDMMC_RINTSTS_RESP_ERR);
  RINTSTS_fail = RINTSTS_rto & RINTSTS_crc_err & RINTSTS_err;
  if (RINTSTS_fail)
  {
    if (RINTSTS_rto)
    {
      dlog_info("RTO\n");
    }
    if (RINTSTS_crc_err)
    {
      dlog_info("RCRC\n");
    }
    if (RINTSTS_err)
    {
      dlog_info("RESP_ERR\n");
    }
  }
  RESP_val = Core_SDMMC_GetResponse(hsd->Instance, SDMMC_RESP0);
  cmd_illegal = (RESP_val & 0x00400000);
  sdio_mem = (RESP_val & 0x08000000);
  sd_mem = RINTSTS_rto || sdio_mem || cmd_illegal;
  if (!sd_mem) {
    return SD_NOTCARD;
  }
  return errorstate;
}

/**
  * @ configuration the register for dma
  */
static SD_ErrorTypedef SD_DMAConfig(SD_HandleTypeDef * hsd, SDMMC_DMATransTypeDef * dma)
{
  SD_ErrorTypedef errorstate = SD_OK;
  SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
  uint32_t get_val, cmd_done;

  Core_SDMMC_SetUHSREG(hsd->Instance, 0x0000FFFF);
  Core_SDMMC_SetCTYPE(hsd->Instance, SDMMC_CTYPE_4BIT);
  Core_SDMMC_SetCTRL(hsd->Instance, SDMMC_CTRL_USE_INTERNAL_IDMAC | \
                     SDMMC_CTRL_INT_ENABLE | \
                     SDMMC_CTRL_FIFO_RESET);
  Core_SDMMC_SetBLKSIZ(hsd->Instance, dma->BlockSize);
  Core_SDMMC_SetBYCTNT(hsd->Instance, dma->SectorNum * dma->BlockSize);

  /* Set Block Size for Card */
  sdmmc_cmdinitstructure.Argument         = dma->BlockSize;
  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SET_BLOCKLEN;
  sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
  sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
      SDMMC_CMD_USE_HOLD_REG | \
      SDMMC_CMD_PRV_DAT_WAIT | \
      SDMMC_CMD_RESP_CRC     | \
      SDMMC_CMD_RESP_EXP;
  Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
  /* Check for error conditions */
  Core_SDMMC_WaiteCmdDone(hsd->Instance);

#ifdef ECHO
  get_val = Core_SDMMC_GetRESP0(hsd->Instance);
  dlog_info("CMD16 RESP0 = 0x%x\n", get_val);
#endif

  /* set up idma descriptor */
  Core_SDMMC_SetBMOD(hsd->Instance, SDMMC_BMOD_ENABLE | SDMMC_BMOD_FB);
  Core_SDMMC_SetIDINTEN(hsd->Instance, 0x0);
  Core_SDMMC_SetDBADDR(hsd->Instance, dma->ListBaseAddr);

  return errorstate;
}
