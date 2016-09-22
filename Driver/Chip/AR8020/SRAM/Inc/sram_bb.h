#ifndef __SRAM_BB_H
#define __SRAM_BB_H



#define SRAM_DMA_READY_LEN         0x80           /* threshold for BaseBand to trigger READY_IRQ:128 word or others */


#define SRAM_WR_ADDR_OFFSET_0      0x40B00028
#define SRAM_WR_ADDR_OFFSET_1      0x40B0002C     /* this value should not smaller than SRAM_WR_MAX_LEN_0 */
#define SRAM_WR_MAX_LEN_0          0x40B00030     /* unit: 1 word = 4 byte */
#define SRAM_WR_MAX_LEN_1          0x40B00034
#define SRAM_DATA_VALID_LEN_0      0x40B00040
#define SRAM_DATA_VALID_LEN_1      0x40B00044





#endif

