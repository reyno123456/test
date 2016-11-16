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

#include "config_functions_sel.h"
#include <stdint.h>
#include "debuglog.h"

//*************************  Data Structure Define  **********************

void grd_add_snr_daq(void);

void Grd_Parm_Initial(void);

void wimax_vsoc_tx_isr(void);

void Grd_Timer1_Init(void);

void Grd_Timer0_Init(void);

void grd_rc_hopfreq(void);

void grd_set_it_work_freq(uint8_t ch);

void reset_it_span_cnt(void);

uint8_t grd_is_bb_fec_lock(void);

uint8_t is_it_need_skip_freq(uint8_t qam_ldpc);

void grd_set_it_skip_freq(uint8_t ch);

void grd_set_txmsg_qam_change(EN_BB_QAM qam, EN_BB_BW bw, EN_BB_LDPC ldpc);

uint8_t merge_qam_ldpc_to_index(EN_BB_QAM qam, EN_BB_LDPC ldpc);

uint8_t snr_static_for_qam_change(uint16_t threshod_left_section,uint16_t threshold_right_section);

#endif
