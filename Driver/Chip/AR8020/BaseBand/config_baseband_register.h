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


/** @ Register File************************************
    @ Instruction:
     Address Space Location  P1: page1, 0x00[7]=1; P2: page2, 0x00[7]=0
            ________________________________________________________________
            |-------- Address  -------   Block name    ------- Number (Byte)
            |      P1&P2:00-01   |                       |
            |      P2: 02-0F     |   FSM WR              |  48
            |      P1: 20-3F     |                       |
            |--------------------------------------------------------
            |      P1: 02-0F     |   TX WR               |  14
            |--------------------------------------------------------
            |      P1: 10-1F     |   ADC/DAC for AR8003  |  16
            |--------------------------------------------------------
            |      P1: 40-7F     |   RC_TX RD/WR         |  64
            |--------------------------------------------------------
            |      P1: 80-8F     |   TX RD               |  16
            |--------------------------------------------------------
            |      P1: 90-9F     |   RF_CALI RD          |  16
            |--------------------------------------------------------
            |      P1: A0-BF     |   RF_CALI WR          |  32
            |--------------------------------------------------------
            |      P2: 10-1F     |                       |
            |      P2: A0-AF     |                       |
            |      P2: B0-BF     |   AGC WR              |  64
            |      P1: C0-CF     |                       |
            |--------------------------------------------------------
            |      P2: 20-2F     |   Frontend WR         |  16
            |--------------------------------------------------------
            |      P2: 30-8F     |   CHE and EQU WR      |  96
            |--------------------------------------------------------
            |      P2: 90-9F     |   FEC WR              |  16
            |--------------------------------------------------------
            |      P2: C0-C7     |   FSM & AGC RD        |   8
            |--------------------------------------------------------
            |      P2: C8-CF     |   Frontend RD         |   8
            |--------------------------------------------------------
            |      P2: D0-DF     |   CHE and EQU RD      |  16
            |--------------------------------------------------------
            |      P2: E0-EF     |   FEC RD              |  16
            |_____________________________________________________________

* ****************************************************************************** */

//  Page1 & page2: (0x00~0x01)
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

/*
  *  There are the definitions for the rc_id_bits.
  */
#define TX_2       ((uint8_t)0x02 ) /* PAGE1
                                     [7]n_tx_ant  1'b1: 2TX, 1'b0: 1TX
                                     [6:5]QAM_mode
                                          2'b00: BPSK, 2'b01: QPSK,
                                          2'b10: 16QAM, 2'b11:64QAM
                                     [4:3]BandWidth
                                          2'b00: 2.5MHz, 2'b01: 5MHz,
                                          2'b10: 10MHz, 2'b11: reserved
                                     [2:0]code_rate_enum
                                          3'b0: 1/2  3'b1: 2/3A 3'd2: 2/3B
                                          3'd3:3/4A  3'd4: 3/4B  3'd5: 5/6
                                   */
#define TX_3       ((uint8_t)0x03 )  //RC_id_bits[39:32]
#define TX_4       ((uint8_t)0x04 )  //RC_id_bits[31:24]
#define TX_5       ((uint8_t)0x05 )  //RC_id_bits[23:16]
#define TX_6       ((uint8_t)0x06 )  //RC_id_bits[15:8]
#define TX_7       ((uint8_t)0x07 )  //RC_id_bits[7:0]

#define TX_9       ((uint8_t)0x09 )/*[7:4]  reserved

                                    [3:0]  shift_sel
                                            4'h0:  -12dB(*0.25),
                                            4'h1:  -11dB(*0.28), бн,
                                            4'h6:  -6dB(*0.5), бн ,
                                            4'hf:   3dB(*1.4)
                                   */


#define TRX_0      ((uint8_t)0x20 )  /*[7]   fb_clk_sel
                                        [6:5] reserved
                                        [4]   RESETB
                                              1'b1: work, 1'b0: reset (for 9363)
                                        [3]   PA_0_on
                                              1'b1: on, 1'b0: off
                                        [2]   PA_1_on
                                              1'b1: on, 1'b0: off
                                        [1:0] Ctrl_pin_9363
                                              00,01: auto TR switch
                                              10: force 9363 or 8003 TX
                                              11: force 9363 or 8003 RX
                                      */
// custom

#define IT_FREQ_TX    ((uint8_t)0x40 )
#define RC_FREQ_TX    ((uint8_t)0x41 )
#define QAM_CHANGE    ((uint8_t)0x42 )
#define RC_FH_PATTERN ((uint8_t)0x43 )


#define TX_A       ((uint8_t)0x8A )    //[7:0]  8'h02 /RO/
#define TX_B       ((uint8_t)0x8B )    //[7:0]  8'h01 /RO/
#define TX_C       ((uint8_t)0x8C )    //[7:0]  8'h00 /RO/
#define TX_D       ((uint8_t)0x8D )    //[7:0]  8'h07 /RO/
#define TX_E       ((uint8_t)0x8E )    //[7:0]  8'hF2 /RO/
#define TX_F       ((uint8_t)0x8F )    //[7:0]  8'h00 /RO/

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
#define CA_2       ((uint8_t)0xA2 )     /*
                                           [7]reg_filter3_gain_sel
                                           [6]reg_tx_iq_swap
                                           [5]reg_fix_rx_cali
                                           [4]reg_rx_iter_on
                                           [3]reg_tx_iqmis_debug
                                           [2]reg_tx_amdet_debug
                                           [1]reg_tx_debug_mode
                                           [0]reg_tx_iter_on
                                        */
#define CA_3       ((uint8_t)0xA3 )     /*
                                           [7]  reg_tssi_cali_fix
                                           [6:4]reserved
                                           [3:2]reg_tssi_time_option
                                           [1]  reg_tssi_diff
                                           [0]  reg_tssi_cali_on
                                        */
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

#define AGC_2      ((uint8_t)0x12 ) //reg_aagc_fix_gain1
#define AGC_3      ((uint8_t)0x13 ) //reg_aagc_fix_gain2

#define FEC_7      ((uint8_t)0x97 )     //PAGE2_rc_id[39:32].
#define FEC_8      ((uint8_t)0x98 )     //PAGE2_rc_id[31:24].
#define FEC_9      ((uint8_t)0x99 )     //PAGE2_rc_id[23:16].
#define FEC_10     ((uint8_t)0x9A )     //PAGE2_rc_id[15:8].
#define FEC_11     ((uint8_t)0x9B )     //PAGE2_rc_id[7:0].

#define AGC3_0     ((uint8_t)0xB0 )     //reg_aggc_work_frequency[7:0].
#define AGC3_1     ((uint8_t)0xB1 )     //reg_aggc_work_frequency[15:8].
#define AGC3_2     ((uint8_t)0xB2 )     //reg_aggc_work_frequency[23:16].
#define AGC3_3     ((uint8_t)0xB3 )     //reg_aggc_work_frequency[31:24].
#define AGC3_4     ((uint8_t)0xB4 )     //reg_aggc_work_frequency[39:32].
#define AGC3_5     ((uint8_t)0xB5 )     //reg_aggc_sweep_frequency[7:0].
#define AGC3_6     ((uint8_t)0xB6 )     //reg_aggc_sweep_frequency[15:8].
#define AGC3_7     ((uint8_t)0xB7 )     //reg_aggc_sweep_frequency[23:16].
#define AGC3_8     ((uint8_t)0xB8 )     //reg_aggc_sweep_frequency[31:24].
#define AGC3_9     ((uint8_t)0xB9 )     //reg_aggc_sweep_frequency[39:32].
#define AGC3_a     ((uint8_t)0xBA )     //reg_aggc_rc_frequency[7:0].
#define AGC3_b     ((uint8_t)0xBB )     //reg_aggc_rc_frequency[15:8].
#define AGC3_c     ((uint8_t)0xBC )     //reg_aggc_rc_frequency[23:16].
#define AGC3_d     ((uint8_t)0xBD )     //reg_aggc_rc_frequency[31:24].
#define AGC3_e     ((uint8_t)0xBE )     //reg_aggc_rc_frequency[39:32].

#define AAGC_0_RD  ((uint8_t)0xC3 )     //Lowest byte of channel power
#define AAGC_1_RD  ((uint8_t)0xC4 )     //Highest byte of channel power
#define AAGC_2_RD  ((uint8_t)0xC5 )     //[6:0]Rx1_again_final
#define AAGC_3_RD  ((uint8_t)0xC6 )     //[6:0]Rx2_again_final
#define AAGC_4_RD  ((uint8_t)0xC7 )     //Middle byte of channel power

#define FEC_0_RD   ((uint8_t)0xE0 )   /*
                                            [7]  Rc_rev_ok_inwimax
                                            [6]  Retx_pkg_req_curr
                                            [5]  Retx_flg
                                            [4:2]Ldpc_rate_in
                                            [1:0]Qam_mode_in
                                       */
#define FEC_1_RD    ((uint8_t)0xE2 )  //[7:0]Dec_rc_id[39:32]
#define FEC_2_RD_1  ((uint8_t)0xE3 )  //[7:0]Dec_rc_id[31:24]
#define FEC_2_RD_2  ((uint8_t)0xE4 )  //[7:0]Dec_rc_id[23:16]
#define FEC_2_RD_3  ((uint8_t)0xE5 )  //[7:0]Dec_rc_id[15:8]
#define FEC_2_RD_4  ((uint8_t)0xE6 )  //[7:0]Dec_rc_id[7:0]

#define FEC_3_RD_1  ((uint8_t)0xE7 )  //Ldpc_err_num_rd[15:8]

#define FEC_3_RD_2  ((uint8_t)0xE8 )  //Ldpc_err_num_rd[7:0]
#define FEC_4_RD    ((uint8_t)0xE9 )  /*
                                          PAGE2
                                          [7]    Rc_err_flg
                                          [6:2]  Reserved
                                          [1]    crc_check_ok
                                          [0]    id_match
                                          */

#define FEC_5_RD    ((uint8_t)0xEA )/*[7:4]Harq_times_out
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
#define CE_01         ((uint8_t) 0x31)
#define CE_9_RD       ((uint8_t) 0xD9)      //SNR_DOUT [15:8]
#define CE_A_RD       ((uint8_t) 0xDA)      //SNR_DOUT [7:0]



#endif

