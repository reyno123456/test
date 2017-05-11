/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: dma.c
Description: The external HAL APIs to use the SDMMC controller.
Author: wumin @ Artosyn Software Team
Version: 0.0.1
Date: 2017/1/14
History: 
        0.0.1    2017/1/14    The initial version of dma.c
        0.1.1    2017/3/31    Version capability for RTOS
        0.1.2    2017/5/9     intergrate signle block mode
                              universe func DMA_transfer
                              added single block transfer mode
                              channel cfg_low ip have been fixed by 0x18B1FFF5
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

typedef struct
{
    uint32_t totalBlkNum;
    
    uint32_t tfr_1_size; 
    uint32_t tfr_2_size; 
    uint32_t tfr_3_size;
    
    uint8_t tfr_1_width; 
    uint8_t tfr_2_width; 
    uint8_t tfr_3_width;
}STRU_DMA_DESCRIPTER;

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
    s_st_dmaRegs->CLEAR.TFR |= (1 << u8_index);
}

static void dmaIntRegDisplay(void)
{
    dlog_info("%d s_st_dmaRegs->RAW.BLOCK = 0x%08x", __LINE__, s_st_dmaRegs->RAW.BLOCK);
    dlog_info("%d s_st_dmaRegs->RAW.TFR = 0x%08x", __LINE__, s_st_dmaRegs->RAW.TFR);
    dlog_info("%d s_st_dmaRegs->RAW.ERROR = 0x%08x", __LINE__, s_st_dmaRegs->RAW.ERROR);
    dlog_info("%d s_st_dmaRegs->RAW.SRCTRAN = 0x%08x", __LINE__, s_st_dmaRegs->RAW.SRCTRAN);
    dlog_info("%d s_st_dmaRegs->RAW.DSTTRAN = 0x%08x", __LINE__, s_st_dmaRegs->RAW.DSTTRAN);
    
    dlog_info("%d s_st_dmaRegs->STATUS.BLOCK = 0x%08x", __LINE__, s_st_dmaRegs->STATUS.BLOCK);
    dlog_info("%d s_st_dmaRegs->STATUS.TFR = 0x%08x", __LINE__, s_st_dmaRegs->STATUS.TFR);
    dlog_info("%d s_st_dmaRegs->STATUS.ERROR = 0x%08x", __LINE__, s_st_dmaRegs->STATUS.ERROR);
    dlog_info("%d s_st_dmaRegs->STATUS.SRCTRAN = 0x%08x", __LINE__, s_st_dmaRegs->STATUS.SRCTRAN);
    dlog_info("%d s_st_dmaRegs->STATUS.DSTTRAN = 0x%08x", __LINE__, s_st_dmaRegs->STATUS.DSTTRAN);


    dlog_info("%d s_st_dmaRegs->MASK.BLOCK = 0x%08x", __LINE__, s_st_dmaRegs->MASK.BLOCK);
    dlog_info("%d s_st_dmaRegs->MASK.TFR = 0x%08x", __LINE__, s_st_dmaRegs->MASK.TFR);
}

static void DMA_irqISR(uint32_t vectorNum)
{
    volatile uint8_t u8_chanIndex = 0;
    volatile uint8_t index = 0;
    volatile uint8_t tmp_tfr;
    volatile uint8_t flag = 0;
    
    tmp_tfr = (uint8_t)s_st_dmaRegs->STATUS.TFR;
    if ( tmp_tfr == 0 )
    {
        dlog_info("error!");
    }
#if 0
    uint32_t *tmp_ptr = NULL;
    tmp_ptr = malloc(1);
    if (tmp_ptr)
    {
        dlog_info("%d tmp_ptr = 0x%08x", __LINE__, tmp_ptr);
        free(tmp_ptr);
    }
#endif

#if 0
    if( tmp_tfr != 0 )
    {
        u8_chanIndex = DMA_getChannel(tmp_tfr);
        if (u8_chanIndex >= 8)
        {
            dlog_error("%d. error u8_chanIndex = %d", __LINE__, u8_chanIndex );
            while(1) dmaIntRegDisplay();
        }
    }
#endif

#if 1
    for (index  = 0; index < 8; index++)
    {
        if (tmp_tfr & (1 << index))
        {
            u8_chanIndex = index;

            if (++flag >= 2)
            {
                dlog_info("%d flag = %d more channel interrupted\n", __LINE__, flag);
/*                 while(1) dmaIntRegDisplay(); */
            }
            
            switch(s_st_transStatus[u8_chanIndex].e_transferType)
            {
                case SINGLE_BLOCK:
                {
                    DMA_clearIRQ(u8_chanIndex);
                    s_st_dmaRegs->CH_EN &=~ ((1 << (u8_chanIndex)) | (1 << (u8_chanIndex +8)));
                    s_st_transStatus[u8_chanIndex].trans_complete = 1;
                    
                    break;
                }
                case LINK_LIST_ITEM:
                {            
                    free(s_st_transStatus[u8_chanIndex].pst_lliMalloc);
                    DMA_clearIRQ(u8_chanIndex);
                    s_st_dmaRegs->CH_EN &=~ ((1 << (u8_chanIndex)) | (1 << (u8_chanIndex +8)));
                    s_st_transStatus[u8_chanIndex].trans_complete = 1;
            
                    break;
                }
                default: break;
            }
        }
    }
#endif

#if 0
    for (index  = 0; index < 8; index++)
    {
        if (tmp_tfr & (1 << index))
        {
            u8_chanIndex = index;

            if (++flag >= 2)
            {
                dlog_info("%d flag = %d more channel interrupted\n", __LINE__, flag);
                while(1) dmaIntRegDisplay();
            }
        }
    }
    
    switch(s_st_transStatus[u8_chanIndex].e_transferType)
    {
        case SINGLE_BLOCK:
        {
            DMA_clearIRQ(u8_chanIndex);
            s_st_dmaRegs->CH_EN &=~ ((1 << (u8_chanIndex)) | (1 << (u8_chanIndex +8)));
            s_st_transStatus[u8_chanIndex].trans_complete = 1;
            
            break;
        }
        case LINK_LIST_ITEM:
        {            
            free(s_st_transStatus[u8_chanIndex].pst_lliMalloc);
            DMA_clearIRQ(u8_chanIndex);
            s_st_dmaRegs->CH_EN &=~ ((1 << (u8_chanIndex)) | (1 << (u8_chanIndex +8)));
            s_st_transStatus[u8_chanIndex].trans_complete = 1;
    
            break;
        }
        default: break;
    }
#endif
}

static void assert_failed(uint8_t* file, uint32_t line)
{
    dlog_info("wrong paraments\n");
}

static uint8_t dma_prepare_LL_pre_Config(uint32_t u32_transByteNum, STRU_DMA_DESCRIPTER *s_configure)
{
    uint32_t u32_1stTfrSize;
    uint32_t u32_2stTfrSize;
    uint32_t u32_3stTfrSize;

    uint32_t rest;

    uint32_t u32_1stBlkNum = u32_transByteNum / AR_DW_CH_MAX_BLK_SIZE;
    rest = u32_transByteNum - u32_1stBlkNum*AR_DW_CH_MAX_BLK_SIZE;
    uint32_t u32_2stBlkNum = rest / 4;
    uint32_t u32_3stBlkNum = rest - u32_2stBlkNum*4;

    
    if ( (!u32_1stBlkNum) && (!u32_2stBlkNum) && (u32_3stBlkNum) )    // 001
    {
        s_configure->totalBlkNum = 1;
        s_configure->tfr_3_size = u32_3stBlkNum;
        s_configure->tfr_3_width = 0;
    }
    else if ( (!u32_1stBlkNum) && (u32_2stBlkNum) && (!u32_3stBlkNum) ) //010
    {
        s_configure->totalBlkNum = 1;
        s_configure->tfr_3_size = u32_2stBlkNum;
        s_configure->tfr_3_width = 2;
    }
    else if ( (u32_1stBlkNum) && (!u32_2stBlkNum) && (!u32_3stBlkNum) ) //100
    {
        s_configure->totalBlkNum = u32_1stBlkNum;
        s_configure->tfr_1_size = DW_CH_MAX_BLK_SIZE;
        s_configure->tfr_2_size = DW_CH_MAX_BLK_SIZE;
        s_configure->tfr_3_size = DW_CH_MAX_BLK_SIZE;
        s_configure->tfr_1_width = 2;
        s_configure->tfr_2_width = 2;
        s_configure->tfr_3_width = 2;
    }
    else if ( (!u32_1stBlkNum) && (u32_2stBlkNum) && (u32_3stBlkNum) ) //011
    {
        s_configure->totalBlkNum = 2;
        s_configure->tfr_2_size = u32_2stBlkNum;
        s_configure->tfr_3_size = u32_3stBlkNum;
        s_configure->tfr_2_width = 2;
        s_configure->tfr_3_width = 0;
    }
    else if ( (u32_1stBlkNum) && (!u32_2stBlkNum) && (u32_3stBlkNum) ) //101
    {
        s_configure->totalBlkNum = u32_1stBlkNum + 1;
        s_configure->tfr_1_size = DW_CH_MAX_BLK_SIZE;            // from Z to A
        s_configure->tfr_2_size = DW_CH_MAX_BLK_SIZE;            // from Z to A
        s_configure->tfr_3_size = u32_3stBlkNum;
        s_configure->tfr_1_width = 2;
        s_configure->tfr_2_width = 2;
        s_configure->tfr_3_width = 0;
    }
    else if ( (u32_1stBlkNum) && (u32_2stBlkNum) && (!u32_3stBlkNum) ) //110
    {
        s_configure->totalBlkNum = u32_1stBlkNum + 1;            // from Z to A
        s_configure->tfr_1_size = DW_CH_MAX_BLK_SIZE;
        s_configure->tfr_2_size = DW_CH_MAX_BLK_SIZE;
        s_configure->tfr_3_size = u32_2stBlkNum;
        s_configure->tfr_1_width = 2;
        s_configure->tfr_2_width = 2;
        s_configure->tfr_3_width = 2;
    }
    else // if ( (u32_1stBlkNum) && (u32_2ndBlkNUm) && (u32_3rdBlkNUm) ) // 111
    {
        s_configure->totalBlkNum = u32_1stBlkNum + u32_2stBlkNum + u32_3stBlkNum;
        s_configure->tfr_1_size = DW_CH_MAX_BLK_SIZE;
        s_configure->tfr_2_size = u32_2stBlkNum;
        s_configure->tfr_3_size = u32_3stBlkNum;
        s_configure->tfr_1_width = 2;
        s_configure->tfr_2_width = 2;
        s_configure->tfr_3_width = 0;
    }

    return 0;
}

static uint8_t dma_prepare_LL_Config(STRU_DMA_DESCRIPTER *s_configure, 
                                                uint8_t u8_chanIndex, uint32_t u32_llpLOC, 
                                                uint32_t u32_srcAddr, uint32_t u32_dstAddr)
{
    uint32_t u32_blkIndex;
    uint32_t u32_dataCtlLO = 0;
    uint32_t u32_dataCtlHI = 0;

    uint32_t u32_totalBlkNum = s_configure->totalBlkNum;
    uint32_t u32_1stTfrBlkSize = s_configure->tfr_1_size;
    uint32_t u32_2ndTfrBlkSize = s_configure->tfr_2_size;
    uint32_t u32_3rdTfrBlkSize = s_configure->tfr_3_size;
    uint32_t u32_1stHSize = s_configure->tfr_1_width;
    uint32_t u32_2ndHSize = s_configure->tfr_2_width;
    uint32_t u32_3rdHSize = s_configure->tfr_3_width;

    for (u32_blkIndex = 1; u32_blkIndex <= u32_totalBlkNum; ++u32_blkIndex)
    {
        if(u32_blkIndex < (s_configure->totalBlkNum -1)) // belong to block 1st
        {
            u32_dataCtlLO = DWC_CTLL_INT_EN | DWC_CTLL_DMS(0x1) | 
                            DWC_CTLL_DST_WIDTH(u32_1stHSize) | 
                            DWC_CTLL_SRC_WIDTH(u32_1stHSize) |
                            DWC_CTLL_DST_MSIZE(0x1) | DWC_CTLL_SRC_MSIZE(0x1) | 
                            DWC_CTLL_LLP_D_EN | DWC_CTLL_LLP_S_EN;
            u32_dataCtlHI = (u32_1stTfrBlkSize & DWC_CTLH_BLOCK_TS_MASK) | 
                            DWC_CTLH_DONE;
        }
        else if(u32_blkIndex == (u32_totalBlkNum -1)) //(the second from the end of the block)
        {
        u32_dataCtlLO = DWC_CTLL_INT_EN | DWC_CTLL_DMS(0x1) | 
                       DWC_CTLL_DST_WIDTH(u32_2ndHSize) | 
                       DWC_CTLL_SRC_WIDTH(u32_2ndHSize) | 
                       DWC_CTLL_DST_MSIZE(0x1) | DWC_CTLL_SRC_MSIZE(0x1) | 
                       DWC_CTLL_LLP_D_EN | DWC_CTLL_LLP_S_EN;
        u32_dataCtlHI = (u32_2ndTfrBlkSize & DWC_CTLH_BLOCK_TS_MASK) | 
                       DWC_CTLH_DONE;
        }
        else //(the last block)
        {
        u32_dataCtlLO = DWC_CTLL_INT_EN | DWC_CTLL_DMS(0x1) | 
                       DWC_CTLL_DST_WIDTH(u32_3rdHSize) | 
                       DWC_CTLL_SRC_WIDTH(u32_3rdHSize) | 
                       DWC_CTLL_DST_MSIZE(0x1) | DWC_CTLL_SRC_MSIZE(0x1) & 
                        ~(DWC_CTLL_LLP_D_EN | DWC_CTLL_LLP_S_EN);
        u32_dataCtlHI = (u32_3rdTfrBlkSize & DWC_CTLH_BLOCK_TS_MASK) | 
                       DWC_CTLH_DONE;
        }
    
        s_st_transStatus[u8_chanIndex].pst_lliMalloc[u32_blkIndex - 1].sar    = u32_srcAddr;
        s_st_transStatus[u8_chanIndex].pst_lliMalloc[u32_blkIndex - 1].dar    = u32_dstAddr;
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
        s_st_transStatus[u8_chanIndex].pst_lliMalloc[u32_blkIndex - 1].llp = 
        u32_llpLOC & 0xFFFFFFFC;
    }

    return 0;
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
    static uint8_t u8_flagInitOnce = 0;

    assert_param(IS_CHANNAL_PRIORITY(u8_chanPriority));

    if (u8_flagInitOnce == 0)
    {
        s_st_dmaRegs->MASK.TFR = 0;
        s_st_dmaRegs->MASK.BLOCK = 0;
        s_st_dmaRegs->MASK.SRCTRAN = 0;
        s_st_dmaRegs->MASK.DSTTRAN = 0;
        s_st_dmaRegs->MASK.ERROR = 0;

        for (u8_chanIndex = 0; u8_chanIndex< DW_DMA_MAX_NR_CHANNELS; u8_chanIndex++)
        {
            s_st_dmaRegs->CHAN[u8_chanIndex].CFG_HI = 0x0;            
        }
        
        u8_flagInitOnce = 1;
    }
    
    if (u8_channel == AUTO)
    {
        for (u8_chanIndex = 0; u8_chanIndex < DW_DMA_MAX_NR_CHANNELS; ++u8_chanIndex)
        {
            if (((s_st_dmaRegs->CH_EN) & ((uint32_t)(1 << u8_chanIndex))) == 0)
            {
                s_st_dmaRegs->CHAN[u8_chanIndex].CFG_LO = DWC_CFGL_HS_SRC | DWC_CFGL_HS_DST | 
                                                          (u8_chanPriority << 5);
                s_st_dmaRegs->MASK.TFR = ((1 << u8_chanIndex) | (1 << (u8_chanIndex + 8)));
                break;
            }
        }

        if (u8_chanIndex >= DW_DMA_MAX_NR_CHANNELS)
        {
            dlog_error("No channel left for DMA!\n");
            return -1;
        }

        s_st_transStatus[u8_chanIndex].trans_complete = 0;
/*         dlog_info("AUTOchan = %d\n", u8_chanIndex); */
        return u8_chanIndex;
    }
    else 
    {
        if ((s_st_dmaRegs->CH_EN & (uint32_t)(1 << u8_channel)) == 0)
        {
            s_st_dmaRegs->CHAN[u8_channel].CFG_LO = DWC_CFGL_HS_SRC | DWC_CFGL_HS_DST | 
                                                    (u8_chanPriority << 5);
            s_st_dmaRegs->MASK.TFR |= ((1 << u8_channel) | (1 << (u8_channel + 8)));  // unmask
            s_st_transStatus[u8_channel].trans_complete = 0;
            return u8_channel;
        }

        return -1;
    }
}

static uint32_t dmaPerpAddrConvert(uint32_t addr)
{
    if (ENUM_CPU0_ID == CPUINFO_GetLocalCpuId())
    {
        return addr + DTCM_CPU0_DMA_ADDR_OFFSET;                    
    }
    else if (ENUM_CPU1_ID == CPUINFO_GetLocalCpuId())
    {
        return addr + DTCM_CPU1_DMA_ADDR_OFFSET;                    
    }
    return -1;        
}

uint32_t DMA_transfer(uint32_t u32_srcAddr, uint32_t u32_dstAddr, uint32_t u32_transByteNum, 
                            uint8_t u8_chanIndex, ENUM_TransferType e_transType)
{
    ENUM_TransferType mode;
    STRU_DMA_DESCRIPTER s_dma_config = {0};
    uint32_t u32_llpBaseAddr = 0;
            
    if ( (u32_transByteNum % 4) == 0 && (u32_transByteNum <= AR_DW_CH_MAX_BLK_SIZE) )
    {
        mode = SINGLE_BLOCK;
    }
    else
    {
        mode = LINK_LIST_ITEM;
    }

    switch(mode)
    {
        case SINGLE_BLOCK:
            s_st_dmaRegs->CHAN[u8_chanIndex].SAR = u32_srcAddr;
            s_st_dmaRegs->CHAN[u8_chanIndex].DAR = u32_dstAddr;

            s_st_dmaRegs->CHAN[u8_chanIndex].CTL_LO = DWC_CTLL_DMS(0x1) | DWC_CTLL_INT_EN | 
                                              DWC_CTLL_DST_WIDTH(2) | DWC_CTLL_SRC_WIDTH(2) | 
                                              DWC_CTLL_DST_MSIZE(0) | DWC_CTLL_SRC_MSIZE(0) |
                                              DWC_CTLL_DST_INC | DWC_CTLL_SRC_INC;
            s_st_dmaRegs->CHAN[u8_chanIndex].CTL_HI = (u32_transByteNum/4) & 
                                                      DWC_CTLH_BLOCK_TS_MASK |
                                                      DWC_CTLH_DONE;
            s_st_dmaRegs->CHAN[u8_chanIndex].LLP = 0x0;            

            s_st_transStatus[u8_chanIndex].e_transferType = SINGLE_BLOCK;
            s_st_transStatus[u8_chanIndex].trans_complete = 0;

            s_st_dmaRegs->DMA_CFG = DW_CFG_DMA_EN;
            s_st_dmaRegs->CH_EN |= DWC_CH_EN(u8_chanIndex) | DWC_CH_EN_WE(u8_chanIndex);
        break;
        
        case LINK_LIST_ITEM:
            dma_prepare_LL_pre_Config(u32_transByteNum, &s_dma_config);
            
            #ifdef DMA_DEBUG
                dlog_info("before malloc, malloc size = %d\n", sizeof(STRU_LinkListItem) * 
                s_dma_config.totalBlkNum);
            #endif
            s_st_transStatus[u8_chanIndex].pst_lliMalloc = (STRU_LinkListItem *)malloc(sizeof(STRU_LinkListItem) * 
            s_dma_config.totalBlkNum);
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

            u32_llpBaseAddr = dmaPerpAddrConvert((uint32_t)s_st_transStatus[u8_chanIndex].pst_lliMalloc);

            dma_prepare_LL_Config(&s_dma_config, u8_chanIndex, u32_llpBaseAddr, 
                                  u32_srcAddr, u32_dstAddr);

#ifdef DMA_DEBUG
            for (int i = 0; i < s_dma_config.totalBlkNum; i++)
            {
                dlog_info("item %d, sar = 0x%08x\n", i, s_st_transStatus[u8_chanIndex].pst_lliMalloc[i].sar);
                dlog_info("item %d, dar = 0x%08x\n", i, s_st_transStatus[u8_chanIndex].pst_lliMalloc[i].dar);
                dlog_info("item %d, llp = 0x%08x\n", i, s_st_transStatus[u8_chanIndex].pst_lliMalloc[i].llp);
                dlog_info("item %d, ctllo = 0x%08x\n", i, s_st_transStatus[u8_chanIndex].pst_lliMalloc[i].ctllo);
                dlog_info("item %d, ctlhi = 0x%08x\n", i, s_st_transStatus[u8_chanIndex].pst_lliMalloc[i].ctlhi);
            }
#endif /* DMA_DEBUG */

            s_st_dmaRegs->CHAN[u8_chanIndex].CTL_LO = DWC_CTLL_DMS(0x1) | 
                                                      DWC_CTLL_INT_EN | 
                                                      DWC_CTLL_DST_WIDTH(0x2) | DWC_CTLL_SRC_WIDTH(0x2) | 
                                                      DWC_CTLL_DST_MSIZE(0x1) | DWC_CTLL_SRC_MSIZE(0x1) | 
                                                      DWC_CTLL_LLP_D_EN | DWC_CTLL_LLP_S_EN;
            s_st_dmaRegs->CHAN[u8_chanIndex].CTL_HI = DW_CH_MAX_BLK_SIZE & DWC_CTLH_BLOCK_TS_MASK |
                                                      DWC_CTLH_DONE;
            s_st_dmaRegs->CHAN[u8_chanIndex].LLP    = u32_llpBaseAddr & 0xFFFFFFFC;
            s_st_transStatus[u8_chanIndex].e_transferType = LINK_LIST_ITEM;

            s_st_dmaRegs->DMA_CFG = DW_CFG_DMA_EN;
            s_st_dmaRegs->CH_EN   = DWC_CH_EN(u8_chanIndex) | DWC_CH_EN_WE(u8_chanIndex);
        break;
        
        case AUTO_RELOAD:
        {
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
    uint32_t u32_start;
    
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
            //dlog_info("line = %d, no channel for channel %d\n", __LINE__, u8_chanIndex);
        }
    }

    if (u8_chanIndex < 4)
    {
        dlog_info("line = %d, all 4 channel occupied!", __LINE__);
        return DMA_BUSY;
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

            while( DMA_getStatus(u8_chanIndex) == 0 )
            {
                if ((SysTicks_GetDiff(u32_start, SysTicks_GetTickCount())) >= u32_ms)            
                {                 
                    break;            
                }

                SysTicks_DelayMS(1);
            }
            return DMA_TIME_OUT;
        break;

        default:
            break;
    }

    return DMA_OK;
}
