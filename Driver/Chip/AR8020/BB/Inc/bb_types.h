#ifndef __BB_TYPES_H_
#define __BB_TYPES_H_


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
    MICS_IT_ONLY_MODE
} ENUM_WIRELESS_FREQ_CHANGE_ITEM;


typedef enum
{
    MCS_MODE_SELECT,
    MCS_MODULATION_SELECT,
    MCS_CODE_RATE_SELECT
} ENUM_WIRELESS_MCS_CHANGE_ITEM;


typedef enum
{
    ENCODER_DYNAMIC_BIT_RATE_MODE,
    ENCODER_DYNAMIC_BIT_RATE_SELECT
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


#endif
