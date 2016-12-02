#ifndef BOOT_CORE_H
#define BOOT_CORE_H

#define RDWR_SECTOR_SIZE           (1024*4)

#define APPLICATION_IMAGE_START    0x10020008
#define APP_ADDR_OFFSET            (0x20000)

#define ITCM0_START                0x00000000
#define ITCM1_START                0x44100000
#define ITCM2_START                0xB0000000

#define MCU1_CPU_WAIT              0x40B000CC  /* ENABLE CPU1 */
#define MCU2_CPU_WAIT              0xA0030088  /* ENABLE CPU2 */
#define MCU0_VECTOR_TABLE_REG      0xE000ED08

#define GET_WORD_FROM_ANY_ADDR(any_addr) ((uint32_t)(*any_addr) | \
                                         (((uint32_t)(*(any_addr+1))) << 8) | \
                                         (((uint32_t)(*(any_addr+2))) << 16) | \
                                         ((uint32_t)((*(any_addr+3))) << 24))

#define INFO_BASE                  0x1000
#define BOOT_ADDR0                 0x10002000
#define BOOT_ADDR1                 0x10011000
#define BOOT_SIZE                  (1024*60)

typedef struct
{
    unsigned char present_boot;
    unsigned char success_boot;
}Boot_Info;

void BOOT_BootApp(void);
void BOOT_CopyFromNorToITCM(void);
void BOOT_StartBoot(uint8_t index);
#endif