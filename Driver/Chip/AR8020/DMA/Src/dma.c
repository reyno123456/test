/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_sd.h
Description: The external HAL APIs to use the SDMMC controller.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2017/1/14
History: 
        0.0.1    2017/1/14    The initial version of dma.c
*****************************************************************************/
#include "dma.h"
#include "debuglog.h"
#include "reg_rw.h"
#include "cpu_info.h"
#include "interrupt.h"
#include "memory_config.h"
#include "cmsis_os.h"
#include "systicks.h"

volatile STRU_DmaRegs *g_st_dmaRegs = (STRU_DmaRegs *)DMA_BASE;

volatile STRU_transStatus g_st_transStatus[8] = {0};
/**
  * @brief  Reports the name of the source file and the source line number
  *         where the assert_param error has occurred.
  * @param  file: pointer to the source file name
  * @param  line: assert_param error line source number
  * @retval None
  */
void assert_failed(uint8_t* file, uint32_t line)
{
	dlog_info("wrong paraments\n");
}

void DMA_clearIRQ(uint8_t u8_index)
{
	g_st_dmaRegs->CLEAR.TFR = (1 << u8_index);
	g_st_dmaRegs->CLEAR.BLOCK = (1 << u8_index);
	g_st_dmaRegs->CLEAR.SRCTRAN = (1 << u8_index);
	g_st_dmaRegs->CLEAR.DSTTRAN = (1 << u8_index);
	g_st_dmaRegs->CLEAR.ERROR = (1 << u8_index);
}

void DMA_irqISR(uint32_t vectorNum)
{
	uint8_t u8_chanIndex = 0;
	uint32_t index = 0;
    // dlog_info("single block transfer completed!\n");

	for (u8_chanIndex = 0; u8_chanIndex < DW_DMA_MAX_NR_CHANNELS; ++u8_chanIndex)
	{
		if (g_st_transStatus[u8_chanIndex].e_transActive == ACTIVE)
		{
			switch(g_st_transStatus[u8_chanIndex].e_transferType)
			{
				case LINK_LIST_ITEM:
				{
					// dlog_info("LINK_LIST_ITEM\n");
					// if ((g_st_dmaRegs->CH_EN & (1 << u8_chanIndex)) == 0)
					// {	
						/* disable the channel */
						g_st_dmaRegs->CHAN[u8_chanIndex].CFG_HI = 0x0;
						g_st_dmaRegs->CHAN[u8_chanIndex].CFG_LO = DWC_CFGL_HS_SRC | DWC_CFGL_HS_DST;
						
						/* clear interrupt */
					    DMA_clearIRQ(u8_chanIndex);
					// }
					break;
				}
				case AUTO_RELOAD:
				{
					// dlog_info("AUTO_RELOAD\n");
					g_st_transStatus[u8_chanIndex].u32_transNum--;
					if (g_st_transStatus[u8_chanIndex].u32_transNum <= 1)
					{
						/* disable the channel auto-reload */
						// dlog_info("u32_transNum-1 = %d\n", g_st_transStatus[u8_chanIndex].u32_transNum);
						g_st_dmaRegs->CHAN[u8_chanIndex].CFG_HI = 0x0;
						g_st_dmaRegs->CHAN[u8_chanIndex].CFG_LO = DWC_CFGL_HS_SRC | DWC_CFGL_HS_DST;
					}
					/* clear interrupt */
					DMA_clearIRQ(u8_chanIndex);
					break;
				}
				default:
					break;
			}

			if ((g_st_dmaRegs->CH_EN & (1 << u8_chanIndex)) == 0)
			{
				// dlog_info("transfered completed!\n");
				g_st_transStatus[u8_chanIndex].e_transActive = NON_ACTIVE;
				break;
			}
		}

	}

    return;
}


void DMA_initIRQ()
{
    /* register the irq handler */
    reg_IrqHandle(DMA_INTR_N_VECTOR_NUM, DMA_irqISR, NULL);
    INTR_NVIC_EnableIRQ(DMA_INTR_N_VECTOR_NUM);
    INTR_NVIC_SetIRQPriority(DMA_INTR_N_VECTOR_NUM, 1);
}

int32_t DMA_Init(ENUM_Chan u8_channel, uint8_t u8_chanPriority)
{
	uint8_t u8_chanIndex = 0;
	uint8_t u8_inited = 1;

	assert_param(IS_CHANNAL_PRIORITY(u8_chanPriority));
	DMA_initIRQ();

	if (u8_channel == AUTO)
	{
		/* find out which channel is idle */
		for (u8_chanIndex = 0; u8_chanIndex < DW_DMA_MAX_NR_CHANNELS; ++u8_chanIndex)
		{
			if ((g_st_dmaRegs->CH_EN & (1 << u8_chanIndex)) == 0)
			{
				 /* disable the channel */
				g_st_dmaRegs->CHAN[u8_chanIndex].CFG_HI = 0x0;
				g_st_dmaRegs->CHAN[u8_chanIndex].CFG_LO = DWC_CFGL_HS_SRC | DWC_CFGL_HS_DST | \
														  (u8_chanPriority << 5);
				/* clear interrupt */
				DMA_clearIRQ(u8_chanIndex);

				/* set interrupt mask */
				g_st_dmaRegs->MASK.TFR = ((1 << u8_chanIndex) | (1 << (u8_chanIndex + 8)));
				g_st_dmaRegs->MASK.BLOCK = ((1 << u8_chanIndex) | (1 << (u8_chanIndex + 8)));
				g_st_dmaRegs->MASK.ERROR = ((1 << u8_chanIndex) | (1 << (u8_chanIndex + 8)));
				g_st_dmaRegs->MASK.SRCTRAN = 0x0;
				g_st_dmaRegs->MASK.DSTTRAN = 0x0;

				g_st_transStatus[u8_chanIndex].e_transActive = ACTIVE;
				u8_inited = 0;
				break;
			}
		}

		if ((u8_inited != 0) && (u8_chanIndex >= DW_DMA_MAX_NR_CHANNELS) )
		{
			dlog_error("No channel left for DMA!\n");
			return -1;
		}
		
		dlog_info("AUTOchan = %d\n", u8_chanIndex);
		return u8_chanIndex;
	}
	else 
	{
		if ((g_st_dmaRegs->CH_EN & (1 << u8_channel)) == 0)
		{
			 /* disable the channel */
			g_st_dmaRegs->CHAN[u8_channel].CFG_HI = 0x0;
			g_st_dmaRegs->CHAN[u8_channel].CFG_LO = DWC_CFGL_HS_SRC | DWC_CFGL_HS_DST | \
													  (u8_chanPriority << 5);
			/* clear interrupt */
			DMA_clearIRQ(u8_channel);

			/* set interrupt mask */
			g_st_dmaRegs->MASK.TFR = ((1 << u8_channel) | (1 << (u8_channel + 8)));
			g_st_dmaRegs->MASK.BLOCK = ((1 << u8_channel) | (1 << (u8_channel + 8)));
			g_st_dmaRegs->MASK.ERROR = ((1 << u8_channel) | (1 << (u8_channel + 8)));
			g_st_dmaRegs->MASK.SRCTRAN = 0x0;
			g_st_dmaRegs->MASK.DSTTRAN = 0x0;

			g_st_transStatus[u8_channel].e_transActive = ACTIVE;
			u8_inited = 0;
		}

		if ((u8_inited != 0) && (u8_channel >= DW_DMA_MAX_NR_CHANNELS) )
		{
			dlog_error("No channel left for DMA!\n");
			return -1;
		}

		dlog_info("chan = %d\n", u8_channel);
		return u8_channel;
	}

	// if ((u8_inited != 0) && (u8_chanIndex >= DW_DMA_MAX_NR_CHANNELS) )
	// {
	// 	dlog_error("No channel left for DMA!\n");
	// 	return -1;
	// }
	// dlog_info("chan = %d\n", u8_chanIndex);
	// return u8_chanIndex;
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
#ifdef DMA_DEBUG
			/* malloc the space for LLI */
				dlog_info("before malloc, malloc size = %d\n", sizeof(STRU_LinkListItem) * u32_totalBlkNum);
				dlog_output(100);
				STRU_LinkListItem *pst_LinkListItem = (STRU_LinkListItem *)malloc(sizeof(STRU_LinkListItem) * u32_totalBlkNum);
				if (pst_LinkListItem)
				{
					dlog_info("addr pst_LinkListItem = 0x%08x\n", pst_LinkListItem);
				}
				else
				{
					dlog_info("Malloc Failed! Exit DMA Transfer\n");
					return -1;
				}
#endif /* DMA_DEBUG */
#endif

			if (ENUM_CPU0_ID == CPUINFO_GetLocalCpuId())
		    {
		        //SRAM: llp_loc[31:2], store the LLP entry        
		        // u32_llpBaseAddr = (((uint32_t)pst_LinkListItem + DTCM_CPU0_DMA_ADDR_OFFSET) >> 2);
				u32_llpBaseAddr = (((uint32_t)pst_LinkListItem + DTCM_CPU0_DMA_ADDR_OFFSET) );  
		    }
		    else if (ENUM_CPU1_ID == CPUINFO_GetLocalCpuId())
		    {
		        //SRAM: llp_loc[31:2], store the LLP entry        
		        // u32_llpBaseAddr = (((uint32_t)pst_LinkListItem + DTCM_CPU1_DMA_ADDR_OFFSET) >> 2); 
		    	u32_llpBaseAddr = (((uint32_t)pst_LinkListItem + DTCM_CPU1_DMA_ADDR_OFFSET) ); 
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

		        pst_LinkListItem[u32_blkIndex - 1].sar   = u32_srcAddr;
		        pst_LinkListItem[u32_blkIndex - 1].dar   = u32_dstAddr;
		        pst_LinkListItem[u32_blkIndex - 1].ctllo = u32_dataCtlLO;
		        pst_LinkListItem[u32_blkIndex - 1].ctlhi = u32_dataCtlHI;

		        /* setup the initial LLP */										 
		    	// g_st_dmaRegs->CHAN[u8_chanIndex].LLP = u32_llpLOC << 2;
				g_st_dmaRegs->CHAN[u8_chanIndex].LLP = u32_llpLOC & 0xFFFFFFFC;

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
		        // pst_LinkListItem[u32_blkIndex - 1].llp   = (u32_llpLOC << 2);
				pst_LinkListItem[u32_blkIndex - 1].llp   = u32_llpLOC & 0xFFFFFFFC;
		    }

			int i = 0;
#ifdef DMA_DEBUG
			for (i = 0; i < u32_totalBlkNum; i++)
			{
				dlog_info("item %d, sar = 0x%08x\n", i, pst_LinkListItem[i].sar);
				dlog_info("item %d, dar = 0x%08x\n", i, pst_LinkListItem[i].dar);
				dlog_info("item %d, llp = 0x%08x\n", i, pst_LinkListItem[i].llp);
				dlog_info("item %d, ctllo = 0x%08x\n", i, pst_LinkListItem[i].ctllo);
				dlog_info("item %d, ctlhi = 0x%08x\n", i, pst_LinkListItem[i].ctlhi);
			}
			dlog_output(1000);
#endif /* DMA_DEBUG */

		    /* setup the initial CTL */
		    g_st_dmaRegs->CHAN[u8_chanIndex].CTL_LO = DWC_CTLL_DMS(0x1) | \
		    										  DWC_CTLL_INT_EN | \
		    										  DWC_CTLL_DST_WIDTH(0x2) | DWC_CTLL_SRC_WIDTH(0x2) | \
		    										  DWC_CTLL_DST_MSIZE(0x1) | DWC_CTLL_SRC_MSIZE(0x1) | \
		    										  DWC_CTLL_LLP_D_EN | DWC_CTLL_LLP_S_EN;;
		    g_st_dmaRegs->CHAN[u8_chanIndex].CTL_HI = DW_CH_MAX_BLK_SIZE & DWC_CTLH_BLOCK_TS_MASK |\
		    										  DWC_CTLH_DONE;
		    // g_st_dmaRegs->CHAN[u8_chanIndex].LLP    =  (u32_llpBaseAddr << 2);
			g_st_dmaRegs->CHAN[u8_chanIndex].LLP    =  u32_llpBaseAddr  & 0xFFFFFFFC;

		    /* setup the DmaCfgReg */
		    g_st_dmaRegs->DMA_CFG = DW_CFG_DMA_EN;
		    g_st_dmaRegs->CH_EN   = DWC_CH_EN(u8_chanIndex) | DWC_CH_EN_WE(u8_chanIndex);

		    g_st_transStatus[u8_chanIndex].e_transferType = LINK_LIST_ITEM;
		    g_st_transStatus[u8_chanIndex].u32_transNum = u32_totalBlkNum;

#ifdef DMA_DEBUG
			i = 0;
			uint32_t tick_count_old = SysTicks_GetTickCount();
			uint32_t tick_count_new = 0;
#endif /* DMA_DEBUG */
		    while(1)
		    {
			    if ((g_st_dmaRegs->CH_EN & (1 << u8_chanIndex)) == 0)
				{
#ifdef USE_MALLOC_DESC
#ifdef DMA_DEBUG
					dlog_info("before free addr pst_LinkListItem = 0x%08x\n", pst_LinkListItem);
	  				free(pst_LinkListItem);				
#endif /* DMA_DEBUG */
#endif
					break;
				}
				if (ENUM_CPU0_ID == CPUINFO_GetLocalCpuId()) osDelay(1);
				i++;
		    }

#ifdef DMA_DEBUG
			tick_count_new = SysTicks_GetTickCount();

			if (tick_count_new >= tick_count_old){
				dlog_info("dma delayed %d ticks, line = %d\n", tick_count_new - tick_count_old, __LINE__);
			} else {
				dlog_info("dma delayed out of range, line = %d\n", __LINE__);
			}
			
		    dlog_info("dma delayed %d times, line = %d\n", i, __LINE__);
			dlog_output(100);
#endif /* DMA_DEBUG */

  			break;
		}
		case AUTO_RELOAD:
		{
			uint32_t u32_blkSize = DW_CH_RELOAD_BLK_SIZE * 4; /* blkSize in byte */
			uint32_t u32_reloadNum = u32_transByteNum / u32_blkSize; /* res1 means the block less than 4095*4 */
		
		    /* setup the initial CTL */
		    g_st_dmaRegs->CHAN[u8_chanIndex].CTL_LO = DWC_CTLL_DMS(0x1) | \
		    										  DWC_CTLL_INT_EN | \
		    										  DWC_CTLL_DST_WIDTH(0x2) | DWC_CTLL_SRC_WIDTH(0x2) | \
		    										  DWC_CTLL_DST_MSIZE(0x1) | DWC_CTLL_SRC_MSIZE(0x1) | \
		    										  DWC_CTLL_DST_INC | DWC_CTLL_SRC_INC;
		    g_st_dmaRegs->CHAN[u8_chanIndex].CTL_HI = DW_CH_RELOAD_BLK_SIZE & DWC_CTLH_BLOCK_TS_MASK |\
		    										  DWC_CTLH_DONE;
		    g_st_dmaRegs->CHAN[u8_chanIndex].SAR    = u32_srcAddr;
		    g_st_dmaRegs->CHAN[u8_chanIndex].DAR    = u32_dstAddr;

		    g_st_dmaRegs->CHAN[u8_chanIndex].LLP    = 0x0;

		    g_st_dmaRegs->CHAN[u8_chanIndex].CFG_HI = 0x0;

		    g_st_transStatus[u8_chanIndex].u32_srcTranAddr = u32_srcAddr;
		    g_st_transStatus[u8_chanIndex].u32_dstTranAddr = u32_dstAddr;
		    g_st_transStatus[u8_chanIndex].u32_blkSize = u32_blkSize;


		    if (u32_reloadNum == 1)
		    {
				g_st_dmaRegs->CHAN[u8_chanIndex].CFG_LO = DWC_CFGL_HS_SRC | DWC_CFGL_HS_DST;
		    }
		    else
		    {
				g_st_dmaRegs->CHAN[u8_chanIndex].CFG_LO = DWC_CFGL_RELOAD_SAR | DWC_CFGL_RELOAD_DAR | \
										                  DWC_CFGL_HS_SRC | DWC_CFGL_HS_DST;
		    }
		    // g_st_dmaRegs->CHAN[u8_chanIndex].CFG_HI = DWC_CFGH_FCMODE | DWC_CFGH_FIFO_MODE;
	        g_st_transStatus[u8_chanIndex].e_transferType = AUTO_RELOAD;
		    g_st_transStatus[u8_chanIndex].u32_transNum = u32_reloadNum;

		    /* setup the DmaCfgReg */
		    g_st_dmaRegs->DMA_CFG = DW_CFG_DMA_EN;
		    g_st_dmaRegs->CH_EN   = DWC_CH_EN(u8_chanIndex) | DWC_CH_EN_WE(u8_chanIndex);
			break;
		}
		default:
		{
			break;
		}
	}
	return 0;
}
