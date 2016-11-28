#ifndef __DMA_H
#define __DMA_H

#include <stdint.h>

//====================================================================
//DMA addr mapping
//====================================================================

#define BASE_ADDR_DMA      0x45000000

#define ADDR_ChEnReg       0x3a0
#define ADDR_ClearBlock    0x340
#define ADDR_ClearSrcTran  0x348
#define ADDR_ClearDstTran  0x350
#define ADDR_ClearErr      0x358
#define ADDR_ClearTfr      0x338
#define ADDR_SAR0          0x000
#define ADDR_DAR0          0x008
#define ADDR_LLP0          0x010
#define ADDR_CTL0_LOW32    0x018
#define ADDR_CTL0_HIGH32   0x01c
#define ADDR_DmaCfgReg     0x398
#define ADDR_CFG0_LOW32    0x040
#define ADDR_CFG0_HIGH32   0x044
#define ADDR_MaskTfr       0x310
#define ADDR_MaskBlock     0x318
#define ADDR_MaskSrcTran   0x320
#define ADDR_MaskDstTran   0x328
#define ADDR_MaskErr       0x330
#define ADDR_StatusTfr     0x2e8  //read only
#define ADDR_StatusBlock   0x2f0  //read only

//====================================================================
//DMA default value 
//====================================================================

#define DATA_ChEnReg       0x0000ffff    //0x3a0
#define DATA_ClearBlock    0x00000001    //0x340
#define DATA_ClearSrcTran  0x00000001    //0x348
#define DATA_ClearDstTran  0x00000001    //0x350
#define DATA_ClearErr      0x00000001    //0x358
#define DATA_ClearTfr      0x00000001    //0x338
#define DATA_SAR0          0x00000004    //0x000
#define DATA_DAR0          0x00000000    //0x008
#define DATA_LLP0          0x00000000    //0x010
#define DATA_CTL0_LOW32    0x00804825    //0x018
#define DATA_CTL0_HIGH32   0x00001004    //0x01c
#define DATA_DmaCfgReg     0x00000001    //0x398
#define DATA_CFG0_LOW32    0x00800c00    //0x040
#define DATA_CFG0_HIGH32   0x00000000    //0x044
#define DATA_MaskTfr       0x00000101    //0x310
#define DATA_MaskBlock     0x00000101    //0x318
#define DATA_MaskSrcTran   0x00000000    //0x320
#define DATA_MaskDstTran   0x00000000    //0x328
#define DATA_MaskErr       0x00000101    //0x330

#define DMA_LLP_ENABLE          0x3
#define DMA_LLP_DISABLE         0


void dma_llp_driver (uint32_t *src_addr,uint32_t *dst_addr, uint32_t byte_num, uint32_t block_size, uint32_t msize);
void dma_clear_irq(void);
void dma_transfer(uint32_t *src, uint32_t *dst, uint32_t byte_num);

#endif

