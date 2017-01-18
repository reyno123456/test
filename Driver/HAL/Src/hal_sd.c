/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_sd.h
Description: The external HAL APIs to use the SDMMC controller.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/12/26
History: 
        0.0.1    2016/12/26    The initial version of hal_sd.h
*****************************************************************************/
#include <stdint.h>
#include "memory_config.h"
#include "hal_sd.h"
#include "hal_nvic.h"
#include "sd_card.h"
#include "sd_core.h"
#include "debuglog.h"
#include "reg_rw.h"
#include "interrupt.h"

extern SDMMC_DMATransTypeDef dma;
extern SD_HandleTypeDef sdhandle;
extern SD_CardInfoTypedef cardinfo;

/**
* @brief  Initializes the SD card according to the specified parameters in the 
            SD_HandleTypeDef and create the associated handle
* @param  none
* @retval HAL_OK            means the initializtion is well done
*         HAL_SD_ERR_ERROR  means some error happens in the initializtion
*/
HAL_RET_T HAL_SD_Init()
{
	sdhandle.Instance = SDMMC_ADDR;
	SDMMC_Status e_errorState = SDMMC_OK;
	e_errorState = Card_SD_Init(&sdhandle, &cardinfo);
	if (e_errorState != SDMMC_OK) {
		return HAL_SD_ERR_ERROR;
	}
	dlog_info("Initialize SD Success!\n");
	return HAL_OK;
}

/**
* @brief  Writes block(s) to a specified address in a card. The Data transfer 
  *         is managed by DMA mode
* @param  u32_dstStartAddr   pointer to the buffer that will contain the data to transmit
*         u32_srcStartAddr   Address from where data is to be read   
*         u32_sectorNum      the SD card Data block size 
* @retval HAL_OK            write to sd card succeed
*         HAL_SD_ERR_ERROR  means some error happens during the writing
*/
HAL_RET_T HAL_SD_Write(uint32_t u32_dstStartAddr, uint32_t u32_srcStartAddr, uint32_t u32_sectorNum)
{
	SDMMC_Status e_errorState = SDMMC_OK;
	SDMMC_DMATransTypeDef st_dma;
	st_dma.BlockSize = 512;
	st_dma.SrcAddr = (uint32_t )DTCMBUSADDR(u32_srcStartAddr);
	st_dma.DstAddr = (uint32_t )u32_dstStartAddr;                        /* [block units] */
	st_dma.SectorNum = u32_sectorNum;


	if (u32_sectorNum == 1)
	{
		e_errorState = Card_SD_WriteBlock_DMA(&sdhandle, &st_dma);	
	}
	else
	{
		e_errorState = Card_SD_WriteMultiBlocks_DMA(&sdhandle, &st_dma);
	}
	
	if (e_errorState != SDMMC_OK) {
		dlog_info("Write SD Failed!\n");
		return HAL_SD_ERR_ERROR;
	}
	// dlog_info("Write SD %d Sectors Done. From Sector %d to Sector %d\n", \
		         st_dma.SectorNum, st_dma.DstAddr, (st_dma.DstAddr + st_dma.SectorNum - 1));
	return HAL_OK;
}

/**
* @brief  Reads block(s) from a specified address in a card. The Data transfer 
  *         is managed by DMA mode. 
* @param  u32_dstStartAddr   Pointer to the buffer that will contain the received data
*         u32_srcStartAddr   Address from where data is to be read  
*         u32_sectorNum      SD card Data block size
* @retval HAL_OK             read from sd card succeed
*         HAL_SD_ERR_ERROR   means some error happens during the reading
*/
HAL_RET_T HAL_SD_Read(uint32_t u32_dstStartAddr, uint32_t u32_srcStartAddr, uint32_t u32_sectorNum)
{
	SDMMC_Status e_errorState = SDMMC_OK;
	SDMMC_DMATransTypeDef st_dma;
	st_dma.BlockSize = 512;
	st_dma.SrcAddr = (uint32_t )u32_srcStartAddr;                     /* [block units] */
	st_dma.DstAddr = (uint32_t )DTCMBUSADDR(u32_dstStartAddr);
	st_dma.SectorNum = u32_sectorNum;
	

	if (u32_sectorNum == 1)
	{
		e_errorState = Card_SD_ReadBlock_DMA(&sdhandle, &st_dma);
	}
	else
	{
		e_errorState = Card_SD_ReadMultiBlocks_DMA(&sdhandle, &st_dma);
	}
	
	if (e_errorState != SDMMC_OK) {
		dlog_info("Read SD Failed!\n");
		return HAL_SD_ERR_ERROR;
	}
	// dlog_info("Read SD %d Sectors Done. From Sector %d to Sector %d\n", \
		         st_dma.SectorNum, st_dma.SrcAddr, (st_dma.SrcAddr + st_dma.SectorNum - 1));
	return HAL_OK;
}

/**
* @brief  Erases the specified memory area of the given SD card
* @param  u32_startBlock    start byte address
*         u32_sectorNum     the SD card data block size
* @retval HAL_OK            erase SD card succeed
*         HAL_SD_ERR_ERROR  means some error happens during the erasing
*/
HAL_RET_T HAL_SD_Erase(uint32_t u32_startAddr, uint32_t u32_sectorNum)
{
	SDMMC_Status e_errorState = SDMMC_OK;
	e_errorState = Card_SD_Erase(&sdhandle, u32_startAddr, u32_sectorNum);  /* [block units] */
	if (e_errorState != SDMMC_OK) {
		dlog_info("Erase SD failed!\n");
		return HAL_SD_ERR_ERROR;
	}
	// dlog_info("Erase %d Sectors Done. From Sector %d to Sector %d\n", \
		         u32_sectorNum, u32_startAddr, (u32_startAddr + u32_sectorNum - 1));
	return HAL_OK;
}

/**
* @brief  De-Initializes the SD card
* @param  none
* @retval HAL_OK      means the de-initialization is well done
      HAL_SD_ERR_ERROR means de-initialization
*/
HAL_RET_T HAL_SD_Deinit()
{
	
	SDMMC_Status e_errorState = SDMMC_OK;
	e_errorState = Card_SD_DeInit(&sdhandle);
	if (e_errorState != SDMMC_OK) {
		dlog_info("Deinit SD Failed!\n");
		return HAL_SD_ERR_ERROR;
	}
	dlog_info("Remove SD Success!\n");
	return HAL_OK;
}

/**
* @brief  IO ctrl function to acquire SD card related information
* @param  e_sdCtrl          the SD IO ctrl enumetation variable to specify the corresponding function
*         pu32_info         IO ctrl function output 
* @retval HAL_OK            get the information succeed
*         HAL_SD_ERR_ERROR  means some error happens in the function
*/
HAL_RET_T HAL_SD_Ioctl(ENUM_HAL_SD_CTRL e_sdCtrl, uint32_t *pu32_info)
{
	SDMMC_Status e_errorState = SDMMC_OK;
  uint32_t u32_tmpValue = 0;
	e_errorState = Card_SD_Get_CardInfo(&sdhandle, &cardinfo);
	if (e_errorState != SDMMC_OK) 
	{
		dlog_info("Get SD Info failed!\n");
		return HAL_SD_ERR_ERROR;
	}
	switch(e_sdCtrl)
	{
		case HAL_SD_GET_SECTOR_COUNT:
			  *pu32_info = (uint32_t)(cardinfo.CardCapacity / 2048);
        dlog_info("SD card size: %dMB\n",cardinfo.CardCapacity / 2048);
			  break;
		case HAL_SD_GET_SECTOR_SIZE:
		    u32_tmpValue = cardinfo.SD_csd.READ_BL_LEN;
        switch(u32_tmpValue)
        {
          case 9:
            dlog_info("Maxi data block length: 512bytes\n");
            *pu32_info = 512;
            break;
          case 10:
            dlog_info("Maxi data block length: 1024bytes\n");
            *pu32_info = 1024;
            break;
          case 11:
            dlog_info("Maxi data block length: 2048bytes\n");
            *pu32_info = 2048;
            break;
          default:
            break;
        }
		    break;
		case HAL_SD_GET_CSD_VERSION:
		    *pu32_info = (uint32_t)cardinfo.SD_csd.CSD_STRUCTURE;
        if (cardinfo.SD_csd.CSD_STRUCTURE)
        {
          dlog_info("CSD Version 2.0\n");
        }
        else
        {
          dlog_info("CSD Version 1.0\n"); 
        }
		    break;
    case HAL_SD_GET_TRAN_SPEED:
        *pu32_info = (uint32_t)(cardinfo.SD_csd.TRAN_SPEED & 0x7);
        switch(*pu32_info)
        {
          case 0:
            dlog_info("Maxi Data Transfer Rate: 100kbit/s\n");
            break;
          case 1:
            dlog_info("Maxi Data Transfer Rate: 1Mbit/s\n");
            break;
          case 2:
            dlog_info("Maxi Data Transfer Rate: 10Mbit/s\n");
            break;
          case 3:
            dlog_info("Maxi Data Transfer Rate: 100Mbit/s\n");
            break;
          default:
            break;
        }
        break;
    case HAL_SD_GET_CARD_STATUS:
        {
          SD_STATUS e_cardStatus;
          SD_CardStatus(&e_cardStatus);
          switch(e_cardStatus)
          {
            case SD_CARD_IDLE:
              dlog_info("Current Card State: IDLE\n");
              break;
            case SD_CARD_READY:
              dlog_info("Current Card State: READY\n");
              break;
            case SD_CARD_IDENTIFICATION:
              dlog_info("Current Card State: IDENTIFICATION\n");
              break;
            case SD_CARD_STANDBY:
              dlog_info("Current Card State: STANDBY\n");
              break;
            case SD_CARD_TRANSFER:
              dlog_info("Current Card State: TRANSFER\n");
              break;
            case SD_CARD_DATA:
              dlog_info("Current Card State: DATA\n");
              break;
            case SD_CARD_RECEIVE:
              dlog_info("Current Card State: RECEIVE\n");
              break;
            case SD_CARD_PROGRAMMING:
              dlog_info("Current Card State: PROGRAMMING\n");
              break;
            case SD_CARD_DISCONNECTED:
              dlog_info("Current Card State: DISCONNECTED\n");
              break;
            case SD_CARD_ERROR:
              dlog_info("Current Card State: ERROR\n");
              break;
            default:
              break;
          }
          break;
        }
		case HAL_SD_GET_MANUID:
		    *pu32_info = (uint32_t)cardinfo.SD_cid.ManufacturerID;
		    break;
	  case HAL_SD_GET_OEMID:
		    *pu32_info = (uint32_t)cardinfo.SD_cid.OEM_AppliID;
		    break;
		default:
			  break;
	}

	return HAL_OK;
}

/**
* @brief  The SD interrupt initialization function which must be called before HAL_SD_Init(), which is 
      used to enable the interrupt function of SD card
* @param  none
* @retval none
*/
void HAL_SD_InitIRQ(void)
{
	 /* register the irq handler */
    reg_IrqHandle(SD_INTR_VECTOR_NUM, SD_IRQHandler, NULL);
    INTR_NVIC_SetIRQPriority(SD_INTR_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_SD,0));
    INTR_NVIC_EnableIRQ(SD_INTR_VECTOR_NUM);
}


