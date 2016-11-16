#ifndef __BOOT__H
#define __BOOT__H

#include "ff.h"

#define RDWR_SECTOR_SIZE           (1024*4) //or (1024*32)
#define RDWR_BLOCK_SIZE            (1024*64) //or (1024*32)

#define APPLICATION_IMAGE_START    0x10010000
#define APP_ADDR_OFFSET            (0x10000)
#define ITCM0_START                0x00000000
#define ITCM1_START                0x44100000
#define ITCM2_START                0xB0000000
#define MCU1_CPU_WAIT              0x40B000CC  /* ENABLE CPU1 */
#define MCU2_CPU_WAIT              0xA0030088  /* ENABLE CPU2 */
#define MCU0_VECTOR_TABLE_REG      0xE000ED08

#define COPY_SECTOR_ERASE_ADDR            0x0000042A
#define RUN_SECTOR_ERASE_ADDR             0x0000042D

#define COPY_BLOCK_ERASE_ADDR            0x0000142A
#define RUN_BLOCK_ERASE_ADDR             0x0000142D

#define GET_WORD_FROM_ANY_ADDR(any_addr) ((uint32_t)(*any_addr) | \
                                         (((uint32_t)(*(any_addr+1))) << 8) | \
                                         (((uint32_t)(*(any_addr+2))) << 16) | \
                                         ((uint32_t)((*(any_addr+3))) << 24))
enum updata_error
{
    BOOTLOAD_SUCCESS = 0,
    BOOTLOAD_READFILE= -1,
    BOOTLOAD_OPENFILE= -2

};


void BOOTLOAD_BootApp(void);
void BOOTLOAD_CopyFromNorToITCM(void);
void BOOTLOAD_RepickSectorErase(void);
void BOOTLOAD_RepickBlockErase(void);
int8_t BOOTLOAD_CopyDataToNor(FIL MyFile,uint32_t u32_addrOffset);
int8_t BOOTLOAD_CopyDataToITCM(FIL MyFile,uint32_t u32_TCMADDR);
int8_t BOOTLOAD_CopyDataToNorWithChecksum(FIL MyFile,uint32_t u32_TCMADDR);
int8_t BOOTLOAD_CopyDataToITCMWithChecksum(FIL MyFile,uint32_t u32_TCMADDR);

#endif