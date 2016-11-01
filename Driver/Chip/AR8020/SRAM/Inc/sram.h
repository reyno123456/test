#ifndef __SRAM_H
#define __SRAM_H



/*
  *******************************************************
  ****************             GROUND         *******************
  *******************************************************
  */

/* these register is for SRAM configuration */
#define DMA_READY_0                     0x40B00038
#define DMA_READY_1                     0x40B0003C


#define SRAM_DMA_READY_LEN              0x800           /* threshold for BaseBand to trigger READY_IRQ:128 word or others */
#define SRAM_WR_ADDR_OFFSET_0           0x40B00028
#define SRAM_WR_ADDR_OFFSET_1           0x40B0002C     /* this value should not smaller than SRAM_WR_MAX_LEN_0 */
#define SRAM_WR_MAX_LEN_0               0x40B00030     /* unit: 1 word = 4 byte */
#define SRAM_WR_MAX_LEN_1               0x40B00034
#define SRAM_DATA_VALID_LEN_0           (*((volatile unsigned int *)0x40B00040))
#define SRAM_DATA_VALID_LEN_1           (*((volatile unsigned int *)0x40B00044))


#define SRAM_BASE_ADDRESS               0x21000000     /* start address of SRAM */
#define SRAM_BB_BYPASS_OFFSET_0         0x00000000     /* BB bypass channel 0 to the address in SRAM */
#define SRAM_BB_BYPASS_OFFSET_1         0x00000800     /* BB bypass channel 1 to the address in SRAM */
#define SRAM_BUFF_0_ADDRESS             (SRAM_BASE_ADDRESS + (SRAM_BB_BYPASS_OFFSET_0 << 2))
#define SRAM_BUFF_1_ADDRESS             (SRAM_BASE_ADDRESS + (SRAM_BB_BYPASS_OFFSET_1 << 2))


void SRAM_Ready0IRQHandler(void);
void SRAM_Ready1IRQHandler(void);
void SRAM_Ready0Confirm(void);
void SRAM_Ready1Confirm(void);
void SRAM_GROUND_BypassVideoConfig(void);


/*
  *******************************************************
  ****************              SKY              *******************
  *******************************************************
  */

#define SRAM_VIEW0_ENABLE_ADDR          0xA003008C
#define SRAM_VIEW1_ENABLE_ADDR          0xA003004C


#define SRAM_SKY_MASTER_ID_ADDR         0xA0030090
#define SRAM_SKY_MASTER_ID_MASK_ADDR    0xA0030094


#define SRAM_SKY_MASTER_ID_VALUE        0x600
#define SRAM_SKY_MASTER_ID_MASK_VALUE   0xE00


void SRAM_SKY_BypassVideoConfig(uint32_t channel);


#endif

