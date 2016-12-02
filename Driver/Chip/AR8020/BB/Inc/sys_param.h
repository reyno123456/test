#ifndef __SYS_PARAM_H
#define __SYS_PARAM_H

#include "BB_ctrl.h"

#define QAM_CHANGE_THRESHOLD_COUNT (6)
#define DISABLE_FLAG (0xff)
#define ENABLE_FLAG  (0x00)

#define FREQ_23BAND_BIT_POS (1 << 0)
#define FREQ_24BAND_BIT_POS (1 << 1)
#define FREQ_25BAND_BIT_POS (1 << 2)
#define FREQ_26BAND_BIT_POS (1 << 3)


typedef enum
{
    IDLE,
    INIT_DATA,
    FEC_UNLOCK,
    FEC_LOCK,
    DELAY_14MS,
    CHECK_FEC_LOCK,
    ID_MATCH_LOCK,
    SEARCH_ID,
    CHECK_ID_MATCH
}DEVICE_STATE;

typedef enum
{
    AUTO,
    MANUAL
}RUN_MODE;

typedef struct
{
    uint8_t cur_IT_ch;
    uint8_t next_IT_ch;
    uint8_t it_manual_ch;
    uint8_t it_manual_rf_band;
    uint8_t fec_unlock_cnt;
    uint8_t rc_unlock_cnt;
    
    RUN_MODE it_skip_freq_mode;
    RUN_MODE rc_skip_freq_mode;
    RUN_MODE qam_skip_mode;
    RUN_MODE brc_mode;
    ENUM_TRX_CTRL trx_ctrl;
    uint8_t  brc_bps;           //unit: 100khz

    ENUM_RF_BAND RF_band;       //2.5G, 5.8G
    ENUM_CH_BW   CH_bandwidth;    //10M, 20M
    ENUM_BB_QAM  qam_mode;
    ENUM_BB_LDPC ldpc;
    uint8_t qam_ldpc;
    uint8_t enable_plot;
    //DEVICE_TYPE dev_type;
    uint8_t id[5];
    uint8_t need_write_flash;
    uint8_t pwr;
    uint8_t rc_mask[32];
    uint8_t it_mask[8];
    uint16_t qam_change_threshold[QAM_CHANGE_THRESHOLD_COUNT];
    uint16_t qam_threshold_range[QAM_CHANGE_THRESHOLD_COUNT][2];
    uint8_t locked;
    uint16_t mosaic;
    uint8_t c5;
    uint8_t c6;
    uint8_t retrans_num;
    uint8_t freq_band;
    uint8_t default_fac_setting;
    uint8_t search_id_enable;
    uint8_t bb_power;
}CONTEXT;

typedef struct
{
     uint8_t usb_sel;
     uint8_t usb_cofig;
     uint8_t freq_band_sel;
     uint8_t it_mode;
     ENUM_BB_QAM qam_mode;
     uint8_t power;
     uint8_t id_num;
     //MIMO_MODE mimo_mode;
     uint8_t test_enable;
     RUN_MODE it_skip_freq_mode;
     RUN_MODE rc_skip_freq_mode;
     uint8_t search_id_enable;
     uint8_t freq_band;     
     ENUM_BB_LDPC ldpc;
     RUN_MODE qam_skip_mode;
     uint8_t rc_mask[32];
     uint8_t it_mask[8];
     uint16_t qam_change_threshold[QAM_CHANGE_THRESHOLD_COUNT];
     uint8_t enable_freq_offset;
}SYS_PARAM;

typedef struct
{
    uint8_t id1[5];
    uint8_t id2[5];
    uint8_t id3[5];
    uint8_t id4[5];
}RC_ID;

typedef struct
{
    uint8_t id1[2];
    uint8_t id2[2];
    uint8_t id3[2];
    uint8_t id4[2];
}IT_ID;


typedef struct param
{
    uint8_t is_init;
    uint8_t update;
    RC_ID   rc_id;
    IT_ID   it_id;
    SYS_PARAM user_param;
}PARAM;

extern CONTEXT context;
extern volatile DEVICE_STATE dev_state;
extern PARAM *sys_param;

#endif


