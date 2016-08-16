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

#include "system_config.h"
#include "sd_core.h"
#include "sd_card.h"
#include "command.h"

/** @defgroup SD_Private_Functions SD Private Functions
  * @{
  */
static SD_ErrorTypedef SD_Initialize_Cards(SD_HandleTypeDef *hsd);
static SD_ErrorTypedef SD_Select_Deselect(SD_HandleTypeDef *hsd, uint32_t addr);
static SD_ErrorTypedef SD_PowerON(SD_HandleTypeDef *hsd);
static SD_ErrorTypedef SD_PowerOFF(SD_HandleTypeDef *hsd);
static SD_ErrorTypedef SD_SendStatus(SD_HandleTypeDef *hsd, uint32_t *pCardStatus);
static SD_CardStateTypedef SD_GetState(SD_HandleTypeDef *hsd);
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
/*static void SD_DMA_RxCplt(DMA_HandleTypeDef *hdma);
static void SD_DMA_RxError(DMA_HandleTypeDef *hdma);
static void SD_DMA_TxCplt(DMA_HandleTypeDef *hdma);
static void SD_DMA_TxError(DMA_HandleTypeDef *hdma);
*/
/**
  * @brief  Initializes the SD card according to the specified parameters in the
            SD_HandleTypeDef and create the associated handle.
  * @param  hsd: SD handle
  * @param  SDCardInfo: SD_CardInfoTypedef structure for SD card information
  * @retval HAL SD error state
  */
SD_ErrorTypedef Card_SD_Init(SD_HandleTypeDef *hsd, SD_CardInfoTypedef *SDCardInfo)
{
  __IO SD_ErrorTypedef errorstate = SD_OK;

  /* Identify card operating voltage */
  serial_puts("enter the SD_PowerON\n");
  errorstate = SD_PowerON(hsd);
  serial_puts("SD_PowerON state = ");
  print_str(errorstate);
  serial_putc('\n');
  if (errorstate != SD_OK)
  {
    return errorstate;
  }


  /* Initialize the present SDMMC card(s) and put them in idle state */
  serial_puts("enter the SD_Initialize_Cards\n");
  errorstate = SD_Initialize_Cards(hsd);
  serial_puts("SD_Initialize_Cards state = ");
  print_str(errorstate);
  serial_putc('\n');
  if (errorstate != SD_OK)
  {
    return errorstate;
  }
#ifdef ECHO
  /* Read CSD/CID MSD registers */
  serial_puts("enter the SD_Card_Get_CardInfo\n");
  errorstate = Card_SD_Get_CardInfo(hsd, SDCardInfo);
#endif
  if (errorstate == SD_OK)
  {
    /* Select the Card */
    serial_puts("enter the SD_Select_Deselect\n");
    errorstate = SD_Select_Deselect(hsd, (uint32_t)(((uint32_t)SDCardInfo->RCA) << 16));
    serial_puts("SD_Select_Deselect state = ");
    print_str(errorstate);
    serial_putc('\n');
  }
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
  errorstate = SD_PowerOFF(hsd);

  return errorstate;
}

/*
@verbatim
  ==============================================================================
                        ##### IO operation functions #####
  ==============================================================================
  [..]
    This subsection provides a set of functions allowing to manage the data
    transfer from/to SD card.

@endverbatim
  * @{
  */

/**
  * @brief  Reads block(s) from a specified address in a card. The Data transfer
  *         is managed by polling mode.
  * @param  hsd: SD handle
  * @param  pReadBuffer: pointer to the buffer that will contain the received data
  * @param  ReadAddr: Address from where data is to be read
  * @param  BlockSize: SD card Data block size
  *   @note BlockSize must be 512 bytes.
  * @param  NumberOfBlocks: Number of SD blocks to read
  * @retval SD Card error state
  */
// SD_ErrorTypedef Card_SD_ReadBlocks(SD_HandleTypeDef *hsd, uint32_t *pReadBuffer, uint64_t ReadAddr, uint32_t BlockSize, uint32_t NumberOfBlocks)
// {
//  SDMMC_CmdInitTypeDef  sdmmc_cmdinitstructure;
//  SDMMC_DataInitTypeDef sdmmc_datainitstructure;
//  SD_ErrorTypedef errorstate = SD_OK;
//  uint32_t count = 0, *tempbuff = (uint32_t *)pReadBuffer;

//  /* Initialize data control register */
//  //hsd->Instance->DCTRL = 0;

//  if (hsd->CardType == HIGH_CAPACITY_SD_CARD)
//  {
//    BlockSize = 512;
//    ReadAddr /= 512;
//  }

//  /* Set Block Size for Card */
//  sdmmc_cmdinitstructure.Argument         = (uint32_t) BlockSize;
//  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SET_BLOCKLEN;
//  //sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_SHORT;
//  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);

//  /* Check for error conditions */
//  errorstate = SD_CmdResp1Error(hsd, SD_CMD_SET_BLOCKLEN);

//  if (errorstate != SD_OK)
//  {
//    return errorstate;
//  }

//  /* Configure the SD DPSM (Data Path State Machine) */
//  sdmmc_datainitstructure.DataTimeOut   = SD_DATATIMEOUT;
//  sdmmc_datainitstructure.DataLength    = NumberOfBlocks * BlockSize;
//  sdmmc_datainitstructure.DataBlockSize = DATA_BLOCK_SIZE;
//  sdmmc_datainitstructure.TransferDir   = SDMMC_TRANSFER_DIR_TO_SDMMC;
//  SDMMC_DataConfig(hsd->Instance, &sdmmc_datainitstructure);

//  if (NumberOfBlocks > 1)
//  {
//    /* Send CMD18 READ_MULT_BLOCK with argument data address */
//    sdmmc_cmdinitstructure.CmdIndex = SD_CMD_READ_MULT_BLOCK;
//  }
//  else
//  {
//    /* Send CMD17 READ_SINGLE_BLOCK */
//    sdmmc_cmdinitstructure.CmdIndex = SD_CMD_READ_SINGLE_BLOCK;
//  }

//  sdmmc_cmdinitstructure.Argument         = (uint32_t)ReadAddr;
//  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);

//  /* Read block(s) in polling mode */
//  if (NumberOfBlocks > 1)
//  {
//    /* Check for error conditions */
//    errorstate = SD_CmdResp1Error(hsd, SD_CMD_READ_MULT_BLOCK);

//    if (errorstate != SD_OK)
//    {
//      return errorstate;
//    }

//    /* Poll on SDMMC flags */
//    while (!__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_RXOVERR | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_DATAEND))
//    {
//      if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_RXFIFOHF))
//      {
//        /* Read data from SDMMC Rx FIFO */
//        for (count = 0; count < 8; count++)
//        {
//          *(tempbuff + count) = SDMMC_ReadFIFO(hsd->Instance);
//        }

//        tempbuff += 8;
//      }
//    }
//  }
//  else
//  {
//    /* Check for error conditions */
//    errorstate = SD_CmdResp1Error(hsd, SD_CMD_READ_SINGLE_BLOCK);

//    if (errorstate != SD_OK)
//    {
//      return errorstate;
//    }

//    /* In case of single block transfer, no need of stop transfer at all */
//    while (!__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_RXOVERR | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_DBCKEND))
//    {
//      if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_RXFIFOHF))
//      {
//        /* Read data from SDMMC Rx FIFO */
//        for (count = 0; count < 8; count++)
//        {
//          *(tempbuff + count) = SDMMC_ReadFIFO(hsd->Instance);
//        }

//        tempbuff += 8;
//      }
//    }
//  }

//  /* Send stop transmission command in case of multiblock read */
//  if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_DATAEND) && (NumberOfBlocks > 1))
//  {
//    if ((hsd->CardType == STD_CAPACITY_SD_CARD_V1_1) || \
//            (hsd->CardType == STD_CAPACITY_SD_CARD_V2_0) || \
//            (hsd->CardType == HIGH_CAPACITY_SD_CARD))
//    {
//      /* Send stop transmission command */
//      errorstate = HAL_SD_StopTransfer(hsd);
//    }
//  }

//  /* Get error state */
//  if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_DTIMEOUT))
//  {
//    __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_FLAG_DTIMEOUT);

//    errorstate = SD_DATA_TIMEOUT;

//    return errorstate;
//  }
//  else if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_DCRCFAIL))
//  {
//    __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_FLAG_DCRCFAIL);

//    errorstate = SD_DATA_CRC_FAIL;

//    return errorstate;
//  }
//  else if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_RXOVERR))
//  {
//    __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_FLAG_RXOVERR);

//    errorstate = SD_RX_OVERRUN;

//    return errorstate;
//  }
//  else
//  {
//    /* No error flag set */
//  }

//  count = SD_DATATIMEOUT;

//  /* Empty FIFO if there is still any data */
//  while ((__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_RXDAVL)) && (count > 0))
//  {
//    *tempbuff = SDMMC_ReadFIFO(hsd->Instance);
//    tempbuff++;
//    count--;
//  }

//  /* Clear all the static flags */
//  __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);

//  return errorstate;
// }

/**
  * @brief  Allows to write block(s) to a specified address in a card. The Data
  *         transfer is managed by polling mode.
  * @param  hsd: SD handle
  * @param  pWriteBuffer: pointer to the buffer that will contain the data to transmit
  * @param  WriteAddr: Address from where data is to be written
  * @param  BlockSize: SD card Data block size
  * @note   BlockSize must be 512 bytes.
  * @param  NumberOfBlocks: Number of SD blocks to write
  * @retval SD Card error state
  */
// SD_ErrorTypedef Card_SD_WriteBlocks(SD_HandleTypeDef *hsd, uint32_t *pWriteBuffer, uint64_t WriteAddr, uint32_t BlockSize, uint32_t NumberOfBlocks)
// {
//  SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
//  SDMMC_DataInitTypeDef sdmmc_datainitstructure;
//  SD_ErrorTypedef errorstate = SD_OK;
//  uint32_t totalnumberofbytes = 0, bytestransferred = 0, count = 0, restwords = 0;
//  uint32_t *tempbuff = (uint32_t *)pWriteBuffer;
//  uint8_t cardstate  = 0;

//  /* Initialize data control register */
//  hsd->Instance->DCTRL = 0;

//  if (hsd->CardType == HIGH_CAPACITY_SD_CARD)
//  {
//    BlockSize = 512;
//    WriteAddr /= 512;
//  }

//  /* Set Block Size for Card */
//  sdmmc_cmdinitstructure.Argument         = (uint32_t)BlockSize;
//  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SET_BLOCKLEN;
//  sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_SHORT;
//  sdmmc_cmdinitstructure.WaitForInterrupt = SDMMC_WAIT_NO;
//  sdmmc_cmdinitstructure.CPSM             = SDMMC_CPSM_ENABLE;
//  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);

//  /* Check for error conditions */
//  errorstate = SD_CmdResp1Error(hsd, SD_CMD_SET_BLOCKLEN);

//  if (errorstate != SD_OK)
//  {
//    return errorstate;
//  }

//  if (NumberOfBlocks > 1)
//  {
//    /* Send CMD25 WRITE_MULT_BLOCK with argument data address */
//    sdmmc_cmdinitstructure.CmdIndex = SD_CMD_WRITE_MULT_BLOCK;
//  }
//  else
//  {
//    /* Send CMD24 WRITE_SINGLE_BLOCK */
//    sdmmc_cmdinitstructure.CmdIndex = SD_CMD_WRITE_SINGLE_BLOCK;
//  }

//  sdmmc_cmdinitstructure.Argument         = (uint32_t)WriteAddr;
//  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);

//  /* Check for error conditions */
//  if (NumberOfBlocks > 1)
//  {
//    errorstate = SD_CmdResp1Error(hsd, SD_CMD_WRITE_MULT_BLOCK);
//  }
//  else
//  {
//    errorstate = SD_CmdResp1Error(hsd, SD_CMD_WRITE_SINGLE_BLOCK);
//  }

//  if (errorstate != SD_OK)
//  {
//    return errorstate;
//  }

//  /* Set total number of bytes to write */
//  totalnumberofbytes = NumberOfBlocks * BlockSize;

//  /* Configure the SD DPSM (Data Path State Machine) */
//  sdmmc_datainitstructure.DataTimeOut   = SD_DATATIMEOUT;
//  sdmmc_datainitstructure.DataLength    = NumberOfBlocks * BlockSize;
//  sdmmc_datainitstructure.DataBlockSize = SDMMC_DATABLOCK_SIZE_512B;
//  sdmmc_datainitstructure.TransferDir   = SDMMC_TRANSFER_DIR_TO_CARD;
//  sdmmc_datainitstructure.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
//  sdmmc_datainitstructure.DPSM          = SDMMC_DPSM_ENABLE;
//  SDMMC_DataConfig(hsd->Instance, &sdmmc_datainitstructure);

//  /* Write block(s) in polling mode */
//  if (NumberOfBlocks > 1)
//  {
//    while (!__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_TXUNDERR | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_DATAEND))
//    {
//      if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_TXFIFOHE))
//      {
//        if ((totalnumberofbytes - bytestransferred) < 32)
//        {
//          restwords = ((totalnumberofbytes - bytestransferred) % 4 == 0) ? ((totalnumberofbytes - bytestransferred) / 4) : (( totalnumberofbytes -  bytestransferred) / 4 + 1);

//          /* Write data to SDMMC Tx FIFO */
//          for (count = 0; count < restwords; count++)
//          {
//            SDMMC_WriteFIFO(hsd->Instance, tempbuff);
//            tempbuff++;
//            bytestransferred += 4;
//          }
//        }
//        else
//        {
//          /* Write data to SDMMC Tx FIFO */
//          for (count = 0; count < 8; count++)
//          {
//            SDMMC_WriteFIFO(hsd->Instance, (tempbuff + count));
//          }

//          tempbuff += 8;
//          bytestransferred += 32;
//        }
//      }
//    }
//  }
//  else
//  {
//    /* In case of single data block transfer no need of stop command at all */
//    while (!__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_TXUNDERR | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_DBCKEND))
//    {
//      if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_TXFIFOHE))
//      {
//        if ((totalnumberofbytes - bytestransferred) < 32)
//        {
//          restwords = ((totalnumberofbytes - bytestransferred) % 4 == 0) ? ((totalnumberofbytes - bytestransferred) / 4) : (( totalnumberofbytes -  bytestransferred) / 4 + 1);

//          /* Write data to SDMMC Tx FIFO */
//          for (count = 0; count < restwords; count++)
//          {
//            SDMMC_WriteFIFO(hsd->Instance, tempbuff);
//            tempbuff++;
//            bytestransferred += 4;
//          }
//        }
//        else
//        {
//          /* Write data to SDMMC Tx FIFO */
//          for (count = 0; count < 8; count++)
//          {
//            SDMMC_WriteFIFO(hsd->Instance, (tempbuff + count));
//          }

//          tempbuff += 8;
//          bytestransferred += 32;
//        }
//      }
//    }
//  }

//  /* Send stop transmission command in case of multiblock write */
//  if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_DATAEND) && (NumberOfBlocks > 1))
//  {
//    if ((hsd->CardType == STD_CAPACITY_SD_CARD_V1_1) || (hsd->CardType == STD_CAPACITY_SD_CARD_V2_0) || \
//            (hsd->CardType == HIGH_CAPACITY_SD_CARD))
//    {
//      /* Send stop transmission command */
//      errorstate = HAL_SD_StopTransfer(hsd);
//    }
//  }

//  /* Get error state */
//  if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_DTIMEOUT))
//  {
//    __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_FLAG_DTIMEOUT);

//    errorstate = SD_DATA_TIMEOUT;

//    return errorstate;
//  }
//  else if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_DCRCFAIL))
//  {
//    __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_FLAG_DCRCFAIL);

//    errorstate = SD_DATA_CRC_FAIL;

//    return errorstate;
//  }
//  else if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_TXUNDERR))
//  {
//    __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_FLAG_TXUNDERR);

//    errorstate = SD_TX_UNDERRUN;

//    return errorstate;
//  }
//  else
//  {
//    /* No error flag set */
//  }

//  /* Clear all the static flags */
//  __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);

//  /* Wait till the card is in programming state */
//  errorstate = SD_IsCardProgramming(hsd, &cardstate);

//  while ((errorstate == SD_OK) && ((cardstate == SD_CARD_PROGRAMMING) || (cardstate == SD_CARD_RECEIVING)))
//  {
//    errorstate = SD_IsCardProgramming(hsd, &cardstate);
//  }

//  return errorstate;
// }

/**
  * @brief  Reads block(s) from a specified address in a card. The Data transfer
  *         is managed by DMA mode.
  * @note   This API should be followed by the function Card_SD_CheckReadOperation()
  *         to check the completion of the read process
  * @param  hsd: SD handle
  * @param  DestAddr: Pointer to the buffer that will contain the received data
  * @param  ReadAddr: Address from where data is to be read
  * @param  BlockSize: SD card Data block size
  * @note   BlockSize must be 512 bytes.
  * @param  NumberOfBlocks: Number of blocks to read.
  * @retval SD Card error state
  */
// SD_ErrorTypedef Card_SD_ReadBlocks_DMA(SD_HandleTypeDef *hsd, SDMMC_DMATransTypeDef *dma)
// {
//   SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
//   //SDMMC_DataInitTypeDef sdmmc_datainitstructure;
//   typedef uint32_t *uint32p;
//   SD_ErrorTypedef errorstate = SD_OK;
//   uint32_t BlockIndex, NextDesp, Dst_Addr;
//   uint32p DES0, DES1, DES2, DES3;
//   uint32_t get_val, cmd_done, data_over, card_busy;
//   /* Initialize handle flags */
//   hsd->SdTransferCplt  = 0;
//   hsd->DmaTransferCplt = 0;
//   hsd->SdTransferErr   = SD_OK;

//   /* Initialize SD Read operation */
//   if (dma->NumberOfBlocks > 1)
//   {
//     hsd->SdOperation = SD_READ_MULTIPLE_BLOCK;
//   }
//   else
//   {
//     hsd->SdOperation = SD_READ_SINGLE_BLOCK;
//   }

//   if (hsd->CardType == HIGH_CAPACITY_SD_CARD)
//   {
//     dma->NumberOfBlocks =  dma->NumberOfBlocks * dma->BlockSize / 512;
//     dma->BlockSize = 512;
//     dma->SrcAddr /= 512;
//   }


//   /* TODO Enable the DMA Channel */
//   //HAL_DMA_Start_IT(hsd->hdmarx, (uint32_t)&hsd->Instance->FIFO, (uint32_t)pReadBuffer, (uint32_t)(BlockSize * NumberOfBlocks) / 4);


//   /* Configure the SD DPSM (Data Path State Machine) */
//   errorstate = SD_DMAConfig(hsd, dma);
//   if (errorstate != SD_OK) {
//     return errorstate;
//   }
//   /*TODO Check number of blocks command */
// //  if (dma->NumberOfBlocks > 1)
// //  {
// //    /* Send CMD18 READ_MULT_BLOCK with argument data address */
// //    sdmmc_cmdinitstructure.CmdIndex = SD_CMD_READ_MULT_BLOCK;
// //  }
// //  else
// //  {
// //    /* Send CMD17 READ_SINGLE_BLOCK */
// //    sdmmc_cmdinitstructure.CmdIndex = SD_CMD_READ_SINGLE_BLOCK;
// //  }

//   for (BlockIndex = 0; BlockIndex < dma->NumberOfBlocks; BlockIndex++)
//   {
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
//     sdmmc_cmdinitstructure.Argument         = dma->SrcAddr + dma->BlockSize * BlockIndex;
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

//     /* reset Next descriptor */
// #ifdef ECHO
//     serial_puts("NTPT = ");
//     print_str(NextDesp);
//     serial_putc('\n');
// #endif
//     write_reg32(&(hsd->Instance->DBADDR), NextDesp);
//   }

//   /* Update the SD transfer error in SD handle */
//   hsd->SdTransferErr = errorstate;
//   return errorstate;
// }


SD_ErrorTypedef Card_SD_ReadBlocks_DMA(SD_HandleTypeDef *hsd, SDMMC_DMATransTypeDef *dma)
{
  SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
  //SDMMC_DataInitTypeDef sdmmc_datainitstructure;
  typedef uint32_t *uint32p;
  SD_ErrorTypedef errorstate = SD_OK;
  uint32_t BlockIndex, NextDesp, Dst_Addr, TotalBlocks;
  uint32p DES0, DES1, DES2, DES3;
  uint32_t get_val, cmd_done, data_over, card_busy;
  uint32_t RestBytes = dma->NumberOfBytes;
  /* Initialize handle flags */
  hsd->SdTransferCplt  = 0;
  hsd->SdTransferErr   = SD_OK;

  if (hsd->CardType == HIGH_CAPACITY_SD_CARD)
  {
    TotalBlocks =  dma->NumberOfBytes / 512;
    dma->BlockSize = 512;
  }

  /* Configure the SD DPSM (Data Path State Machine) */
  errorstate = SD_DMAConfig(hsd, dma);
  if (errorstate != SD_OK) {
    return errorstate;
  }
  Core_SDMMC_SetBYCTNT(hsd->Instance, dma->BlockSize);

  for (BlockIndex = 0; BlockIndex < TotalBlocks; BlockIndex++)
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

    /* reset Next descriptor */
#ifdef ECHO
    serial_puts("NTPT = ");
    print_str(NextDesp);
    serial_putc('\n');
#endif
    write_reg32(&(hsd->Instance->DBADDR), NextDesp);
    RestBytes -= dma->BlockSize;
#ifdef ECHO
    serial_puts("RestBytes = ");
    print_str(RestBytes);
    serial_putc('\n');
#endif
  }

  /* Update the SD transfer error in SD handle */
  hsd->SdTransferErr = errorstate;
  return errorstate;
}

/**
  * @brief  Writes block(s) to a specified address in a card. The Data transfer
  *         is managed by DMA mode.
  * @note   This API should be followed by the function HAL_SD_CheckWriteOperation()
  *         to check the completion of the write process (by SD current status polling).
  * @param  hsd: SD handle
  * @param  pWriteBuffer: pointer to the buffer that will contain the data to transmit
  * @param  WriteAddr: Address from where data is to be read
  * @param  BlockSize: the SD card Data block size
  * @note   BlockSize must be 512 bytes.
  * @param  NumberOfBlocks: Number of blocks to write
  * @retval SD Card error state
  */
// SD_ErrorTypedef Card_SD_WriteBlocks_DMA(SD_HandleTypeDef *hsd, SDMMC_DMATransTypeDef *dma)
// {
//   SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
//   SD_ErrorTypedef errorstate = SD_OK;
//   typedef uint32_t *uint32p;
//   uint32_t BlockIndex, NextDesp, Src_Addr;
//   uint32p DES0, DES1, DES2, DES3;
//   uint32_t get_val, cmd_done, data_over, card_busy;

//   /* Initialize handle flags */
//   hsd->SdTransferCplt  = 0;
//   hsd->DmaTransferCplt = 0;
//   hsd->SdTransferErr   = SD_OK;
//   /* Initialize SD Read operation */
//   if (dma->NumberOfBlocks > 1)
//   {
//     hsd->SdOperation = SD_READ_MULTIPLE_BLOCK;
//   }
//   else
//   {
//     hsd->SdOperation = SD_READ_SINGLE_BLOCK;

//   }

//   if (hsd->CardType == HIGH_CAPACITY_SD_CARD)
//   {
//     dma->NumberOfBlocks =  dma->NumberOfBlocks * dma->BlockSize / 512;
//     dma->BlockSize = 512;
//     //dma->DstAddr /= 512;
//   }

//   /* TODO Enable the DMA Channel */
//   //HAL_DMA_Start_IT(hsd->hdmarx, (uint32_t)&hsd->Instance->FIFO, (uint32_t)pReadBuffer, (uint32_t)(BlockSize * NumberOfBlocks) / 4);

//   /* Configure the SD DPSM (Data Path State Machine) */
//   errorstate = SD_DMAConfig(hsd, dma);
//   if (errorstate != SD_OK) {
//     return errorstate;
//   }

//   for (BlockIndex = 0; BlockIndex < dma->NumberOfBlocks; BlockIndex++)
//   {
// #ifdef ECHO
//     serial_puts("dma->dstaddr = ");
//     print_str(dma->DstAddr);
//     serial_putc('\n');
// #endif
//     Src_Addr = dma->SrcAddr + dma->BlockSize * BlockIndex;
//     DES0 = (uint32_t *)(dma->ListBaseAddr + 0x00000010 * BlockIndex + 0x00000000);
//     DES1 = (uint32_t *)(dma->ListBaseAddr + 0x00000010 * BlockIndex + 0x00000004);
//     DES2 = (uint32_t *)(dma->ListBaseAddr + 0x00000010 * BlockIndex + 0x00000008);
//     DES3 = (uint32_t *)(dma->ListBaseAddr + 0x00000010 * BlockIndex + 0x0000000c);
//     NextDesp = dma->ListBaseAddr + 0x00000010 * (BlockIndex + 1);

//     write_reg32(DES0, SDMMC_DES0_OWN | \
//                 SDMMC_DES0_FS | \
//                 SDMMC_DES0_CH | \
//                 SDMMC_DES0_LD);
//     write_reg32(DES1, 0x00000200);
//     write_reg32(DES2, Src_Addr);   //the buffer address
//     write_reg32(DES3, 0x00000000);

//     /* send CMD24 */
//     sdmmc_cmdinitstructure.Argument         = dma->DstAddr;
//     sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_WRITE_SINGLE_BLOCK;
//     sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R1;
//     sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
//         SDMMC_CMD_USE_HOLD_REG | \
//         SDMMC_CMD_PRV_DAT_WAIT | \
//         SDMMC_CMD_DAT_READ_WRITE | \
//         SDMMC_CMD_DAT_EXP      | \
//         SDMMC_CMD_RESP_CRC     | \
//         SDMMC_CMD_RESP_EXP;
//     Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
//     Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);

//     /* Check for error conditions */
//     Core_SDMMC_WaiteCmdDone(hsd->Instance);
//     Core_SDMMC_WaiteDataOver(hsd->Instance);
//     Core_SDMMC_WaiteCardBusy(hsd->Instance);

//     /* reset Next descriptor */
// #ifdef ECHO
//     serial_puts("NTPT = ");
//     print_str(NextDesp);
//     serial_putc('\n');
// #endif
//     write_reg32(&(hsd->Instance->DBADDR), NextDesp);
//     dma->DstAddr += 1;
//   }

//   /* Update the SD transfer error in SD handle */
//   hsd->SdTransferErr = errorstate;
//   return errorstate;
// }

SD_ErrorTypedef Card_SD_WriteBlocks_DMA(SD_HandleTypeDef *hsd, SDMMC_DMATransTypeDef *dma)
{
  SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
  SD_ErrorTypedef errorstate = SD_OK;
  typedef uint32_t *uint32p;
  uint32_t BlockIndex, NextDesp, SrcAddr, TotalBlocks;
  uint32p DES0, DES1, DES2, DES3;
  uint32_t get_val, cmd_done, data_over, card_busy;
  uint32_t RestBytes = dma->NumberOfBytes;
  /* Initialize handle flags */
  hsd->SdTransferCplt  = 0;
  hsd->SdTransferErr   = SD_OK;
  /* Initialize SD Read operation */
  if (dma->NumberOfBytes > dma->BlockSize)
  {
    hsd->SdOperation = SD_WRITE_MULTIPLE_BLOCK;
  }
  else
  {
    hsd->SdOperation = SD_WRITE_SINGLE_BLOCK;
  }

  if (hsd->CardType == HIGH_CAPACITY_SD_CARD)
  {
    dma->BlockSize = 512;
    dma->DstAddr /= 512;
  }
  /* Configure the SD DPSM (Data Path State Machine) */
  errorstate = SD_DMAConfig(hsd, dma);
  if (errorstate != SD_OK) {
    serial_puts("SD_DMAConfig Fail\n");
    return errorstate;
  }
  Core_SDMMC_SetBYCTNT(hsd->Instance, dma->BlockSize);

  TotalBlocks =  dma->NumberOfBytes / dma->BlockSize;
  for (BlockIndex = 0; BlockIndex <= TotalBlocks; BlockIndex++)
  {

#ifdef ECHO
    serial_puts("dma->dstaddr = ");
    print_str(dma->DstAddr);
    serial_putc('\n');
#endif

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

    if (RestBytes <= dma->BlockSize)
    {
      Core_SDMMC_SetBYCTNT(hsd->Instance, RestBytes);
    }

    /* send CMD24 */
    if (hsd->CardType == HIGH_CAPACITY_SD_CARD)
    {
      sdmmc_cmdinitstructure.Argument       = dma->DstAddr + BlockIndex;
    }
    else
    {
      sdmmc_cmdinitstructure.Argument       = dma->DstAddr + dma->BlockSize * BlockIndex;
    }

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
    //Core_SDMMC_WaiteDataOver(hsd->Instance);
    Core_SDMMC_WaiteCardBusy(hsd->Instance);

    /* reset Next descriptor */
// #ifdef ECHO
//     serial_puts("NTPT = ");
//     print_str(NextDesp);
//     serial_putc('\n');
// #endif

    write_reg32(&(hsd->Instance->DBADDR), NextDesp);
    RestBytes -= dma->BlockSize;
  }

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
  if ((hsd->CardType == STD_CAPACITY_SD_CARD_V1_1) || (hsd->CardType == STD_CAPACITY_SD_CARD_V2_0) || \
      (hsd->CardType == HIGH_CAPACITY_SD_CARD))
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

    /* Check for error conditions */
    errorstate = SD_CmdResp1Error(hsd, SD_CMD_SD_ERASE_WR_BLK_START);
    if (errorstate != SD_OK)
    {
      return errorstate;
    }

    /* Send CMD33 SD_ERASE_GRP_END with argument as addr  */
    sdmmc_cmdinitstructure.Argument         = (uint32_t)(startaddr + blocknum);
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

    /* Check for error conditions */
    errorstate = SD_CmdResp1Error(hsd, SD_CMD_SD_ERASE_WR_BLK_END);
    if (errorstate != SD_OK)
    {
      return errorstate;
    }
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

  /* Check for error conditions */
  errorstate = SD_CmdResp1Error(hsd, SD_CMD_ERASE);
  if (errorstate != SD_OK)
  {
    return errorstate;
  }


  /* Wait until the card is in programming state */
  errorstate = SD_IsCardProgramming(hsd, &cardstate);

  while ((errorstate == SD_OK) && ((cardstate == SD_CARD_PROGRAMMING) || (cardstate == SD_CARD_RECEIVE)))
  {
    errorstate = SD_IsCardProgramming(hsd, &cardstate);
#ifdef ECHO
    serial_puts("\nsd is in programming");
#endif
  }
  return errorstate;
}

/**
  * @brief  This function handles SD card interrupt request.
  * @param  hsd: SD handle
  * @retval None
  */
// void Card_SD_IRQHandler(SD_HandleTypeDef *hsd)
// {
//  /* Check for SDMMC interrupt flags */
//  if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_IT_DATAEND))
//  {
//    __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_IT_DATAEND);

//    /* SD transfer is complete */
//    hsd->SdTransferCplt = 1;

//    /* No transfer error */
//    hsd->SdTransferErr  = SD_OK;

//    HAL_SD_XferCpltCallback(hsd);
//  }
//  else if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_IT_DCRCFAIL))
//  {
//    __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_FLAG_DCRCFAIL);

//    hsd->SdTransferErr = SD_DATA_CRC_FAIL;

//    HAL_SD_XferErrorCallback(hsd);

//  }
//  else if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_IT_DTIMEOUT))
//  {
//    __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_FLAG_DTIMEOUT);

//    hsd->SdTransferErr = SD_DATA_TIMEOUT;

//    HAL_SD_XferErrorCallback(hsd);
//  }
//  else if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_IT_RXOVERR))
//  {
//    __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_FLAG_RXOVERR);

//    hsd->SdTransferErr = SD_RX_OVERRUN;

//    HAL_SD_XferErrorCallback(hsd);
//  }
//  else if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_IT_TXUNDERR))
//  {
//    __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_FLAG_TXUNDERR);

//    hsd->SdTransferErr = SD_TX_UNDERRUN;

//    HAL_SD_XferErrorCallback(hsd);
//  }
//  else
//  {
//    /* No error flag set */
//  }

//  /* Disable all SDMMC peripheral interrupt sources */
//  __HAL_SD_SDMMC_DISABLE_IT(hsd, SDMMC_IT_DCRCFAIL | SDMMC_IT_DTIMEOUT | SDMMC_IT_DATAEND  | \
//                            SDMMC_IT_TXFIFOHE | SDMMC_IT_RXFIFOHF | SDMMC_IT_TXUNDERR | \
//                            SDMMC_IT_RXOVERR);
// }


/**
  * @brief  SD end of transfer callback.
  * @param  hsd: SD handle
  * @retval None
  */
// __weak void Card_SD_XferCpltCallback(SD_HandleTypeDef *hsd)
// {
//  /* NOTE : This function Should not be modified, when the callback is needed,
//            the HAL_SD_XferCpltCallback could be implemented in the user file
//   */
// }

/**
  * @brief  SD Transfer Error callback.
  * @param  hsd: SD handle
  * @retval None
  */
// __weak void Card_SD_XferErrorCallback(SD_HandleTypeDef *hsd)
// {
//  /* NOTE : This function Should not be modified, when the callback is needed,
//            the HAL_SD_XferErrorCallback could be implemented in the user file
//   */
// }

/**
  * @brief  SD Transfer complete Rx callback in non blocking mode.
  * @param  hdma: pointer to a DMA_HandleTypeDef structure that contains
  *                the configuration information for the specified DMA module.
  * @retval None
  */
// __weak void Card_SD_DMA_RxCpltCallback(DMA_HandleTypeDef *hdma)
// {
//  /* NOTE : This function Should not be modified, when the callback is needed,
//            the HAL_SD_DMA_RxCpltCallback could be implemented in the user file
//   */
// }

/**
  * @brief  SD DMA transfer complete Rx error callback.
  * @param  hdma: pointer to a DMA_HandleTypeDef structure that contains
  *                the configuration information for the specified DMA module.
  * @retval None
  */
// __weak void Card_SD_DMA_RxErrorCallback(DMA_HandleTypeDef *hdma)
// {
//  /* NOTE : This function Should not be modified, when the callback is needed,
//            the HAL_SD_DMA_RxErrorCallback could be implemented in the user file
//   */
// }

/**
  * @brief  SD Transfer complete Tx callback in non blocking mode.
  * @param  hdma: pointer to a DMA_HandleTypeDef structure that contains
  *                the configuration information for the specified DMA module.
  * @retval None
  */
// __weak void Card_SD_DMA_TxCpltCallback(DMA_HandleTypeDef *hdma)
// {
//  /* NOTE : This function Should not be modified, when the callback is needed,
//            the HAL_SD_DMA_TxCpltCallback could be implemented in the user file
//   */
// }

/**
  * @brief  SD DMA transfer complete error Tx callback.
  * @param  hdma: pointer to a DMA_HandleTypeDef structure that contains
  *                the configuration information for the specified DMA module.
  * @retval None
  */
// __weak void Card_SD_DMA_TxErrorCallback(DMA_HandleTypeDef *hdma)
// {
//  /* NOTE : This function Should not be modified, when the callback is needed,
//            the HAL_SD_DMA_TxErrorCallback could be implemented in the user file
//   */
// }

/**
  * @}
  */

/** @addtogroup SD_Exported_Functions_Group3
 *  @brief   management functions
 *
@verbatim
  ==============================================================================
                      ##### Peripheral Control functions #####
  ==============================================================================
  [..]
    This subsection provides a set of functions allowing to control the SD card
    operations.

@endverbatim
  * @{
  */

/** TODO
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
  tmp = (hsd->CSD[0] & 0xFF000000) >> 24;
  pCardInfo->SD_csd.CSDStruct      = (uint8_t)((tmp & 0xC0) >> 6);
  pCardInfo->SD_csd.SysSpecVersion = (uint8_t)((tmp & 0x3C) >> 2);
  pCardInfo->SD_csd.Reserved1      = tmp & 0x03;
  /* Byte 1 */
  tmp = (hsd->CSD[0] & 0x00FF0000) >> 16;
  pCardInfo->SD_csd.TAAC = (uint8_t)tmp;
  /* Byte 2 */
  tmp = (hsd->CSD[0] & 0x0000FF00) >> 8;
  pCardInfo->SD_csd.NSAC = (uint8_t)tmp;
  /* Byte 3 */
  hsd->CardType = HIGH_CAPACITY_SD_CARD;
  tmp = hsd->CSD[0] & 0x000000FF;
  pCardInfo->SD_csd.MaxDataTransRate = (uint8_t)tmp;
  /* Byte 4 */
  tmp = (hsd->CSD[1] & 0xFF000000) >> 24;
  pCardInfo->SD_csd.CardComdClasses = (uint16_t)(tmp << 4);

  /* Byte 5 */
  tmp = (hsd->CSD[1] & 0x00FF0000) >> 16;
  pCardInfo->SD_csd.CardComdClasses |= (uint16_t)((tmp & 0xF0) >> 4);
  pCardInfo->SD_csd.RdBlockLen       = (uint8_t)(tmp & 0x0F);

  /* Byte 6 */
  tmp = (hsd->CSD[1] & 0x0000FF00) >> 8;
  pCardInfo->SD_csd.PartBlockRead   = (uint8_t)((tmp & 0x80) >> 7);
  pCardInfo->SD_csd.WrBlockMisalign = (uint8_t)((tmp & 0x40) >> 6);
  pCardInfo->SD_csd.RdBlockMisalign = (uint8_t)((tmp & 0x20) >> 5);
  pCardInfo->SD_csd.DSRImpl         = (uint8_t)((tmp & 0x10) >> 4);
  pCardInfo->SD_csd.Reserved2       = 0; /*!< Reserved */

  if ((hsd->CardType == STD_CAPACITY_SD_CARD_V1_1) || (hsd->CardType == STD_CAPACITY_SD_CARD_V2_0))
  {
    pCardInfo->SD_csd.DeviceSize = (tmp & 0x03) << 10;

    /* Byte 7 */
    tmp = (uint8_t)(hsd->CSD[1] & 0x000000FF);
    pCardInfo->SD_csd.DeviceSize |= (tmp) << 2;

    /* Byte 8 */
    tmp = (uint8_t)((hsd->CSD[2] & 0xFF000000) >> 24);
    pCardInfo->SD_csd.DeviceSize |= (tmp & 0xC0) >> 6;

    pCardInfo->SD_csd.MaxRdCurrentVDDMin = (tmp & 0x38) >> 3;
    pCardInfo->SD_csd.MaxRdCurrentVDDMax = (tmp & 0x07);

    /* Byte 9 */
    tmp = (uint8_t)((hsd->CSD[2] & 0x00FF0000) >> 16);
    pCardInfo->SD_csd.MaxWrCurrentVDDMin = (tmp & 0xE0) >> 5;
    pCardInfo->SD_csd.MaxWrCurrentVDDMax = (tmp & 0x1C) >> 2;
    pCardInfo->SD_csd.DeviceSizeMul      = (tmp & 0x03) << 1;
    /* Byte 10 */
    tmp = (uint8_t)((hsd->CSD[2] & 0x0000FF00) >> 8);
    pCardInfo->SD_csd.DeviceSizeMul |= (tmp & 0x80) >> 7;

    pCardInfo->CardCapacity  = (pCardInfo->SD_csd.DeviceSize + 1) ;
    pCardInfo->CardCapacity *= (1 << (pCardInfo->SD_csd.DeviceSizeMul + 2));
    pCardInfo->CardBlockSize = 1 << (pCardInfo->SD_csd.RdBlockLen);
    pCardInfo->CardCapacity *= pCardInfo->CardBlockSize;
  }
  else if (hsd->CardType == HIGH_CAPACITY_SD_CARD)
  {
    /* Byte 7 */
    tmp = (uint8_t)(hsd->CSD[1] & 0x000000FF);
    pCardInfo->SD_csd.DeviceSize = (tmp & 0x3F) << 16;

    /* Byte 8 */
    tmp = (uint8_t)((hsd->CSD[2] & 0xFF000000) >> 24);

    pCardInfo->SD_csd.DeviceSize |= (tmp << 8);

    /* Byte 9 */
    tmp = (uint8_t)((hsd->CSD[2] & 0x00FF0000) >> 16);

    pCardInfo->SD_csd.DeviceSize |= (tmp);

    /* Byte 10 */
    tmp = (uint8_t)((hsd->CSD[2] & 0x0000FF00) >> 8);

    pCardInfo->CardCapacity  = ((pCardInfo->SD_csd.DeviceSize + 1)) * 512 * 1024;
    pCardInfo->CardBlockSize = 512;
  }
  else
  {
    /* Not supported card type */
    errorstate = SD_ERROR;
  }

  pCardInfo->SD_csd.EraseGrSize = (tmp & 0x40) >> 6;
  pCardInfo->SD_csd.EraseGrMul  = (tmp & 0x3F) << 1;

  /* Byte 11 */
  tmp = (uint8_t)(hsd->CSD[2] & 0x000000FF);
  pCardInfo->SD_csd.EraseGrMul     |= (tmp & 0x80) >> 7;
  pCardInfo->SD_csd.WrProtectGrSize = (tmp & 0x7F);

  /* Byte 12 */
  tmp = (uint8_t)((hsd->CSD[3] & 0xFF000000) >> 24);
  pCardInfo->SD_csd.WrProtectGrEnable = (tmp & 0x80) >> 7;
  pCardInfo->SD_csd.ManDeflECC        = (tmp & 0x60) >> 5;
  pCardInfo->SD_csd.WrSpeedFact       = (tmp & 0x1C) >> 2;
  pCardInfo->SD_csd.MaxWrBlockLen     = (tmp & 0x03) << 2;

  /* Byte 13 */
  tmp = (uint8_t)((hsd->CSD[3] & 0x00FF0000) >> 16);
  pCardInfo->SD_csd.MaxWrBlockLen      |= (tmp & 0xC0) >> 6;
  pCardInfo->SD_csd.WriteBlockPaPartial = (tmp & 0x20) >> 5;
  pCardInfo->SD_csd.Reserved3           = 0;
  pCardInfo->SD_csd.ContentProtectAppli = (tmp & 0x01);

  /* Byte 14 */
  tmp = (uint8_t)((hsd->CSD[3] & 0x0000FF00) >> 8);
  pCardInfo->SD_csd.FileFormatGrouop = (tmp & 0x80) >> 7;
  pCardInfo->SD_csd.CopyFlag         = (tmp & 0x40) >> 6;
  pCardInfo->SD_csd.PermWrProtect    = (tmp & 0x20) >> 5;
  pCardInfo->SD_csd.TempWrProtect    = (tmp & 0x10) >> 4;
  pCardInfo->SD_csd.FileFormat       = (tmp & 0x0C) >> 2;
  pCardInfo->SD_csd.ECC              = (tmp & 0x03);

  /* Byte 15 */
  tmp = (uint8_t)(hsd->CSD[3] & 0x000000FF);
  pCardInfo->SD_csd.CSD_CRC   = (tmp & 0xFE) >> 1;
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
  * @brief  Enables wide bus operation for the requested card if supported by
  *         card.
  * @param  hsd: SD handle
  * @param  WideMode: Specifies the SD card wide bus mode
  *          This parameter can be one of the following values:
  *            @arg SDMMC_BUS_WIDE_8B: 8-bit data transfer (Only for MMC)
  *            @arg SDMMC_BUS_WIDE_4B: 4-bit data transfer
  *            @arg SDMMC_BUS_WIDE_1B: 1-bit data transfer
  * @retval SD Card error state
  */
// SD_ErrorTypedef Card_SD_WideBusOperation_Config(SD_HandleTypeDef *hsd, uint32_t WideMode)
// {
//  SD_ErrorTypedef errorstate = SD_OK;
//  SDMMC_InitTypeDef tmpinit;

//  /* MMC Card does not support this feature */
//  if (hsd->CardType == MULTIMEDIA_CARD)
//  {
//    errorstate = SD_UNSUPPORTED_FEATURE;

//    return errorstate;
//  }
//  else if ((hsd->CardType == STD_CAPACITY_SD_CARD_V1_1) || (hsd->CardType == STD_CAPACITY_SD_CARD_V2_0) || \
//           (hsd->CardType == HIGH_CAPACITY_SD_CARD))
//  {
//    if (WideMode == SDMMC_BUS_WIDE_8B)
//    {
//      errorstate = SD_UNSUPPORTED_FEATURE;
//    }
//    else if (WideMode == SDMMC_BUS_WIDE_4B)
//    {
//      errorstate = SD_WideBus_Enable(hsd);
//    }
//    else if (WideMode == SDMMC_BUS_WIDE_1B)
//    {
//      errorstate = SD_WideBus_Disable(hsd);
//    }
//    else
//    {
//      /* WideMode is not a valid argument*/
//      errorstate = SD_INVALID_PARAMETER;
//    }

//    if (errorstate == SD_OK)
//    {
//      /* Configure the SDMMC peripheral */
//      tmpinit.ClockEdge           = hsd->Init.ClockEdge;
//      tmpinit.ClockBypass         = hsd->Init.ClockBypass;
//      tmpinit.ClockPowerSave      = hsd->Init.ClockPowerSave;
//      tmpinit.BusWide             = WideMode;
//      tmpinit.HardwareFlowControl = hsd->Init.HardwareFlowControl;
//      tmpinit.ClockDiv            = hsd->Init.ClockDiv;
//      SDMMC_Init(hsd->Instance, tmpinit);
//    }
//  }

//  return errorstate;
// }

// /**
//   * @brief  Aborts an ongoing data transfer.
//   * @param  hsd: SD handle
//   * @retval SD Card error state
//   */
// SD_ErrorTypedef Card_SD_StopTransfer(SD_HandleTypeDef *hsd)
// {
//  SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
//  SD_ErrorTypedef errorstate = SD_OK;

//  /* Send CMD12 STOP_TRANSMISSION  */
//  sdmmc_cmdinitstructure.Argument         = 0;
//  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_STOP_TRANSMISSION;
//  sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_SHORT;
//  sdmmc_cmdinitstructure.WaitForInterrupt = SDMMC_WAIT_NO;
//  sdmmc_cmdinitstructure.CPSM             = SDMMC_CPSM_ENABLE;
//  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);

//  /* Check for error conditions */
//  errorstate = SD_CmdResp1Error(hsd, SD_CMD_STOP_TRANSMISSION);

//  return errorstate;
// }

// /**
//   * @brief  Switches the SD card to High Speed mode.
//   *         This API must be used after "Transfer State"
//   * @note   This operation should be followed by the configuration
//   *         of PLL to have SDMMCCK clock between 67 and 75 MHz
//   * @param  hsd: SD handle
//   * @retval SD Card error state
//   */
// SD_ErrorTypedef Card_SD_HighSpeed (SD_HandleTypeDef *hsd)
// {
//  SD_ErrorTypedef errorstate = SD_OK;
//  SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
//  SDMMC_DataInitTypeDef sdmmc_datainitstructure;

//  uint8_t SD_hs[64]  = {0};
//  uint32_t SD_scr[2] = {0, 0};
//  uint32_t SD_SPEC   = 0 ;
//  uint32_t count = 0, *tempbuff = (uint32_t *)SD_hs;

//  /* Initialize the Data control register */
//  hsd->Instance->DCTRL = 0;

//  /* Get SCR Register */
//  errorstate = SD_FindSCR(hsd, SD_scr);

//  if (errorstate != SD_OK)
//  {
//    return errorstate;
//  }

//  /* Test the Version supported by the card*/
//  SD_SPEC = (SD_scr[1]  & 0x01000000) | (SD_scr[1]  & 0x02000000);

//  if (SD_SPEC != SD_ALLZERO)
//  {
//    /* Set Block Size for Card */
//    sdmmc_cmdinitstructure.Argument         = (uint32_t)64;
//    sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SET_BLOCKLEN;
//    sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_SHORT;
//    sdmmc_cmdinitstructure.WaitForInterrupt = SDMMC_WAIT_NO;
//    sdmmc_cmdinitstructure.CPSM             = SDMMC_CPSM_ENABLE;
//    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);

//    /* Check for error conditions */
//    errorstate = SD_CmdResp1Error(hsd, SD_CMD_SET_BLOCKLEN);

//    if (errorstate != SD_OK)
//    {
//      return errorstate;
//    }

//    /* Configure the SD DPSM (Data Path State Machine) */
//    sdmmc_datainitstructure.DataTimeOut   = SD_DATATIMEOUT;
//    sdmmc_datainitstructure.DataLength    = 64;
//    sdmmc_datainitstructure.DataBlockSize = SDMMC_DATABLOCK_SIZE_64B ;
//    sdmmc_datainitstructure.TransferDir   = SDMMC_TRANSFER_DIR_TO_SDMMC;
//    sdmmc_datainitstructure.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
//    sdmmc_datainitstructure.DPSM          = SDMMC_DPSM_ENABLE;
//    SDMMC_DataConfig(hsd->Instance, &sdmmc_datainitstructure);

//    /* Send CMD6 switch mode */
//    sdmmc_cmdinitstructure.Argument         = 0x80FFFF01;
//    sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_HS_SWITCH;
//    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);

//    /* Check for error conditions */
//    errorstate = SD_CmdResp1Error(hsd, SD_CMD_HS_SWITCH);

//    if (errorstate != SD_OK)
//    {
//      return errorstate;
//    }

//    while (!__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_RXOVERR | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_DBCKEND))
//    {
//      if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_RXFIFOHF))
//      {
//        for (count = 0; count < 8; count++)
//        {
//          *(tempbuff + count) = SDMMC_ReadFIFO(hsd->Instance);
//        }

//        tempbuff += 8;
//      }
//    }

//    if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_DTIMEOUT))
//    {
//      __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_FLAG_DTIMEOUT);

//      errorstate = SD_DATA_TIMEOUT;

//      return errorstate;
//    }
//    else if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_DCRCFAIL))
//    {
//      __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_FLAG_DCRCFAIL);

//      errorstate = SD_DATA_CRC_FAIL;

//      return errorstate;
//    }
//    else if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_RXOVERR))
//    {
//      __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_FLAG_RXOVERR);

//      errorstate = SD_RX_OVERRUN;

//      return errorstate;
//    }
//    else
//    {
//      /* No error flag set */
//    }

//    count = SD_DATATIMEOUT;

//    while ((__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_RXDAVL)) && (count > 0))
//    {
//      *tempbuff = SDMMC_ReadFIFO(hsd->Instance);
//      tempbuff++;
//      count--;
//    }

//    /* Clear all the static flags */
//    __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);

//    /* Test if the switch mode HS is ok */
//    if ((SD_hs[13] & 2) != 2)
//    {
//      errorstate = SD_UNSUPPORTED_FEATURE;
//    }
//  }

//  return errorstate;
// }

/**
  * @}
  */

/** @addtogroup SD_Exported_Functions_Group4
 *  @brief   Peripheral State functions
 *
@verbatim
  ==============================================================================
                      ##### Peripheral State functions #####
  ==============================================================================
  [..]
    This subsection permits to get in runtime the status of the peripheral
    and the data flow.

@endverbatim
  * @{
  */

/**
  * @brief  Returns the current SD card's status.
  * @param  hsd: SD handle
  * @param  pSDstatus: Pointer to the buffer that will contain the SD card status
  *         SD Status register)
  * @retval SD Card error state
  */
// SD_ErrorTypedef Card_SD_SendSDStatus(SD_HandleTypeDef *hsd, uint32_t *pSDstatus)
// {
//  SDMMC_CmdInitTypeDef  sdmmc_cmdinitstructure;
//  SDMMC_DataInitTypeDef sdmmc_datainitstructure;
//  SD_ErrorTypedef errorstate = SD_OK;
//  uint32_t count = 0;

//  /* Check SD response */
//  if ((SDMMC_GetResponse(hsd->Instance, SDMMC_RESP1) & SD_CARD_LOCKED) == SD_CARD_LOCKED)
//  {
//    errorstate = SD_LOCK_UNLOCK_FAILED;

//    return errorstate;
//  }

//  /* Set block size for card if it is not equal to current block size for card */
//  sdmmc_cmdinitstructure.Argument         = 64;
//  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SET_BLOCKLEN;
//  sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_SHORT;
//  sdmmc_cmdinitstructure.WaitForInterrupt = SDMMC_WAIT_NO;
//  sdmmc_cmdinitstructure.CPSM             = SDMMC_CPSM_ENABLE;
//  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);

//  /* Check for error conditions */
//  errorstate = SD_CmdResp1Error(hsd, SD_CMD_SET_BLOCKLEN);

//  if (errorstate != SD_OK)
//  {
//    return errorstate;
//  }

//  /* Send CMD55 */
//  sdmmc_cmdinitstructure.Argument         = (uint32_t)(hsd->RCA << 16);
//  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_APP_CMD;
//  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);

//  /* Check for error conditions */
//  errorstate = SD_CmdResp1Error(hsd, SD_CMD_APP_CMD);

//  if (errorstate != SD_OK)
//  {
//    return errorstate;
//  }

//  /* Configure the SD DPSM (Data Path State Machine) */
//  sdmmc_datainitstructure.DataTimeOut   = SD_DATATIMEOUT;
//  sdmmc_datainitstructure.DataLength    = 64;
//  sdmmc_datainitstructure.DataBlockSize = SDMMC_DATABLOCK_SIZE_64B;
//  sdmmc_datainitstructure.TransferDir   = SDMMC_TRANSFER_DIR_TO_SDMMC;
//  sdmmc_datainitstructure.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
//  sdmmc_datainitstructure.DPSM          = SDMMC_DPSM_ENABLE;
//  SDMMC_DataConfig(hsd->Instance, &sdmmc_datainitstructure);

//  /* Send ACMD13 (SD_APP_STAUS)  with argument as card's RCA */
//  sdmmc_cmdinitstructure.Argument         = 0;
//  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SD_APP_STATUS;
//  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);

//  /* Check for error conditions */
//  errorstate = SD_CmdResp1Error(hsd, SD_CMD_SD_APP_STATUS);

//  if (errorstate != SD_OK)
//  {
//    return errorstate;
//  }

//  /* Get status data */
//  while (!__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_RXOVERR | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_DBCKEND))
//  {
//    if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_RXFIFOHF))
//    {
//      for (count = 0; count < 8; count++)
//      {
//        *(pSDstatus + count) = SDMMC_ReadFIFO(hsd->Instance);
//      }

//      pSDstatus += 8;
//    }
//  }

//  if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_DTIMEOUT))
//  {
//    __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_FLAG_DTIMEOUT);

//    errorstate = SD_DATA_TIMEOUT;

//    return errorstate;
//  }
//  else if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_DCRCFAIL))
//  {
//    __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_FLAG_DCRCFAIL);

//    errorstate = SD_DATA_CRC_FAIL;

//    return errorstate;
//  }
//  else if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_RXOVERR))
//  {
//    __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_FLAG_RXOVERR);

//    errorstate = SD_RX_OVERRUN;

//    return errorstate;
//  }
//  else
//  {
//    /* No error flag set */
//  }

//  count = SD_DATATIMEOUT;
//  while ((__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_RXDAVL)) && (count > 0))
//  {
//    *pSDstatus = SDMMC_ReadFIFO(hsd->Instance);
//    pSDstatus++;
//    count--;
//  }

//  /* Clear all the static status flags*/
//  __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);

//  return errorstate;
// }

/**
  * @brief  Gets the current sd card data status.
  * @param  hsd: SD handle
  * @retval Data Transfer state
  */
// HAL_SD_TransferStateTypedef Card_SD_GetStatus(SD_HandleTypeDef *hsd)
// {
//  SD_CardStateTypedef cardstate =  SD_CARD_TRANSFER;

//  /* Get SD card state */
//  cardstate = SD_GetState(hsd);

//  /* Find SD status according to card state*/
//  if (cardstate == SD_CARD_TRANSFER)
//  {
//    return SD_TRANSFER_OK;
//  }
//  else if (cardstate == SD_CARD_ERROR)
//  {
//    return SD_TRANSFER_ERROR;
//  }
//  else
//  {
//    return SD_TRANSFER_BUSY;
//  }
// }

// /**
//   * @brief  Gets the SD card status.
//   * @param  hsd: SD handle
//   * @param  pCardStatus: Pointer to the HAL_SD_CardStatusTypedef structure that
//   *         will contain the SD card status information
//   * @retval SD Card error state
//   */
// SD_ErrorTypedef Card_SD_GetCardStatus(SD_HandleTypeDef *hsd, HAL_SD_CardStatusTypedef *pCardStatus)
// {
//  SD_ErrorTypedef errorstate = SD_OK;
//  uint32_t tmp = 0;
//  uint32_t sd_status[16];

//  errorstate = HAL_SD_SendSDStatus(hsd, sd_status);

//  if (errorstate  != SD_OK)
//  {
//    return errorstate;
//  }

//  /* Byte 0 */
//  tmp = (sd_status[0] & 0xC0) >> 6;
//  pCardStatus->DAT_BUS_WIDTH = (uint8_t)tmp;

//  /* Byte 0 */
//  tmp = (sd_status[0] & 0x20) >> 5;
//  pCardStatus->SECURED_MODE = (uint8_t)tmp;

//  /* Byte 2 */
//  tmp = (sd_status[2] & 0xFF);
//  pCardStatus->SD_CARD_TYPE = (uint8_t)(tmp << 8);

//  /* Byte 3 */
//  tmp = (sd_status[3] & 0xFF);
//  pCardStatus->SD_CARD_TYPE |= (uint8_t)tmp;

//  /* Byte 4 */
//  tmp = (sd_status[4] & 0xFF);
//  pCardStatus->SIZE_OF_PROTECTED_AREA = (uint8_t)(tmp << 24);

//  /* Byte 5 */
//  tmp = (sd_status[5] & 0xFF);
//  pCardStatus->SIZE_OF_PROTECTED_AREA |= (uint8_t)(tmp << 16);

//  /* Byte 6 */
//  tmp = (sd_status[6] & 0xFF);
//  pCardStatus->SIZE_OF_PROTECTED_AREA |= (uint8_t)(tmp << 8);

//  /* Byte 7 */
//  tmp = (sd_status[7] & 0xFF);
//  pCardStatus->SIZE_OF_PROTECTED_AREA |= (uint8_t)tmp;

//  /* Byte 8 */
//  tmp = (sd_status[8] & 0xFF);
//  pCardStatus->SPEED_CLASS = (uint8_t)tmp;

//  /* Byte 9 */
//  tmp = (sd_status[9] & 0xFF);
//  pCardStatus->PERFORMANCE_MOVE = (uint8_t)tmp;

//  /* Byte 10 */
//  tmp = (sd_status[10] & 0xF0) >> 4;
//  pCardStatus->AU_SIZE = (uint8_t)tmp;

//  /* Byte 11 */
//  tmp = (sd_status[11] & 0xFF);
//  pCardStatus->ERASE_SIZE = (uint8_t)(tmp << 8);

//  /* Byte 12 */
//  tmp = (sd_status[12] & 0xFF);
//  pCardStatus->ERASE_SIZE |= (uint8_t)tmp;

//  /* Byte 13 */
//  tmp = (sd_status[13] & 0xFC) >> 2;
//  pCardStatus->ERASE_TIMEOUT = (uint8_t)tmp;

//  /* Byte 13 */
//  tmp = (sd_status[13] & 0x3);
//  pCardStatus->ERASE_OFFSET = (uint8_t)tmp;

//  return errorstate;
// }


/* Private function ----------------------------------------------------------*/
/** @addtogroup SD_Private_Functions
  * @{
  */

/**
  * @brief  SD DMA transfer complete Rx callback.
  * @param  hdma: pointer to a DMA_HandleTypeDef structure that contains
  *                the configuration information for the specified DMA module.
  * @retval None
  */
// static void SD_DMA_RxCplt(DMA_HandleTypeDef *hdma)
// {
//  SD_HandleTypeDef *hsd = (SD_HandleTypeDef*)((DMA_HandleTypeDef*)hdma)->Parent;

//  /* DMA transfer is complete */
//  hsd->DmaTransferCplt = 1;

//  /* Wait until SD transfer is complete */
//  while (hsd->SdTransferCplt == 0)
//  {
//  }

//  /* Disable the DMA channel */
//  HAL_DMA_Abort(hdma);

//  /* Transfer complete user callback */
//  //HAL_SD_DMA_RxCpltCallback(hsd->hdmarx);
// }

/**
  * @brief  SD DMA transfer Error Rx callback.
  * @param  hdma: pointer to a DMA_HandleTypeDef structure that contains
  *                the configuration information for the specified DMA module.
  * @retval None
  */
// static void SD_DMA_RxError(DMA_HandleTypeDef *hdma)
// {
//  SD_HandleTypeDef *hsd = (SD_HandleTypeDef*)((DMA_HandleTypeDef*)hdma)->Parent;

//  /* Transfer complete user callback */
//  //HAL_SD_DMA_RxErrorCallback(hsd->hdmarx);
// }

/**
  * @brief  SD DMA transfer complete Tx callback.
  * @param  hdma: pointer to a DMA_HandleTypeDef structure that contains
  *                the configuration information for the specified DMA module.
  * @retval None
  */
// static void SD_DMA_TxCplt(DMA_HandleTypeDef *hdma)
// {
//  SD_HandleTypeDef *hsd = (SD_HandleTypeDef*)((DMA_HandleTypeDef*)hdma)->Parent;

//  /* DMA transfer is complete */
//  hsd->DmaTransferCplt = 1;

//  /* Wait until SD transfer is complete */
//  while (hsd->SdTransferCplt == 0)
//  {
//  }

//  /* Disable the DMA channel */
//  HAL_DMA_Abort(hdma);

//  /* Transfer complete user callback */
//  //HAL_SD_DMA_TxCpltCallback(hsd->hdmatx);
// }

/**
  * @brief  SD DMA transfer Error Tx callback.
  * @param  hdma: pointer to a DMA_HandleTypeDef structure that contains
  *                the configuration information for the specified DMA module.
  * @retval None
  */
// static void SD_DMA_TxError(DMA_HandleTypeDef *hdma)
// {
//  SD_HandleTypeDef *hsd = ( SD_HandleTypeDef* )((DMA_HandleTypeDef* )hdma)->Parent;

//  /* Transfer complete user callback */
//  //HAL_SD_DMA_TxErrorCallback(hsd->hdmatx);
// }

/**
  * @brief  Returns the SD current state.
  * @param  hsd: SD handle
  * @retval SD card current state
  */
// static SD_CardStateTypedef SD_GetState(SD_HandleTypeDef *hsd)
// {
//  uint32_t resp1 = 0;

//  if (SD_SendStatus(hsd, &resp1) != SD_OK)
//  {
//    return SD_CARD_ERROR;
//  }
//  else
//  {
//    return (SD_CardStateTypedef)((resp1 >> 9) & 0x0F);
//  }
// }

/**
  * @brief  Initializes all cards or single card as the case may be Card(s) come
  *         into standby state.
  * @param  hsd: SD handle
  * @retval SD Card error state
  */
static SD_ErrorTypedef SD_Initialize_Cards(SD_HandleTypeDef *hsd)
{
  SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
  SD_ErrorTypedef errorstate = SD_OK;
  uint16_t sd_rca = 1;
  uint32_t cmd_done, response, get_val;
  if (hsd->CardType != SECURE_DIGITAL_IO_CARD)
  {
#ifdef ECHO
    serial_puts("Init: send CMD2\n\n");
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
  }

  if ((hsd->CardType == STD_CAPACITY_SD_CARD_V1_1)    || (hsd->CardType == STD_CAPACITY_SD_CARD_V2_0) || \
      (hsd->CardType == SECURE_DIGITAL_IO_COMBO_CARD) || (hsd->CardType == HIGH_CAPACITY_SD_CARD))
  {
    /* Send CMD3 SET_REL_ADDR with argument 0 */
    /* SD Card publishes its RCA. */
#ifdef ECHO
    serial_puts("Init: send CMD3\n\n");
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
    sd_rca = (uint16_t)(response >> 16);
#ifdef ECHO
    serial_puts("RCA = ");
    print_str(sd_rca);
    serial_putc('\n');
#endif
  }

  if (hsd->CardType != SECURE_DIGITAL_IO_CARD)
  {
    /* Get the SD card RCA */
    hsd->RCA = sd_rca;

    /* Send CMD9 SEND_CSD with argument as card's RCA */
#ifdef ECHO
    serial_puts("Init: send CMD9\n\n");
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
    hsd->CSD[0] = Core_SDMMC_GetResponse(hsd->Instance, SDMMC_RESP0);
    hsd->CSD[1] = Core_SDMMC_GetResponse(hsd->Instance, SDMMC_RESP1);
    hsd->CSD[2] = Core_SDMMC_GetResponse(hsd->Instance, SDMMC_RESP2);
    hsd->CSD[3] = Core_SDMMC_GetResponse(hsd->Instance, SDMMC_RESP3);
  }

  /* All cards are initialized */
  return errorstate;
}

/**
  * @brief  Selects or Deselects the corresponding card.
  * @param  hsd: SD handle
  * @param  addr: Address of the card to be selected
  * @retval SD Card error state
  */
static SD_ErrorTypedef SD_Select_Deselect(SD_HandleTypeDef *hsd, uint32_t addr)
{
  SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
  SD_ErrorTypedef errorstate = SD_OK;
  uint32_t cmd_done, get_val;
#ifdef ECHO
  serial_puts("send CMD7\n");
#endif
  /* Send CMD7 SDMMC_SEL_DESEL_CARD */
  sdmmc_cmdinitstructure.Argument         = (uint32_t)addr;
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
// #ifdef ECHO
//   serial_puts("CMD7 RESP0 = : ");
//   print_str(get_val);
//   serial_putc('\n');
//   serial_puts("CMD7 RINTSTS = : ");
//   print_str(get_val);
// #endif
  get_val = Core_SDMMC_GetRINTSTS(hsd->Instance);

  /* Send CMD55*/
#ifdef ECHO
  serial_puts("send CMD55\n");
#endif
  sdmmc_cmdinitstructure.Argument         = (uint32_t)addr;
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
// #ifdef ECHO
//   serial_puts("CMD55 RESP0 = : ");
//   print_str(get_val);
//   serial_putc('\n');
//   serial_puts("CMD55 RINTSTS = : ");
//   print_str(get_val);
// #endif
  get_val = Core_SDMMC_GetRINTSTS(hsd->Instance);

  /* Send ACMD6 */
#ifdef ECHO
  serial_puts("send ACMD6\n");
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
// #ifdef ECHO
//   serial_puts("ACMD6 RESP0 = : ");
//   print_str(get_val);
//   serial_putc('\n');
//   serial_puts("ACMD6 RINTSTS = : ");
//   print_str(get_val);
// #endif
  get_val = Core_SDMMC_GetRINTSTS(hsd->Instance);

  /* Send CMD16*/
#ifdef ECHO
  serial_puts("send CMD16\n");
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
// #ifdef ECHO
//   serial_puts("CMD16 RESP0 = : ");
//   print_str(get_val);
//   serial_putc('\n');
//   serial_puts("CMD16 RINTSTS = : ");
//   print_str(get_val);
// #endif
  get_val = Core_SDMMC_GetRINTSTS(hsd->Instance);

  return errorstate;
}

/**
  * @brief  Enquires cards about their operating voltage and configures clock
  *         controls and stores SD information that will be needed in future
  *         in the SD handle.
  * @param  hsd: SD handle
  * @retval SD Card error state
  */
static SD_ErrorTypedef SD_PowerON(SD_HandleTypeDef *hsd)
{
  SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
  __IO SD_ErrorTypedef errorstate = SD_OK;
  uint32_t get_val = 0, count = 0, validvoltage = 0, vs_busy = 0, loop = 0;
  uint32_t cmd_start, volt_switch_int;
  hsd->CardType = STD_CAPACITY_SD_CARD_V1_1;

  /* Set Power State to ON */
  //Core_SDMMC_PowerState_ON(hsd->Instance);
  Core_SDMMC_SetPWREN(hsd->Instance, SDMMC_PWREN_0 | \
                      SDMMC_PWREN_1 | \
                      SDMMC_PWREN_2 | \
                      SDMMC_PWREN_3);
  /* 1ms: required power up waiting time before starting the SD initialization
     sequence */
  delay_ms(1);
  Core_SDMMC_SetINTMASK(hsd->Instance, 0x0000FFFF);
  /* clear intreq status */
  Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
  Core_SDMMC_SetCTRL(hsd->Instance, SDMMC_CTRL_INT_ENABLE );
  Core_SDMMC_SetCLKDIV(hsd->Instance, SDMMC_CLKDIV_BIT1 | \
                       SDMMC_CLKDIV_BIT3 );
  Core_SDMMC_SetCLKSRC(hsd->Instance, SDMMC_CLKSRC_CLKDIV0);
  Core_SDMMC_SetCLKENA(hsd->Instance, 0x0000FFFF);

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


  /* should set or not? */
  Core_SDMMC_SetFIFOTH(hsd->Instance, 0x00070008);
  Core_SDMMC_SetCTRL(hsd->Instance, SDMMC_CTRL_USE_INTERNAL_IDMAC | \
                     SDMMC_CTRL_INT_ENABLE);
  Core_SDMMC_SetBMOD(hsd->Instance, SDMMC_BMOD_ENABLE | \
                     SDMMC_BMOD_FB);
  /* close the FIFO RX and TX interrupt */
  Core_SDMMC_SetINTMASK(hsd->Instance, (uint32_t)0x0000FFFF & \
                        ~(SDMMC_INTMASK_TXDR | SDMMC_INTMASK_RXDR));
  Core_SDMMC_SetDBADDR(hsd->Instance, 0x21001000);
  Core_SDMMC_SetIDINTEN(hsd->Instance, 0xFFFFFFFF);
  /* Enable SDMMC Clock */
  //__HAL_SD_SDMMC_ENABLE(hsd);
#ifdef  ENUMERATE
  serial_puts("enter the SD_ENUM\n");

  errorstate = SD_ENUM(hsd);
  if (errorstate != SD_OK)
  {
    serial_puts("SD_ENUM state = ");
    print_str(errorstate);
    serial_putc('\n');
    return errorstate;
  }
#endif

  /* CMD0: GO_IDLE_STATE -----------------------------------------------------*/
  /* No CMD get_val required */
#ifdef ECHO
  serial_puts("PowerON: send CMD0\n");
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
  // serial_puts("CMD0 state = ");
  // print_str(errorstate);
  // serial_putc('\n');
  if (errorstate != SD_OK)
  {
    /* CMD Response Timeout (wait for CMDSENT flag) */
    return errorstate;
  }
  /* CMD8: SEND_IF_COND ------------------------------------------------------*/
  /* Send CMD8 to verify SD card interface operating condition */
  /* Argument: - [31:12]: Reserved (shall be set to '0')
  - [11:8]: Supply Voltage (VHS) 0x1 (Range: 2.7-3.6 V)
  - [7:0]: Check Pattern (recommended 0xAA) */
  /* CMD Response: R7 */
#ifdef ECHO
  serial_puts("PowerON: send CMD8\n");
#endif
  sdmmc_cmdinitstructure.Argument         = SD_CHECK_PATTERN;
  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SEND_IF_COND;
  sdmmc_cmdinitstructure.Response     = SDMMC_RESPONSE_R7;
  sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
      SDMMC_CMD_USE_HOLD_REG | \
      SDMMC_CMD_PRV_DAT_WAIT | \
      SDMMC_CMD_RESP_CRC | \
      SDMMC_CMD_RESP_EXP;
  Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);

  /* Check for error conditions */
  // serial_puts("CMD8 state = ");
  // print_str(errorstate);
  // serial_putc('\n');
  errorstate = SD_CmdError(hsd);
  if (errorstate != SD_OK)
  {
    /* CMD Response Timeout (wait for CMDSENT flag) */
    return errorstate;
  }
  errorstate = SD_CmdResp7(hsd);
  // serial_puts("CMD8-1 state = ");
  // print_str(errorstate);
  // serial_putc('\n');
  if (errorstate != SD_OK)
  {
    serial_puts("CMD8: SD_UNSUPPORTED_VOLTAGE\n");
  }

  /* Send CMD55 */
#ifdef ECHO
  serial_puts("PowerON: Send CMD55\n");
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
  // serial_puts("CMD55 state = ");
  // print_str(errorstate);
  // serial_putc('\n');
  if (errorstate != SD_OK)
  {
    /* CMD Response Timeout (wait for CMDSENT flag) */
    return errorstate;
  }

  /* Send ACMD41 */
#ifdef ECHO
  serial_puts("PowerON: Send ACMD41\n");
#endif
  /*  sdmmc_cmdinitstructure.Argument         = SD_ACMD41_HCS | \
            SD_ACMD41_XPC | \
            SD_ACMD41_S18R;*/
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
  // serial_puts("ACMD41 state = ");
  // print_str(errorstate);
  // serial_putc('\n');
  if (errorstate != SD_OK)
  {
    /* CMD Response Timeout (wait for CMDSENT flag) */
    return errorstate;
  }
  /* Get command get_val */
  get_val = Core_SDMMC_GetResponse(hsd->Instance, SDMMC_RESP0);
// #ifdef ECHO
//   serial_puts("ACMD41 get_val = ");
//   print_str(get_val);
//   serial_putc('\n');
// #endif
  vs_busy = (get_val & SD_R3_BUSY);

  if (vs_busy == 0) {
#ifdef ECHO
    serial_puts("ACMD Loop Start:\n");
#endif
    do {
      loop++;
#ifdef ECHO
      serial_puts("\n\n");
      serial_puts("LOOP: Send CMD55\n");
#endif
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
      // serial_puts("CMD8-1 state = ");
      // print_str(errorstate);
      // serial_putc('\n');
      if (errorstate != SD_OK)
      {
        /* CMD Response Timeout (wait for CMDSENT flag) */
        return errorstate;
      }
#ifdef ECHO
      serial_puts("LOOP: Send ACMD41\n");
#endif
      /* Send ACMD41 */
      sdmmc_cmdinitstructure.Argument         = 0x51FF8000;
      sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_APP_SD_SEND_OP_COND;
      sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_R3;
      sdmmc_cmdinitstructure.Attribute        = SDMMC_CMD_START_CMD | \
          SDMMC_CMD_USE_HOLD_REG | \
          SDMMC_CMD_PRV_DAT_WAIT | \
          SDMMC_CMD_RESP_EXP;
      Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
      Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
      /* Get command get_val */
      get_val = Core_SDMMC_GetResponse(hsd->Instance, SDMMC_RESP0);
// #ifdef ECHO
//       serial_puts("LOOP ACMD41 RESP0 = \n");
//       print_str(get_val);
//       serial_puts("\n\n");
// #endif
      vs_busy = (get_val & SD_R3_BUSY);
      if (loop == 1000) {
#ifdef ECHO
        serial_puts("ACMD41 Loop Fail\n");
#endif
      }
    } while (vs_busy == 0);
    if (vs_busy != 0) {
// #ifdef ECHO
//       serial_puts("ACMD41 Loop Done\n");
//       serial_puts("ACMD41 RESP0 = \n");
//       print_str(get_val);
//       serial_puts("\n\n");
// #endif
    }
  }
  if (get_val & SD_R3_CCS) {
#ifdef ECHO
    serial_puts("SDHC Support\n");
#endif
    hsd->CardType = HIGH_CAPACITY_SD_CARD;
  }
  if (get_val & SD_R3_S18A) {
    serial_puts("1.8v support\n");
    serial_puts("send CMD11\n");
    /* send CMD11 to switch 1.8V bus signaling level */
    Core_SDMMC_SetCLKENA(hsd->Instance, 0x0000FFFF);
    sdmmc_cmdinitstructure.Argument  = 0x00000000;
    sdmmc_cmdinitstructure.CmdIndex  = SD_CMD_VOLTAGE_SWITCH;
    sdmmc_cmdinitstructure.Response  = SDMMC_RESPONSE_R1;
    sdmmc_cmdinitstructure.Attribute = SDMMC_CMD_START_CMD | \
                                       SDMMC_CMD_USE_HOLD_REG | \
                                       SDMMC_CMD_VOLT_SWITCH  | \
                                       SDMMC_CMD_PRV_DAT_WAIT | \
                                       SDMMC_CMD_RESP_CRC;
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
// #ifdef ECHO
//     serial_puts("1.8v switch start\n");
// #endif
    Core_SDMMC_WaiteCmdStart(hsd->Instance)
    Core_SDMMC_WaiteVoltSwitchInt(hsd->Instance)
// #ifdef ECHO
//     serial_puts("cmd11 RESP 1.8v switch successfully\n");
// #endif
    Core_SDMMC_SetRINTSTS(hsd->Instance, 0xFFFFFFFF);
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
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
// #ifdef ECHO
//     serial_puts("host supply 1.8v clock\n");
// #endif
    Core_SDMMC_SetCLKENA(hsd->Instance, 0x0000FFFF);
    sdmmc_cmdinitstructure.Argument  = 0x00000000;
    sdmmc_cmdinitstructure.CmdIndex  = SD_CMD_GO_IDLE_STATE;
    sdmmc_cmdinitstructure.Response  = SDMMC_RESPONSE_NO;
    sdmmc_cmdinitstructure.Attribute = SDMMC_CMD_START_CMD | \
                                       SDMMC_CMD_USE_HOLD_REG | \
                                       SDMMC_CMD_VOLT_SWITCH  | \
                                       SDMMC_CMD_PRV_DAT_WAIT | \
                                       SDMMC_CMD_UPDATE_CLK;
    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);
    Core_SDMMC_WaiteVoltSwitchInt(hsd->Instance);
// #ifdef ECHO
//     serial_puts("voltage switching successfully! \n");
// #endif
  }
  else {
    serial_puts("1.8v not support\n");
  }

  return errorstate;
}

/**
  * @brief  deselect the card.
  * @param  hsd: SD handle
  * @retval SD Card error state
  */
static SD_ErrorTypedef SD_PowerOFF(SD_HandleTypeDef *hsd)
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
  return errorstate;
}

/**
  * @brief  Returns the current card's status.
  * @param  hsd: SD handle
  * @param  pCardStatus: pointer to the buffer that will contain the SD card
  *         status (Card Status register)
  * @retval SD Card error state
  */
// static SD_ErrorTypedef SD_SendStatus(SD_HandleTypeDef *hsd, uint32_t *pCardStatus)
// {
//  SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
//  SD_ErrorTypedef errorstate = SD_OK;

//  if (pCardStatus == NULL)
//  {
//    errorstate = SD_INVALID_PARAMETER;

//    return errorstate;
//  }

//  /* Send Status command */
//  sdmmc_cmdinitstructure.Argument         = (uint32_t)(hsd->RCA << 16);
//  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SEND_STATUS;
//  sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_SHORT;
//  sdmmc_cmdinitstructure.WaitForInterrupt = SDMMC_WAIT_NO;
//  sdmmc_cmdinitstructure.CPSM             = SDMMC_CPSM_ENABLE;
//  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);

//  /* Check for error conditions */
//  errorstate = SD_CmdResp1Error(hsd, SD_CMD_SEND_STATUS);

//  if (errorstate != SD_OK)
//  {
//    return errorstate;
//  }

//  /* Get SD card status */
//  *pCardStatus = SDMMC_GetResponse(hsd->Instance, SDMMC_RESP1);

//  return errorstate;
// }

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
    serial_puts("CMD ERROR\n");
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
  if (RESP_val == SD_CHECK_PATTERN) {
// #ifdef ECHO
//     serial_puts("CMD8 Resp Right: v2\n");
// #endif
  }
  else {
// #ifdef ECHO
//     serial_puts("CMD8 Resp Error: v2\n");
// #endif
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

// /**
//   * @brief  Checks for error conditions for R3 (OCR) response.
//   * @param  hsd: SD handle
//   * @retval SD Card error state
//   */
// static SD_ErrorTypedef SD_CmdResp3Error(SD_HandleTypeDef *hsd)
// {
//  SD_ErrorTypedef errorstate = SD_OK;

//  while (!__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_CCRCFAIL | SDMMC_FLAG_CMDREND | SDMMC_FLAG_CTIMEOUT))
//  {
//  }

//  if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_CTIMEOUT))
//  {
//    errorstate = SD_CMD_RSP_TIMEOUT;

//    __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_FLAG_CTIMEOUT);

//    return errorstate;
//  }

//  /* Clear all the static flags */
//  __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);

//  return errorstate;
// }

// /**
//   * @brief  Checks for error conditions for R2 (CID or CSD) response.
//   * @param  hsd: SD handle
//   * @retval SD Card error state
//   */
// static SD_ErrorTypedef SD_CmdResp2Error(SD_HandleTypeDef *hsd)
// {
//  SD_ErrorTypedef errorstate = SD_OK;

//  while (!__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_CCRCFAIL | SDMMC_FLAG_CMDREND | SDMMC_FLAG_CTIMEOUT))
//  {
//  }

//  if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_CTIMEOUT))
//  {
//    errorstate = SD_CMD_RSP_TIMEOUT;

//    __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_FLAG_CTIMEOUT);

//    return errorstate;
//  }
//  else if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_CCRCFAIL))
//  {
//    errorstate = SD_CMD_CRC_FAIL;

//    __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_FLAG_CCRCFAIL);

//    return errorstate;
//  }
//  else
//  {
//    /* No error flag set */
//  }

//  /* Clear all the static flags */
//  __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);

//  return errorstate;
// }

// /**
//   * @brief  Checks for error conditions for R6 (RCA) response.
//   * @param  hsd: SD handle
//   * @param  SD_CMD: The sent command index
//   * @param  pRCA: Pointer to the variable that will contain the SD card relative
//   *         address RCA
//   * @retval SD Card error state
//   */
// static SD_ErrorTypedef SD_CmdResp6Error(SD_HandleTypeDef *hsd, uint8_t SD_CMD, uint16_t *pRCA)
// {
//  SD_ErrorTypedef errorstate = SD_OK;
//  uint32_t response_r1;

//  while (!__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_CCRCFAIL | SDMMC_FLAG_CMDREND | SDMMC_FLAG_CTIMEOUT))
//  {
//  }

//  if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_CTIMEOUT))
//  {
//    errorstate = SD_CMD_RSP_TIMEOUT;

//    __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_FLAG_CTIMEOUT);

//    return errorstate;
//  }
//  else if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_CCRCFAIL))
//  {
//    errorstate = SD_CMD_CRC_FAIL;

//    __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_FLAG_CCRCFAIL);

//    return errorstate;
//  }
//  else
//  {
//    /* No error flag set */
//  }

//  /* Check response received is of desired command */
//  if (SDMMC_GetCommandResponse(hsd->Instance) != SD_CMD)
//  {
//    errorstate = SD_ILLEGAL_CMD;

//    return errorstate;
//  }

//  /* Clear all the static flags */
//  __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);

//  /* We have received response, retrieve it.  */
//  response_r1 = SDMMC_GetResponse(hsd->Instance, SDMMC_RESP1);

//  if ((response_r1 & (SD_R6_GENERAL_UNKNOWN_ERROR | SD_R6_ILLEGAL_CMD | SD_R6_COM_CRC_FAILED)) == SD_ALLZERO)
//  {
//    *pRCA = (uint16_t) (response_r1 >> 16);

//    return errorstate;
//  }

//  if ((response_r1 & SD_R6_GENERAL_UNKNOWN_ERROR) == SD_R6_GENERAL_UNKNOWN_ERROR)
//  {
//    return (SD_GENERAL_UNKNOWN_ERROR);
//  }

//  if ((response_r1 & SD_R6_ILLEGAL_CMD) == SD_R6_ILLEGAL_CMD)
//  {
//    return (SD_ILLEGAL_CMD);
//  }

//  if ((response_r1 & SD_R6_COM_CRC_FAILED) == SD_R6_COM_CRC_FAILED)
//  {
//    return (SD_COM_CRC_FAILED);
//  }

//  return errorstate;
// }

/**
  * @brief  Enables the SDMMC wide bus mode.
  * @param  hsd: SD handle
  * @retval SD Card error state
  */
// static SD_ErrorTypedef SD_WideBus_Enable(SD_HandleTypeDef *hsd)
// {
//  SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
//  SD_ErrorTypedef errorstate = SD_OK;

//  uint32_t scr[2] = {0, 0};

//  if ((SDMMC_GetResponse(hsd->Instance, SDMMC_RESP1) & SD_CARD_LOCKED) == SD_CARD_LOCKED)
//  {
//    errorstate = SD_LOCK_UNLOCK_FAILED;

//    return errorstate;
//  }

//  /* Get SCR Register */
//  errorstate = SD_FindSCR(hsd, scr);

//  if (errorstate != SD_OK)
//  {
//    return errorstate;
//  }

//  /* If requested card supports wide bus operation */
//  if ((scr[1] & SD_WIDE_BUS_SUPPORT) != SD_ALLZERO)
//  {
//    /* Send CMD55 APP_CMD with argument as card's RCA.*/
//    sdmmc_cmdinitstructure.Argument         = (uint32_t)(hsd->RCA << 16);
//    sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_APP_CMD;
//    sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_SHORT;
//    sdmmc_cmdinitstructure.WaitForInterrupt = SDMMC_WAIT_NO;
//    sdmmc_cmdinitstructure.CPSM             = SDMMC_CPSM_ENABLE;
//    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);

//    /* Check for error conditions */
//    errorstate = SD_CmdResp1Error(hsd, SD_CMD_APP_CMD);

//    if (errorstate != SD_OK)
//    {
//      return errorstate;
//    }

//    /* Send ACMD6 APP_CMD with argument as 2 for wide bus mode */
//    sdmmc_cmdinitstructure.Argument         = 2;
//    sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_APP_SD_SET_BUSWIDTH;
//    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);

//    /* Check for error conditions */
//    errorstate = SD_CmdResp1Error(hsd, SD_CMD_APP_SD_SET_BUSWIDTH);

//    if (errorstate != SD_OK)
//    {
//      return errorstate;
//    }

//    return errorstate;
//  }
//  else
//  {
//    errorstate = SD_REQUEST_NOT_APPLICABLE;

//    return errorstate;
//  }
// }

// /**
//   * @brief  Disables the SDMMC wide bus mode.
//   * @param  hsd: SD handle
//   * @retval SD Card error state
//   */
// static SD_ErrorTypedef SD_WideBus_Disable(SD_HandleTypeDef *hsd)
// {
//  SDMMC_CmdInitTypeDef sdmmc_cmdinitstructure;
//  SD_ErrorTypedef errorstate = SD_OK;

//  uint32_t scr[2] = {0, 0};

//  if ((SDMMC_GetResponse(hsd->Instance, SDMMC_RESP1) & SD_CARD_LOCKED) == SD_CARD_LOCKED)
//  {
//    errorstate = SD_LOCK_UNLOCK_FAILED;

//    return errorstate;
//  }

//  /* Get SCR Register */
//  errorstate = SD_FindSCR(hsd, scr);

//  if (errorstate != SD_OK)
//  {
//    return errorstate;
//  }

//  /* If requested card supports 1 bit mode operation */
//  if ((scr[1] & SD_SINGLE_BUS_SUPPORT) != SD_ALLZERO)
//  {
//    /* Send CMD55 APP_CMD with argument as card's RCA */
//    sdmmc_cmdinitstructure.Argument         = (uint32_t)(hsd->RCA << 16);
//    sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_APP_CMD;
//    sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_SHORT;
//    sdmmc_cmdinitstructure.WaitForInterrupt = SDMMC_WAIT_NO;
//    sdmmc_cmdinitstructure.CPSM             = SDMMC_CPSM_ENABLE;
//    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);

//    /* Check for error conditions */
//    errorstate = SD_CmdResp1Error(hsd, SD_CMD_APP_CMD);

//    if (errorstate != SD_OK)
//    {
//      return errorstate;
//    }

//    /* Send ACMD6 APP_CMD with argument as 0 for single bus mode */
//    sdmmc_cmdinitstructure.Argument         = 0;
//    sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_APP_SD_SET_BUSWIDTH;
//    Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);

//    /* Check for error conditions */
//    errorstate = SD_CmdResp1Error(hsd, SD_CMD_APP_SD_SET_BUSWIDTH);

//    if (errorstate != SD_OK)
//    {
//      return errorstate;
//    }

//    return errorstate;
//  }
//  else
//  {
//    errorstate = SD_REQUEST_NOT_APPLICABLE;

//    return errorstate;
//  }
// }


// /**
//   * @brief  Finds the SD card SCR register value.
//   * @param  hsd: SD handle
//   * @param  pSCR: pointer to the buffer that will contain the SCR value
//   * @retval SD Card error state
//   */
// static SD_ErrorTypedef SD_FindSCR(SD_HandleTypeDef *hsd, uint32_t *pSCR)
// {
//  SDMMC_CmdInitTypeDef  sdmmc_cmdinitstructure;
//  SDMMC_DataInitTypeDef sdmmc_datainitstructure;
//  SD_ErrorTypedef errorstate = SD_OK;
//  uint32_t index = 0;
//  uint32_t tempscr[2] = {0, 0};

//  /* Set Block Size To 8 Bytes */
//  /* Send CMD55 APP_CMD with argument as card's RCA */
//  sdmmc_cmdinitstructure.Argument         = (uint32_t)8;
//  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SET_BLOCKLEN;
//  sdmmc_cmdinitstructure.Response         = SDMMC_RESPONSE_SHORT;
//  sdmmc_cmdinitstructure.WaitForInterrupt = SDMMC_WAIT_NO;
//  sdmmc_cmdinitstructure.CPSM             = SDMMC_CPSM_ENABLE;
//  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);

//  /* Check for error conditions */
//  errorstate = SD_CmdResp1Error(hsd, SD_CMD_SET_BLOCKLEN);

//  if (errorstate != SD_OK)
//  {
//    return errorstate;
//  }

//  /* Send CMD55 APP_CMD with argument as card's RCA */
//  sdmmc_cmdinitstructure.Argument         = (uint32_t)((hsd->RCA) << 16);
//  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_APP_CMD;
//  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);

//  /* Check for error conditions */
//  errorstate = SD_CmdResp1Error(hsd, SD_CMD_APP_CMD);

//  if (errorstate != SD_OK)
//  {
//    return errorstate;
//  }
//  sdmmc_datainitstructure.DataTimeOut   = SD_DATATIMEOUT;
//  sdmmc_datainitstructure.DataLength    = 8;
//  sdmmc_datainitstructure.DataBlockSize = SDMMC_DATABLOCK_SIZE_8B;
//  sdmmc_datainitstructure.TransferDir   = SDMMC_TRANSFER_DIR_TO_SDMMC;
//  sdmmc_datainitstructure.TransferMode  = SDMMC_TRANSFER_MODE_BLOCK;
//  sdmmc_datainitstructure.DPSM          = SDMMC_DPSM_ENABLE;
//  SDMMC_DataConfig(hsd->Instance, &sdmmc_datainitstructure);

//  /* Send ACMD51 SD_APP_SEND_SCR with argument as 0 */
//  sdmmc_cmdinitstructure.Argument         = 0;
//  sdmmc_cmdinitstructure.CmdIndex         = SD_CMD_SD_APP_SEND_SCR;
//  Core_SDMMC_SendCommand(hsd->Instance, &sdmmc_cmdinitstructure);

//  /* Check for error conditions */
//  errorstate = SD_CmdResp1Error(hsd, SD_CMD_SD_APP_SEND_SCR);

//  if (errorstate != SD_OK)
//  {
//    return errorstate;
//  }

//  while (!__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_RXOVERR | SDMMC_FLAG_DCRCFAIL | SDMMC_FLAG_DTIMEOUT | SDMMC_FLAG_DBCKEND))
//  {
//    if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_RXDAVL))
//    {
//      *(tempscr + index) = SDMMC_ReadFIFO(hsd->Instance);
//      index++;
//    }
//  }

//  if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_DTIMEOUT))
//  {
//    __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_FLAG_DTIMEOUT);

//    errorstate = SD_DATA_TIMEOUT;

//    return errorstate;
//  }
//  else if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_DCRCFAIL))
//  {
//    __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_FLAG_DCRCFAIL);

//    errorstate = SD_DATA_CRC_FAIL;

//    return errorstate;
//  }
//  else if (__HAL_SD_SDMMC_GET_FLAG(hsd, SDMMC_FLAG_RXOVERR))
//  {
//    __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_FLAG_RXOVERR);

//    errorstate = SD_RX_OVERRUN;

//    return errorstate;
//  }
//  else
//  {
//    /* No error flag set */
//  }

//  /* Clear all the static flags */
//  __HAL_SD_SDMMC_CLEAR_FLAG(hsd, SDMMC_STATIC_FLAGS);

//  *(pSCR + 1) = ((tempscr[0] & SD_0TO7BITS) << 24)  | ((tempscr[0] & SD_8TO15BITS) << 8) | \
//                ((tempscr[0] & SD_16TO23BITS) >> 8) | ((tempscr[0] & SD_24TO31BITS) >> 24);

//  *(pSCR) = ((tempscr[1] & SD_0TO7BITS) << 24)  | ((tempscr[1] & SD_8TO15BITS) << 8) | \
//            ((tempscr[1] & SD_16TO23BITS) >> 8) | ((tempscr[1] & SD_24TO31BITS) >> 24);

//  return errorstate;
// }

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
  serial_puts("RINTSTS_val = ");
  print_str(RINTSTS_val);
  serial_putc('\n');
#endif
  do {
    RINTSTS_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
    cmd_done = (RINTSTS_val & SDMMC_RINTSTS_CMD_DONE);
  } while (!cmd_done);
  RINTSTS_val = Core_SDMMC_GetRINTSTS(hsd->Instance);
#ifdef ECHO
  serial_puts("SD_ENUM: RINTSTS = ");
  print_str(RINTSTS_val);
  serial_putc('\n');
#endif
  RINTSTS_rto = (RINTSTS_val & SDMMC_RINTSTS_RTO);
  RINTSTS_crc_err = (RINTSTS_val & SDMMC_RINTSTS_RCRC);
  RINTSTS_err = (RINTSTS_val & SDMMC_RINTSTS_RESP_ERR);
  RINTSTS_fail = RINTSTS_rto | RINTSTS_crc_err | RINTSTS_err;
  if (RINTSTS_fail)
  {
    if (RINTSTS_rto)
    {
      serial_puts("RTO\n");
    }
    if (RINTSTS_crc_err)
    {
      serial_puts("RCRC\n");
    }
    if (RINTSTS_err)
    {
      serial_puts("RESP_ERR\n");
    }
  }
  RESP_val = Core_SDMMC_GetResponse(hsd->Instance, SDMMC_RESP0);
#ifdef ECHO
  serial_puts("SD_ENUM: RESPONSE0 = ");
  print_str(RESP_val);
  serial_putc('\n');
#endif
  cmd_illegal = (RESP_val & 0x00400000);
  sdio_mem = (RESP_val & 0x08000000);
  sd_mem = RINTSTS_rto || sdio_mem || cmd_illegal;
#ifdef ECHO
  serial_puts("SD_ENUM: sd_mem = ");
  print_str(sd_mem);
  serial_putc('\n');
#endif
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
// #ifdef ECHO
//   serial_puts("\n\n\nNow enter SD_DMAConfig\n");
// #endif
  Core_SDMMC_SetUHSREG(hsd->Instance, 0x0000FFFF);
  Core_SDMMC_SetCTYPE(hsd->Instance, SDMMC_CTYPE_4BIT);
  Core_SDMMC_SetCTRL(hsd->Instance, SDMMC_CTRL_USE_INTERNAL_IDMAC | \
                     SDMMC_CTRL_INT_ENABLE | \
                     SDMMC_CTRL_FIFO_RESET);
  Core_SDMMC_SetBLKSIZ(hsd->Instance, dma->BlockSize);
  //Core_SDMMC_SetBYCTNT(hsd->Instance, 0x00000200);

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
// #ifdef ECHO
//   get_val = Core_SDMMC_GetRESP0(hsd->Instance);
//   serial_puts("CMD16 RESP0 = ");
//   print_str(get_val);
//   serial_puts(" \n");
// #endif
  /* set up idma descriptor */
  Core_SDMMC_SetBMOD(hsd->Instance, SDMMC_BMOD_ENABLE | SDMMC_BMOD_FB);
  Core_SDMMC_SetIDINTEN(hsd->Instance, 0xFFFFFFFF);
  Core_SDMMC_SetDBADDR(hsd->Instance, dma->ListBaseAddr);

  return errorstate;
}





/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
