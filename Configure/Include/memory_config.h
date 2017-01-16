#ifndef __MEMORYCONFIG_H
#define __MEMORYCONFIG_H

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

//--------------------------------------------------------------------
// memory address space define here
//--------------------------------------------------------------------
#define HP_SPI_BASE_ADDR      0x40C00000
#define HP_SPI_CMD_BASE       ( 0   )*4
#define HP_SPI_AW_MEM_BASE    ( 32  )*4
#define HP_SPI_CODE_MEM_BASE  ( 48  )*4
#define HP_SPI_CONFIG_REG     ( 112 )*4
#define HP_SPI_INIT_REG       ( 113 )*4
#define HP_SPI_RD_HW_REG      ( 114 )*4
#define HP_SPI_RD_LW_REG      ( 115 )*4
#define HP_SPI_WR_HW_REG      ( 116 )*4
#define HP_SPI_WR_LW_REG      ( 117 )*4
#define HP_SPI_UPDATE_RD_REG  ( 118 )*4
#define HP_SPI_UPDATE_WR_REG  ( 119 )*4

//-------------------------------------------------------------------
// spansion instruction address mapping
//-------------------------------------------------------------------
#define INSTRUCT_05_ADDR  ( 0 )*4
#define INSTRUCT_35_ADDR  ( 1 )*4
#define INSTRUCT_33_ADDR  ( 2 )*4
#define INSTRUCT_04_ADDR  ( 3 )*4
#define INSTRUCT_01_ADDR  ( 4 )*4
#define INSTRUCT_ab_ADDR  ( 5 )*4
#define INSTRUCT_90_ADDR  ( 6 )*4
#define INSTRUCT_9f_ADDR  ( 7 )*4
#define INSTRUCT_77_ADDR  ( 8 )*4
#define INSTRUCT_C7_ADDR  ( 9 )*4
#define INSTRUCT_75_ADDR  ( 10 )*4
#define INSTRUCT_7a_ADDR  ( 11 )*4
#define INSTRUCT_66_ADDR  ( 12 )*4
#define INSTRUCT_99_ADDR  ( 13 )*4
#define INSTRUCT_ff_ADDR  ( 14 )*4
#define INSTRUCT_b9_ADDR  ( 15 )*4
#define INSTRUCT_06_ADDR  ( 16 )*4
#define INSTRUCT_50_ADDR  ( 17 )*4
#define INSTRUCT_18_ADDR  ( 18 )*4
#define INSTRUCT_19_ADDR  ( 19 )*4
#define INSTRUCT_20_ADDR  ( 20 )*4
#define INSTRUCT_d8_ADDR  ( 21 )*4
#define INSTRUCT_5a_ADDR  ( 22 )*4
#define INSTRUCT_48_ADDR  ( 23 )*4
#define INSTRUCT_44_ADDR  ( 24 )*4
#define INSTRUCT_42_ADDR  ( 25 )*4
#define INSTRUCT_MEM_OFFSET  ( 16 )*4

//---------------------------------------------------------------------------------
//Board Memory Mapping
//---------------------------------------------------------------------------------
#define ITCM0_BASE        0x00000000
#define ITCM_BASE         0x00080000
#define QUAD_SPI_BASE     0x10000000
#define DTCM0_BASE        0x20000000
#define DTCM_BASE         0x20080000
#define SRAM_BASE         0x21000000
#define AHB2APB1_BASE     0x40000000
#define APB2APB2_BASE     0x41000000
#define AXI2APB_BASE      0x42000000
#define USB_BASE          0x43000000
#define CPU1_ITCM_BASE    0x44000000
#define CPU1_DTCM_BASE    0x44080000
#define CPU2_ITCM_BASE    0x44100000
#define CPU2_DTCM_BASE    0x44180000
#define DMA_BASE          0x45000000
#define LEON_SRAM_BASE    0x80000000
#define SDRAM_BASE        0x81000000
#define UART_APB_BASE     0xA0000000
#define VIDEO_APB_BASE    0xA0010000
#define WATCH_DOG_BASE    0xA0020000
#define GLOBAL_REGISTER2_BASE   0xA0030000
#define SPI_BASE          0xA0040000
#define I2C               0xA0050000
#define CPU3_ITCM_BASE    0xB0000000
#define CPU3_DTCM_BASE    0xB0080000

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

//====================================================================
//MOBILE STORAGE RELATED
//====================================================================
#define SDMMC_BASE  0x42000000

//====================================================================
//SRAM MEMORY MAP
//====================================================================
#define SRAM_BASE_ADDRESS                             0x21000000     /* start address of SRAM */

// 8K video0
#define SRAM_BB_VIDEO_BUFFER_0_ST_ADDRESS             SRAM_BASE_ADDRESS
#define SRAM_BB_VIDEO_BUFFER_0_SIZE                   0x2000

// 8K video1
#define SRAM_BB_VIDEO_BUFFER_1_ST_ADDRESS             (SRAM_BB_VIDEO_BUFFER_0_ST_ADDRESS + SRAM_BB_VIDEO_BUFFER_0_SIZE)
#define SRAM_BB_VIDEO_BUFFER_1_SIZE                   0x2000

// 4K non-cache start, initialized by inter core module.

// 256 inter core message
#define SRAM_INTER_CORE_MSG_SHARE_MEMORY_ST_ADDRESS   (SRAM_BB_VIDEO_BUFFER_1_ST_ADDRESS + SRAM_BB_VIDEO_BUFFER_1_SIZE)
#define SRAM_INTER_CORE_MSG_SHARE_MEMORY_SIZE         0x100

// 256 module lock
#define SRAM_MODULE_LOCK_ST_ADDRESS                   (SRAM_INTER_CORE_MSG_SHARE_MEMORY_ST_ADDRESS + SRAM_INTER_CORE_MSG_SHARE_MEMORY_SIZE)
#define SRAM_MODULE_LOCK_SIZE                         0x100
#define SRAM_MODULE_LOCK_BB_UART_MUTEX_FLAG           (SRAM_MODULE_LOCK_ST_ADDRESS)
#define SRAM_MODULE_LOCK_BB_UART_INIT_FLAG            (SRAM_MODULE_LOCK_ST_ADDRESS + 4)

// 256 module share
#define SRAM_MODULE_SHARE_ST_ADDRESS                  (SRAM_MODULE_LOCK_ST_ADDRESS + SRAM_MODULE_LOCK_SIZE)
#define SRAM_MODULE_SHARE_SIZE                        0x100
#define SRAM_MODULE_SHARE_PLL_INIT_FLAG               (SRAM_MODULE_SHARE_ST_ADDRESS)
#define SRAM_MODULE_SHARE_PLL_CPU0CPU1                (SRAM_MODULE_SHARE_ST_ADDRESS + 4)
#define SRAM_MODULE_SHARE_PLL_CPU2                    (SRAM_MODULE_SHARE_ST_ADDRESS + 8)

// 512 bb status
#define SRAM_BB_STATUS_SHARE_MEMORY_ST_ADDR           (SRAM_MODULE_SHARE_ST_ADDRESS + SRAM_MODULE_SHARE_SIZE)
#define SRAM_BB_STATUS_SHARE_MEMORY_SIZE              0x200

// 1K bb uart com buffer
#define SRAM_BB_UART_COM_SESSION_1_SHARE_MEMORY_ST_ADDR    (SRAM_BB_STATUS_SHARE_MEMORY_ST_ADDR + SRAM_BB_STATUS_SHARE_MEMORY_SIZE)
#define SRAM_BB_UART_COM_SESSION_1_SHARE_MEMORY_SIZE       0x100
#define SRAM_BB_UART_COM_SESSION_2_SHARE_MEMORY_ST_ADDR    (SRAM_BB_UART_COM_SESSION_1_SHARE_MEMORY_ST_ADDR + SRAM_BB_UART_COM_SESSION_1_SHARE_MEMORY_SIZE)
#define SRAM_BB_UART_COM_SESSION_2_SHARE_MEMORY_SIZE       0x100
#define SRAM_BB_UART_COM_SESSION_3_SHARE_MEMORY_ST_ADDR    (SRAM_BB_UART_COM_SESSION_2_SHARE_MEMORY_ST_ADDR + SRAM_BB_UART_COM_SESSION_2_SHARE_MEMORY_SIZE)
#define SRAM_BB_UART_COM_SESSION_3_SHARE_MEMORY_SIZE       0x100
#define SRAM_BB_UART_COM_SESSION_4_SHARE_MEMORY_ST_ADDR    (SRAM_BB_UART_COM_SESSION_3_SHARE_MEMORY_ST_ADDR + SRAM_BB_UART_COM_SESSION_3_SHARE_MEMORY_SIZE)
#define SRAM_BB_UART_COM_SESSION_4_SHARE_MEMORY_SIZE       0x100




#define GET_WORD_FROM_ANY_ADDR(any_addr) ((uint32_t)(*any_addr) | \
                                         (((uint32_t)(*(any_addr+1))) << 8) | \
                                         (((uint32_t)(*(any_addr+2))) << 16) | \
                                         ((uint32_t)((*(any_addr+3))) << 24))
typedef struct 
{    
#ifdef USE_BB_REG_CONFIG_BIN
    unsigned char bb_sky_configure[4][256];
    unsigned char bb_grd_configure[4][256];
    unsigned char rf_configure[128];
#endif
#ifdef USE_ADV7611_EDID_CONFIG_BIN
    unsigned char hdmi_configure[263][3];
#endif
}STRU_SettingConfigure;

#define GET_CONFIGURE_FROM_FLASH(structaddress) {do\
                                                 {\
                                                 uint8_t* cpu0_app_size_addr = (uint8_t*)0x10020022;\
                                                 uint32_t cpu0_app_size = GET_WORD_FROM_ANY_ADDR(cpu0_app_size_addr);\
                                                 uint32_t cpu0_app_start_addr = 0x10020022 + 4;\
                                                 uint8_t* cpu1_app_size_addr = (uint8_t*)(cpu0_app_start_addr + cpu0_app_size);\
                                                 uint32_t cpu1_app_size = GET_WORD_FROM_ANY_ADDR(cpu1_app_size_addr);\
                                                 uint32_t cpu1_app_start_addr = cpu0_app_start_addr + cpu0_app_size + 4;\
                                                 uint8_t* cpu2_app_size_addr = (uint8_t*)(cpu1_app_start_addr + cpu1_app_size);\
                                                 uint32_t cpu2_app_size = GET_WORD_FROM_ANY_ADDR(cpu2_app_size_addr);\
                                                 uint32_t cpu2_app_start_addr = cpu1_app_start_addr + cpu1_app_size + 4;\
                                                 structaddress=(STRU_SettingConfigure *)(cpu2_app_start_addr + cpu2_app_size);\
                                                 }while(0);\
                                                 }
#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MEMORYCONFIG_H */

