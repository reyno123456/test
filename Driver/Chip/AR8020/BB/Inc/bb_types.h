#ifndef __BB_TYPES_H_
#define __BB_TYPES_H_

#include "memory_config.h"

typedef enum
{
    AUTO,
    MANUAL
}ENUM_RUN_MODE;


typedef enum
{
    BB_SKY_MODE     = 0x00,
    BB_GRD_MODE     = 0x01,
    BB_MODE_UNKNOWN = 0xFF
} ENUM_BB_MODE;


typedef enum
{
    IT_ONLY_MODE    = 0x00,     //iamge transmit only
    IT_RC_MODE      = 0x01,     //iamge transmit and RC mode
    IT_MAX_TRX_CTRL = 0xff
} ENUM_TRX_CTRL;


typedef enum _ENUM_RF_BAND
{
    RF_2G       = 0x00,
    RF_5G       = 0x01,
}ENUM_RF_BAND;


typedef enum _ENUM_BB_QAM
{
    MOD_BPSK    = 0x00,
    MOD_4QAM    = 0x01,
    MOD_16QAM   = 0x02,
    MOD_64QAM   = 0x03,
    MOD_MAX     = 0xff,
}ENUM_BB_QAM;


typedef enum ENUM_BB_LDPC
{
    LDPC_1_2    = 0x00,
    LDPC_2_3    = 0x01,
    LDPC_3_4    = 0x02,
    LDPC_5_6    = 0x03,
}ENUM_BB_LDPC;


typedef enum _ENUM_CH_BW
{
    BW_10M      = 0x02,
    BW_20M      = 0x00,
}ENUM_CH_BW;



typedef enum
{
    WIRELESS_FREQ_CHANGE,
    WIRELESS_MCS_CHANGE,
    WIRELESS_ENCODER_CHANGE,
    WIRELESS_DEBUG_CHANGE,
    WIRELESS_MISC,
    WIRELESS_AUTO_SEARCH_ID,
    WIRELESS_OTHER,
} ENUM_WIRELESS_CONFIG_CHANGE;


typedef enum
{
    FREQ_BAND_WIDTH_SELECT,
    FREQ_BAND_SELECT,
    FREQ_BAND_MODE,
    FREQ_CHANNEL_MODE,
    FREQ_CHANNEL_SELECT,
    RC_CHANNEL_MODE,
    RC_CHANNEL_SELECT,
    RC_CHANNEL_FREQ,
    IT_CHANNEL_FREQ,
    MICS_IT_ONLY_MODE,
    GET_DEV_INFO,
    SWITCH_ON_OFF_CH1,
    SWITCH_ON_OFF_CH2,
    BB_SOFT_RESET,
} ENUM_WIRELESS_FREQ_CHANGE_ITEM;


typedef enum
{
    MCS_MODE_SELECT,
    MCS_MODULATION_SELECT,
    MCS_CODE_RATE_SELECT,
    MCS_IT_QAM_SELECT,
    MCS_IT_CODE_RATE_SELECT,
    MCS_RC_QAM_SELECT,
    MCS_RC_CODE_RATE_SELECT,
} ENUM_WIRELESS_MCS_CHANGE_ITEM;


typedef enum
{
    ENCODER_DYNAMIC_BIT_RATE_MODE,
    ENCODER_DYNAMIC_BIT_RATE_SELECT_CH1,
    ENCODER_DYNAMIC_BIT_RATE_SELECT_CH2
} ENUM_WIRELESS_ENCODER_CHANGE_ITEM;


typedef struct
{
    uint8_t                 u8_configClass;
    uint8_t                 u8_configItem;
    uint32_t                u32_configValue;
} STRU_WIRELESS_CONFIG_CHANGE;


typedef enum
{
    BB_UART_COM_SESSION_0 = 0,
    BB_UART_COM_SESSION_1,
    BB_UART_COM_SESSION_2,
    BB_UART_COM_SESSION_3,
    BB_UART_COM_SESSION_4,
    BB_UART_COM_SESSION_MAX
} ENUM_BBUARTCOMSESSIONID;


typedef enum
{
    PAGE0 = 0x00,
    PAGE1 = 0x40,
    PAGE2 = 0x80,
    PAGE3 = 0xc0,
    PAGE_UNKNOW = 0xFF,
} ENUM_REG_PAGES;


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
    uint8_t         osd_enable;
    uint8_t         IT_channel;
    uint8_t         head;
    uint8_t         tail;
    uint8_t         in_debug;
    uint8_t         lock_status;
    uint16_t        video_width[2];
    uint16_t        video_height[2];
    uint8_t         frameRate[2];
    uint8_t         encoder_bitrate[2];
    uint8_t         rc_modulation_mode;
    uint8_t         rc_code_rate;
    uint8_t         encoder_status;
    uint8_t         reserved[1];
} STRU_WIRELESS_INFO_DISPLAY;


typedef struct
{
    uint8_t         messageId;
    uint8_t         paramLen;
    uint8_t         skyGround;
    uint8_t         band;
    uint8_t         bandWidth;
    uint8_t         itHopping;
    uint8_t         rcHopping;
    uint8_t         adapterBitrate;
    uint8_t         channel1_on;
    uint8_t         channel2_on;
    uint8_t         isDebug;
    uint8_t         itQAM;
    uint8_t         itCodeRate;
    uint8_t         rcQAM;
    uint8_t         rcCodeRate;
    uint8_t         ch1Bitrates;
    uint8_t         ch2Bitrates;
    uint8_t         reserved[3];
    uint8_t         u8_itRegs[4];
    uint8_t         u8_rcRegs[4];    
    uint8_t         u8_startWrite;  //u8_startWrite: cpu2 start update the flag
    uint8_t         u8_endWrite;    //u8_endWrite:   cpu2 end update the flag
} STRU_DEVICE_INFO;

// CPU0 and CPU2 share memory for osd status info, offset in SRAM: 16K + 512Byte
// last 16 bytes is for DEVICE INFO
#define OSD_STATUS_SHM_ADDR              SRAM_BB_STATUS_SHARE_MEMORY_ST_ADDR
#define DEVICE_INFO_SHM_SIZE             (sizeof(STRU_DEVICE_INFO))
#define DEVICE_INFO_SHM_ADDR             ((SRAM_BB_STATUS_SHARE_MEMORY_ST_ADDR + SRAM_BB_STATUS_SHARE_MEMORY_SIZE) - DEVICE_INFO_SHM_SIZE)

#endif
