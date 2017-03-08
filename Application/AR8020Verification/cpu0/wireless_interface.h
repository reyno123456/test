#ifndef __WIRELESS_INTERFACE_H
#define __WIRELESS_INTERFACE_H

#include <stdint.h>
#include "memory_config.h"

#define WIRELESS_INTERFACE_MAX_MESSAGE_NUM          0x10
#define WIRELESS_INTERFACE_SEND_FAIL_RETRY          25


#define HID_UPGRADE_BASE_ADDR                       ((uint8_t *)0x81000000)
#define HID_UPGRADE_ONE_PACKET                      508

#define HID_UPGRADE_FLASH_SECTOR_SIZE               4096
#define HID_UPGRADE_FLASH_SECTOR_NUM                16

#define HID_UPGRADE_APP_ADDR_IN_NOR                 0x20000
#define HID_UPGRADE_FLASH_BASE_ADDR                 ((uint8_t *)0x10000000)


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
    WIRELESS_INTERFACE_READ_RF_REG                  = 0x39,
    WIRELESS_INTERFACE_WRITE_RF_REG                 = 0x3A,
    WIRELESS_INTERFACE_OPEN_ADAPTION_BIT_STREAM     = 0x3B,
    WIRELESS_INTERFACE_SWITCH_CH1                   = 0x3C,
    WIRELESS_INTERFACE_SWITCH_CH2                   = 0x3D,
    WIRELESS_INTERFACE_SET_CH1_CODE_RATE            = 0x3E,
    WIRELESS_INTERFACE_SET_CH2_CODE_RATE            = 0x3F,
    WIRELESS_INTERFACE_VIDEO_QAM                    = 0x40,
    WIRELESS_INTERFACE_VIDEO_BIT_RATE               = 0x41,
    WIRELESS_INTERFACE_RC_QAM                       = 0x42,
    WIRELESS_INTERFACE_RC_BIT_RATE                  = 0x43,
    WIRELESS_INTERFACE_OPEN_VIDEO                   = 0x44,
    WIRELESS_INTERFACE_CLOSE_VIDEO                  = 0x45,
    WIRELESS_INTERFACE_VIDEO_AUTO_HOPPING           = 0x46,
    WIRELESS_INTERFACE_VIDEO_BAND_WIDTH             = 0x47,
    WIRELESS_INTERFACE_RESET_BB                     = 0x48,
    WIRELESS_INTERFACE_OPERATE_REG                  = 0x49,
    PAD_FREQUENCY_BAND_WIDTH_SELECT                 = 0x50,
    PAD_FREQUENCY_BAND_OPERATION_MODE               = 0x51,
    PAD_FREQUENCY_BAND_SELECT                       = 0x52,
    PAD_FREQUENCY_CHANNEL_OPERATION_MODE            = 0x53,
    PAD_FREQUENCY_CHANNEL_SELECT                    = 0x54,
    PAD_MCS_OPERATION_MODE                          = 0x55,
    PAD_MCS_MODULATION_MODE                         = 0x56,
    PAD_ENCODER_DYNAMIC_BITRATE_MODE                = 0x57,
    PAD_ENCODER_DYNAMIC_BITRATE_SELECT              = 0x58,
    PAD_WIRELESS_INTERFACE_OSD_DISPLAY              = 0x59,
    MAX_PID_NUM
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


typedef enum 
{
    MCU_TO_PC,
    MCU_TO_PAD
} ENUM_WIRELESS_TOOL;


typedef struct
{
    uint8_t                     messageId;
    uint8_t                     paramLen;
    uint8_t                     paramData[18];
} STRU_WIRELESS_PARAM_CONFIG_MESSAGE;


typedef struct
{
    STRU_WIRELESS_PARAM_CONFIG_MESSAGE      stMsgPool[WIRELESS_INTERFACE_MAX_MESSAGE_NUM];
    uint8_t                                 u8_buffHead;
    uint8_t                                 u8_buffTail;
    uint16_t                                u16_sendfailCount;
} STRU_WIRELESS_MESSAGE_BUFF;



typedef struct
{
    uint8_t                     messageId;
    uint8_t                     packetFlag;
    uint16_t                    imageLen;
    uint8_t                     imageText[HID_UPGRADE_ONE_PACKET];
} STRU_USBD_UPGRADE_PACKET;



typedef uint8_t(*WIRELESS_CONFIG_HANDLER)(void *);

void WITELESS_GetOSDInfo(void);
void WIRELESS_SendDisplayInfo(ENUM_WIRELESS_TOOL host);
uint8_t WIRELESS_INTERFACE_UPGRADE_Handler(void *param);
uint8_t WIRELESS_INTERFACE_ENTER_TEST_MODE_Handler(void *param);
uint8_t WIRELESS_INTERFACE_SYNCHRONIZE_FREQ_CHANNEL_Handler(void *param);
uint8_t WIRELESS_INTERFACE_AUTO_MODE_Handler(void *param);
uint8_t WIRELESS_INTERFACE_SELF_ADAPTION_MODE_Handler(void *param);
uint8_t WIRELESS_INTERFACE_SWITCH_TX_MODE_Handler(void *param);
uint8_t WIRELESS_INTERFACE_SWITCH_RX_MODE_Handler(void *param);
uint8_t WIRELESS_INTERFACE_SWITCH_USB1_MODE_Handler(void *param);
uint8_t WIRELESS_INTERFACE_SWITCH_USB2_MODE_Handler(void *param);
uint8_t WIRELESS_INTERFACE_ALL_RESET_Handler(void *param);
uint8_t WIRELESS_INTERFACE_RX_RESET_Handler(void *param);
uint8_t WIRELESS_INTERFACE_TX_RESET_Handler(void *param);
uint8_t WIRELESS_INTERFACE_MIMO_1T2R_Handler(void *param);
uint8_t WIRELESS_INTERFACE_WRITE_BB_REG_Handler(void *param);
uint8_t WIRELESS_INTERFACE_READ_BB_REG_Handler(void *param);
uint8_t WIRELESS_INTERFACE_MIMO_2T2R_Handler(void *param);
uint8_t WIRELESS_INTERFACE_OSD_DISPLAY_Handler(void *param);
uint8_t WIRELESS_INTERFACE_GET_ID_Handler(void *param);
uint8_t WIRELESS_INTERFACE_GET_GROUND_TX_PWR_Handler(void *param);
uint8_t WIRELESS_INTERFACE_GET_SKY_TX_PWR_Handler(void *param);
uint8_t WIRELESS_INTERFACE_GET_RC_FREQ_CHANNEL_Handler(void *param);
uint8_t WIRELESS_INTERFACE_GET_VIDEO_FREQ_CHANNEL_Handler(void *param);
uint8_t WIRELESS_INTERFACE_GET_SOFTWARE_VERSION_Handler(void *param);
uint8_t WIRELESS_INTERFACE_GET_DEV_INFO_Handler(void *param);
uint8_t WIRELESS_INTERFACE_SET_RC_FREQ_CHANNEL_MASK_Handler(void *param);
uint8_t WIRELESS_INTERFACE_SET_VIDEO_FREQ_CHANNEL_MASK_Handler(void *param);
uint8_t WIRELESS_INTERFACE_SELECT_VIDEO_FREQ_CHANNEL_Handler(void *param);
uint8_t WIRELESS_INTERFACE_SWITCH_QAM_MODE_Handler(void *param);
uint8_t WIRELESS_INTERFACE_SET_RC_FREQ_CHANNEL_Handler(void *param);
uint8_t WIRELESS_INTERFACE_SET_TX_PWR_Handler(void *param);
uint8_t WIRELESS_INTERFACE_SET_VIDEO_TX_ID_Handler(void *param);
uint8_t WIRELESS_INTERFACE_SET_CTRL_ID_Handler(void *param);
uint8_t WIRELESS_INTERFACE_SWITCH_WORKING_FREQ_BAND_Handler(void *param);
uint8_t WIRELESS_INTERFACE_RC_SCAN_ALL_BAND_Handler(void *param);
uint8_t WIRELESS_INTERFACE_RC_SCAN_WORKING_BAND_Handler(void *param);
uint8_t WIRELESS_INTERFACE_VIDEO_SCAN_ALL_BAND_Handler(void *param);
uint8_t WIRELESS_INTERFACE_VIDEO_SCAN_WORKING_BAND_Handler(void *param);
uint8_t WIRELESS_INTERFACE_RECOVER_TO_FACTORY_Handler(void *param);
uint8_t WIRELESS_INTERFACE_RC_HOPPING_Handler(void *param);
uint8_t WIRELESS_INTERFACE_SAVE_CONFIGURE_Handler(void *param);
uint8_t WIRELESS_INTERFACE_READ_MCU_ID_Handler(void *param);
uint8_t WIRELESS_INTERFACE_LOAD_SKY_REG_TABLE_Handler(void *param);
uint8_t WIRELESS_INTERFACE_LOAD_GRD_REG_TABLE_Handler(void *param);
uint8_t WIRELESS_INTERFACE_SWITCH_BB_POWER_Handler(void *param);
uint8_t WIRELESS_INTERFACE_SKY_ONLY_RX_Handler(void *param);
uint8_t WIRELESS_INTERFACE_SWITCH_RF_PWR_0_Handler(void *param);
uint8_t WIRELESS_INTERFACE_SWITCH_RF_PWR_1_Handler(void *param);
uint8_t WIRELESS_INTERFACE_EXT_ONEKEY_IT_Handler(void *param);
uint8_t WIRELESS_INTERFACE_SWITCH_IT_CHAN_Handler(void *param);
uint8_t WIRELESS_INTERFACE_SWITCH_RMT_CHAN_Handler(void *param);
uint8_t WIRELESS_INTERFACE_SET_PWR_CAL_0_Handler(void *param);
uint8_t WIRELESS_INTERFACE_SET_PWR_CAL_1_Handler(void *param);
uint8_t WIRELESS_INTERFACE_RST_MCU_Handler(void *param);
uint8_t WIRELESS_INTERFACE_RF_PWR_AUTO_Handler(void *param);
uint8_t WIRELESS_INTERFACE_SWITCH_DEBUG_MODE_Handler(void *param);
uint8_t WIRELESS_INTERFACE_WRITE_RF_REG_Handler(void *param);
uint8_t WIRELESS_INTERFACE_READ_RF_REG_Handler(void *param);
uint8_t WIRELESS_INTERFACE_OPEN_VIDEO_Handler(void *param);
uint8_t WIRELESS_INTERFACE_CLOSE_VIDEO_Handler(void *param);
uint8_t WIRELESS_INTERFACE_RESET_BB_Handler(void *param);
uint8_t PAD_FREQUENCY_BAND_WIDTH_SELECT_Handler(void *param);
uint8_t PAD_FREQUENCY_BAND_OPERATION_MODE_Handler(void *param);
uint8_t PAD_FREQUENCY_BAND_SELECT_Handler(void *param);
uint8_t PAD_FREQUENCY_CHANNEL_OPERATION_MODE_Handler(void *param);
uint8_t PAD_FREQUENCY_CHANNEL_SELECT_Handler(void *param);
uint8_t PAD_MCS_OPERATION_MODE_Handler(void *param);
uint8_t PAD_MCS_MODULATION_MODE_Handler(void *param);
uint8_t PAD_MCS_CODE_RATE_MODE_Handler(void *param);
uint8_t PAD_ENCODER_DYNAMIC_BITRATE_MODE_Handler(void *param);
uint8_t PAD_ENCODER_DYNAMIC_BITRATE_SELECT_Handler(void *param);
uint8_t PAD_WIRELESS_OSD_DISPLAY_Handler(void *param);
void WIRELESS_ParseParamConfig(void *param);
void Wireless_TaskInit(void);
static void Wireless_InsertMsgIntoReplyBuff(STRU_WIRELESS_PARAM_CONFIG_MESSAGE *pstMessage);
uint8_t WIRELESS_INTERFACE_OPEN_ADAPTION_BIT_STREAM_Handler(void *param);
uint8_t WIRELESS_INTERFACE_SWITCH_CH1_Handler(void *param);
uint8_t WIRELESS_INTERFACE_SWITCH_CH2_Handler(void *param);
uint8_t WIRELESS_INTERFACE_SET_CH1_BIT_RATE_Handler(void *param);
uint8_t WIRELESS_INTERFACE_SET_CH2_BIT_RATE_Handler(void *param);
uint8_t WIRELESS_INTERFACE_VIDEO_QAM_Handler(void *param);
uint8_t WIRELESS_INTERFACE_VIDEO_CODE_RATE_Handler(void *param);
uint8_t WIRELESS_INTERFACE_RC_QAM_Handler(void *param);
uint8_t WIRELESS_INTERFACE_RC_CODE_RATE_Handler(void *param);
uint8_t WIRELESS_INTERFACE_VIDEO_AUTO_HOPPING_Handler(void *param);
uint8_t WIRELESS_INTERFACE_VIDEO_BAND_WIDTH_Handler(void *param);
uint8_t WIRELESS_INTERFACE_OPERATE_REG_Handler(void *param);

#endif

