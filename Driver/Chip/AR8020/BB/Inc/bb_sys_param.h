#ifndef __SYS_PARAM_H
#define __SYS_PARAM_H

#include "bb_ctrl_internal.h"
#include "bb_sys_param.h"


#define QAM_CHANGE_THRESHOLD_COUNT (6)
#define DISABLE_FLAG (0xff)
#define ENABLE_FLAG  (0x00)


typedef struct
{
    uint8_t cur_IT_ch;
    uint8_t next_IT_ch;
    uint8_t it_manual_ch;
    uint8_t it_manual_rf_band;
    uint8_t fec_unlock_cnt;
    uint16_t rc_unlock_cnt;
    
    ENUM_RUN_MODE it_skip_freq_mode;
    ENUM_RUN_MODE rc_skip_freq_mode;
    ENUM_RUN_MODE qam_skip_mode;
    ENUM_RUN_MODE brc_mode;
    ENUM_TRX_CTRL trx_ctrl;
    uint8_t  brc_bps;           //unit: Mbps

    ENUM_RF_BAND freq_band;       //2.5G, 5.8G
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
    uint8_t rc_status;
    uint16_t mosaic;
    uint8_t retrans_num;
    uint8_t default_fac_setting;
    uint8_t search_id_enable;
    uint8_t bb_power;    
    ENUM_RUN_MODE rf_power_mode;
    uint8_t enable_freq_offset;
    DEVICE_STATE dev_state;
    uint8_t u8_idSrcSel; /* 0:id comes from flash,this is default value.
                            1:id comes from automatic search*/
    uint8_t u8_flashId[6]; // 5B(rc_id) + 1B(check)
    uint8_t u8_debugMode;
    uint8_t u8_flagdebugRequest;
    ENUM_BB_MODE en_bbmode;
}CONTEXT;

typedef struct
{
    uint8_t usb_sel;
    uint8_t usb_cofig;
    ENUM_RF_BAND freq_band_sel;
    uint8_t it_mode;
    ENUM_BB_QAM qam_mode;
    uint8_t power;
    uint8_t id_num;
    //MIMO_MODE mimo_mode;
    uint8_t test_enable;
    ENUM_RUN_MODE it_skip_freq_mode;
    ENUM_RUN_MODE rc_skip_freq_mode;
    uint8_t search_id_enable;
    uint8_t freq_band;  
    uint8_t gp20dbm[4];
    uint8_t sp20dbm[4]; 
    ENUM_BB_LDPC ldpc;
    ENUM_RUN_MODE qam_skip_mode;
    uint8_t rc_mask[32];
    uint8_t it_mask[8];
    uint16_t qam_change_threshold[QAM_CHANGE_THRESHOLD_COUNT];
    uint8_t enable_freq_offset;
    uint8_t gp20dbmb[4];
    uint8_t sp20dbmb[4];     
    ENUM_RUN_MODE rf_power_mode;     
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

PARAM * BB_get_sys_param(void);


#endif
