#ifndef __SRAM_COMMON_H
#define __SRAM_COMMON_H


#define SRAM_BASE_ADDRESS          0x21000000     /* start address of SRAM */
#define SRAM_BB_BYPASS_OFFSET_0    0x00000000     /* BB bypass channel 0 to the address in SRAM */
#define SRAM_BB_BYPASS_OFFSET_1    0x00000800     /* BB bypass channel 1 to the address in SRAM */
#define SRAM_BUFF_0_ADDRESS        (SRAM_BASE_ADDRESS + (SRAM_BB_BYPASS_OFFSET_0 << 2))
#define SRAM_BUFF_1_ADDRESS        (SRAM_BASE_ADDRESS + (SRAM_BB_BYPASS_OFFSET_1 << 2))


#endif

