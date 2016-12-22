#ifndef __WIRELESS_INTERFACE_H
#define __WIRELESS_INTERFACE_H

#include <stdint.h>
#include "memory_config.h"

// CPU0 and CPU2 share memory for osd status info, offset in SRAM: 16K + 512Byte
#define OSD_STATUS_SHM_ADDR              SRAM_BB_STATUS_SHARE_MEMORY_ST_ADDR


typedef enum
{
    WIRELESS_INTERFACE_UPGRADE                      = 0x01,
    WIRELESS_INTERFACE_ENTER_TEST_MODE              = 0x02,
    WIRELESS_INTERFACE_SYNCHRONIZE_FREQ_CHANNEL     = 0x03,
    WIRELESS_INTERFACE_AUTO_MODE                    = 0x04,
    WIRELESS_INTERFACE_SELF_ADAPTION_MODE           = 0x05,
    WIRELESS_INTERFACE_SWITCH_TX_MODE               = 0x06,
    WIRELESS_INTERFACE_SWITCH_RX_MODE               = 0x07,
    WIRELESS_INTERFACE_SWITCH_USB1_MODE             = 0x08,
    WIRELESS_INTERFACE_SWITCH_USB2_MODE             = 0x09,
    WIRELESS_INTERFACE_ALL_RESET                    = 0x0A,
    WIRELESS_INTERFACE_RX_RESET                     = 0x0B,
    WIRELESS_INTERFACE_TX_RESET                     = 0x0C,
    WIRELESS_INTERFACE_MIMO_1T2R                    = 0x0D,
    WIRELESS_INTERFACE_WRITE_BB_REG                 = 0x0E,
    WIRELESS_INTERFACE_READ_BB_REG                  = 0x0F,
    WIRELESS_INTERFACE_MIMO_2T2R                    = 0x10,
    WIRELESS_INTERFACE_OSD_DISPLAY                  = 0x11,
    WIRELESS_INTERFACE_GET_ID                       = 0x13,
    WIRELESS_INTERFACE_GET_GROUND_TX_PWR            = 0x14,
    WIRELESS_INTERFACE_GET_SKY_TX_PWR               = 0x15,
    WIRELESS_INTERFACE_GET_RC_FREQ_CHANNEL          = 0x16,
    WIRELESS_INTERFACE_GET_VIDEO_FREQ_CHANNEL       = 0x17,
    WIRELESS_INTERFACE_GET_SOFTWARE_VERSION         = 0x18,
    WIRELESS_INTERFACE_GET_DEV_INFO                 = 0x19,
    WIRELESS_INTERFACE_SET_RC_FREQ_CHANNEL_MASK     = 0x1A,
    WIRELESS_INTERFACE_SET_VIDEO_FREQ_CHANNEL_MASK  = 0x1B,
    WIRELESS_INTERFACE_SELECT_VIDEO_FREQ_CHANNEL    = 0x1C,
    WIRELESS_INTERFACE_SWITCH_QAM_MODE              = 0x1D,
    WIRELESS_INTERFACE_SET_RC_FREQ_CHANNEL          = 0x1E,
    WIRELESS_INTERFACE_SET_TX_PWR                   = 0x1F,
    WIRELESS_INTERFACE_SET_VIDEO_TX_ID              = 0x20,
    WIRELESS_INTERFACE_SET_CTRL_ID                  = 0x21,
    WIRELESS_INTERFACE_SWITCH_WORKING_FREQ_BAND     = 0x22,
    WIRELESS_INTERFACE_RC_SCAN_ALL_BAND             = 0x23,
    WIRELESS_INTERFACE_RC_SCAN_WORKING_BAND         = 0x24,
    WIRELESS_INTERFACE_VIDEO_SCAN_ALL_BAND          = 0x25,
    WIRELESS_INTERFACE_VIDEO_SCAN_WORKING_BAND      = 0x26,
    WIRELESS_INTERFACE_RECOVER_TO_FACTORY           = 0x27,
    WIRELESS_INTERFACE_RC_HOPPING                   = 0x28,
    WIRELESS_INTERFACE_SAVE_CONFIGURE               = 0x29,
    WIRELESS_INTERFACE_READ_MCU_ID                  = 0x2A,
    WIRELESS_INTERFACE_LOAD_SKY_REG_TABLE           = 0x2B,
    WIRELESS_INTERFACE_LOAD_GRD_REG_TABLE           = 0x2C,
    WIRELESS_INTERFACE_SWITCH_BB_POWER              = 0x2D,
    WIRELESS_INTERFACE_SKY_ONLY_RX                  = 0x2E,
    WIRELESS_INTERFACE_SWITCH_RF_PWR_0              = 0x2F,
    WIRELESS_INTERFACE_SWITCH_RF_PWR_1              = 0x30,
    WIRELESS_INTERFACE_EXT_ONEKEY_IT                = 0x31,
    WIRELESS_INTERFACE_SWITCH_IT_CHAN               = 0x32,
    WIRELESS_INTERFACE_SWITCH_RMT_CHAN              = 0x33,
    WIRELESS_INTERFACE_SET_PWR_CAL_0                = 0x34,               
    WIRELESS_INTERFACE_SET_PWR_CAL_1                = 0x35,
    WIRELESS_INTERFACE_RST_MCU                      = 0x36,
    WIRELESS_INTERFACE_RF_PWR_AUTO                  = 0x37,
    WIRELESS_INTERFACE_SWITCH_DEBUG_MODE            = 0x38,
    WIRELESS_INTERFACE_WRITE_RF_REG                 = 0x39,
    WIRELESS_INTERFACE_READ_RF_REG                  = 0x3a,
    PAD_FREQUENCY_BAND_WIDTH_SELECT                 = 0x40,
    PAD_FREQUENCY_BAND_OPERATION_MODE               = 0x41,
    PAD_FREQUENCY_BAND_SELECT                       = 0x42,
    PAD_FREQUENCY_CHANNEL_OPERATION_MODE            = 0x43,
    PAD_FREQUENCY_CHANNEL_SELECT                    = 0x44,
    PAD_MCS_OPERATION_MODE                          = 0x45,
    PAD_MCS_MODULATION_MODE                         = 0x46,
    PAD_ENCODER_DYNAMIC_BITRATE_MODE                = 0x47,
    PAD_ENCODER_DYNAMIC_BITRATE_SELECT              = 0x48,
    PAD_WIRELESS_INTERFACE_OSD_DISPLAY              = 0x49,
    PAD_WIRELESS_INTERFACE_PID_NUM
} WIRELESS_INTRTFACE_PID_DEF;


typedef enum
{
    FREQUENCY_BAND_WIDTH_10M     = 0x0,
    FREQUENCY_BAND_WIDTH_20M     = 0x1
} ENUM_FREQUENCY_BAND_WIDTH;


typedef enum
{
    FREQUENCY_BAND_2_4_G        = 0x0,      /* 2.4G */
    FREQUENCY_BAND_5_G          = 0x1       /* 5G */
} ENUM_FREQUENCY_BAND;


typedef enum
{
    MCS_MODULATION_BPSK         = 0x0,
    MCS_MODULATION_QPSK         = 0x1,
    MCS_MODULATION_QAM16        = 0x2,
    MCS_MODULATION_QAM64        = 0x3
} ENUM_MCS_MODULATION_MODE;


typedef enum
{
    MCS_CODE_RATE_1_OF_2        = 0x0,
    MCS_OCDE_RATE_2_OF_3        = 0x1
} ENUM_MCS_CODERATE_MODE;


typedef enum
{
    WIRELESS_AUTO_MODE          = 0,
    WIRELESS_MANUAL_MODE        = 1
} ENUM_WIRELESS_MODE;


/* To PC or PAD, display wireless info */
/*if head == 0x00 && tail == 0xff, the sram data is valid*/
typedef struct
{
    uint16_t        messageId;
    uint16_t        paramLen;
    uint16_t        snr_vlaue[4];
    int16_t         sweep_energy[21*8];     //Max channel: 21
    uint16_t        ldpc_error;
    uint8_t         agc_value[4];
    uint8_t         harq_count;
    uint8_t         modulation_mode;
    uint8_t         ch_bandwidth;
    uint8_t         code_rate;
    uint8_t         encoder_bitrate;
    uint8_t         IT_channel;
    uint8_t         head;
    uint8_t         tail;
    uint8_t         in_debug;
    uint8_t         rc_error;
} STRU_WIRELESS_INFO_DISPLAY;

typedef enum 
{
    MCU_TO_PC,
    MCU_TO_PAD
} ENUM_WIRELESS_TOOL;


typedef struct
{
    uint8_t                     messageId;
    uint8_t                     paramLen;
    uint8_t                     paramData[10];
} STRU_WIRELESS_PARAM_CONFIG_MESSAGE;


typedef void(*WIRELESS_CONFIG_HANDLER)(void *);

void WITELESS_GetOSDInfo(void);
void WIRELESS_SendDisplayInfo(ENUM_WIRELESS_TOOL host);
void WIRELESS_INTERFACE_UPGRADE_Handler(void *param);
void WIRELESS_INTERFACE_ENTER_TEST_MODE_Handler(void *param);
void WIRELESS_INTERFACE_SYNCHRONIZE_FREQ_CHANNEL_Handler(void *param);
void WIRELESS_INTERFACE_AUTO_MODE_Handler(void *param);
void WIRELESS_INTERFACE_SELF_ADAPTION_MODE_Handler(void *param);
void WIRELESS_INTERFACE_SWITCH_TX_MODE_Handler(void *param);
void WIRELESS_INTERFACE_SWITCH_RX_MODE_Handler(void *param);
void WIRELESS_INTERFACE_SWITCH_USB1_MODE_Handler(void *param);
void WIRELESS_INTERFACE_SWITCH_USB2_MODE_Handler(void *param);
void WIRELESS_INTERFACE_ALL_RESET_Handler(void *param);
void WIRELESS_INTERFACE_RX_RESET_Handler(void *param);
void WIRELESS_INTERFACE_TX_RESET_Handler(void *param);
void WIRELESS_INTERFACE_MIMO_1T2R_Handler(void *param);
void WIRELESS_INTERFACE_WRITE_BB_REG_Handler(void *param);
void WIRELESS_INTERFACE_READ_BB_REG_Handler(void *param);
void WIRELESS_INTERFACE_MIMO_2T2R_Handler(void *param);
void WIRELESS_INTERFACE_OSD_DISPLAY_Handler(void *param);
void WIRELESS_INTERFACE_GET_ID_Handler(void *param);
void WIRELESS_INTERFACE_GET_GROUND_TX_PWR_Handler(void *param);
void WIRELESS_INTERFACE_GET_SKY_TX_PWR_Handler(void *param);
void WIRELESS_INTERFACE_GET_RC_FREQ_CHANNEL_Handler(void *param);
void WIRELESS_INTERFACE_GET_VIDEO_FREQ_CHANNEL_Handler(void *param);
void WIRELESS_INTERFACE_GET_SOFTWARE_VERSION_Handler(void *param);
void WIRELESS_INTERFACE_GET_DEV_INFO_Handler(void *param);
void WIRELESS_INTERFACE_SET_RC_FREQ_CHANNEL_MASK_Handler(void *param);
void WIRELESS_INTERFACE_SET_VIDEO_FREQ_CHANNEL_MASK_Handler(void *param);
void WIRELESS_INTERFACE_SELECT_VIDEO_FREQ_CHANNEL_Handler(void *param);
void WIRELESS_INTERFACE_SWITCH_QAM_MODE_Handler(void *param);
void WIRELESS_INTERFACE_SET_RC_FREQ_CHANNEL_Handler(void *param);
void WIRELESS_INTERFACE_SET_TX_PWR_Handler(void *param);
void WIRELESS_INTERFACE_SET_VIDEO_TX_ID_Handler(void *param);
void WIRELESS_INTERFACE_SET_CTRL_ID_Handler(void *param);
void WIRELESS_INTERFACE_SWITCH_WORKING_FREQ_BAND_Handler(void *param);
void WIRELESS_INTERFACE_RC_SCAN_ALL_BAND_Handler(void *param);
void WIRELESS_INTERFACE_RC_SCAN_WORKING_BAND_Handler(void *param);
void WIRELESS_INTERFACE_VIDEO_SCAN_ALL_BAND_Handler(void *param);
void WIRELESS_INTERFACE_VIDEO_SCAN_WORKING_BAND_Handler(void *param);
void WIRELESS_INTERFACE_RECOVER_TO_FACTORY_Handler(void *param);
void WIRELESS_INTERFACE_RC_HOPPING_Handler(void *param);
void WIRELESS_INTERFACE_SAVE_CONFIGURE_Handler(void *param);
void WIRELESS_INTERFACE_READ_MCU_ID_Handler(void *param);
void WIRELESS_INTERFACE_LOAD_SKY_REG_TABLE_Handler(void *param);
void WIRELESS_INTERFACE_LOAD_GRD_REG_TABLE_Handler(void *param);
void WIRELESS_INTERFACE_SWITCH_BB_POWER_Handler(void *param);
void WIRELESS_INTERFACE_SKY_ONLY_RX_Handler(void *param);
void WIRELESS_INTERFACE_SWITCH_RF_PWR_0_Handler(void *param);
void WIRELESS_INTERFACE_SWITCH_RF_PWR_1_Handler(void *param);
void WIRELESS_INTERFACE_EXT_ONEKEY_IT_Handler(void *param);
void WIRELESS_INTERFACE_SWITCH_IT_CHAN_Handler(void *param);
void WIRELESS_INTERFACE_SWITCH_RMT_CHAN_Handler(void *param);
void WIRELESS_INTERFACE_SET_PWR_CAL_0_Handler(void *param);
void WIRELESS_INTERFACE_SET_PWR_CAL_1_Handler(void *param);
void WIRELESS_INTERFACE_RST_MCU_Handler(void *param);
void WIRELESS_INTERFACE_RF_PWR_AUTO_Handler(void *param);
void WIRELESS_INTERFACE_SWITCH_DEBUG_MODE_Handler(void *param);
void WIRELESS_INTERFACE_WRITE_RF_REG_Handler(void *param);
void WIRELESS_INTERFACE_READ_RF_REG_Handler(void *param);
void PAD_FREQUENCY_BAND_WIDTH_SELECT_Handler(void *param);
void PAD_FREQUENCY_BAND_OPERATION_MODE_Handler(void *param);
void PAD_FREQUENCY_BAND_SELECT_Handler(void *param);
void PAD_FREQUENCY_CHANNEL_OPERATION_MODE_Handler(void *param);
void PAD_FREQUENCY_CHANNEL_SELECT_Handler(void *param);
void PAD_MCS_OPERATION_MODE_Handler(void *param);
void PAD_MCS_MODULATION_MODE_Handler(void *param);
void PAD_MCS_CODE_RATE_MODE_Handler(void *param);
void PAD_ENCODER_DYNAMIC_BITRATE_MODE_Handler(void *param);
void PAD_ENCODER_DYNAMIC_BITRATE_SELECT_Handler(void *param);
void PAD_WIRELESS_OSD_DISPLAY_Handler(void *param);
void WIRELESS_ParseParamConfig(void *param);
void convert_endian(void *src_data, void *dst_data, uint32_t dataLen);


#endif

