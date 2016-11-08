/**
  ******************************************************************************
  * @file    sd_host.h
  * @author  Minzhao
  * @version V1.0.0
  * @date    7-7-2016
  * @brief   This file contains the common defines and functions prototypes for
  *          the sdmmc driver.
  */
#ifndef __SD_HOST_H
#define __SD_HOST_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sd_card.h"
#include "sd_core.h"

static SDMMC_DMATransTypeDef dma;
static SD_HandleTypeDef sdhandle;
static SD_CardInfoTypedef cardinfo;

SDMMC_Status sd_init();
SDMMC_Status sd_getstatus();
SDMMC_Status sd_write(uint32_t DstStartAddr, uint32_t SrcStartAddr, uint32_t SectorNum);
SDMMC_Status sd_read(uint32_t DstStartAddr, uint32_t SrcStartAddr, uint32_t SectorNum);
SDMMC_Status sd_erase(uint32_t StartBlock, uint32_t SectorNum);
SDMMC_Status sd_ioctl(unsigned char cmd, void *buff);
SDMMC_Status sd_deinit();
void sd_getcardinfo(SD_CardInfoTypedef *CardInfo);
void sd_IRQHandler(void);


#ifdef __cplusplus
}
#endif

#endif
