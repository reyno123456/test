#ifndef __SRAM_H
#define __SRAM_H



/* these register is for SRAM configuration */
#define DMA_READY_0                0x40B00038
#define DMA_READY_1                0x40B0003C


void SRAM_Ready0IRQHandler(void);
void SRAM_Ready1IRQHandler(void);
void SRAM_Ready0Confirm(void);
void SRAM_Ready1Confirm(void);



#endif

