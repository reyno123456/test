#ifndef __CONFIG_BASEBAND_REGISTER_H
#define __CONFIG_BASEBAND_REGISTER_H

/** ******************************************************************************
  * @file    register.c
  * @author  Artosyn AE/FAE Team _ HJ
  * @version V2.0.0
  * @date    01-25-2016
  * @brief
  *          This file contains all registers address for AR8003 or AD9363.
  * ****************************************************************************** */

#define FSM_0      ((uint8_t)0x00 )  /*[7]page_sel 1: TRX FSM and TX, page1  0: RX, page2
                                      [6]fec_reset_en
                                      [5]fch_reset_en
                                      [4]pkd_reset_en
                                      [3]force to 1
                                      [2]manual_reset_tx_en  1'b1: reset TX on  1'b0: off
                                      [1]manual_reset_rx_en  1'b1: reset RX on  1'b0: off
                                      [0]manual_reset_trx_en 1'b1: reset TX/RX on 1'b0: off
                                    */
#define FSM_1      ((uint8_t)0x01 )  /*[7:2]reserved
                                      [1:0]spi_bus_sel
                                           2'b00: spi for fpga
                                           2'b01: spi for ad9363 or ar8003
                                           2'b10: spi from agc to ad9363 or ar8003
                                    */


// ************                    Page1                ***************

#define TX_2       ((uint8_t)0x04 )/* page 2
                                      QAM_mode        RW  [7:6] 2 0x2
                                      BandWidth       RW  [5:3] 3 0x2
                                      code_rate_enum  RW  [2:0] 3 0x0
                                   */

#define TX_3       ((uint8_t)(0x5b))  //RC_id_bits[39:32]
#define TX_4       ((uint8_t)(0x5c))  //RC_id_bits[31:24]
#define TX_5       ((uint8_t)(0x5d))  //RC_id_bits[23:16]
#define TX_6       ((uint8_t)(0x5e))  //RC_id_bits[15:8]
#define TX_7       ((uint8_t)(0x5f))  //RC_id_bits[7:0]

// custom
#define IT_FREQ_TX_0    ((uint8_t)0x60 )
#define IT_FREQ_TX_1    ((uint8_t)0x61 )

#define RC_FREQ_TX_0    ((uint8_t)0x62 )
#define RC_FREQ_TX_1    ((uint8_t)0x63 )

#define QAM_CHANGE_0    ((uint8_t)0x64 )
#define QAM_CHANGE_1    ((uint8_t)0x65 )

#define RC_FH_PATTERN   ((uint8_t)0x66 )


#define CALI_0_1     ((uint8_t)(0xD0+0x00))/* page 0 0xD0+0x00
                                            txa_i_ofs RD  7 0 8
                                           */
#define CALI_0_2     ((uint8_t)(0xD0+0x00))/*
                                            txa_q_ofs RD  7 0 8
                                           */
#define CALI_0     ((uint8_t)0x90 ) /*[7:0] If reg_rd_cali_result is 0, txa_i_ofs
                                            If reg_rd_cali_result is 1, rxa_i_ofs
                                    */

#define CALI_1     ((uint8_t)0x91 ) /*[7:0] If reg_rd_cali_result is 0, txa_q_ofs
                                            If reg_rd_cali_result is 1, rxa_q_ofs
                                    */
#define CALI_2     ((uint8_t)0x92 ) /*[7:0] If reg_rd_cali_result is 0, txa_sin_ofs[7:0]
                                            If reg_rd_cali_result is 1, rxa_sin_ofs[7:0]
                                    */
#define CALI_3     ((uint8_t)0x93 ) /*[7:4] Reserved
                                      [3:0] If reg_rd_cali_result is 0, txa_sin_ofs[11:8]
                                            If reg_rd_cali_result is 1, rxa_sin_ofs[11:8]
                                    */
#define CALI_4     ((uint8_t)0x94 ) /*[7:0] If reg_rd_cali_result is 0, txa_alph_ofs
                                            If reg_rd_cali_result is 1, rxa_alph_ofs
                                    */
#define CALI_5     ((uint8_t)0x95 ) /*[7:0] If reg_rd_cali_result is 0, txb_i_ofs
                                            If reg_rd_cali_result is 1, rxb_i_ofs
                                    */
#define CALI_6     ((uint8_t)0x96 ) /*[7:0] If reg_rd_cali_result is 0, txb_q_ofs
                                            If reg_rd_cali_result is 1, rxb_q_ofs
                                    */

#define CALI_7     ((uint8_t)0x97 ) /*[7:0] If reg_rd_cali_result is 0, txb_sin_ofs[7:0]
                                            If reg_rd_cali_result is 1, rxb_sin_ofs[7:0]
                                    */
#define CALI_8     ((uint8_t)0x98 ) /*[7:4] Reserved
                                      [3:0] If reg_rd_cali_result is 0, txb_sin_ofs[11:8]
                                            If reg_rd_cali_result is 1, rxb_sin_ofs[11:8]
                                    */

#define CALI_9     ((uint8_t)0x99 ) /*[7:0] If reg_rd_cali_result is 0, txb_alph_ofs
                                            If reg_rd_cali_result is 1, rxb_alph_ofs
                                    */

#define CALI_A     ((uint8_t)0x9A ) //[7:0] tssi_dc_code
#define CA_0       ((uint8_t)0xA0 )
#define CA_1       ((uint8_t)0xA1 )     /*
                                           [7]reg_rx_cali_on
                                           [6]reg_rx_dcoff_on
                                           [5]reg_rx_iq_on
                                           [4]reg_rx_gain_on
                                           [3]reg_tx_cali_on
                                           [2]reg_tx_dcoff_on
                                           [1]reg_tx_iq_on
                                           [0]reg_tx_gain_on
                                       */

#define     SWEEP_ENERGY_HIGH       (0xa2)
#define     SWEEP_ENERGY_MID        (0xa3)
#define     SWEEP_ENERGY_LOW        (0xa4)

#define CA_5       ((uint8_t)0xA5 )     /*
                                          [7:0]reg_fix_tx_sign; 1: minus; 0:plus
                                          [7]  reg_txa_ofs_i_sign
                                          [6]  reg_txa_ofs_q_sign
                                          [5]  reg_txa_alph_sign
                                          [4]  reg_txa_phase_sign
                                          [3]  reg_txb_ofs_i_sign
                                          [2]  reg_txb_ofs_q_sign
                                          [1]  reg_txb_alph_sign
                                          [0]  reg_txb_phase_sign
                                        */
#define CA_6        ((uint8_t)0xA6 )     /*[7:4] reg_txa_ofs_q
                                           [3:0] reg_txa_ofs_i
                                        */
#define CA_7        ((uint8_t)0xA7 )    /*[7:4] reg_txa_alph
                                          [3:0] reg_txa_cos_ofs
                                        */
#define CA_8        ((uint8_t)0xA8 )    /*[7:0] reg_txa_sin_ofs

                                        */
#define CA_9        ((uint8_t)0xA9 )    /*[7:4] reg_txb_ofs_q
                                          [3:0] reg_txb_ofs_i
                                        */
#define CA_A        ((uint8_t)0xAA )    /*[7:4] reg_txb_alph
                                          [3:0] reg_txb_cos_ofs
                                        */
#define CA_B        ((uint8_t)0xAB )     // [7:0]reg_txb_sin_ofs
#define CA_C        ((uint8_t)0xAC )     // [7:0]reg_tx_gain_cali
#define CA_D        ((uint8_t)0xAD )     // [7:0]reg_txgain_for_rxcali
#define CA_E        ((uint8_t)0xAE )     // [7:0]reg_tssi_dc_code

#define CA_C        ((uint8_t)0xAC )      // [7:0]reg_tx_gain_cali
#define AGC4_e      ((uint8_t)0xCE )

// ************                    Page2                ***************

#define FSM_2       ((uint8_t)0x02 )/* [7]  s_rec_time_decision
                                       [6]  s_send_time_decision
                                       [5]  g_rec_time_decision
                                       [4]  g_send_time_decision
                                       [3]  reg_rd_freeze
                                       [2]  send_inside_en
                                       [1]  it_en
                                       [0]  rc_en
                                      */
#define FSM_3_H    ((uint8_t) 0x03 )  //Idle_time_packet[23:16]
#define FSM_3_M    ((uint8_t) 0x04 )  //Idle_time_packet[15:8]
#define FSM_3_L    ((uint8_t) 0x05 )  //Idle_time_packet[7:0]
#define FSM_4_H    ((uint8_t) 0x06 )  //Idle_times[23:16]
#define FSM_4_M    ((uint8_t) 0x07 )  //Idle_times[15:8]
#define FSM_4_L    ((uint8_t) 0x08 )  //Idle_times[7:0]

#define MCS_0      ((uint8_t) 0x09 )  /*[7]  mcs_auto_switch_on
                                        [6]  one_mcs_reset_en
                                        [5]  mcs5_on
                                        [4]  mcs4_on
                                        [3]  mcs3_on
                                        [2]  mcs2_on
                                        [1]  mcs1_on
                                        [0]  mcs0_on
                                      */
#define MCS_1      ((uint8_t) 0x0A )  /*[7:2]Reserved
                                        [1]  mcs_ldpc_judge_on
                                        [0]  mcs_rssi_judge_on
                                      */

#define MCS_2      ((uint8_t) 0x0B )  // [7:0]mcs_rssi_th1 (mcs5->mcs4)
#define MCS_3      ((uint8_t) 0x0C )  // [7:0]mcs_rssi_th2 (mcs4->mcs5)
#define MCS_4      ((uint8_t) 0x0D )  /* [7:4]  mcs_rssi_diff
                                         [3:0]  mcs_gain_diff
                                      */

#define MCS_5      ((uint8_t) 0x0E )  //Mcs_ldpc_period[7:0]

#define FSM_5      ((uint8_t) 0x0F )  //pkd_times

#define AGC_0      ((uint8_t) 0x10 )    /*PAGE2
                                          [7]reg_power_diff_fix
                                               1:fix; 0:auto
                                          [6]reg_aagc_fix.
                                               1:fix AGC gain; 0:auto
                                          [5]iq_exchange_1
                                          [4]iq_exchange_2
                                          [3]reg_enable_fre_hop
                                               1:enable fre hop; 0:disable
                                          [2]reg_8x_switch_en
                                               1:switch; 0:not switch
                                          [1]bypass_dgain
                                               1:bypass digital gain; 0:use digital gain
                                          [0]rx_din_ori
                                               0:2's complement; 1:ori code
                                   */
//page2 0xa0
#define AGC_2      ((uint8_t)0x92 ) //reg_aagc_fix_gain1 0x00  RW  [7:0]
#define AGC_3      ((uint8_t)0x93 ) //reg_aagc_fix_gain2 0x01  RW  [7:0]


//page2 0x50
#define FEC_7        ((uint8_t)(0x50))  //PAGE2_rc_id[39:32]  0x00 RW  [7:0]
#define FEC_8        ((uint8_t)(0x51))  //PAGE2_rc_id[31:24]  0x01 RW  [7:0]
#define FEC_9        ((uint8_t)(0x52))  //PAGE2_rc_id[23:16]  0x02 RW  [7:0]
#define FEC_10       ((uint8_t)(0x53))  //PAGE2_rc_id[15:8]   0x03 RW  [7:0]
#define FEC_11       ((uint8_t)(0x54))  //PAGE2_rc_id[7:0]    0x04 RW  [7:0]

#define AGC3_0     ((uint8_t)0x10 )     //reg_aggc_work_frequency[7:0].
#define AGC3_1     ((uint8_t)0x11 )     //reg_aggc_work_frequency[15:8].
#define AGC3_2     ((uint8_t)0x12 )     //reg_aggc_work_frequency[23:16].
#define AGC3_3     ((uint8_t)0x13 )     //reg_aggc_work_frequency[31:24].

#define AGC3_6     ((uint8_t)0x14 )     //reg_aggc_sweep_frequency[15:8].
#define AGC3_7     ((uint8_t)0x15 )     //reg_aggc_sweep_frequency[23:16].
#define AGC3_8     ((uint8_t)0x16 )     //reg_aggc_sweep_frequency[31:24].
#define AGC3_9     ((uint8_t)0x17 )     //reg_aggc_sweep_frequency[39:32].


//page2 0x10
#define AGC3_a     ((uint8_t)0x18 )     //reg_aggc_rc_frequency[7:0].    0x0b    RW  [7:0] 
#define AGC3_b     ((uint8_t)0x19 )     //reg_aggc_rc_frequency[15:8].   0x0a    RW  [7:0]  
#define AGC3_c     ((uint8_t)0x1a )     //reg_aggc_rc_frequency[23:16].  0x09    RW  [7:0]  
#define AGC3_d     ((uint8_t)0x1b )     //reg_aggc_rc_frequency[31:24].  0x08    RW  [7:0]   
#define AGC3_e     ((uint8_t)0x1e )     //reg_aggc_rc_frequency[39:32].


//page2                                       
#define AAGC_0_RD  ((uint8_t)0x02 )     //Lowest byte of channel power
#define AAGC_4_RD  ((uint8_t)0x03 )     //Middle byte of channel power
#define AAGC_1_RD  ((uint8_t)0x04 )     //Highest byte of channel power                                              

//page2
#define AAGC_2_RD  ((uint8_t)0xA0 )     //0x00  rx1_gain_all_r  RW  [7:0]
#define AAGC_3_RD  ((uint8_t)0xA1 )     //0x01  rx2_gain_all_r  RW  [7:0]



#define FEC_0_RD   ((uint8_t)0xE0 )   /*
                                            [7]  Rc_rev_ok_inwimax
                                            [6]  Retx_pkg_req_curr
                                            [5]  Retx_flg
                                            [4:2]Ldpc_rate_in
                                            [1:0]Qam_mode_in
                                       */
                                       
#define GRD_FEC_QAM_CR_TLV  ((uint8_t)0xD0 )    /*
                                               [6:5] TLV mode
                                               [2:4] coderate
                                               [0:1] QAM
                                            */


#define FEC_1_RD    ((uint8_t)0xD2 )  //[7:0]Dec_rc_id[39:32]
#define FEC_2_RD_1  ((uint8_t)0xD3 )  //[7:0]Dec_rc_id[31:24]
#define FEC_2_RD_2  ((uint8_t)0xD4 )  //[7:0]Dec_rc_id[23:16]
#define FEC_2_RD_3  ((uint8_t)0xD5 )  //[7:0]Dec_rc_id[15:8]
#define FEC_2_RD_4  ((uint8_t)0xD6 )  //[7:0]Dec_rc_id[7:0]


#define FEC_3_RD_1  ((uint8_t)0xE7 )  //Ldpc_err_num_rd[15:8]

#define FEC_3_RD_2  ((uint8_t)0xE8 )  //Ldpc_err_num_rd[7:0]

#define FEC_4_RD    ((uint8_t)0xD9 )      /*PAGE2   0xc8  0x11  0xD9                                       
                                          rc_err_flg  RO  [7:7]
                                          nr_check_ok_hold  RO  [2:2]
                                          crc_check_ok  RO  [1:1]
                                          id_match  RO  [0:0]
                                          */

#define FEC_5_RD    ((uint8_t)0xDA )/*[7:4]Harq_times_out
                                      [3:0]Debug_fec_sel:rdout_sclr_cnt:{1'b0, Scale_reg_out }
                                     */                                     

#define FEC_6_RD    ((uint8_t)0xEB )/*[7]  1'b0
                                      [6]  Fch_err_flg
                                      [5]  Dat_err_flg_frame
                                      [4]  1'b0
                                      [3:1]pkg_index_fch
                                      [0]  Fec_lock
                                     */

#define FEC_9_RD    ((uint8_t)0xEE )   //Harq_ldpc_err_num_rd [15:8]
#define FEC_10_RD   ((uint8_t)0xEF )   //Fec_debug_sel?Harq_ldpc_err_pkg: Harq_ldpc_err_num_rd[7:0]

//
#define     CE_01                   (0x31)
#define     CE_9_RD                 (0xD9)      //SNR_DOUT [15:8]
#define     CE_A_RD                 (0xDA)      //SNR_DOUT [7:0]

#define     GRD_RC_ID_BIT07_00_REG  (0x5f)
#define     GRD_RC_ID_BIT15_08_REG  (0x5e)
#define     GRD_RC_ID_BIT23_16_REG  (0x5d)
#define     GRD_RC_ID_BIT31_24_REG  (0x5c)
#define     GRD_RC_ID_BIT39_32_REG  (0x5b)

#define     SWEEP_FREQ_0        (0x14)
#define     SWEEP_FREQ_1        (0x15)
#define     SWEEP_FREQ_2        (0x16)
#define     SWEEP_FREQ_3        (0x17)

#define     SNR_REG_0           (0xc0)
#define     SNR_REG_1           (0xc1)

#define     LDPC_ERR_HIGH_REG   (0xde)
#define     LDPC_ERR_LOW_REG    (0xdf)

#endif

