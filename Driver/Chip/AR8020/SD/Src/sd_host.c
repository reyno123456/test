#include "sd_host.h"
#include "system_config.h"
#include "command.h"

SDMMC_Status sd_init()
{
	serial_puts("Init SD\n");
	sdhandle.Instance = SDMMC_ADDR;
	SD_CardInfoTypedef cardinfo;
	SDMMC_Status errorstate = SDMMC_OK;
	errorstate = Card_SD_Init(&sdhandle, &cardinfo);
	if (errorstate != SDMMC_OK) {
		serial_puts("init SD failed!\n");
		return SDMMC_ERROR;
	}

	return errorstate;
}

SDMMC_Status sd_write(uint32_t DstStartAddr, uint32_t SrcStartAddr, uint32_t SectorNum)
{
	serial_puts("Write SD\n");
	SDMMC_Status errorstate = SDMMC_OK;
	SDMMC_DMATransTypeDef dma;
	dma.BlockSize = 512;
	dma.SrcAddr = (uint32_t)SrcStartAddr;
	dma.NumberOfBytes = SectorNum * dma.BlockSize;
	dma.ListBaseAddr = SRAM_BASE;
	dma.DstAddr = DstStartAddr / dma.BlockSize;              /* [block unit] */
	errorstate = Card_SD_WriteBlocks_DMA(&sdhandle, &dma);
	if (errorstate != SDMMC_OK) {
		serial_puts("write SD failed!\n");
		return SDMMC_ERROR;
	}

	return errorstate;
}

SDMMC_Status sd_read(uint32_t DstStartAddr, uint32_t SrcStartAddr, uint32_t SectorNum)
{
	serial_puts("Read SD\n");
	SDMMC_Status errorstate = SDMMC_OK;
	SDMMC_DMATransTypeDef dma;
	dma.BlockSize = 512;
	dma.SrcAddr = (uint32_t )SrcStartAddr;                     /* [byte units] */
	dma.NumberOfBytes = SectorNum * 512;
	dma.ListBaseAddr = SRAM_BASE;
	dma.DstAddr = (uint32_t)DstStartAddr;
	errorstate = Card_SD_ReadBlocks_DMA(&sdhandle, &dma);
	if (errorstate != SDMMC_OK) {
		serial_puts("read SD failed!\n");
		return SDMMC_ERROR;
	}

	return errorstate;
}

SDMMC_Status sd_erase(uint32_t StartAddr, uint32_t BlockNum)
{
	serial_puts("Erase SD\n");
	SDMMC_Status errorstate = SDMMC_OK;
	errorstate = Card_SD_Erase(&sdhandle, StartAddr, BlockNum);  /* [block units] */
	if (errorstate != SDMMC_OK) {
		serial_puts("Erase SD failed!\n");
		return SDMMC_ERROR;
	}

	return errorstate;
}

SDMMC_Status sd_deinit()
{
	serial_puts("Deinit SD\n");
	SDMMC_Status errorstate = SDMMC_OK;
	errorstate = Card_SD_DeInit(&sdhandle);
	if (errorstate != SDMMC_OK) {
		serial_puts("deinit SD failed!\n");
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
