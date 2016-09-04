#include "memory_config.h"
#include "sd_host.h"
#include "sd_card.h"
#include "sd_core.h"
#include "debuglog.h"

SDMMC_Status sd_init()
{
	dlog_info("Init SD\n");
	sdhandle.Instance = SDMMC_ADDR;
	SD_CardInfoTypedef cardinfo;
	SDMMC_Status errorstate = SDMMC_OK;
	errorstate = Card_SD_Init(&sdhandle, &cardinfo);
	if (errorstate != SDMMC_OK) {
		dlog_info("init SD failed!\n");
		return SDMMC_ERROR;
	}

	return errorstate;
}

SDMMC_Status sd_write(uint32_t DstStartAddr, uint32_t SrcStartAddr, uint32_t SectorNum)
{
	dlog_info("\nWrite SD");
	SDMMC_Status errorstate = SDMMC_OK;
	SDMMC_DMATransTypeDef dma;
	dma.BlockSize = 512;
	// dma.SrcAddr = (uint32_t)DstStartAddr;
	// dma.DstAddr = SrcStartAddr;                               /* [byte units] */
	dma.SrcAddr = SrcStartAddr;
	dma.DstAddr = DstStartAddr;                                  /* [byte units] */

	dma.SectorNum = SectorNum;
	dma.ListBaseAddr = SRAM_BASE;
	if (SectorNum == 1)
	{
		errorstate = Card_SD_WriteBlock_DMA(&sdhandle, &dma);	
	}
	else
	{
		errorstate = Card_SD_WriteMultiBlocks_DMA(&sdhandle, &dma);
	}

	if (errorstate != SDMMC_OK) {
		dlog_info("write SD failed!\n");
		return SDMMC_ERROR;
	}

	return errorstate;
}

SDMMC_Status sd_read(uint32_t DstStartAddr, uint32_t SrcStartAddr, uint32_t SectorNum)
{
	dlog_info("\nRead SD\n");
	SDMMC_Status errorstate = SDMMC_OK;
	SDMMC_DMATransTypeDef dma;
	dma.BlockSize = 512;
	dma.SrcAddr = (uint32_t )SrcStartAddr;                     /* [byte units] */
	dma.SectorNum = SectorNum;
	dma.ListBaseAddr = SRAM_BASE;
	dma.DstAddr = (uint32_t)DstStartAddr;
	if (SectorNum == 1)
	{
		errorstate = Card_SD_ReadBlock_DMA(&sdhandle, &dma);
	}
	else
	{
		errorstate = Card_SD_ReadMultiBlocks_DMA(&sdhandle, &dma);
	}
	
	if (errorstate != SDMMC_OK) {
		dlog_info("read SD failed!\n");
		return SDMMC_ERROR;
	}

	return errorstate;
}

SDMMC_Status sd_erase(uint32_t StartAddr, uint32_t SectorNum)
{
	dlog_info("Erase SD\n");
	SDMMC_Status errorstate = SDMMC_OK;
	errorstate = Card_SD_Erase(&sdhandle, StartAddr, SectorNum);  /* [block units] */
	if (errorstate != SDMMC_OK) {
		dlog_info("Erase SD failed!\n");
		return SDMMC_ERROR;
	}

	return errorstate;
}

SDMMC_Status sd_deinit()
{
	dlog_info("Deinit SD\n");
	SDMMC_Status errorstate = SDMMC_OK;
	errorstate = Card_SD_DeInit(&sdhandle);
	if (errorstate != SDMMC_OK) {
		dlog_info("deinit SD failed!\n");
		return SDMMC_ERROR;
	}

	return errorstate;
}

SDMMC_Status sd_getstatus()
{
	return SDMMC_OK;
}

void sd_getcardinfo(SD_CardInfoTypedef *CardInfo)
{
	Card_SD_Get_CardInfo(&sdhandle, CardInfo);
}