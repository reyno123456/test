/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: dma.c
Description: The external HAL APIs to use the SDMMC controller.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2017/1/14
History: 
        0.0.1    2017/1/14    The initial version of dma.c
        0.1.1    2017/3/31    Version capability for RTOS
*****************************************************************************/
#include "dma.h"
#include "debuglog.h"
#include "reg_rw.h"
#include "cpu_info.h"
#include "interrupt.h"
#include "memory_config.h"
#include "cmsis_os.h"
#include "systicks.h"
#include <string.h>
#include "data_type.h"

static volatile STRU_DmaRegs *s_st_dmaRegs = (STRU_DmaRegs *)DMA_BASE;
static volatile STRU_transStatus s_st_transStatus[8] = {0};

static uint8_t DMA_getChannel(uint32_t data)
{
	uint8_t u8_chanIndex;
	
	for (u8_chanIndex = 0; u8_chanIndex< DW_DMA_MAX_NR_CHANNELS; u8_chanIndex++)
	{
		if (READ_BIT(data, BIT(u8_chanIndex)))
		{
			return u8_chanIndex;
		}
	}

	return u8_chanIndex;
}

static void DMA_clearIRQ(uint8_t u8_index)
{
	s_st_dmaRegs->CLEAR.TFR = (1 << u8_index);
}

static void DMA_irqISR(uint32_t vectorNum)
{
	uint8_t u8_chanIndex = 0;
	uint32_t index = 0;

	u8_chanIndex = DMA_getChannel(s_st_dmaRegs->STATUS.TFR);
	if (u8_chanIndex >= 8)
	{
		dlog_error("%d. error\n", __LINE__);
		return;
	}
	
	switch(s_st_transStatus[u8_chanIndex].e_transferType)
	{
		case LINK_LIST_ITEM:
		{
			s_st_transStatus[u8_chanIndex].e_transActive = NON_ACTIVE;
			
			#ifdef USE_MALLOC_DESC
				free(s_st_transStatus[u8_chanIndex].pst_lliMalloc);
			#endif
			
			s_st_transStatus[u8_chanIndex].trans_complete = 1;
			DMA_clearIRQ(u8_chanIndex);
			s_st_dmaRegs->CH_EN &=~ ((1 << (u8_chanIndex)) | (1 << (u8_chanIndex +8)));

			break;
		}
		case AUTO_RELOAD:
		{
			s_st_transStatus[u8_chanIndex].u32_transNum--;
			if (s_st_transStatus[u8_chanIndex].u32_transNum <= 1)
			{
				/* disable the channel auto-reload */
				s_st_dmaRegs->CHAN[u8_chanIndex].CFG_HI = 0x0;
				s_st_dmaRegs->CHAN[u8_chanIndex].CFG_LO = DWC_CFGL_HS_SRC | DWC_CFGL_HS_DST;
			}
			/* clear interrupt */
			DMA_clearIRQ(u8_chanIndex);
			break;
		}
		default: break;
	}
}

void assert_failed(uint8_t* file, uint32_t line)
{
	dlog_info("wrong paraments\n");
}

void DMA_initIRQ()
{
    /* register the irq handler */
    reg_IrqHandle(DMA_INTR_N_VECTOR_NUM, DMA_irqISR, NULL);
    INTR_NVIC_EnableIRQ(DMA_INTR_N_VECTOR_NUM);
    INTR_NVIC_SetIRQPriority(DMA_INTR_N_VECTOR_NUM, 7);
}

int32_t DMA_Init(ENUM_Chan u8_channel, uint8_t u8_chanPriority)
{
	uint8_t u8_chanIndex = 0;
	uint8_t u8_inited = 1;

	assert_param(IS_CHANNAL_PRIORITY(u8_chanPriority));

	if (u8_channel == AUTO)
	{
		/* find out which channel is idle */
		for (u8_chanIndex = 0; u8_chanIndex < DW_DMA_MAX_NR_CHANNELS; ++u8_chanIndex)
		{
			if ((s_st_dmaRegs->CH_EN & (1 << u8_chanIndex)) == 0)
			{
				 /* disable the channel */
				s_st_dmaRegs->CHAN[u8_chanIndex].CFG_HI = 0x0;
				s_st_dmaRegs->CHAN[u8_chanIndex].CFG_LO = DWC_CFGL_HS_SRC | DWC_CFGL_HS_DST | \
														  (u8_chanPriority << 5);

				/* clear interrupt */
				DMA_clearIRQ(u8_chanIndex);

				/* set interrupt mask */
				s_st_dmaRegs->MASK.TFR = ((1 << u8_chanIndex) | (1 << (u8_chanIndex + 8)));
				s_st_dmaRegs->MASK.BLOCK = 0;
				s_st_dmaRegs->MASK.SRCTRAN = 0;
				s_st_dmaRegs->MASK.DSTTRAN = 0;
				s_st_dmaRegs->MASK.ERROR = 0;
				
				s_st_transStatus[u8_chanIndex].e_transActive = ACTIVE;
				u8_inited = 0;
				break;
			}
		}

		if ((u8_inited != 0) && (u8_chanIndex >= DW_DMA_MAX_NR_CHANNELS) )
		{
			dlog_error("No channel left for DMA!\n");
			return -1;
		}

		s_st_transStatus[u8_chanIndex].trans_complete = 0;
/* 		dlog_info("AUTOchan = %d\n", u8_chanIndex); */
		return u8_chanIndex;
	}
	else 
	{
		if ((s_st_dmaRegs->CH_EN & (1 << u8_channel)) == 0)
		{
			 /* disable the channel */
			s_st_dmaRegs->CHAN[u8_channel].CFG_HI = 0x0;
			s_st_dmaRegs->CHAN[u8_channel].CFG_LO = DWC_CFGL_HS_SRC | DWC_CFGL_HS_DST | \
													  (u8_chanPriority << 5);
			/* clear interrupt */
			DMA_clearIRQ(u8_channel);

			/* set interrupt mask */
			s_st_dmaRegs->MASK.TFR = ((1 << u8_channel) | (1 << (u8_channel + 8)));
			s_st_dmaRegs->MASK.BLOCK = 0;
			s_st_dmaRegs->MASK.ERROR = 0;
			s_st_dmaRegs->MASK.SRCTRAN = 0;
			s_st_dmaRegs->MASK.DSTTRAN = 0;

			s_st_transStatus[u8_channel].e_transActive = ACTIVE;
			u8_inited = 0;
			
			s_st_transStatus[u8_channel].trans_complete = 0;
			
			return u8_channel;
		}

		return -1;
	}
}

uint32_t DMA_transfer(uint32_t u32_srcAddr, uint32_t u32_dstAddr, uint32_t u32_transByteNum, uint8_t u8_chanIndex, ENUM_TransferType e_transType)
{
	uint32_t u32_totalBlkNum = 0;  /* the num of block to be transfered */
	uint32_t u32_llpBaseAddr = 0;
	uint32_t u32_llpLOC = 0; 
	uint32_t u32_blkIndex = 0;
	uint32_t u32_dataCtlLO = 0, u32_dataCtlHI = 0;

#ifndef USE_MALLOC_DESC
	STRU_LinkListItem pst_LinkListItem[32] = {0};
#endif

	switch(e_transType)
	{
		case LINK_LIST_ITEM:
		{
			uint32_t u32_1stTfrBlkSize = 0, u32_2ndTfrBlkSize = 0, u32_3rdTfrBlkSize = 0; /* the block size */
			uint32_t u32_1stHSize = 0, u32_2ndHSize = 0, u32_3rdHSize = 0; /* the length of WIDTH*/
			uint32_t u32_blkSize = DW_CH_MAX_BLK_SIZE * 4; /* blkSize in byte */
			uint32_t u32_blkRes1 = u32_transByteNum % u32_blkSize; /* res1 means the block less than 4095*4 */
			uint32_t u32_blkRes2 = u32_blkRes1 % 4;   /* res2 means the block less than 4*/
			/* the first block = DW_CH_MAX_BLK_SIZE */
			uint32_t u32_1stBlkSize = DW_CH_MAX_BLK_SIZE;    
			/* the second block less than DW_CH_MAX_BLK_SIZE */
			uint32_t u32_2ndBlkSize = (u32_blkRes1 - u32_blkRes2) / 4;
			/* the third block less than 4  */
			uint32_t u32_3rdBlkSize = u32_blkRes2;

			/* the num of first block */
			uint32_t u32_1stBlkNum = (uint32_t)u32_transByteNum / u32_blkSize;
			/* the num of first block */
			uint32_t u32_2ndBlkNUm = (u32_2ndBlkSize > 0) ? 1 : 0;
			/* the num of first block */
			uint32_t u32_3rdBlkNUm = (u32_3rdBlkSize > 0) ? 1 : 0;

			if ( (!u32_1stBlkNum) && (!u32_2ndBlkNUm) && (u32_3rdBlkNUm) )  // 001
		    {
		        u32_totalBlkNum = 1;
		        u32_1stTfrBlkSize = 0;   //BLOCK SIZE
		        u32_2ndTfrBlkSize = 0;
		        u32_3rdTfrBlkSize = u32_3rdBlkSize;
		        u32_1stHSize = 0;
		        u32_2ndHSize = 0;
		        u32_3rdHSize = 0;  //TR_WIDTH
		    }
		    else if ( (!u32_1stBlkNum) && (u32_2ndBlkNUm) && (!u32_3rdBlkNUm) ) //010
		    {
		        u32_totalBlkNum = 1;
		        u32_1stTfrBlkSize = 0;
		        u32_2ndTfrBlkSize = 0;
		        u32_3rdTfrBlkSize = u32_2ndBlkSize;
		        u32_1stHSize = 0;	
		        u32_2ndHSize = 0;
		        u32_3rdHSize = 2;
		    }
		    else if ( (u32_1stBlkNum) && (!u32_2ndBlkNUm) && (!u32_3rdBlkNUm) ) //100
		    {
		        u32_totalBlkNum = u32_1stBlkNum;
		        u32_1stTfrBlkSize = u32_1stBlkSize;
		        u32_2ndTfrBlkSize = u32_1stBlkSize;
		        u32_3rdTfrBlkSize = u32_1stBlkSize;
		        u32_1stHSize = 2;
		        u32_2ndHSize = 2;
		        u32_3rdHSize = 2;
		    }
		    else if ( (!u32_1stBlkNum) && (u32_2ndBlkNUm) && (u32_3rdBlkNUm) ) //011
		    {
		        u32_totalBlkNum = 2;
		        u32_1stTfrBlkSize = 0;
		        u32_2ndTfrBlkSize = u32_2ndBlkSize;
		        u32_3rdTfrBlkSize = u32_3rdBlkSize;
		        u32_1stHSize = 0;
		        u32_2ndHSize = 2;
		        u32_3rdHSize = 0;
		    }
		    else if ( (u32_1stBlkNum) && (!u32_2ndBlkNUm) && (u32_3rdBlkNUm) ) //101
		    {
		        u32_totalBlkNum = u32_1stBlkNum + 1;
		        u32_1stTfrBlkSize = u32_1stBlkSize;
		        u32_2ndTfrBlkSize = u32_1stBlkSize;
		        u32_3rdTfrBlkSize = u32_3rdBlkSize;
		        u32_1stHSize = 2;
		        u32_2ndHSize = 2;
		        u32_3rdHSize = 0;
		    }
		    else if ( (u32_1stBlkNum) && (u32_2ndBlkNUm) && (!u32_3rdBlkNUm) ) //110
		    {
		        u32_totalBlkNum = u32_1stBlkNum + 1;
		        u32_1stTfrBlkSize = u32_1stBlkSize;
		        u32_2ndTfrBlkSize = u32_1stBlkSize;
		        u32_3rdTfrBlkSize = u32_2ndBlkSize;
		        u32_1stHSize = 2;
		        u32_2ndHSize = 2;
		        u32_3rdHSize = 2;
		    }
		    else // if ( (u32_1stBlkNum) && (u32_2ndBlkNUm) && (u32_3rdBlkNUm) ) // 111
		    {
		        u32_totalBlkNum = u32_1stBlkNum + u32_2ndBlkNUm + u32_3rdBlkNUm;
		        u32_1stTfrBlkSize = u32_1stBlkSize;
		        u32_2ndTfrBlkSize = u32_2ndBlkSize;
		        u32_3rdTfrBlkSize = u32_3rdBlkSize;
		        u32_1stHSize = 2;
		        u32_2ndHSize = 2;
		        u32_3rdHSize = 0;
		    }
			
#ifdef USE_MALLOC_DESC
			/* malloc the space for LLI */
			#ifdef DMA_DEBUG
				dlog_info("before malloc, malloc size = %d\n", sizeof(STRU_LinkListItem) * u32_totalBlkNum);
			#endif
			//pst_LinkListItem = (STRU_LinkListItem *)malloc(sizeof(STRU_LinkListItem) * u32_totalBlkNum);
			s_st_transStatus[u8_chanIndex].pst_lliMalloc = (STRU_LinkListItem *)malloc(sizeof(STRU_LinkListItem) * u32_totalBlkNum);
			if (!s_st_transStatus[u8_chanIndex].pst_lliMalloc)
			{
				dlog_info("Malloc Failed! Exit DMA Transfer\n");
				return -1;
			}
			else
			{
				#ifdef DMA_DEBUG
					dlog_info("addr pst_LinkListItem = 0x%08x\n", s_st_transStatus[u8_chanIndex].pst_lliMalloc);
				#endif
			}
#endif

			if (ENUM_CPU0_ID == CPUINFO_GetLocalCpuId())
		    {
		        //SRAM: llp_loc[31:2], store the LLP entry        
				u32_llpBaseAddr = (((uint32_t)s_st_transStatus[u8_chanIndex].pst_lliMalloc + DTCM_CPU0_DMA_ADDR_OFFSET) );  
		    }
		    else if (ENUM_CPU1_ID == CPUINFO_GetLocalCpuId())
		    {
		        //SRAM: llp_loc[31:2], store the LLP entry        
		    	u32_llpBaseAddr = (((uint32_t)s_st_transStatus[u8_chanIndex].pst_lliMalloc + DTCM_CPU1_DMA_ADDR_OFFSET) ); 
			}

		    u32_llpLOC = u32_llpBaseAddr;

		    for (u32_blkIndex = 1; u32_blkIndex <= u32_totalBlkNum; ++u32_blkIndex)
		    {
		        if(u32_blkIndex < (u32_totalBlkNum -1)) // belong to block 1st
		        {
		            u32_dataCtlLO = DWC_CTLL_INT_EN | DWC_CTLL_DMS(0x1) | \
		            				DWC_CTLL_DST_WIDTH(u32_1stHSize) | \
		            				DWC_CTLL_SRC_WIDTH(u32_1stHSize) |\
		            			    DWC_CTLL_DST_MSIZE(0x1) | DWC_CTLL_SRC_MSIZE(0x1) | \
		            			    DWC_CTLL_LLP_D_EN | DWC_CTLL_LLP_S_EN;
		            u32_dataCtlHI = (u32_1stTfrBlkSize & DWC_CTLH_BLOCK_TS_MASK) | \
		            				DWC_CTLH_DONE;
		        }
		        else if(u32_blkIndex == (u32_totalBlkNum -1)) //(the second from the end of the block)
		        {
		            u32_dataCtlLO = DWC_CTLL_INT_EN | DWC_CTLL_DMS(0x1) | \
		            				DWC_CTLL_DST_WIDTH(u32_2ndHSize) | \
		            				DWC_CTLL_SRC_WIDTH(u32_2ndHSize) | \
		            				DWC_CTLL_DST_MSIZE(0x1) | DWC_CTLL_SRC_MSIZE(0x1) | \
		            				DWC_CTLL_LLP_D_EN | DWC_CTLL_LLP_S_EN;
		            u32_dataCtlHI = (u32_2ndTfrBlkSize & DWC_CTLH_BLOCK_TS_MASK) | \
		                            DWC_CTLH_DONE;
		        }
		        else //(the last block)
		        {
		            u32_dataCtlLO = DWC_CTLL_INT_EN | DWC_CTLL_DMS(0x1) | \
		            				DWC_CTLL_DST_WIDTH(u32_3rdHSize) | \
		            				DWC_CTLL_SRC_WIDTH(u32_3rdHSize) | \
		            				DWC_CTLL_DST_MSIZE(0x1) | DWC_CTLL_SRC_MSIZE(0x1) & \
		            				 ~(DWC_CTLL_LLP_D_EN | DWC_CTLL_LLP_S_EN);
		            u32_dataCtlHI = (u32_3rdTfrBlkSize & DWC_CTLH_BLOCK_TS_MASK) | \
		                            DWC_CTLH_DONE;
		        }

			s_st_transStatus[u8_chanIndex].pst_lliMalloc[u32_blkIndex - 1].sar   = u32_srcAddr;
			s_st_transStatus[u8_chanIndex].pst_lliMalloc[u32_blkIndex - 1].dar   = u32_dstAddr;
			s_st_transStatus[u8_chanIndex].pst_lliMalloc[u32_blkIndex - 1].ctllo = u32_dataCtlLO;
			s_st_transStatus[u8_chanIndex].pst_lliMalloc[u32_blkIndex - 1].ctlhi = u32_dataCtlHI;

		        /* setup the initial LLP */										 
				s_st_dmaRegs->CHAN[u8_chanIndex].LLP = u32_llpLOC & 0xFFFFFFFC;

                if(u32_blkIndex < (u32_totalBlkNum -1)) // belong to block 1st
		        {
		            u32_llpLOC += sizeof(STRU_LinkListItem); 
		            u32_srcAddr += 4 * u32_1stTfrBlkSize;
		            u32_dstAddr += 4 * u32_1stTfrBlkSize;
		        }
		        else if (u32_blkIndex == (u32_totalBlkNum -1)) //belong to block 2nd
		        {
		            u32_llpLOC += sizeof(STRU_LinkListItem);
		            u32_srcAddr += 4 * u32_2ndTfrBlkSize;
		            u32_dstAddr += 4 * u32_2ndTfrBlkSize;
		        }
		        else // belong to block 3rd
		        {
		            u32_llpLOC = 0;
		            u32_srcAddr += 4 * u32_3rdTfrBlkSize;
		            u32_dstAddr += 4 * u32_3rdTfrBlkSize;
		        }

				/* setup the next llp baseaddr */
				s_st_transStatus[u8_chanIndex].pst_lliMalloc[u32_blkIndex - 1].llp   = u32_llpLOC & 0xFFFFFFFC;
		    }

#ifdef DMA_DEBUG
			for (int i = 0; i < u32_totalBlkNum; i++)
			{
				dlog_info("item %d, sar = 0x%08x\n", i, s_st_transStatus[u8_chanIndex].pst_lliMalloc[i].sar);
				dlog_info("item %d, dar = 0x%08x\n", i, s_st_transStatus[u8_chanIndex].pst_lliMalloc[i].dar);
				dlog_info("item %d, llp = 0x%08x\n", i, s_st_transStatus[u8_chanIndex].pst_lliMalloc[i].llp);
				dlog_info("item %d, ctllo = 0x%08x\n", i, s_st_transStatus[u8_chanIndex].pst_lliMalloc[i].ctllo);
				dlog_info("item %d, ctlhi = 0x%08x\n", i, s_st_transStatus[u8_chanIndex].pst_lliMalloc[i].ctlhi);
			}
#endif /* DMA_DEBUG */

		    /* setup the initial CTL */
		    s_st_dmaRegs->CHAN[u8_chanIndex].CTL_LO = DWC_CTLL_DMS(0x1) | \
		    										  DWC_CTLL_INT_EN | \
		    										  DWC_CTLL_DST_WIDTH(0x2) | DWC_CTLL_SRC_WIDTH(0x2) | \
		    										  DWC_CTLL_DST_MSIZE(0x1) | DWC_CTLL_SRC_MSIZE(0x1) | \
		    										  DWC_CTLL_LLP_D_EN | DWC_CTLL_LLP_S_EN;;
		    s_st_dmaRegs->CHAN[u8_chanIndex].CTL_HI = DW_CH_MAX_BLK_SIZE & DWC_CTLH_BLOCK_TS_MASK |\
		    										  DWC_CTLH_DONE;
		    s_st_dmaRegs->CHAN[u8_chanIndex].LLP    =  u32_llpBaseAddr  & 0xFFFFFFFC;


		    s_st_transStatus[u8_chanIndex].e_transferType = LINK_LIST_ITEM;
		    s_st_transStatus[u8_chanIndex].u32_transNum = u32_totalBlkNum;

			/* setup the DmaCfgReg */
			s_st_dmaRegs->DMA_CFG = DW_CFG_DMA_EN;
			s_st_dmaRegs->CH_EN   = DWC_CH_EN(u8_chanIndex) | DWC_CH_EN_WE(u8_chanIndex);

			break;
		}
		case AUTO_RELOAD:
		{
			uint32_t u32_blkSize = DW_CH_RELOAD_BLK_SIZE * 4; /* blkSize in byte */
			uint32_t u32_reloadNum = u32_transByteNum / u32_blkSize; /* res1 means the block less than 4095*4 */
		
		    /* setup the initial CTL */
		    s_st_dmaRegs->CHAN[u8_chanIndex].CTL_LO = DWC_CTLL_DMS(0x1) | \
		    										  DWC_CTLL_INT_EN | \
		    										  DWC_CTLL_DST_WIDTH(0x2) | DWC_CTLL_SRC_WIDTH(0x2) | \
		    										  DWC_CTLL_DST_MSIZE(0x1) | DWC_CTLL_SRC_MSIZE(0x1) | \
		    										  DWC_CTLL_DST_INC | DWC_CTLL_SRC_INC;
		    s_st_dmaRegs->CHAN[u8_chanIndex].CTL_HI = DW_CH_RELOAD_BLK_SIZE & DWC_CTLH_BLOCK_TS_MASK |\
		    										  DWC_CTLH_DONE;
		    s_st_dmaRegs->CHAN[u8_chanIndex].SAR    = u32_srcAddr;
		    s_st_dmaRegs->CHAN[u8_chanIndex].DAR    = u32_dstAddr;

		    s_st_dmaRegs->CHAN[u8_chanIndex].LLP    = 0x0;

		    s_st_dmaRegs->CHAN[u8_chanIndex].CFG_HI = 0x0;

		    s_st_transStatus[u8_chanIndex].u32_srcTranAddr = u32_srcAddr;
		    s_st_transStatus[u8_chanIndex].u32_dstTranAddr = u32_dstAddr;
		    s_st_transStatus[u8_chanIndex].u32_blkSize = u32_blkSize;


		    if (u32_reloadNum == 1)
		    {
				s_st_dmaRegs->CHAN[u8_chanIndex].CFG_LO = DWC_CFGL_HS_SRC | DWC_CFGL_HS_DST;
		    }
		    else
		    {
				s_st_dmaRegs->CHAN[u8_chanIndex].CFG_LO = DWC_CFGL_RELOAD_SAR | DWC_CFGL_RELOAD_DAR | \
										                  DWC_CFGL_HS_SRC | DWC_CFGL_HS_DST;
		    }
		    // s_st_dmaRegs->CHAN[u8_chanIndex].CFG_HI = DWC_CFGH_FCMODE | DWC_CFGH_FIFO_MODE;
		    s_st_transStatus[u8_chanIndex].e_transferType = AUTO_RELOAD;
			s_st_transStatus[u8_chanIndex].u32_transNum = u32_reloadNum;

		    /* setup the DmaCfgReg */
		    s_st_dmaRegs->DMA_CFG = DW_CFG_DMA_EN;
		    s_st_dmaRegs->CH_EN   = DWC_CH_EN(u8_chanIndex) | DWC_CH_EN_WE(u8_chanIndex);
			break;
		}
		default:
		{
			break;
		}
	}
	return 0;
}

uint32_t DMA_getStatus(uint8_t u8_chanIndex)
{
	return s_st_transStatus[u8_chanIndex].trans_complete;
}

uint32_t DMA_forDriverTransfer(uint32_t u32_srcAddr, uint32_t u32_dstAddr, uint32_t u32_transByteNum, 
											ENUM_blockMode e_blockMode, uint32_t u32_ms)
{
	uint8_t u8_chanIndex;
	uint8_t u8_chanInitFlag = 0;
	uint32_t u32_start, u32_end;
	
	for (u8_chanIndex = 7; u8_chanIndex >= 4; u8_chanIndex--)
	{
		/* channel 7 priority 0, channel 6 priority 1, channel 5 priority 2, channel 4 priority 3 */
		if (DMA_Init(u8_chanIndex, DMA_LOWEST_PRIORITY - u8_chanIndex) >= 0 )
		{
			DMA_transfer(u32_srcAddr, u32_dstAddr, u32_transByteNum, u8_chanIndex, LINK_LIST_ITEM);
			break;
		}
		else
		{
			dlog_info("line = %d, no channel for channel %d\n", __LINE__, u8_chanIndex);
		}
	}

	if (u8_chanIndex < 4)
	{
		dlog_info("line = %d, all 4 channel occupied!\n", __LINE__);
		return -1;
	}


	switch (e_blockMode)
	{
		case DMA_blocked:
			while( DMA_getStatus(u8_chanIndex)  ==  0);
			break;

		case DMA_noneBlocked:
			break;

		case DMA_blockTimer:
			u32_start = SysTicks_GetTickCount();
			u32_end = u32_start + u32_ms;
			while(SysTicks_GetTickCount() <= u32_end)
			{
				if (DMA_getStatus(u8_chanIndex) == 1)
				{
					break;
				}
			}
			break;

		default: break;
	}

	return 0;
}
