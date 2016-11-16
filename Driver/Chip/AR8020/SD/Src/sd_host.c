#include "memory_config.h"
#include "sd_host.h"
#include "sd_card.h"
#include "sd_core.h"
#include "debuglog.h"

SDMMC_Status sd_init()
{
	sdhandle.Instance = SDMMC_ADDR;
	SD_CardInfoTypedef *cardinfo;
	SDMMC_Status errorstate = SDMMC_OK;
	errorstate = Card_SD_Init(&sdhandle, cardinfo);
	if (errorstate != SDMMC_OK) {
		return SDMMC_ERROR;
	}
	dlog_info("Initialize SD Success!\n");
	return errorstate;
}

SDMMC_Status sd_write(uint32_t DstStartAddr, uint32_t SrcStartAddr, uint32_t SectorNum)
{
	//dlog_info("Write SD\n");
	SDMMC_Status errorstate = SDMMC_OK;
	SDMMC_DMATransTypeDef dma;
	dma.BlockSize = 512;
	dma.SrcAddr = (uint32_t )DTCMBUSADDR(SrcStartAddr);
	dma.DstAddr = (uint32_t )DstStartAddr;                        /* [block units] */
	dma.SectorNum = SectorNum;
	dma.ListBaseAddr = 0x440F0000;

	dlog_info("DstStartAddr = %x\n",DstStartAddr);
	dlog_info("SrcStartAddr = %x\n",SrcStartAddr);
	dlog_info("SectorNum = %x\n",SectorNum);
	if (SectorNum == 1)
	{
		errorstate = Card_SD_WriteBlock_DMA(&sdhandle, &dma);	
	}
	else
	{
		errorstate = Card_SD_WriteMultiBlocks_DMA(&sdhandle, &dma);
	}
	
	if (errorstate != SDMMC_OK) {
		dlog_info("Write SD Failed!\n");
		return SDMMC_ERROR;
	}
	dlog_info("Write SD %d Sectors Done. From Sector %d to Sector %d\n", \
		         dma.SectorNum, dma.DstAddr, (dma.DstAddr + dma.SectorNum - 1));
	return errorstate;
}

SDMMC_Status sd_read(uint32_t DstStartAddr, uint32_t SrcStartAddr, uint32_t SectorNum)
{
	SDMMC_Status errorstate = SDMMC_OK;
	SDMMC_DMATransTypeDef dma;
	dma.BlockSize = 512;
	dma.SrcAddr = (uint32_t )SrcStartAddr;                     /* [block units] */
	dma.DstAddr = (uint32_t )DTCMBUSADDR(DstStartAddr);
	dma.SectorNum = SectorNum;
	dma.ListBaseAddr = 0x440F0000;

	if (SectorNum == 1)
	{
		errorstate = Card_SD_ReadBlock_DMA(&sdhandle, &dma);
	}
	else
	{
		errorstate = Card_SD_ReadMultiBlocks_DMA(&sdhandle, &dma);
	}
	
	if (errorstate != SDMMC_OK) {
		dlog_info("Read SD Failed!\n");
		return SDMMC_ERROR;
	}
	dlog_info("Read SD %d Sectors Done. From Sector %d to Sector %d\n", \
		         dma.SectorNum, dma.SrcAddr, (dma.SrcAddr + dma.SectorNum - 1));
	return errorstate;
}

SDMMC_Status sd_erase(uint32_t StartAddr, uint32_t SectorNum)
{
	//dlog_info("Erase SD\n");
	SDMMC_Status errorstate = SDMMC_OK;
	errorstate = Card_SD_Erase(&sdhandle, StartAddr, SectorNum);  /* [block units] */
	if (errorstate != SDMMC_OK) {
		dlog_info("Erase SD failed!\n");
		return SDMMC_ERROR;
	}
	dlog_info("Erase %d Sectors Done. From Sector %d to Sector %d\n", \
		         SectorNum, StartAddr, (StartAddr + SectorNum - 1));
	return errorstate;
}

SDMMC_Status sd_deinit()
{
	
	SDMMC_Status errorstate = SDMMC_OK;
	errorstate = Card_SD_DeInit(&sdhandle);
	if (errorstate != SDMMC_OK) {
		dlog_info("Deinit SD Failed!\n");
		return SDMMC_ERROR;
	}
	dlog_info("Remove SD Success!\n");
	return errorstate;
}

SD_CardStateTypedef sd_getcardstatus()
{
	//dlog_info("Get Card Status\n");
	uint32_t RespState = 0;
	SD_CardStateTypedef cardstate = SD_CARD_TRANSFER;
	SDMMC_Status errorstate = SDMMC_OK;
	errorstate = SD_GetState(&sdhandle, (uint32_t *)&RespState);
	if (errorstate != SDMMC_OK)
	{
		dlog_info("Get SD Status Failed!\n");
		return SDMMC_ERROR;
	}

	cardstate = (SD_CardStateTypedef)((RespState >> 9) & 0x0F);
	  /* Find SD status according to card state*/
	if (cardstate == SD_CARD_TRANSFER)
	{
		return SD_TRANSFER_OK;
	}
	else if(cardstate == SD_CARD_ERROR)
	{
		return SD_TRANSFER_ERROR;
	}
	else
	{
		return SD_TRANSFER_BUSY;
	}
}

void sd_getcardinfo(SD_CardInfoTypedef *CardInfo)
{
	Card_SD_Get_CardInfo(&sdhandle, CardInfo);
}

void sd_IRQHandler(void)
{

    uint32_t status, pending, cdetect;
   
	status  = read_reg32(SDMMC_BASE + 0x44);  /* RINTSTS */
	pending = read_reg32(SDMMC_BASE + 0x40);  /* MINTSTS */
    cdetect = read_reg32(SDMMC_BASE + 0x50);  /* CDETECT*/

	// dlog_info("RINTSTS = 0x%08x\n", status);
	// dlog_info("MINTSTS = 0x%08x\n", pending);

	if(pending)
	{
	  if (pending & SDMMC_RINTSTS_CARD_DETECT)
	  {
	  	if (status & SDMMC_RINTSTS_CARD_DETECT)
	  	{
	  		if (!cdetect)
	  		{
		        dlog_info("Initializing the SD Card...\n");
		        write_reg32((SDMMC_BASE + 0x44), status);
		        sd_init();
		        write_reg32((SDMMC_BASE + 0x44), 0xFFFFFFFF);
	  		}
	  		else
	  		{
	  			dlog_info("Removing the SD Card...\n");
		        write_reg32((SDMMC_BASE + 0x44), status);
		        sd_deinit();
		        write_reg32((SDMMC_BASE + 0x44), 0xFFFFFFFF);
	  		}
	  	}
	  }
	  if (pending & SDMMC_RINTSTS_RESP_ERR)
	  {
	  	if (status & SDMMC_RINTSTS_RESP_ERR)
	  	{
	        dlog_info("SDMMC_RINTSTS_RESP_ERR\n");
	        status &= ~SDMMC_RINTSTS_RESP_ERR;
	        write_reg32((SDMMC_BASE + 0x44), status);
	        return 0;
	      
	  	}
	  }
	  if (pending & SDMMC_RINTSTS_CMD_DONE)
	  {
	  	if (status & SDMMC_RINTSTS_CMD_DONE)
	  	{
	        dlog_info("SDMMC_RINTSTS_CMD_DONE\n");
	        status &= ~SDMMC_RINTSTS_CMD_DONE;
	        write_reg32((SDMMC_BASE + 0x44), status);
	        return 0;
	      
	  	}
	  }
	  if (pending & SDMMC_RINTSTS_DATA_OVER)
	  {
	  	if (status & SDMMC_RINTSTS_DATA_OVER)
	  	{
	        dlog_info("SDMMC_RINTSTS_DATA_OVER\n");
	        status &= ~SDMMC_RINTSTS_DATA_OVER;
	        write_reg32((SDMMC_BASE + 0x44), status);
	        return 0;
	      
	  	}
	  }
	  if (pending & SDMMC_RINTSTS_TXDR)
	  {
	  	if (status & SDMMC_RINTSTS_TXDR)
	  	{
	        dlog_info("SDMMC_RINTSTS_TXDR\n");
	        status &= ~SDMMC_RINTSTS_TXDR;
	        write_reg32((SDMMC_BASE + 0x44), status);
	        return 0;
	      
	  	}
	  }
	  if (pending & SDMMC_RINTSTS_RXDR)
	  {
	  	if (status & SDMMC_RINTSTS_RXDR)
	  	{
	        dlog_info("SDMMC_RINTSTS_RXDR\n");
	        status &= ~SDMMC_RINTSTS_RXDR;
	        write_reg32((SDMMC_BASE + 0x44), status);
	        return 0;
	      
	  	}
	  }
	  if (pending & SDMMC_RINTSTS_RCRC)
	  {
	  	if (status & SDMMC_RINTSTS_RCRC)
	  	{
	        dlog_info("SDMMC_RINTSTS_RCRC\n");
	        status &= ~SDMMC_RINTSTS_RCRC;
	        write_reg32((SDMMC_BASE + 0x44), status);
	        return 0;
	      
	  	}
	  }
	  if (pending & SDMMC_RINTSTS_RTO)
	  {
	  	if (status & SDMMC_RINTSTS_RTO)
	  	{
	        dlog_info("SDMMC_RINTSTS_RTO\n");
	        status &= ~SDMMC_RINTSTS_RTO;
	        write_reg32((SDMMC_BASE + 0x44), status);
	        return 0;
	      
	  	}
	  }
	  if (pending & SDMMC_RINTSTS_DRTO)
	  {
	  	if (status & SDMMC_RINTSTS_DRTO)
	  	{
	        dlog_info("SDMMC_RINTSTS_DRTO\n");
	        status &= ~SDMMC_RINTSTS_DRTO;
	        write_reg32((SDMMC_BASE + 0x44), status);
	        return 0;
	      
	  	}
	  }
	  if (pending & SDMMC_RINTSTS_HTO)
	  {
	  	if (status & SDMMC_RINTSTS_HTO)
	  	{
	        dlog_info("SDMMC_RINTSTS_HTO\n");
	        status &= ~SDMMC_RINTSTS_HTO;
	        write_reg32((SDMMC_BASE + 0x44), status);
	        return 0;
	      
	  	}
	  }

	  if (pending & SDMMC_INTMASK_FRUN)
	  {
	  	if (status & SDMMC_INTMASK_FRUN)
	  	{
	        dlog_info("SDMMC_INTMASK_FRUN\n");
	        status &= ~SDMMC_INTMASK_FRUN;
	        write_reg32((SDMMC_BASE + 0x44), status);
	        return 0;
	      
	  	}
	  }
	  if (pending & SDMMC_INTMASK_HLE)
	  {
	  	if (status & SDMMC_INTMASK_HLE)
	  	{
	        dlog_info("SDMMC_INTMASK_HLE\n");
	        status &= ~SDMMC_INTMASK_HLE;
	        write_reg32((SDMMC_BASE + 0x44), status);
	        return 0;
	      
	  	}
	  }
	  if (pending & SDMMC_INTMASK_SBE)
	  {
	  	if (status & SDMMC_INTMASK_SBE)
	  	{
	        dlog_info("SDMMC_INTMASK_SBE\n");
	        status &= ~SDMMC_INTMASK_SBE;
	        write_reg32((SDMMC_BASE + 0x44), status);
	        return 0;
	      
	  	}
	  }
	  if (pending & SDMMC_INTMASK_ACD)
	  {
	  	if (status & SDMMC_INTMASK_ACD)
	  	{
	        dlog_info("SDMMC_INTMASK_ACD\n");
	        status &= ~SDMMC_INTMASK_ACD;
	        write_reg32((SDMMC_BASE + 0x44), status);
	        return 0;
	      
	  	}
	  }
	}

}