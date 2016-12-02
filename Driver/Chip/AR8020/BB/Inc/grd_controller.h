/**
  ******************************************************************************
  * @file    ground_controller.h
  * @author  Artosyn AE/FAE Team
  * @version V1.0
  * @date    03-21-2016
  * @brief
  *
  *
  ******************************************************************************
  */
#ifndef __GRD_CONTROLLER_H
#define __GRD_CONTROLLER_H

//#include "config_functions_sel.h"
#include <stdint.h>
#include "debuglog.h"

//*************************  Data Structure Define  **********************

void grd_add_snr_daq(void);

void Grd_Parm_Initial(void);

void BB_Grd_Id_Initial(void);

void wimax_vsoc_tx_isr(void);

void Grd_Timer1_Init(void);

void Grd_Timer0_Init(void);

void grd_rc_hopfreq(void);

void grd_set_it_work_freq(ENUM_RF_BAND rf_band, uint8_t ch);

void reset_it_span_cnt(void);

uint8_t grd_is_bb_fec_lock(void);

uint8_t is_it_need_skip_freq(uint8_t qam_ldpc);

void grd_set_it_skip_freq(uint8_t ch);

void grd_set_txmsg_qam_change(ENUM_BB_QAM qam, ENUM_CH_BW bw, ENUM_BB_LDPC ldpc);

uint8_t merge_qam_ldpc_to_index(ENUM_BB_QAM qam, ENUM_BB_LDPC ldpc);

uint8_t snr_static_for_qam_change(uint16_t threshod_left_section,uint16_t threshold_right_section);

void grd_handle_IT_mode_cmd(RUN_MODE mode);

void grd_handle_IT_CH_cmd(uint8_t ch);

static void grd_handle_RC_cmd(RUN_MODE mode, uint8_t ch);

static void grd_handle_RF_band_cmd(ENUM_RF_BAND rf_band);

static void grd_handle_CH_bandwitdh_cmd(ENUM_CH_BW bw);

static void grd_handle_MCS_mode_cmd(RUN_MODE mode);

static void grd_handle_MCS_cmd(ENUM_BB_QAM qam, ENUM_BB_LDPC ldpc);

static void grd_handle_brc_mode_cmd(RUN_MODE mode);

static void grd_handle_brc_cmd(uint8_t coderate);

void grd_handle_all_cmds(void);

void grd_handle_events_callback(void *p);

void grd_get_osd_info(void);

void grd_add_spi_cmds(uint32_t type, uint32_t value);

#endif
