#ifndef __SYS_PARAM_H
#define __SYS_PARAM_H

#include "bb_ctrl_internal.h"


#define QAM_CHANGE_THRESHOLD_COUNT (6)

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
    uint8_t  brc_bps[2];           //unit: Mbps

    ENUM_RF_BAND freq_band;       //2.5G, 5.8G
    ENUM_CH_BW   CH_bandwidth;    //10M, 20M
    ENUM_BB_QAM  qam_mode;
    ENUM_BB_QAM  rc_qam_mode;
    ENUM_BB_LDPC ldpc;
    ENUM_BB_LDPC rc_ldpc;
    uint8_t qam_ldpc;
    uint8_t enable_plot;
    //DEVICE_TYPE dev_type;
    uint8_t id[5];
    uint8_t need_write_flash;
    uint8_t pwr;
    uint16_t qam_threshold_range[QAM_CHANGE_THRESHOLD_COUNT][2];
    uint8_t locked;
    uint8_t rc_status;
    uint16_t mosaic;
    uint8_t default_fac_setting;
    uint8_t search_id_enable;
    uint8_t bb_power;    
    ENUM_RUN_MODE rf_power_mode;
    uint8_t enable_freq_offset;
    uint8_t flag_mrs;
    uint8_t flag_updateRc; //need to update Remote controller frequency
    uint32_t u32_rcValue;
    uint16_t cycle_count;
    uint32_t qam_switch_time;
    uint8_t ldpc_error_move_cnt;
    DEVICE_STATE dev_state;
    uint8_t u8_idSrcSel; /* 0:id comes from flash,this is default value.
                            1:id comes from automatic search*/
    uint8_t u8_flashId[6]; // 5B(rc_id) + 1B(check)
    uint8_t u8_debugMode;
    uint8_t u8_flagdebugRequest;
    STRU_FRQ_CHANNEL stru_rcRegs;
    STRU_FRQ_CHANNEL stru_itRegs;
    ENUM_BB_MODE en_bbmode;
    uint8_t agclevel;
}CONTEXT;

typedef struct param
{
    ENUM_RF_BAND  freq_band;
    ENUM_RUN_MODE it_skip_freq_mode;
    ENUM_RUN_MODE rc_skip_freq_mode;
    ENUM_RUN_MODE qam_skip_mode;
    uint16_t qam_change_threshold[QAM_CHANGE_THRESHOLD_COUNT][2];
}PARAM;

#define CALC_DIST_RAW_DATA_MAX_RECORD    (100)

typedef enum
{
    INVALID,
    CALI_ZERO,
    CALC_DIST_PREPARE,
    CALC_DIST
}ENUM_CALC_DIST_STATUS;

typedef struct
{
    ENUM_CALC_DIST_STATUS e_status;
    uint8_t u8_rawData[CALC_DIST_RAW_DATA_MAX_RECORD][3];
    uint32_t u32_calcDistValue;
    uint32_t u32_calcDistZero;
    uint32_t u32_cnt;
    uint32_t u32_lockCnt;
} STRU_CALC_DIST_DATA;


#define RC_FRQ_MAX_MASK_NUM         (10)
#define RC_FRQ_MASK_THRESHOLD       (5)
#define RF_FRQ_MAX_NUM              ((MAX_2G_RC_FRQ_SIZE>=MAX_5G_RC_FRQ_SIZE)?MAX_2G_RC_FRQ_SIZE:MAX_5G_RC_FRQ_SIZE)
typedef struct
{
    uint64_t u64_mask; // bit0 <-> freq0  ... bit63 <-> freq63
    uint64_t u64_rcvGrdMask; // received mask from spi, send by grd
    uint8_t  u8_unLock[RF_FRQ_MAX_NUM]; // continue unlock cnt 
    uint8_t  u8_maskOrderIndex;
    uint8_t  u8_maskOrder[RC_FRQ_MAX_MASK_NUM]; // u8_maskOrder[0] earlist masked frq's channel                                      
} STRU_RC_FRQ_MASK;




extern volatile CONTEXT context;
extern volatile DEVICE_STATE dev_state;

PARAM * BB_get_sys_param(void);


#endif
