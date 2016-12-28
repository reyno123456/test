#include "dma.h"
#include "debuglog.h"
#include "reg_rw.h"
#include "cpu_info.h"

uint32_t g_dmaSettingConfig[60];

void dma_transfer(uint32_t *src, uint32_t *dst, uint32_t byte_num)
{
    volatile uint32_t   dma_finish;

    dma_finish          = 0;

    dma_llp_driver(src, dst, byte_num, 4095, 1);

    while(dma_finish==0)
    {
        dma_finish = Reg_Read32(BASE_ADDR_DMA + ADDR_StatusTfr); 
    }

    dma_clear_irq();
}


void dma_llp_driver (uint32_t *src_addr,uint32_t *dst_addr, uint32_t byte_num, uint32_t block_size, uint32_t msize)
{
    uint32_t data_ctl0_low32;
    uint32_t data_ctl0_high32;
    uint32_t data_ctl0_lint; 
    uint32_t data_ctl0_hint;
    uint32_t data_llp;
    uint32_t data_sar,data_dar; 
    uint32_t dest_msize;
    uint32_t src_msize;
    uint32_t llp_loc,llp_lms,llp_base,llp0;
    uint32_t addr_sar,addr_dar,addr_llp,addr_ctl,addr_cth;
    uint32_t block_num;
    uint32_t blk_size_1st,blk_size_2nd,blk_size_3rd;
    uint32_t bsize_1st, bsize_2nd, bsize_3rd;
    uint32_t blk_numb_1st,blk_numb_2nd,blk_numb_3rd;
    uint32_t hsize_1st,hsize_2nd,hsize_3rd;
    uint32_t blk_byte_max;
    uint32_t blk_resides1,blk_resides2;
    uint32_t i;

    dest_msize = msize & 0x7;
    src_msize  = msize & 0x7;

    Reg_Write32(BASE_ADDR_DMA + ADDR_ChEnReg     , 0x00000000       ); //0x3a0 ChEnReg : not busy
    Reg_Write32(BASE_ADDR_DMA + ADDR_CFG0_HIGH32 , DATA_CFG0_HIGH32 ); //0x044
    Reg_Write32(BASE_ADDR_DMA + ADDR_CFG0_LOW32  , DATA_CFG0_LOW32  ); //0x040

    //Clear Interrupt
    Reg_Write32(BASE_ADDR_DMA + ADDR_ClearBlock  , DATA_ClearBlock  ); //0x340 ClearBlock  
    Reg_Write32(BASE_ADDR_DMA + ADDR_ClearSrcTran, DATA_ClearSrcTran); //0x348 ClearSrcTran
    Reg_Write32(BASE_ADDR_DMA + ADDR_ClearDstTran, DATA_ClearDstTran); //0x350 ClearDstTran
    Reg_Write32(BASE_ADDR_DMA + ADDR_ClearErr    , DATA_ClearErr    ); //0x358 ClearErr    
    Reg_Write32(BASE_ADDR_DMA + ADDR_ClearTfr    , DATA_ClearTfr    ); //0x338 ClearTfr  
    //Interrupt Mask
    Reg_Write32(BASE_ADDR_DMA + ADDR_MaskTfr     , DATA_MaskTfr     ); //0x310
    //Reg_Write32(BASE_ADDR_DMA + ADDR_MaskBlock   , DATA_MaskBlock   ); //0x318
    Reg_Write32(BASE_ADDR_DMA + ADDR_MaskBlock   , 0x00000000       ); //0x318
    Reg_Write32(BASE_ADDR_DMA + ADDR_MaskSrcTran , DATA_MaskSrcTran ); //0x320
    Reg_Write32(BASE_ADDR_DMA + ADDR_MaskDstTran , DATA_MaskDstTran ); //0x328
    Reg_Write32(BASE_ADDR_DMA + ADDR_MaskErr     , DATA_MaskErr     ); //0x330

    //LLP set up: multi-block 
    blk_byte_max = block_size * 4;
    blk_resides1 = byte_num % blk_byte_max; 
    blk_resides2 = blk_resides1 % 4; 

    blk_size_1st = block_size;
    blk_size_2nd = (blk_resides1 - blk_resides2)/4; // maybe 0 1 2 3 ... 
    blk_size_3rd = blk_resides2;                    // maybe 0 1 2 3

    blk_numb_1st = (unsigned int) (byte_num / blk_byte_max); // 0 or other integer
    blk_numb_2nd = (blk_size_2nd > 0) ? 0x00000001 : 0x00000000; 
    blk_numb_3rd = (blk_size_3rd > 0) ? 0x00000001 : 0x00000000;

    if ( (!blk_numb_1st) && (!blk_numb_2nd) && (blk_numb_3rd) )  // 001
    {
        block_num = 1;
        bsize_1st = 0;
        bsize_2nd = 0;
        bsize_3rd = blk_size_3rd;
        hsize_1st = 0;
        hsize_2nd = 0;
        hsize_3rd = 0;
    }
    else if ( (!blk_numb_1st) && (blk_numb_2nd) && (!blk_numb_3rd) ) //010
    {
        block_num = 1;
        bsize_1st = 0;
        bsize_2nd = 0;
        bsize_3rd = blk_size_2nd;
        hsize_1st = 0;	
        hsize_2nd = 0;
        hsize_3rd = 2;
    }
    else if ( (blk_numb_1st) && (!blk_numb_2nd) && (!blk_numb_3rd) ) //100
    {
        block_num = blk_numb_1st;
        bsize_1st = blk_size_1st;
        bsize_2nd = blk_size_1st;
        bsize_3rd = blk_size_1st;
        hsize_1st = 2;
        hsize_2nd = 2;
        hsize_3rd = 2;
    }
    else if ( (!blk_numb_1st) && (blk_numb_2nd) && (blk_numb_3rd) ) //011
    {
        block_num = 2;
        bsize_1st = 0;
        bsize_2nd = blk_size_2nd;
        bsize_3rd = blk_size_3rd;
        hsize_1st = 0;
        hsize_2nd = 2;
        hsize_3rd = 0;
    }
    else if ( (blk_numb_1st) && (!blk_numb_2nd) && (blk_numb_3rd) ) //101
    {
        block_num = blk_numb_1st + 1;
        bsize_1st = blk_size_1st;
        bsize_2nd = blk_size_1st;
        bsize_3rd = blk_size_3rd;
        hsize_1st = 2;
        hsize_2nd = 2;
        hsize_3rd = 0;
    }
    else if ( (blk_numb_1st) && (blk_numb_2nd) && (!blk_numb_3rd) ) //110
    {
        block_num = blk_numb_1st + 1;
        bsize_1st = blk_size_1st;
        bsize_2nd = blk_size_1st;
        bsize_3rd = blk_size_2nd;
        hsize_1st = 2;
        hsize_2nd = 2;
        hsize_3rd = 2;
    }
    else // if ( (blk_numb_1st) && (blk_numb_2nd) && (blk_numb_3rd) ) // 111
    {
        block_num = blk_numb_1st + blk_numb_2nd + blk_numb_3rd;
        bsize_1st = blk_size_1st;
        bsize_2nd = blk_size_2nd;
        bsize_3rd = blk_size_3rd;
        hsize_1st = 2;
        hsize_2nd = 2;
        hsize_3rd = 0;
    }

    if (ENUM_CPU0_ID == CPUINFO_GetLocalCpuId())
    {
        llp_base = (((uint32_t)g_dmaSettingConfig + DTCM_CPU0_DMA_ADDR_OFFSET)>>2); //SRAM: llp_loc[31:2], store the LLP entry        
    }
    else if (ENUM_CPU1_ID == CPUINFO_GetLocalCpuId())
    {
        llp_base = (((uint32_t)g_dmaSettingConfig + DTCM_CPU1_DMA_ADDR_OFFSET)>>2); //SRAM: llp_loc[31:2], store the LLP entry        
    }
    llp_lms = 0; //use master 0 interface memory
    llp_loc = llp_base;
    llp0 = (llp_loc<<2) + llp_lms;

    data_sar = (uint32_t)src_addr;
    data_dar = (uint32_t)dst_addr;

    for(i=1;i<=block_num;i++)	
    {
        if(i < (block_num -1)) // belong to block 1st
        {
            data_ctl0_low32 = (DATA_CTL0_LOW32 & 0xe7fe0781) + (hsize_1st << 1) + (hsize_1st << 4) + (dest_msize << 11) + (src_msize << 14) + (DMA_LLP_ENABLE << 27);
            data_ctl0_high32= (bsize_1st & 0xfff) + (DATA_CTL0_HIGH32 & 0xfffff000);
        }
        else if(i == (block_num -1)) // belong to block 2nd
        {
            data_ctl0_low32 = (DATA_CTL0_LOW32 & 0xe7fe0781) + (hsize_2nd << 1) + (hsize_2nd << 4) + (dest_msize << 11) + (src_msize << 14) + (DMA_LLP_ENABLE << 27);
            data_ctl0_high32= (bsize_2nd & 0xfff) + (DATA_CTL0_HIGH32 & 0xfffff000);
        }
        else // belong to block 3rd
        {
            data_ctl0_low32 = (DATA_CTL0_LOW32 & 0xe7fe0781) + (hsize_3rd << 1) + (hsize_3rd << 4) + (dest_msize << 11) + (src_msize << 14) + (DMA_LLP_DISABLE << 27);
            data_ctl0_high32= (bsize_3rd & 0xfff) + (DATA_CTL0_HIGH32 & 0xfffff000);
        }

        addr_sar = (llp_loc + 0) << 2;
        addr_dar = (llp_loc + 1) << 2;
        addr_llp = (llp_loc + 2) << 2;
        addr_ctl = (llp_loc + 3) << 2;
        addr_cth = (llp_loc + 4) << 2;

        //store the llp entry in memory
        Reg_Write32(addr_sar, data_sar         ); //SAR0
        Reg_Write32(addr_dar, data_dar         ); //DAR0
        Reg_Write32(addr_ctl, data_ctl0_low32  ); //CTLL
        Reg_Write32(addr_cth, data_ctl0_high32 ); //CTLH

        if(i < (block_num -1)) // belong to block 1st
        {
            llp_loc=llp_loc+5;
            data_sar=data_sar+4*bsize_1st;
            data_dar=data_dar+4*bsize_1st;
        }
        else if (i == (block_num -1)) //belong to block 2nd
        {
            llp_loc=llp_loc+5;
            data_sar=data_sar+4*bsize_2nd;
            data_dar=data_dar+4*bsize_2nd;
        }
        else // belong to block 3rd
        {
            llp_loc=0;
            data_sar=data_sar+4*bsize_3rd;
            data_dar=data_dar+4*bsize_3rd;
        }

        data_llp = (llp_loc << 2) + llp_lms;
        Reg_Write32(addr_llp, data_llp ); //LLP0 
    }

    data_ctl0_lint= (DATA_CTL0_LOW32 & 0xe7fe07ff) + (dest_msize << 11) + (src_msize << 14) + (DMA_LLP_ENABLE << 27);
    data_ctl0_hint= (block_size & 0xfff) + (DATA_CTL0_HIGH32 & 0xfffff000);

    //LLP0 set up 
    Reg_Write32(BASE_ADDR_DMA + ADDR_CTL0_LOW32  , data_ctl0_lint   ); //0x018
    Reg_Write32(BASE_ADDR_DMA + ADDR_CTL0_HIGH32 , data_ctl0_hint   ); //0x01c
    Reg_Write32(BASE_ADDR_DMA + ADDR_LLP0        , llp0             ); //0x010
    Reg_Write32(BASE_ADDR_DMA + ADDR_DmaCfgReg   , DATA_DmaCfgReg   ); //0x398
    Reg_Write32(BASE_ADDR_DMA + ADDR_ChEnReg     , DATA_ChEnReg     ); //0x3a0

    return;
}


void dma_clear_irq(void)
{
    Reg_Write32(BASE_ADDR_DMA + ADDR_ClearBlock  , DATA_ClearBlock  ); //0x340 ClearBlock  
    Reg_Write32(BASE_ADDR_DMA + ADDR_ClearSrcTran, DATA_ClearSrcTran); //0x348 ClearSrcTran
    Reg_Write32(BASE_ADDR_DMA + ADDR_ClearDstTran, DATA_ClearDstTran); //0x350 ClearDstTran
    Reg_Write32(BASE_ADDR_DMA + ADDR_ClearErr    , DATA_ClearErr    ); //0x358 ClearErr    
    Reg_Write32(BASE_ADDR_DMA + ADDR_ClearTfr    , DATA_ClearTfr    ); //0x338 ClearTfr    

    return;
}


