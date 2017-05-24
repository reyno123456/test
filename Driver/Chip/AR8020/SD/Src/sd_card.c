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
  *           + bug fix for status check after erase (wumin) 2017-4-21
  *           + some for stability improvement (wumin) 2017-5-21
  */

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>
#include "reg_rw.h"
#include "sd_core.h"
#include "sd_card.h"
#include "debuglog.h"
#include "systicks.h"
#include "sys_event.h"


SDMMC_DMATransTypeDef dma;
SD_HandleTypeDef sdhandle;
SD_CardInfoTypedef cardinfo;

#define USE_MALLOC_DESC

#ifndef USE_MALLOC_DESC
  IDMAC_DescTypeDef desc[32] = {0};
#endif
/** @defgroup SD_Private_Functions SD Private Functions
  * @{
  */
static SD_ErrorTypedef IdentificateCard(SD_HandleTypeDef *hsd,SD_CardInfoTypedef *pCardInfo);
static SD_ErrorTypedef PowerOnCard(SD_HandleTypeDef *hsd);
static SD_ErrorTypedef InitializeCard(SD_HandleTypeDef *hsd);
static SD_ErrorTypedef PowerOffCard(SD_HandleTypeDef *hsd);
static SD_ErrorTypedef SD_CmdError(SD_HandleTypeDef *hsd);
static SD_ErrorTypedef SD_CmdResp1Error(SD_HandleTypeDef *hsd, uint32_t SD_CMD);
static SD_ErrorTypedef SD_CmdResp7(SD_HandleTypeDef *hsd);
static SD_ErrorTypedef SD_DMAConfig(SD_HandleTypeDef *hsd, SDMMC_DMATransTypeDef *dma);
static SD_ErrorTypedef SD_ENUM(SD_HandleTypeDef *hsd);
static SD_ErrorTypedef SD_IsCardProgramming(SD_HandleTypeDef *hsd, uint8_t *status);
static SD_STATUS sd_getState(SD_HandleTypeDef *hsd);

static void delay_ms(uint32_t num)
{
    volatile int i;
    for (i = 0; i < num * 100; i++);
}

SD_ErrorTypedef Card_SD_Init(SD_HandleTypeDef *hsd, SD_CardInfoTypedef *SDCardInfo)
{
  __IO SD_ErrorTypedef errorstate = SD_OK;
  uint32_t get_val;
  /* To identify if card is powered on */
  Core_SDMMC_SetPWREN(hsd->Instance, SDMMC_PWREN_0 | 
                                     SDMMC_PWREN_1 | 
                                     SDMMC_PWREN_2 | 
                                     SDMMC_PWREN_3);    
  delay_ms(100);
  
  get_val = Core_SDMMC_GetPWREN(hsd->Instance);
  if (!get_val)
  {
    Core_SDMMC_SetPWREN(hsd->Instance, SDMMC_PWREN_0 | 
                                       SDMMC_PWREN_1 | 
                                       SDMMC_PWREN_2 | 
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

SD_ErrorTypedef Card_SD_CMD6(SD_HandleTypeDef *hsd, SDMMC_DMATransTypeDef *dma)
{
  SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
  SD_ErrorTypedef errorstate = SD_OK;
  IDMAC_DescTypeDef desc = {0};
  /* Initialize handle flags */
  hsd->SdTransferCplt  = 0;
  hsd->SdTransferErr   = SD_OK;

  if (hsd->CardType == HIGH_CAPACITY_SD_CARD)
  {
    dma->BlockSize = 512;
  }

  /* Configure the SD DPSM (Data Path State Machine) */
  errorstate = SD_DMAConfig(hsd, dma);
  Core_SDMMC_SetDBADDR(hsd->Instance, DTCMBUSADDR((uint32_t)&desc));
  Core_SDMMC_SetBYCTNT(hsd->Instance, 0x40);
  if (errorstate != SD_OK) {
    return errorstate;
  }

  desc.des0 = SDMMC_DES0_OWN | SDMMC_DES0_FS |  SDMMC_DES0_CH | SDMMC_DES0_LD;
  desc.des1 = 64;
  desc.des2 = 0x44080000;
  desc.des3 = 0x0;

  uint32_t RINTSTS_val, STATUS_val, RESP0_val;
  dlog_info("Send CMD6");
  sdmmc_cmdinitstructure.Argument         = 0x80ffff02;
  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_HS_SWITCH;
  sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
                                            SDMMC_CMD_USE_HOLD_REG | 
                                            SDMMC_CMD_PRV_DAT_WAIT | 
                                            SDMMC_CMD_DAT_EXP      | 
                                            SDMMC_CMD_RESP_CRC     | 
                                            SDMMC_CMD_RESP_EXP;
  Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
  Core_SDMMC_WaiteCmdDone(hsd->Instance);
  // Core_SDMMC_WaiteDataOver(hsd->Instance);
  RESP0_val = Core_SDMMC_GetRESP0(hsd->Instance);
  dlog_info("CMD6 RESP0 = %x", RESP0_val);
  RINTSTS_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
  dlog_info("CMD6 RINTSTS = %x", RINTSTS_val);
  RESP0_val = Core_SDMMC_GetSTATUS(hsd->Instance);
  dlog_info("CMD6 STATUS = %x", RESP0_val);


  /* Update the SD transfer error in SD handle */
  hsd->SdTransferErr = errorstate;
  return errorstate;
}

SD_ErrorTypedef Card_SD_CMD19(SD_HandleTypeDef *hsd, SDMMC_DMATransTypeDef *dma)
{
  SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
  SD_ErrorTypedef errorstate = SD_OK;
  IDMAC_DescTypeDef desc = {0};
  /* Initialize handle flags */
  hsd->SdTransferCplt  = 0;
  hsd->SdTransferErr   = SD_OK;

  if (hsd->CardType == HIGH_CAPACITY_SD_CARD)
  {
    dma->BlockSize = 512;
  }

  /* Configure the SD DPSM (Data Path State Machine) */
  errorstate = SD_DMAConfig(hsd, dma);
  Core_SDMMC_SetDBADDR(hsd->Instance, DTCMBUSADDR((uint32_t)&desc));
  Core_SDMMC_SetBYCTNT(hsd->Instance, 0x40);
  if (errorstate != SD_OK) {
    return errorstate;
  }

  desc.des0 = SDMMC_DES0_OWN | SDMMC_DES0_FS |  SDMMC_DES0_CH | SDMMC_DES0_LD;
  desc.des1 = 64;
  desc.des2 = 0x44080000;
  desc.des3 = 0x0;

  uint32_t RINTSTS_val, STATUS_val, RESP0_val;
  dlog_info("Send CMD19");
  sdmmc_cmdinitstructure.Argument         = 0x0;
  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SEND_TUNING_PATTERN;
  sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
  sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
                                            SDMMC_CMD_USE_HOLD_REG | 
                                            SDMMC_CMD_PRV_DAT_WAIT | 
                                            SDMMC_CMD_DAT_EXP      | 
                                            SDMMC_CMD_RESP_CRC     | 
                                            SDMMC_CMD_RESP_EXP;
  Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
  Core_SDMMC_WaiteCmdDone(hsd->Instance);
  // Core_SDMMC_WaiteDataOver(hsd->Instance);
  RESP0_val = Core_SDMMC_GetRESP0(hsd->Instance);
  dlog_info("RESP0 = %x", RESP0_val);
  RESP0_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
  dlog_info("RINTSTS = %x", RESP0_val);
  RESP0_val = Core_SDMMC_GetSTATUS(hsd->Instance);
  dlog_info("STATUS = %x", RESP0_val);

    // sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_STOP_TRANSMISSION;
  // sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
  // sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
  //                                           SDMMC_CMD_USE_HOLD_REG | \
  //                                           SDMMC_CMD_PRV_DAT_WAIT | \
  //                                           SDMMC_CMD_RESP_CRC | \
  //                                           SDMMC_CMD_RESP_EXP;
  // Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
  // Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
  // Core_SDMMC_WaiteCmdDone(hsd->Instance);
  // RESP0_val = Core_SDMMC_GetRESP0(hsd->Instance);
  // dlog_info("RESP0 = %x", RESP0_val);
  // RESP0_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
  // dlog_info("RINTSTS = %x", RESP0_val);
  // RESP0_val = Core_SDMMC_GetSTATUS(hsd->Instance);
  // dlog_info("STATUS = %x", RESP0_val);

  /* Update the SD transfer error in SD handle */
  hsd->SdTransferErr = errorstate;
  return errorstate;
}



SD_ErrorTypedef Card_SD_ReadBlock_DMA(SD_HandleTypeDef *hsd, SDMMC_DMATransTypeDef *dma)
{
  SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
  SD_ErrorTypedef errorstate = SD_OK;
  IDMAC_DescTypeDef desc = {0};
  /* Initialize handle flags */
  hsd->SdTransferCplt  = 0;
  hsd->SdTransferErr   = SD_OK;

  if (hsd->CardType == HIGH_CAPACITY_SD_CARD)
  {
    dma->BlockSize = 512;
  }

  /* Configure the SD DPSM (Data Path State Machine) */
  errorstate = SD_DMAConfig(hsd, dma);
  Core_SDMMC_SetDBADDR(hsd->Instance, DTCMBUSADDR((uint32_t)&desc));
  Core_SDMMC_SetBYCTNT(hsd->Instance, dma->BlockSize);
  if (errorstate != SD_OK) {
    return errorstate;
  }

  desc.des0 = SDMMC_DES0_OWN | SDMMC_DES0_FS |  SDMMC_DES0_CH | SDMMC_DES0_LD;
  desc.des1 = dma->BlockSize;
  desc.des2 = dma->DstAddr;
  desc.des3 = 0x0;

  /* send CMD17*/
  sdmmc_cmdinitstructure.Argument         = dma->SrcAddr;
  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_READ_SINGLE_BLOCK;
  sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
  sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
      SDMMC_CMD_USE_HOLD_REG | 
      SDMMC_CMD_PRV_DAT_WAIT | 
      SDMMC_CMD_DAT_EXP      | 
      SDMMC_CMD_RESP_CRC     | 
      SDMMC_CMD_RESP_EXP;
  Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);

  /* Check for error conditions */
  Core_SDMMC_WaiteCmdDone(hsd->Instance);
  Core_SDMMC_WaiteDataOver(hsd->Instance);
  Core_SDMMC_WaiteCardBusy(hsd->Instance);

  /* Update the SD transfer error in SD handle */
  hsd->SdTransferErr = errorstate;
  return errorstate;
}



SD_ErrorTypedef Card_SD_ReadMultiBlocks_DMA(SD_HandleTypeDef *hsd, SDMMC_DMATransTypeDef *dma)
{
  SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
  SD_ErrorTypedef errorstate = SD_OK;
  uint32_t BlockIndex, TmpAddr = dma->DstAddr, DstAddr = dma->DstAddr;
  uint32_t BuffSize = BUFFSIZE8;
  uint32_t SectorDivid = dma->SectorNum / BuffSize;
  uint32_t SectorRmd = dma->SectorNum % BuffSize;

#ifdef USE_MALLOC_DESC
  /* malloc the space for descriptor */
  IDMAC_DescTypeDef *desc = (IDMAC_DescTypeDef *)malloc(sizeof(IDMAC_DescTypeDef) * (SectorDivid + SectorRmd));
  if (!desc){
    dlog_info("Malloc Failed! Exit Read\n");
    errorstate = SD_ERROR;
    return errorstate;
  }
#endif

  /* Initialize handle flags */
  hsd->SdTransferCplt  = 0;
  hsd->SdTransferErr   = SD_OK;

  if (hsd->CardType == HIGH_CAPACITY_SD_CARD)
  {
    dma->BlockSize = 512;
  }

  /* Configure the SD DPSM (Data Path State Machine) */
  errorstate = SD_DMAConfig(hsd, dma);
  Core_SDMMC_SetDBADDR(hsd->Instance, DTCMBUSADDR((uint32_t)&desc[0]));
  if (errorstate != SD_OK) {
    free(desc);
    return errorstate;
  }

  if (SectorDivid)
  {
    Core_SDMMC_SetBYCTNT(hsd->Instance, SectorDivid * BuffSize * dma->BlockSize);
    for (BlockIndex = 0; BlockIndex < SectorDivid; BlockIndex++)
    {

      DstAddr = dma->DstAddr + dma->BlockSize * BuffSize * BlockIndex;

      if (BlockIndex == 0 && (SectorDivid != 1))
      {

        desc[BlockIndex].des0 = SDMMC_DES0_OWN | SDMMC_DES0_FS |  SDMMC_DES0_CH;
        desc[BlockIndex].des1 = dma->BlockSize * BuffSize;
        desc[BlockIndex].des2 = DstAddr;
        desc[BlockIndex].des3 = DTCMBUSADDR((uint32_t)&desc[BlockIndex+1]);;

      }
      else if ((BlockIndex == 0) && (SectorDivid == 1))
      {

        desc[BlockIndex].des0 = SDMMC_DES0_OWN | SDMMC_DES0_CH | SDMMC_DES0_LD | SDMMC_DES0_FS;
        desc[BlockIndex].des1 = dma->BlockSize * BuffSize;
        desc[BlockIndex].des2 = DstAddr;
        desc[BlockIndex].des3 = 0x0;
        TmpAddr = DstAddr + dma->BlockSize * BuffSize;
      }
      else if (BlockIndex == SectorDivid - 1)
      {

        desc[BlockIndex].des0 = SDMMC_DES0_OWN | SDMMC_DES0_CH | SDMMC_DES0_LD;
        desc[BlockIndex].des1 = dma->BlockSize * BuffSize;
        desc[BlockIndex].des2 = DstAddr;
        desc[BlockIndex].des3 = 0x0;
        TmpAddr = DstAddr + dma->BlockSize * BuffSize;
      }
      else
      {
        desc[BlockIndex].des0 = SDMMC_DES0_OWN | SDMMC_DES0_CH;
        desc[BlockIndex].des1 = dma->BlockSize * BuffSize;
        desc[BlockIndex].des2 = DstAddr;
        desc[BlockIndex].des3 = DTCMBUSADDR((uint32_t)&desc[BlockIndex+1]);;
      }
    }
    /* send CMD18 */
    sdmmc_cmdinitstructure.Argument         = dma->SrcAddr;
    sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_READ_MULTIPLE_BLOCK;
    sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
    sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
        SDMMC_CMD_USE_HOLD_REG | 
        SDMMC_CMD_PRV_DAT_WAIT | 
        SDMMC_CMD_SEND_STOP    | 
        SDMMC_CMD_DAT_EXP      | 
        SDMMC_CMD_RESP_CRC     | 
        SDMMC_CMD_RESP_EXP;
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
    /* Check for error conditions */
    Core_SDMMC_WaiteCmdDone(hsd->Instance);
    //Core_SDMMC_WaiteDataOver(hsd->Instance);
    Core_SDMMC_WaiteCardBusy(hsd->Instance);
  }
  // dlog_info("Divid Addr = %x", dma->SrcAddr);
  // dlog_info("Divid Num = %d", SectorDivid);
  if (SectorRmd)
  {
    Core_SDMMC_SetBYCTNT(hsd->Instance, SectorRmd * dma->BlockSize);
    // dlog_info("------------read rmd-----------------\n");
    for (BlockIndex = 0; BlockIndex < SectorRmd; BlockIndex++)
    {

      DstAddr = TmpAddr + dma->BlockSize * BlockIndex;

      if (BlockIndex == 0)
      {

        desc[BlockIndex].des0 = SDMMC_DES0_OWN | SDMMC_DES0_FS |  SDMMC_DES0_CH;
        desc[BlockIndex].des1 = dma->BlockSize;
        desc[BlockIndex].des2 = DstAddr;
        desc[BlockIndex].des3 = DTCMBUSADDR((uint32_t)&desc[BlockIndex+1]);;

      }
      else if (BlockIndex == SectorRmd - 1)
      {

        desc[BlockIndex].des0 = SDMMC_DES0_OWN | SDMMC_DES0_CH | SDMMC_DES0_LD;
        desc[BlockIndex].des1 = dma->BlockSize;
        desc[BlockIndex].des2 = DstAddr;
        desc[BlockIndex].des3 = 0x0;
      }
      else
      {
        desc[BlockIndex].des0 = SDMMC_DES0_OWN | SDMMC_DES0_CH;
        desc[BlockIndex].des1 = dma->BlockSize;
        desc[BlockIndex].des2 = DstAddr;
        desc[BlockIndex].des3 = DTCMBUSADDR((uint32_t)&desc[BlockIndex+1]);;
      }
    }
    // dlog_info("Rmd Addr = %x", dma->SrcAddr + SectorDivid * BuffSize * dma->BlockSize);
    /* send CMD18 */
    sdmmc_cmdinitstructure.Argument         = (dma->SrcAddr + SectorDivid * BuffSize);
    sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_READ_MULTIPLE_BLOCK;
    sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
    sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
        SDMMC_CMD_USE_HOLD_REG | 
        SDMMC_CMD_PRV_DAT_WAIT | 
        SDMMC_CMD_SEND_STOP    | 
        SDMMC_CMD_DAT_EXP      | 
        SDMMC_CMD_RESP_CRC     | 
        SDMMC_CMD_RESP_EXP;
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
    /* Check for error conditions */
    Core_SDMMC_WaiteCmdDone(hsd->Instance);
    //Core_SDMMC_WaiteDataOver(hsd->Instance);
    Core_SDMMC_WaiteCardBusy(hsd->Instance);
  }

  /* free the space applied from heap */
#ifdef USE_MALLOC_DESC
  free(desc);
#endif
  /* Update the SD transfer error in SD handle */
  hsd->SdTransferErr = errorstate;
  return errorstate;
}


SD_ErrorTypedef Card_SD_WriteBlock_DMA(SD_HandleTypeDef *hsd, SDMMC_DMATransTypeDef *dma)
{
  SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
  SD_ErrorTypedef errorstate = SD_OK;
  IDMAC_DescTypeDef desc = {0};
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
  Core_SDMMC_SetDBADDR(hsd->Instance, DTCMBUSADDR((uint32_t)&desc));
  Core_SDMMC_SetBYCTNT(hsd->Instance, dma->BlockSize);
  if (errorstate != SD_OK) {
    dlog_info("SD_DMAConfig Fail\n");
    return errorstate;
  }

  desc.des0 = SDMMC_DES0_OWN | SDMMC_DES0_FS |  SDMMC_DES0_CH | SDMMC_DES0_LD;
  desc.des1 = 0x200;
  desc.des2 = dma->SrcAddr;
  desc.des3 = 0x0;

  /* send CMD24 */
  sdmmc_cmdinitstructure.Argument         = dma->DstAddr;
  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_WRITE_SINGLE_BLOCK;
  sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
  sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
                                            SDMMC_CMD_USE_HOLD_REG | 
                                            SDMMC_CMD_PRV_DAT_WAIT | 
											SDMMC_CMD_TRANSFER_MODE |
                                            SDMMC_CMD_DAT_READ_WRITE | 
                                            SDMMC_CMD_DAT_EXP      | 
                                            SDMMC_CMD_RESP_CRC     | 
                                            SDMMC_CMD_RESP_EXP;
  Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
   /* Check for error conditions */
  Core_SDMMC_WaiteCmdDone(hsd->Instance);
  Core_SDMMC_WaiteDataOver(hsd->Instance);
  Core_SDMMC_WaiteCardBusy(hsd->Instance); 
  
  /* Update the SD transfer error in SD handle */
  hsd->SdTransferErr = errorstate;
  return errorstate;
}


SD_ErrorTypedef Card_SD_WriteMultiBlocks_DMA(SD_HandleTypeDef *hsd, SDMMC_DMATransTypeDef *dma)
{
    SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
    SD_ErrorTypedef errorstate = SD_OK;
    uint32_t BlockIndex, TmpAddr = dma->SrcAddr, SrcAddr = dma->SrcAddr;
    uint32_t BuffSize = BUFFSIZE8;
    uint32_t SectorDivid = dma->SectorNum / BuffSize;
    uint32_t SectorRmd = dma->SectorNum % BuffSize;

#if 1
    SD_STATUS cardstate;
    cardstate = sd_getState(hsd);
    dlog_info("%d, cardstate = 0x%02x", __LINE__, cardstate);

    if (cardstate != SD_CARD_TRANSFER)
    {
        // not in transfer state
        /* Send CMD7 SDMMC_SEL_DESEL_CARD */
        dlog_info("%d, send CMD7", __LINE__);
        sdmmc_cmdinitstructure.Argument         = (uint32_t)(hsd->RCA << 16);
        sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SEL_DESEL_CARD;
        sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
        sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
                                                  SDMMC_CMD_USE_HOLD_REG | 
                                                  SDMMC_CMD_PRV_DAT_WAIT | 
                                                  SDMMC_CMD_RESP_CRC    | 
                                                  SDMMC_CMD_RESP_EXP;
        Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
        Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
        /* waite for command finish*/
        Core_SDMMC_WaiteCmdDone(hsd->Instance);
    }

    cardstate = sd_getState(hsd);
    dlog_info("%d, cardstate = 0x%02x", __LINE__, cardstate);
#endif

#ifdef USE_MALLOC_DESC
  // dlog_info("Ues malloc\n");
  /* malloc the space for descriptor */
  IDMAC_DescTypeDef *desc = (IDMAC_DescTypeDef *)malloc(sizeof(IDMAC_DescTypeDef) * (SectorDivid + SectorRmd));
  if (!desc){
    dlog_info("malloc failed! Exit writing\n");
    errorstate = SD_ERROR;
    return errorstate;
  }
#endif

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
  Core_SDMMC_SetDBADDR(hsd->Instance, DTCMBUSADDR((uint32_t)&desc[0]));
  if (errorstate != SD_OK) {
    dlog_info("SD_DMAConfig Fail\n");
    free(desc);
    return errorstate;
  }

  if (SectorDivid)
  { 
    Core_SDMMC_SetBYCTNT(hsd->Instance, SectorDivid * BuffSize * dma->BlockSize);
    for (BlockIndex = 0; BlockIndex < SectorDivid; BlockIndex++)
    {
      SrcAddr = dma->SrcAddr + dma->BlockSize * BuffSize * BlockIndex;
      if ((BlockIndex == 0) && (SectorDivid == 1))
      {
        desc[BlockIndex].des0 = SDMMC_DES0_OWN | SDMMC_DES0_FS | SDMMC_DES0_LD | SDMMC_DES0_CH;
        desc[BlockIndex].des1 = dma->BlockSize * BuffSize;
        desc[BlockIndex].des2 = SrcAddr;
        desc[BlockIndex].des3 = 0x0;
        TmpAddr = SrcAddr + dma->BlockSize * BuffSize;
      }
      else if ((BlockIndex == 0) && (SectorDivid != 1))
      {
        desc[BlockIndex].des0 = SDMMC_DES0_OWN | SDMMC_DES0_FS | SDMMC_DES0_CH;
        desc[BlockIndex].des1 = dma->BlockSize * BuffSize;
        desc[BlockIndex].des2 = SrcAddr;
        desc[BlockIndex].des3 = DTCMBUSADDR((uint32_t)&desc[BlockIndex+1]);

      }
      else if (BlockIndex == SectorDivid - 1)
      {
        desc[BlockIndex].des0 = SDMMC_DES0_OWN | SDMMC_DES0_LD | SDMMC_DES0_CH;
        desc[BlockIndex].des1 = dma->BlockSize * BuffSize;
        desc[BlockIndex].des2 = SrcAddr;
        desc[BlockIndex].des3 = 0x0;
        TmpAddr = SrcAddr + dma->BlockSize * BuffSize;
      }
      else
      {
        desc[BlockIndex].des0 = SDMMC_DES0_OWN | SDMMC_DES0_CH;
        desc[BlockIndex].des1 = dma->BlockSize * BuffSize;
        desc[BlockIndex].des2 = SrcAddr;
        desc[BlockIndex].des3 = DTCMBUSADDR((uint32_t)&desc[BlockIndex+1]);
      }
    }
      /* Send CMD55 */
    //sdmmc_cmdinitstructure.Argument         = 0;
    //sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_APP_CMD;
    //sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
    //    SDMMC_CMD_USE_HOLD_REG | \
    //    SDMMC_CMD_PRV_DAT_WAIT | \
    //    SDMMC_CMD_RESP_CRC | \
    //    SDMMC_CMD_RESP_EXP;
    //Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    //Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
    /* Send ACMD23 */
    //sdmmc_cmdinitstructure.Argument         = BuffSize * SectorDivid;
    //sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_APP_SET_WR_BLK_ERASE_COUNT;
    //sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
    //sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
    //    SDMMC_CMD_USE_HOLD_REG | \
    //    SDMMC_CMD_PRV_DAT_WAIT | \
    //    SDMMC_CMD_RESP_EXP;
    //Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    //Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
    
    // dlog_info("\n");
    //delay_ms(1);
    /* send CMD25 */
    sdmmc_cmdinitstructure.Argument         = dma->DstAddr;
    sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_WRITE_MULTIPLE_BLOCK;
    sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
    sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
        SDMMC_CMD_USE_HOLD_REG | 
        SDMMC_CMD_PRV_DAT_WAIT | 
        SDMMC_CMD_SEND_STOP    | 
		SDMMC_CMD_TRANSFER_MODE |
        SDMMC_CMD_DAT_READ_WRITE | 
        SDMMC_CMD_DAT_EXP      | 
        SDMMC_CMD_RESP_CRC     | 
        SDMMC_CMD_RESP_EXP;
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);


    /* Check for error conditions */
    Core_SDMMC_WaiteCmdDone(hsd->Instance);
    Core_SDMMC_WaiteDataOver(hsd->Instance);
    Core_SDMMC_WaiteCardBusy(hsd->Instance);

    // #ifdef ECHO
    //     dlog_info("Stop: send CMD12\n");
    // #endif
    sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_STOP_TRANSMISSION;
    sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
    sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
        SDMMC_CMD_USE_HOLD_REG | 
        SDMMC_CMD_PRV_DAT_WAIT | 
        SDMMC_CMD_RESP_CRC | 
        SDMMC_CMD_RESP_EXP;
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
    Core_SDMMC_WaiteCmdDone(hsd->Instance);
  }


  if (SectorRmd)
  {
    Core_SDMMC_SetBYCTNT(hsd->Instance, SectorRmd * dma->BlockSize);
    // dlog_info("------------write rmd-----------------\n");
    for (BlockIndex = 0; BlockIndex < SectorRmd; ++BlockIndex)
    {
      SrcAddr = TmpAddr + dma->BlockSize * BlockIndex;
      // dlog_info("BlockIndex = %d\n",BlockIndex);
      // dlog_info("SrcAddr = %x\n",SrcAddr);
      if ((BlockIndex ==  SectorRmd - 1) && (SectorRmd == 1))
      {
        desc[BlockIndex].des0 = SDMMC_DES0_OWN | SDMMC_DES0_LD | SDMMC_DES0_FS | SDMMC_DES0_CH;
        desc[BlockIndex].des1 = dma->BlockSize;
        desc[BlockIndex].des2 = SrcAddr;
        desc[BlockIndex].des3 = 0x0;
      }
      else if (BlockIndex ==  SectorRmd - 1)
      {
        desc[BlockIndex].des0 = SDMMC_DES0_OWN | SDMMC_DES0_LD | SDMMC_DES0_CH;
        desc[BlockIndex].des1 = dma->BlockSize;
        desc[BlockIndex].des2 = SrcAddr;
        desc[BlockIndex].des3 = 0x0;
      }
      else
      {
        desc[BlockIndex].des0 = SDMMC_DES0_OWN | SDMMC_DES0_FS |  SDMMC_DES0_CH;
        desc[BlockIndex].des1 = dma->BlockSize;
        desc[BlockIndex].des2 = SrcAddr;
        desc[BlockIndex].des3 = DTCMBUSADDR((uint32_t)&desc[BlockIndex+1]);
      }
    }

    /* send CMD25 */
    sdmmc_cmdinitstructure.Argument         = dma->DstAddr + SectorDivid * BuffSize;
    sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_WRITE_MULTIPLE_BLOCK;
    sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
    sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
        SDMMC_CMD_USE_HOLD_REG | 
        SDMMC_CMD_PRV_DAT_WAIT | 
        SDMMC_CMD_SEND_STOP    | 
        SDMMC_CMD_DAT_READ_WRITE | 
        SDMMC_CMD_DAT_EXP      | 
        SDMMC_CMD_RESP_CRC     | 
        SDMMC_CMD_RESP_EXP;
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);


    /* Check for error conditions */
    Core_SDMMC_WaiteCmdDone(hsd->Instance);
    Core_SDMMC_WaiteDataOver(hsd->Instance);
    Core_SDMMC_WaiteCardBusy(hsd->Instance);

    // #ifdef ECHO
    //     dlog_info("Stop: send CMD12\n");
    // #endif
    sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_STOP_TRANSMISSION;
    sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
    sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
        SDMMC_CMD_USE_HOLD_REG | 
        SDMMC_CMD_PRV_DAT_WAIT | 
        SDMMC_CMD_RESP_CRC | 
        SDMMC_CMD_RESP_EXP;
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
    Core_SDMMC_WaiteCmdDone(hsd->Instance);
  }
  /* free the space applied from heap */
#ifdef USE_MALLOC_DESC
  free(desc);
#endif
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
#if 0
        cardstate = sd_getState(hsd);
        dlog_info("%d, cardstate = 0x%02x", __LINE__, cardstate);
    
        if (cardstate != SD_CARD_TRANSFER)
        {
            // not in transfer state
            /* Send CMD7 SDMMC_SEL_DESEL_CARD */
            dlog_info("%d, send CMD7", __LINE__);
            sdmmc_cmdinitstructure.Argument         = (uint32_t)(hsd->RCA << 16);
            sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SEL_DESEL_CARD;
            sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
            sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
                                                      SDMMC_CMD_USE_HOLD_REG | 
                                                      SDMMC_CMD_PRV_DAT_WAIT | 
                                                      SDMMC_CMD_RESP_CRC    | 
                                                      SDMMC_CMD_RESP_EXP;
            Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
            Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
            /* waite for command finish*/
            Core_SDMMC_WaiteCmdDone(hsd->Instance);
        }
    
        cardstate = sd_getState(hsd);
        dlog_info("%d, cardstate = 0x%02x", __LINE__, cardstate);
#endif

  /* According to sd-card spec 3.0 ERASE_GROUP_START (CMD32) and erase_group_end(CMD33) */
    if ((hsd->CardType == STD_CAPACITY_SD_CARD)||(hsd->CardType == HIGH_CAPACITY_SD_CARD))
    {
        /* Send CMD32 SD_ERASE_GRP_START with argument as addr  */
        sdmmc_cmdinitstructure.Argument = (uint32_t)startaddr;
        sdmmc_cmdinitstructure.CmdIndex = SD_CMD_SD_ERASE_WR_BLK_START;
        sdmmc_cmdinitstructure.Response = SDMMC_RESPONSE_R1;
        sdmmc_cmdinitstructure.Attribute = SDMMC_CMD_START_CMD | 
                                            SDMMC_CMD_USE_HOLD_REG | 
                                            SDMMC_CMD_PRV_DAT_WAIT | 
                                            SDMMC_CMD_RESP_CRC     | 
                                            SDMMC_CMD_RESP_EXP;
        Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
        Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
        /* Check for error conditions */
        Core_SDMMC_WaiteCmdDone(hsd->Instance);
        Core_SDMMC_WaiteCardBusy(hsd->Instance);


        /* Send CMD33 SD_ERASE_GRP_END with argument as addr  */
        sdmmc_cmdinitstructure.Argument = (uint32_t)(startaddr + blocknum - 1);
        sdmmc_cmdinitstructure.CmdIndex = SD_CMD_SD_ERASE_WR_BLK_END;
        sdmmc_cmdinitstructure.Response = SDMMC_RESPONSE_R1;
        sdmmc_cmdinitstructure.Attribute = SDMMC_CMD_START_CMD | 
                                            SDMMC_CMD_USE_HOLD_REG | 
                                            SDMMC_CMD_PRV_DAT_WAIT | 
                                            SDMMC_CMD_RESP_CRC     | 
                                            SDMMC_CMD_RESP_EXP;
        Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
        Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
        /* Check for error conditions */
        Core_SDMMC_WaiteCmdDone(hsd->Instance);
        Core_SDMMC_WaiteCardBusy(hsd->Instance);
    }

    /* Send CMD38 ERASE */
    sdmmc_cmdinitstructure.Argument = 0;
    sdmmc_cmdinitstructure.CmdIndex = SD_CMD_ERASE;
    sdmmc_cmdinitstructure.Response = SDMMC_RESPONSE_R1;
    sdmmc_cmdinitstructure.Attribute = SDMMC_CMD_START_CMD | 
                                        SDMMC_CMD_USE_HOLD_REG | 
                                        SDMMC_CMD_PRV_DAT_WAIT | 
                                        SDMMC_CMD_RESP_CRC     | 
                                        SDMMC_CMD_RESP_EXP;
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);


	/* Wait until the card is in programming state */

    errorstate = SD_IsCardProgramming(hsd, &cardstate);
    uint8_t tmp_state = cardstate;
    while ( cardstate == SD_CARD_TRANSFER ||  cardstate == SD_CARD_PROGRAMMING)
    {
        errorstate = SD_IsCardProgramming(hsd, &cardstate);
        if (tmp_state != cardstate)
            break;
    }
        
    while ( cardstate == SD_CARD_PROGRAMMING )
    {
    	errorstate = SD_IsCardProgramming(hsd, &cardstate);

    	if ( cardstate == SD_CARD_TRANSFER)
    	{
    		break;
    	}
    }

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
    // dlog_info("C_SIZE = %d\n", pCardInfo->SD_csd.C_SIZE);  
    // dlog_output(200);

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
    pCardInfo->CardBlockSize = 512;

    // dlog_info("pCardInfo->CardCapacity = %llu\n", pCardInfo->CardCapacity);  
    // dlog_info("C_SIZE = %d\n", pCardInfo->SD_csd.C_SIZE);  
    // dlog_output(200);
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
static SD_ErrorTypedef IdentificateCard(SD_HandleTypeDef *hsd,SD_CardInfoTypedef *SDCardInfo)
{
  SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
  SD_ErrorTypedef errorstate = SD_OK;
  uint16_t sd_rca = 1;
  uint32_t cmd_done, response, get_val;
  if (hsd->CardType != SECURE_DIGITAL_IO_CARD)
  {
#ifdef ECHO
    dlog_info("Send CMD2");
#endif
    /* Send CMD2 ALL_SEND_CID */
    sdmmc_cmdinitstructure.Argument = 0;
    sdmmc_cmdinitstructure.CmdIndex = SD_CMD_ALL_SEND_CID;
    sdmmc_cmdinitstructure.Response = SDMMC_RESPONSE_R2;
    sdmmc_cmdinitstructure.Attribute  = SDMMC_CMD_START_CMD | 
                                        SDMMC_CMD_USE_HOLD_REG | 
                                        SDMMC_CMD_PRV_DAT_WAIT | 
                                        SDMMC_CMD_RESP_CRC     | 
                                        SDMMC_CMD_RESP_LONG    | 
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
  }

  if ((hsd->CardType == STD_CAPACITY_SD_CARD)    ||  
      (hsd->CardType == SECURE_DIGITAL_IO_COMBO_CARD) || 
      (hsd->CardType == HIGH_CAPACITY_SD_CARD))
  {
    /* Send CMD3 SET_REL_ADDR with argument 0 */
    /* SD Card publishes its RCA. */
    dlog_info("Send CMD3");
    sdmmc_cmdinitstructure.Argument         = 0;
    sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SEND_RELATIVE_ADDR;
    sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R6;
    sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
        SDMMC_CMD_USE_HOLD_REG | 
        SDMMC_CMD_PRV_DAT_WAIT | 
        SDMMC_CMD_RESP_CRC | 
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

    dlog_info("Card Relative Address = 0x%x", sd_rca);
    hsd->RCA = sd_rca;
  }

  /* switch clock */
  // Core_SDMMC_SetCLKSRC(hsd->Instance, 0x00000000);
  // Core_SDMMC_SetCLKENA(hsd->Instance, 0x00000000);
  // sdmmc_cmdinitstructure.Argument  = 0x00000000;
  // sdmmc_cmdinitstructure.CmdIndex  = SD_CMD_GO_IDLE_STATE;
  // sdmmc_cmdinitstructure.Response  = SDMMC_RESPONSE_NO;
  // sdmmc_cmdinitstructure.Attribute = SDMMC_CMD_START_CMD | \
  //                                    SDMMC_CMD_USE_HOLD_REG | \
  //                                    SDMMC_CMD_PRV_DAT_WAIT | \
  //                                    SDMMC_CMD_UPDATE_CLK;
  // Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
  // Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
  // Core_SDMMC_WaiteCmdStart(hsd->Instance);

  // //delay_ms(100);
  // delay_ms(10);
  // Core_SDMMC_SetCLKDIV(hsd->Instance, 0x00000002);  //switch frequency from 400Khz to 100Mhz

  // Core_SDMMC_SetCLKSRC(hsd->Instance, 0x00000000);
  // Core_SDMMC_SetCLKENA(hsd->Instance, 0x00000001);
  // sdmmc_cmdinitstructure.Argument  = 0x00000000;
  // sdmmc_cmdinitstructure.CmdIndex  = SD_CMD_GO_IDLE_STATE;
  // sdmmc_cmdinitstructure.Response  = SDMMC_RESPONSE_NO;
  // sdmmc_cmdinitstructure.Attribute = SDMMC_CMD_START_CMD | \
  //                                    SDMMC_CMD_USE_HOLD_REG | \
  //                                    SDMMC_CMD_PRV_DAT_WAIT | \
  //                                    SDMMC_CMD_UPDATE_CLK;
  // Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
  // Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
  // Core_SDMMC_WaiteCmdStart(hsd->Instance);

  if (hsd->CardType != SECURE_DIGITAL_IO_CARD)
  {
    /* Get the SD card RCA */
    hsd->RCA = sd_rca;
#ifdef ECHO
    dlog_info("Send CMD10");
#endif
    sdmmc_cmdinitstructure.Argument         = (uint32_t)(hsd->RCA << 16);
    sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SEND_CID;
    sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R2;
    sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
        SDMMC_CMD_USE_HOLD_REG | 
        SDMMC_CMD_PRV_DAT_WAIT | 
        SDMMC_CMD_RESP_CRC     | 
        SDMMC_CMD_RESP_LONG    | 
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
    dlog_info("Send CMD9");
#endif
    sdmmc_cmdinitstructure.Argument         = (uint32_t)(hsd->RCA << 16);
    sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SEND_CSD;
    sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R2;
    sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
        SDMMC_CMD_USE_HOLD_REG | 
        SDMMC_CMD_PRV_DAT_WAIT | 
        SDMMC_CMD_RESP_CRC     | 
        SDMMC_CMD_RESP_LONG    | 
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
    sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
        SDMMC_CMD_USE_HOLD_REG | 
        SDMMC_CMD_PRV_DAT_WAIT | 
        SDMMC_CMD_RESP_CRC    | 
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
    dlog_info("Send CMD55");
  #endif
    sdmmc_cmdinitstructure.Argument         = (uint32_t)(hsd->RCA << 16);
    sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_APP_CMD;
    sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
    sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
        SDMMC_CMD_USE_HOLD_REG | 
        SDMMC_CMD_PRV_DAT_WAIT | 
        SDMMC_CMD_RESP_CRC | 
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
    dlog_info("Send ACMD6");
  #endif
    /* define the data bus width */
    sdmmc_cmdinitstructure.Argument         = SDMMC_BUS_WIDE_4B;
    sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_APP_SD_SET_BUSWIDTH;
    sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
    sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
        SDMMC_CMD_USE_HOLD_REG | 
        SDMMC_CMD_PRV_DAT_WAIT | 
        SDMMC_CMD_RESP_CRC | 
        SDMMC_CMD_RESP_EXP;
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
    /* Check for error conditions */
    Core_SDMMC_WaiteCmdDone(hsd->Instance);
    get_val = Core_SDMMC_GetRESP0(hsd->Instance);
    // dlog_info("ACMD6 RESP0 = %x\n", get_val);
    get_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
    // dlog_info("ACMD6 RINTSTS = %x\n", get_val);

#if 0
    /* Send CMD16*/
  #ifdef ECHO
    dlog_info("Send CMD16");
  #endif
    /* set the block length*/
    Core_SDMMC_SetCTYPE(hsd->Instance, SDMMC_CTYPE_4BIT);
    Core_SDMMC_SetCTRL(hsd->Instance, SDMMC_CTRL_USE_INTERNAL_IDMAC | 
                       SDMMC_CTRL_INT_ENABLE | 
                       SDMMC_CTRL_FIFO_RESET);
    Core_SDMMC_SetBLKSIZ(hsd->Instance, DATA_BLOCK_LEN);
    Core_SDMMC_SetBYCTNT(hsd->Instance, DATA_BYTE_CNT);

    sdmmc_cmdinitstructure.Argument         = DATA_BLOCK_LEN;
    sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SET_BLOCKLEN;
    sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
    sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
        SDMMC_CMD_USE_HOLD_REG | 
        SDMMC_CMD_PRV_DAT_WAIT | 
        SDMMC_CMD_RESP_CRC | 
        SDMMC_CMD_RESP_EXP;
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
    /* Check for error conditions */
    Core_SDMMC_WaiteCmdDone(hsd->Instance);
#endif

    // get_val = Core_SDMMC_GetRESP0(hsd->Instance);
    // dlog_info("CMD16 RESP0 = %x\n", get_val);
    // get_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
    // dlog_info("CMD16 RINTSTS = %x\n", get_val);
    /* All cards are initialized */

    static uint8_t flag_init_once = 0;
    /* Send CMD6*/
    if (flag_init_once == 0)
    {
#ifdef ECHO
        dlog_info("Send CMD6");
#endif
        IDMAC_DescTypeDef desc = {0};
        uint8_t CMD6Data[64] = {0};
        uint32_t CMD6Dst = DTCMBUSADDR((uint32_t)&CMD6Data);
        /* Initialize handle flags */
        hsd->SdTransferCplt  = 0;
        hsd->SdTransferErr   = SD_OK;
        SDMMC_DMATransTypeDef dmaTmp;
        if (hsd->CardType == HIGH_CAPACITY_SD_CARD)
        {
            dmaTmp.BlockSize = 512;
        }
        /* Configure the SD DPSM (Data Path State Machine) */
        errorstate = SD_DMAConfig(hsd, &dmaTmp);
        Core_SDMMC_SetDBADDR(hsd->Instance, DTCMBUSADDR((uint32_t)&desc));
        Core_SDMMC_SetBYCTNT(hsd->Instance, 0x40);
        if (errorstate != SD_OK) 
        {
            return errorstate;
        }
        desc.des0 = SDMMC_DES0_OWN | SDMMC_DES0_FS |  SDMMC_DES0_CH | SDMMC_DES0_LD;
        desc.des1 = 64;
        desc.des2 = CMD6Dst;
        desc.des3 = 0x0;
        sdmmc_cmdinitstructure.Argument         = (0x80ffff00 | hsd->SpeedMode);
        // dlog_info("argumen = %x", sdmmc_cmdinitstructure.Argument);
        sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
        sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_HS_SWITCH;
        sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
                                                  SDMMC_CMD_USE_HOLD_REG | 
                                                  SDMMC_CMD_PRV_DAT_WAIT | 
                                                  SDMMC_CMD_DAT_EXP      | 
                                                  SDMMC_CMD_RESP_CRC     | 
                                                  SDMMC_CMD_RESP_EXP;
        Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
        Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
        Core_SDMMC_WaiteCmdDone(hsd->Instance);
        flag_init_once = 1;
   }

#if 0
    /*print 512 bits CMD6 info*/
    unsigned int readAddress = CMD6Dst;
    unsigned char row;
    readAddress -= (readAddress % 4);
    /* print to serial */
    for (row = 0; row < 2; row++)
    {
        /* new line */
        dlog_info("0x%08x: 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n ", 
                  readAddress,
                  *(uint32_t *)readAddress,
                  *(uint32_t *)(readAddress + 4),
                  *(uint32_t *)(readAddress + 8),
                  *(uint32_t *)(readAddress + 12),
                  *(uint32_t *)(readAddress + 16),
                  *(uint32_t *)(readAddress + 20),
                  *(uint32_t *)(readAddress + 24),
                  *(uint32_t *)(readAddress + 28));

        readAddress += 32;
    }
#endif

    /*switch frequency*/
    sdmmc_cmdinitstructure.Argument  = 0x00000000;
    sdmmc_cmdinitstructure.CmdIndex  = SD_CMD_GO_IDLE_STATE;
    sdmmc_cmdinitstructure.Response  = SDMMC_RESPONSE_NO;
    sdmmc_cmdinitstructure.Attribute = SDMMC_CMD_START_CMD | 
                                       SDMMC_CMD_USE_HOLD_REG | 
                                       SDMMC_CMD_PRV_DAT_WAIT | 
                                       SDMMC_CMD_UPDATE_CLK;
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
    if (SDMMC_OK == Core_SDMMC_WaiteCmdStart(hsd->Instance))
    {
        delay_ms(10);
    }
    else
    {
        return -1;
    }

    switch(hsd->SpeedMode)
    {
      case CARD_SDR12:
        Core_SDMMC_SetCLKDIV(hsd->Instance, 0x00000004);  //switch frequency from 400Khz to 25Mhz,SDR12
        dlog_info("Now in SDR12 Mode\n");
        break;
      case CARD_SDR25:
        Core_SDMMC_SetCLKDIV(hsd->Instance, 0x00000002);  //switch frequency from 400Khz to 50Mhz,SDR25
        dlog_info("Now in SDR25 Mode\n");
        break;
      case CARD_SDR50:
        Core_SDMMC_SetCLKDIV(hsd->Instance, 0x00000001);  //switch frequency from 400Khz to 100Mhz,SDR50
        dlog_info("Now in SDR50 Mode\n");
        break;
      case CARD_SDR104:
        Core_SDMMC_SetCLKDIV(hsd->Instance, 0x00000000);  //switch frequency from 400Khz to 100Mhz,SDR104
        dlog_info("Now in SDR104 Mode\n");
        break;
      default:
        Core_SDMMC_SetCLKDIV(hsd->Instance, 0x00000002);  //switch frequency from 400Khz to 100Mhz,SDR25
        dlog_info("Default in SDR25 Mode\n");
        break;
    }

    Core_SDMMC_SetCLKSRC(hsd->Instance, 0x00000000);
    Core_SDMMC_SetCLKENA(hsd->Instance, 0x00000001);
    sdmmc_cmdinitstructure.Argument  = 0x00000000;
    sdmmc_cmdinitstructure.CmdIndex  = SD_CMD_GO_IDLE_STATE;
    sdmmc_cmdinitstructure.Response  = SDMMC_RESPONSE_NO;
    sdmmc_cmdinitstructure.Attribute = SDMMC_CMD_START_CMD | 
                                       SDMMC_CMD_USE_HOLD_REG | 
                                       SDMMC_CMD_PRV_DAT_WAIT | 
                                       SDMMC_CMD_UPDATE_CLK;
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
    Core_SDMMC_WaiteCmdStart(hsd->Instance);

    return errorstate;
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
  sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
                                            SDMMC_CMD_USE_HOLD_REG | 
                                            SDMMC_CMD_UPDATE_CLK | 
                                            SDMMC_CMD_PRV_DAT_WAIT;
  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
  Core_SDMMC_WaiteCmdStart(hsd->Instance);

  Core_SDMMC_SetTMOUT(hsd->Instance, SDMMC_TMOUT_DEFAULT);
  Core_SDMMC_SetCTYPE(hsd->Instance, SDMMC_CTYPE_1BIT);
  /* FIFO_DEPTH = 16 words */
  Core_SDMMC_SetFIFOTH(hsd->Instance, 0x00070008);
  Core_SDMMC_SetCTRL(hsd->Instance, SDMMC_CTRL_USE_INTERNAL_IDMAC | 
                                    SDMMC_CTRL_INT_ENABLE);
  Core_SDMMC_SetBMOD(hsd->Instance, SDMMC_BMOD_ENABLE | 
                                    SDMMC_BMOD_FB);
  /* close the FIFO RX and TX interrupt */
  //Core_SDMMC_SetINTMASK(hsd->Instance, (uint32_t)0x0000FFFF & \
  //                      ~(SDMMC_INTMASK_TXDR | SDMMC_INTMASK_RXDR));
  // Core_SDMMC_SetDBADDR(hsd->Instance, 0x440B5000);
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
  dlog_info("Send CMD0\n");
#endif
  delay_ms(500);  /*add the delay to fix the initialize fail when print to sram */
  sdmmc_cmdinitstructure.Argument         = 0;
  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_GO_IDLE_STATE;
  sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_NO;
  sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
                                            SDMMC_CMD_USE_HOLD_REG | 
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

  //get_val = Core_SDMMC_GetRESP0(hsd->Instance);
  // dlog_info("CMD0 RESP0 = %x\n", get_val);
  //get_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
  // dlog_info("CMD0 RINTSTS = %x\n", get_val);


#ifdef ECHO
  dlog_info("Send CMD8\n");
#endif
  sdmmc_cmdinitstructure.Argument         = SD_CHECK_PATTERN;
  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SEND_IF_COND;
  sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R7;
  sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
                                            SDMMC_CMD_USE_HOLD_REG | 
                                            SDMMC_CMD_PRV_DAT_WAIT | 
                                            SDMMC_CMD_RESP_CRC | 
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
  // get_val = Core_SDMMC_GetRESP0(hsd->Instance);
  // dlog_info("CMD8 RESP0 = %x\n", get_val);
  // get_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
  // dlog_info("CMD8 RINTSTS = %x\n", get_val);

  /* Send CMD55 */
#ifdef ECHO
  dlog_info("Send CMD55\n");
#endif
  sdmmc_cmdinitstructure.Argument         = 0;
  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_APP_CMD;
  sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
  sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
      SDMMC_CMD_USE_HOLD_REG | 
      SDMMC_CMD_PRV_DAT_WAIT | 
      SDMMC_CMD_RESP_CRC | 
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
  //get_val = Core_SDMMC_GetRESP0(hsd->Instance);
  // dlog_info("CMD55 RESP0 = %x\n", get_val);
  //get_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
  // dlog_info("CMD55 RINTSTS = %x\n", get_val);

  /* Send ACMD41 */
#ifdef ECHO
  dlog_info("Send ACMD41");
#endif
  //sdmmc_cmdinitstructure.Argument         = SD_ACMD41_HCS | 
  //                                          SD_ACMD41_XPC | 
  //                                          SD_ACMD41_S18R;
  sdmmc_cmdinitstructure.Argument         = 0x51FF8000;
  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_APP_SD_SEND_OP_COND;
  sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R3;
  sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
                                            SDMMC_CMD_USE_HOLD_REG | 
                                            SDMMC_CMD_PRV_DAT_WAIT | 
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
  // get_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
  // dlog_info("ACMD41 RINTSTS = 0x%x\n", get_val);
  response = Core_SDMMC_GetRESP0(hsd->Instance);
  dlog_info("ACMD41 RESP0 = 0x%x", response);

  vs_busy = (response & SD_ACMD41_BUSY);
  if (vs_busy == 0) 
  {
    dlog_info("ACMD41 Loop Start:");
    do 
    {
        dlog_info("loop %d: Send CMD55", loop);
/*       delay_ms(100); */
      SysTicks_DelayMS(1);
      /* Send CMD55 */
      sdmmc_cmdinitstructure.Argument         = 0;
      sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_APP_CMD;
      sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
      sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
                                                SDMMC_CMD_USE_HOLD_REG | 
                                                SDMMC_CMD_PRV_DAT_WAIT | 
                                                SDMMC_CMD_RESP_CRC | 
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

      //response = Core_SDMMC_GetRESP0(hsd->Instance);
      // dlog_info("CMD55 RESP0 = %x\n", response);
      //get_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
      // dlog_info("CMD55 RINTSTS = %x\n", get_val);
      /* Send ACMD41 */
      sdmmc_cmdinitstructure.Argument         = 0x51FF8000;
      //sdmmc_cmdinitstructure.Argument         = SD_ACMD41_HCS | \
      //                                          SD_ACMD41_XPC | \
      //                                          SD_ACMD41_S18R |\
      //                                          0xFFF000;
      sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_APP_SD_SEND_OP_COND;
      sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R3;
      sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
                                                SDMMC_CMD_USE_HOLD_REG | 
                                                SDMMC_CMD_PRV_DAT_WAIT | 
                                                SDMMC_CMD_RESP_EXP;
      Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
      Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
      /* Get command get_val */
      response = Core_SDMMC_GetResponse(hsd->Instance, SDMMC_RESP0);
      vs_busy = (response & SD_ACMD41_BUSY);

      // get_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
      // dlog_info("ACMD41 RINTSTS = %x\n", get_val);
      response = Core_SDMMC_GetRESP0(hsd->Instance);
      // dlog_info("ACMD41 RESP0 = %x\n", response);
      if (loop == 1000) 
      {
        dlog_info("ACMD41 Loop Fail");
      }
      vs_busy = (response & SD_ACMD41_BUSY);
      
      loop++;
    } while ((vs_busy == 0) && loop <= 1000);

    if (vs_busy != 0) 
    {
      delay_ms(10);
      dlog_info("ACMD41 Loop Done, loop = %d", loop);
    }
  }

 if (response & SD_ACMD41_S18R) 
  {
    delay_ms(10);
    dlog_info("1.8V Support");

#ifdef ECHO
    dlog_info("Send CMD11");
#endif
    delay_ms(500);
    /* send CMD11 to switch 1.8V bus signaling level */
    Core_SDMMC_SetCLKENA(hsd->Instance, 0x00001);
    sdmmc_cmdinitstructure.Argument  = 0x41ffffff;
    sdmmc_cmdinitstructure.CmdIndex  = SD_CMD_VOLTAGE_SWITCH;
    sdmmc_cmdinitstructure.Response  = SDMMC_RESPONSE_R1;
    sdmmc_cmdinitstructure.Attribute = SDMMC_CMD_START_CMD | 
                                       SDMMC_CMD_USE_HOLD_REG | 
                                       SDMMC_CMD_VOLT_SWITCH  | 
                                       SDMMC_CMD_PRV_DAT_WAIT | 
                                       SDMMC_CMD_RESP_EXP  | 
                                       SDMMC_CMD_RESP_CRC;
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
#ifdef ECHO
    dlog_info("1.8v Switch Start");
#endif
    dlog_output(100);
    Core_SDMMC_WaiteCmdStart(hsd->Instance);
    Core_SDMMC_WaiteVoltSwitchInt(hsd->Instance);
    // delay_ms(1);
    dlog_info("CMD11 RESP 1.8v Switch Success");
    //get_val = Core_SDMMC_GetRESP0(hsd->Instance);
    // dlog_info("CMD11 RESP0 = %x\n", get_val);
    //get_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
    // dlog_info("CMD11 RINTSTS = %x\n", get_val);


    /* disable all the clock */
    Core_SDMMC_SetCLKENA(hsd->Instance, 0x00000000);
    Core_SDMMC_SetUHSREG(hsd->Instance, 0x0000FFFF);

    sdmmc_cmdinitstructure.Argument  = 0x00000000;
    sdmmc_cmdinitstructure.CmdIndex  = SD_CMD_GO_IDLE_STATE;
    sdmmc_cmdinitstructure.Response  = SDMMC_RESPONSE_NO;
    sdmmc_cmdinitstructure.Attribute = SDMMC_CMD_START_CMD | 
                                       SDMMC_CMD_USE_HOLD_REG | 
                                       SDMMC_CMD_VOLT_SWITCH  | 
                                       SDMMC_CMD_PRV_DAT_WAIT | 
                                       SDMMC_CMD_UPDATE_CLK;
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
    //get_val = Core_SDMMC_GetRESP0(hsd->Instance);
    // dlog_info("CMD0 RESP0 = %x\n", get_val);
    //get_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
    // dlog_info("CMD0 RINTSTS = %x\n", get_val);
    delay_ms(500);
    dlog_info("Host Supply 1.8v Clock");

    Core_SDMMC_SetCLKENA(hsd->Instance, 0x00001);

    sdmmc_cmdinitstructure.Argument  = 0x00000000;
    sdmmc_cmdinitstructure.CmdIndex  = SD_CMD_GO_IDLE_STATE;
    sdmmc_cmdinitstructure.Response  = SDMMC_RESPONSE_NO;
    sdmmc_cmdinitstructure.Attribute = SDMMC_CMD_START_CMD | 
                                       SDMMC_CMD_USE_HOLD_REG | 
                                       SDMMC_CMD_VOLT_SWITCH  | 
                                       SDMMC_CMD_PRV_DAT_WAIT | 
                                       SDMMC_CMD_UPDATE_CLK;
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
    Core_SDMMC_WaiteVoltSwitchInt(hsd->Instance);
    //get_val = Core_SDMMC_GetRESP0(hsd->Instance);
    //dlog_info("CMD0 RESP0 = %x\n", get_val);
    //get_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
    //dlog_info("CMD0 RINTSTS = %x\n", get_val);
    dlog_info("Voltage Switching Success!");
    dlog_output(100);
  }
  else {
    dlog_info("already in 1.8V state");
    errorstate = SD_FAIL;
  }
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
  // SD clock = 50MHz
  // Core_SDMMC_SetCLKDIV(hsd->Instance, SDMMC_CLKDIV_BIT1);
  //SD clock = 1MHz
  //Core_SDMMC_SetCLKDIV(hsd->Instance, SDMMC_CLKDIV_BIT2 | \
  //                     SDMMC_CLKDIV_BIT5 | \
  //                     SDMMC_CLKDIV_BIT6 );
  //SD clock = 25MHz
  // Core_SDMMC_SetCLKDIV(hsd->Instance, SDMMC_CLKDIV_BIT2);
  //SD clock = 100MHz
  // Core_SDMMC_SetCLKDIV(hsd->Instance, SDMMC_CLKDIV_BIT0);
  // Core_SDMMC_SetCLKSRC(hsd->Instance, SDMMC_CLKSRC_CLKDIV0);
  // SD clock = 400KHz
  Core_SDMMC_SetCLKDIV(hsd->Instance, 0xFA);
  Core_SDMMC_SetCLKSRC(hsd->Instance, SDMMC_CLKSRC_CLKDIV0);
  Core_SDMMC_SetCLKENA(hsd->Instance, 0x00000001);

  return 0;
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

  //   /* Send CMD7 SDMMC_SEL_DESEL_CARD */
  // sdmmc_cmdinitstructure.Argument         = (uint32_t)(hsd->RCA << 16);
  // sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_GO_INACTIVE_STATE;
  // sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
  // sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
  //     SDMMC_CMD_USE_HOLD_REG | \
  //     SDMMC_CMD_PRV_DAT_WAIT | \
  //     SDMMC_CMD_RESP_CRC    | \
  //     SDMMC_CMD_RESP_EXP;
  // Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
  // Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
  // /* waite for command finish*/
  // Core_SDMMC_WaiteCmdDone(hsd->Instance);


  /* Set Power State to OFF */
/*   sdmmc_cmdinitstructure.Argument         = 0xFFFFFFFF; */
  sdmmc_cmdinitstructure.Argument         = hsd->RCA << 16 & 0xFFFFFFFF;
  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SEL_DESEL_CARD;
  sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
  sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
                                            SDMMC_CMD_USE_HOLD_REG | 
                                            SDMMC_CMD_PRV_DAT_WAIT | 
                                            SDMMC_CMD_RESP_CRC | 
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
  Core_SDMMC_SetCLKENA(hsd->Instance, 0x0);
  Core_SDMMC_SetCLKSRC(hsd->Instance, 0x0);

  Core_SDMMC_SetBMOD(hsd->Instance, SDMMC_BMOD_SWRESET);
  Core_SDMMC_SetCTRL(hsd->Instance, SDMMC_CTRL_CONTROLLER_RESET |
                                    SDMMC_CTRL_FIFO_RESET |
                                    SDMMC_CTRL_DMA_RESET);
  Core_SDMMC_SetUHSREG(hsd->Instance, 0x0);
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
  sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
      SDMMC_CMD_USE_HOLD_REG | 
      SDMMC_CMD_PRV_DAT_WAIT | 
      SDMMC_CMD_RESP_CRC | 
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
  // dlog_info("Card State = %x\n", *CardStatus);

  return errorstate;
}

/**
  * @brief  Checks for error conditions for CMD.
  * @param  hsd: SD handle
  * @retval SD Card error state
  */
static SD_ErrorTypedef SD_CmdError(SD_HandleTypeDef *hsd)
{
  SD_ErrorTypedef errorstate = SD_OK;
  uint32_t RINTSTS_val, cmd_done, endBitErr, startBitErr, hdLockErr, datStarvTo, resp_to;
  uint32_t datCrcErr, resp_crc_err, datTranOver, resp_err, errHappen;
    uint32_t start = SysTicks_GetTickCount();
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
    
    if((SysTicks_GetDiff(start, SysTicks_GetTickCount())) > 100)
    {
        dlog_error("time out");
    }
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
  sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
      SDMMC_CMD_USE_HOLD_REG | 
      SDMMC_CMD_PRV_DAT_WAIT | 
      SDMMC_CMD_RESP_CRC     | 
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
  // *status = responseR1 & SDMMC_RESP1_CURRENT_STATE;
  *status = responseR1 >> 9;

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
  sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
      SDMMC_CMD_USE_HOLD_REG | 
      SDMMC_CMD_SEND_INIT | 
      SDMMC_CMD_PRV_DAT_WAIT | 
      SDMMC_CMD_RESP_CRC | 
      SDMMC_CMD_RESP_EXP;
  Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
  RINTSTS_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
#ifdef ECHO
  dlog_info("CMD5 RINTSTS_val = %x\n", RINTSTS_val);
#endif
    uint32_t start = SysTicks_GetTickCount();
  do {
    RINTSTS_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
    cmd_done = (RINTSTS_val & SDMMC_RINTSTS_CMD_DONE);
    if((SysTicks_GetDiff(start, SysTicks_GetTickCount())) > 100)
    {
        dlog_error("time out");
        return SD_ERROR;
    }
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

  Core_SDMMC_SetUHSREG(hsd->Instance, 0x0000FFFF);
  Core_SDMMC_SetCTYPE(hsd->Instance, SDMMC_CTYPE_4BIT);
  Core_SDMMC_SetCTRL(hsd->Instance, SDMMC_CTRL_USE_INTERNAL_IDMAC | 
                     SDMMC_CTRL_INT_ENABLE | 
                     SDMMC_CTRL_FIFO_RESET);
  Core_SDMMC_SetBLKSIZ(hsd->Instance, dma->BlockSize);
  // Core_SDMMC_SetBYCTNT(hsd->Instance, dma->SectorNum * dma->BlockSize);
  /* Set Block Size for Card */
  sdmmc_cmdinitstructure.Argument         = dma->BlockSize;
  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SET_BLOCKLEN;
  sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
  sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
      SDMMC_CMD_USE_HOLD_REG | 
      SDMMC_CMD_PRV_DAT_WAIT | 
      SDMMC_CMD_RESP_CRC     | 
      SDMMC_CMD_RESP_EXP;
  Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
  /* Check for error conditions */
  Core_SDMMC_WaiteCmdDone(hsd->Instance);
// #ifdef ECHO
//   get_val = Core_SDMMC_GetRESP0(hsd->Instance);
//   dlog_info("CMD16 RESP0 = 0x%x\n", get_val);
// #endif

  /* set up idma descriptor */
  Core_SDMMC_SetBMOD(hsd->Instance, SDMMC_BMOD_ENABLE | SDMMC_BMOD_FB);
  Core_SDMMC_SetIDINTEN(hsd->Instance, 0x0);
  // Core_SDMMC_SetDBADDR(hsd->Instance, DTCMBUSADDR((uint32_t)&desc[0]));
  return errorstate;
}

void SD_IRQHandler(uint32_t vectorNum)
{

    uint32_t status, pending, cdetect;

    static uint8_t flag_plug_out = 0;

    status  = read_reg32((uint32_t *)(SDMMC_BASE + 0x44));  /* RINTSTS */
    pending = read_reg32((uint32_t *)(SDMMC_BASE + 0x40));  /* MINTSTS */
    cdetect = read_reg32((uint32_t *)(SDMMC_BASE + 0x50));  /* CDETECT*/

    write_reg32((uint32_t *)(SDMMC_BASE + 0x44), 0xFFFFFFFF);  /* RINTSTS */

    if ((read_reg32((uint32_t *)(SDMMC_BASE + 0x50)) & (1<<0)) == 0) // card in
    {
        flag_plug_out = 0;
        SYS_EVENT_Notify_From_ISR(SYS_EVENT_ID_SD_CARD_CHANGE, (void*)(&flag_plug_out));
        dlog_info("event: SYS_EVENT_ID_SD_CARD_CHANGE state 0 notify succeed");
    }
    else    // card out
    {
        
        flag_plug_out = 1;
        SYS_EVENT_Notify_From_ISR(SYS_EVENT_ID_SD_CARD_CHANGE, (void*)(&flag_plug_out));
        dlog_info("event: SYS_EVENT_ID_SD_CARD_CHANGE state 1 notify succeed");
    }


  if(pending)
  {

#if 0
    if (pending & SDMMC_RINTSTS_RESP_ERR)
    {
      if (status & SDMMC_RINTSTS_RESP_ERR)
      {
          dlog_info("SDMMC_RINTSTS_RESP_ERR\n");
          status &= ~SDMMC_RINTSTS_RESP_ERR;
          write_reg32((uint32_t *)(SDMMC_BASE + 0x44), status);
          return;
        
      }
    }
    if (pending & SDMMC_RINTSTS_CMD_DONE)
    {
      if (status & SDMMC_RINTSTS_CMD_DONE)
      {
          dlog_info("SDMMC_RINTSTS_CMD_DONE\n");
          status &= ~SDMMC_RINTSTS_CMD_DONE;
          write_reg32((uint32_t *)(SDMMC_BASE + 0x44), status);
          return;
        
      }
    }
    if (pending & SDMMC_RINTSTS_DATA_OVER)
    {
      if (status & SDMMC_RINTSTS_DATA_OVER)
      {
          dlog_info("SDMMC_RINTSTS_DATA_OVER\n");
          status &= ~SDMMC_RINTSTS_DATA_OVER;
          write_reg32((uint32_t *)(SDMMC_BASE + 0x44), status);
          return;
        
      }
    }
    if (pending & SDMMC_RINTSTS_TXDR)
    {
      if (status & SDMMC_RINTSTS_TXDR)
      {
          dlog_info("SDMMC_RINTSTS_TXDR\n");
          status &= ~SDMMC_RINTSTS_TXDR;
          write_reg32((uint32_t *)(SDMMC_BASE + 0x44), status);
          return;
        
      }
    }
    if (pending & SDMMC_RINTSTS_RXDR)
    {
      if (status & SDMMC_RINTSTS_RXDR)
      {
          dlog_info("SDMMC_RINTSTS_RXDR\n");
          status &= ~SDMMC_RINTSTS_RXDR;
          write_reg32((uint32_t *)(SDMMC_BASE + 0x44), status);
          return;
        
      }
    }
    if (pending & SDMMC_RINTSTS_RCRC)
    {
      if (status & SDMMC_RINTSTS_RCRC)
      {
          dlog_info("SDMMC_RINTSTS_RCRC\n");
          status &= ~SDMMC_RINTSTS_RCRC;
          write_reg32((uint32_t *)(SDMMC_BASE + 0x44), status);
          return;
        
      }
    }
    if (pending & SDMMC_RINTSTS_RTO)
    {
      if (status & SDMMC_RINTSTS_RTO)
      {
          dlog_info("SDMMC_RINTSTS_RTO\n");
          status &= ~SDMMC_RINTSTS_RTO;
          write_reg32((uint32_t *)(SDMMC_BASE + 0x44), status);
          return;
        
      }
    }
    if (pending & SDMMC_RINTSTS_DRTO)
    {
      if (status & SDMMC_RINTSTS_DRTO)
      {
          dlog_info("SDMMC_RINTSTS_DRTO\n");
          status &= ~SDMMC_RINTSTS_DRTO;
          write_reg32((uint32_t *)(SDMMC_BASE + 0x44), status);
          return;
        
      }
    }
    if (pending & SDMMC_RINTSTS_HTO)
    {
      if (status & SDMMC_RINTSTS_HTO)
      {
          dlog_info("SDMMC_RINTSTS_HTO\n");
          status &= ~SDMMC_RINTSTS_HTO;
          write_reg32((uint32_t *)(SDMMC_BASE + 0x44), status);
          return;
        
      }
    }

    if (pending & SDMMC_INTMASK_FRUN)
    {
      if (status & SDMMC_INTMASK_FRUN)
      {
          dlog_info("SDMMC_INTMASK_FRUN\n");
          status &= ~SDMMC_INTMASK_FRUN;
          write_reg32((uint32_t *)(SDMMC_BASE + 0x44), status);
          return;
        
      }
    }
    if (pending & SDMMC_INTMASK_HLE)
    {
      if (status & SDMMC_INTMASK_HLE)
      {
          dlog_info("SDMMC_INTMASK_HLE\n");
          status &= ~SDMMC_INTMASK_HLE;
          write_reg32((uint32_t *)(SDMMC_BASE + 0x44), status);
          return;
        
      }
    }
    if (pending & SDMMC_INTMASK_SBE)
    {
      if (status & SDMMC_INTMASK_SBE)
      {
          dlog_info("SDMMC_INTMASK_SBE\n");
          status &= ~SDMMC_INTMASK_SBE;
          write_reg32((uint32_t *)(SDMMC_BASE + 0x44), status);
          return;
        
      }
    }
    if (pending & SDMMC_INTMASK_ACD)
    {
      if (status & SDMMC_INTMASK_ACD)
      {
          dlog_info("SDMMC_INTMASK_ACD\n");
          status &= ~SDMMC_INTMASK_ACD;
          write_reg32((uint32_t *)(SDMMC_BASE + 0x44), status);
          return;
        
      }
    }
#endif
  }
}


SD_TRANSFER_STATUS SD_CardStatus(SD_STATUS *e_cardStatus)
{
  //dlog_info("Get Card Status\n");
  uint32_t RespState = 0;
  SD_STATUS e_cardStatusTmp = SD_CARD_IDLE;
  SDMMC_Status errorstate = SDMMC_OK;
  errorstate = SD_GetState(&sdhandle, (uint32_t *)&RespState);
  if (errorstate != SDMMC_OK)
  {
    dlog_info("Get SD Status Failed!\n");
    return SD_CARD_ERROR;
  }

  /* Find SD status according to card state*/
  e_cardStatusTmp = (SD_STATUS)((RespState >> 9) & 0x0F);
  *e_cardStatus = e_cardStatusTmp;
  if (e_cardStatusTmp == SD_CARD_TRANSFER)
  {
    return SD_TRANSFER_READY;
  }
  else if(e_cardStatusTmp == SD_CARD_ERROR)
  {
    return SD_TRANSFER_ERROR;
  }
  else
  {
    return SD_TRANSFER_BUSY;
  }
}

void SD_init_deInit_Callback(void *p)
{
    uint32_t status, pending, cdetect;
    static uint8_t flag_initting = 0;
    static uint8_t flag_card_state = 0;

#if 0
    dlog_info("%d, *p = %d", __LINE__, *((int*)p));
    dlog_info("cardinfo.inited = %d", cardinfo.inited);
#endif

#if 0
    status = read_reg32((uint32_t *)(SDMMC_BASE + 0x44));  /* RINTSTS */
    pending = read_reg32((uint32_t *)(SDMMC_BASE + 0x40));  /* MINTSTS */
    cdetect = read_reg32((uint32_t *)(SDMMC_BASE + 0x50));  /* CDETECT*/
#endif

    if (flag_initting == 0)
    {
        flag_initting = 1;
        if (getCardPresence == CARD_IN && flag_card_state == 0)
        {
            dlog_info("Initializing the SD Card...\n");
            sdhandle.Instance = SDMMC_ADDR;
            Card_SD_Init(&sdhandle, &cardinfo);
            flag_card_state = 1;
        }
        else
        {
            if (flag_card_state == 1)
            {
                dlog_info("Removing the SD Card...\n");
                SDMMC_Status errorstate = SDMMC_OK;
                Card_SD_DeInit(&sdhandle);
                dlog_info("Remove SD Success!\n");
                flag_card_state = 0;
            }
        }
        flag_initting = 0;
    }
 
    
#if 0
    write_reg32((uint32_t *)(SDMMC_BASE + 0x44), 0xFFFFFFFF);  /* RINTSTS */
#endif
    
#if 0
    uint32_t status, pending, cdetect;
    static uint8_t flag_plug_out = 0;
     
    status = read_reg32((uint32_t *)(SDMMC_BASE + 0x44));  /* RINTSTS */
    pending = read_reg32((uint32_t *)(SDMMC_BASE + 0x40));  /* MINTSTS */
    cdetect = read_reg32((uint32_t *)(SDMMC_BASE + 0x50));  /* CDETECT*/
    
    dlog_info("Initializing the SD Card...\n");
    write_reg32((uint32_t *)(SDMMC_BASE + 0x44), status);
    sdhandle.Instance = SDMMC_ADDR;
    SD_CardInfoTypedef *cardinfo;
    Card_SD_Init(&sdhandle, cardinfo);
    write_reg32((uint32_t *)(SDMMC_BASE + 0x44), 0xFFFFFFFF);    
#endif
    // dlog_info("Initialize SD Success!\n");
}
/* SYS_EVENT_RegisterHandler(SYS_EVENT_ID_IDLE, H264_Encoder_IdleCallback); */

static SD_STATUS sd_getState(SD_HandleTypeDef *hsd)
{
    SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
    uint32_t responseR1 = 0;
    SD_ErrorTypedef errorstate = SD_OK;
    sdmmc_cmdinitstructure.Argument         = (uint32_t)(hsd->RCA << 16);
    sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SEND_STATUS;
    sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
    sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | 
                                              SDMMC_CMD_USE_HOLD_REG | 
                                              SDMMC_CMD_PRV_DAT_WAIT | 
                                              SDMMC_CMD_RESP_CRC     | 
                                              SDMMC_CMD_RESP_EXP;
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
    
    responseR1 = Core_SDMMC_GetResponse(hsd->Instance, SDMMC_RESP0);
    dlog_info("%d, responseR1 = 0x%08x", __LINE__, responseR1);
    return responseR1>>9;
}
