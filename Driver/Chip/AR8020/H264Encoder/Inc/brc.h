#ifndef _BRC_H_
#define _BRC_H_

#define ARMCM7_RC //!!!!!!!!!! IMPORTANT !!!!!!!!!!!

#ifdef ARMCM7_RC //###########
    #define RC_MODE_0 0
    #define RC_MODE_1 1
    #define RC_MODE_2 2
    #define RC_MODE_3 3
    #define I_SLICE 2
    #define P_SLICE 0
    #define RC_MODEL_HISTORY 21
    #define MIN_INT64 0x8000000000000000
    #define MIN_INT   0x80000000
    #define MAX_INT   0x7fffffff
    typedef long long int64;
    typedef int Boolean;
#endif	  //###########

#define MIN_INT64 0x8000000000000000
#define MIN_INT   0x80000000
#define MAX_INT   0x7fffffff

//// generic rate control variables
struct my_rc{
//// top set(img,params)
  int    v0_enable;
  int    v0_rc_enable;
  int    v0_height;
  int    v0_width;
  int    v0_basicunit;
  int    v0_RCUpdateMode;
  int    v0_bit_rate;
  int    v0_framerate;
  int    v0_PMaxQpChange;
  int    v0_SeinitialQP;
  int    v0_size;
  int    v0_intra_period;
  int    v0_RCISliceBitRatio;
  int    v0_no_frm_base;
  int    v0_RCMinQP;
  int    v0_RCMaxQP;
  int    v0_FrameSizeInMbs;  
  int    v0_FrameHeightInMbs;
  int    v0_type;
  int    v0_BasicUnit;
  int    v0_MBPerRow;
  int    v0_qp;
  int    v0_RCIoverPRatio;
  int    v0_header_bits;
  int    v0_c1_over; //
  int    v0_re_bitrate;
  int    v0_new_bitrate;
  int    v0_PrevBitRate;// lhumod
  int    v0_cmadequ0;// lhumod
  int    v0_TotalFrameMAD;// lhumod
  int    v0_RCEnableAutoConfigGOP;// @lhu
  int    v0_RCEnableAutoConfigIOPRatio;// @lhu
  int    v0_prev_ac_br_index;// @lhu
//
  int    v0_gop_cnt;
  int    v0_frame_cnt;
  int    v0_bu_cnt;
  int    v0_mb_cnt;
  int    v0_rc_start;
  
  int    v0_frame_mad;
  int    v0_frame_tbits;
  int    v0_frame_hbits;
  
  int    v0_frame_bs;
  int    v0_slice_qp;
  int    v0_codedbu_cnt;
  int    v0_NumberofHeaderBits;
  int    v0_NumberofTextureBits;
  int    v0_NumberofBasicUnitHeaderBits;
  int    v0_NumberofBasicUnitTextureBits;
  int    v0_NumberofGOP;
  int    v0_TotalMADBasicUnit;
  int    v0_CurrentBufferFullness; //LIZG 25/10/2002
  int    v0_RemainingBits;
  int    v0_RCPSliceBits;
  int    v0_RCISliceBits;
  int    v0_NPSlice;
  int    v0_NISlice;
  int    v0_GAMMAP_1p;                //LIZG, JVT019r1
  int    v0_BETAP_1p;                 //LIZG, JVT019r1
  int    v0_TargetBufferLevel;     //LIZG 25/10/2002
  int    v0_MyInitialQp;
  int    v0_PAverageQp;
  int    v0_PreviousPictureMAD_8p;
  int    v0_MADPictureC1_12p;
  int    v0_MADPictureC2_12p;
  int    v0_PMADPictureC1_12p;
  int    v0_PMADPictureC2_12p;

  int    v0_PPictureMAD_8p;
  int    v0_PictureMAD_8p  [20];
  int    v0_ReferenceMAD_8p[20];
  int    v0_mad_tmp0 [20];
  int    v0_mad_tmp0_valid [20];
  int    v0_mad_tmp1 [20];
  int    v0_mad_tmp2 [20];
  int    v0_m_rgQp_8p [20];
  int    v0_m_rgRp_8p [20];
  int    v0_m_rgRp_8prr8 [20];
  int    v0_rc_tmp0 [20];
  int    v0_rc_tmp1 [20];
  int    v0_rc_tmp2 [20];
  int    v0_rc_tmp3 [20];
  int    v0_rc_tmp4 [20];
  
  int    v0_rc_hold;
  int    v0_mad_hold;
  
  char   v0_rc_rgRejected [20];
  char   v0_mad_rgRejected [20];
  int    v0_m_X1_8p;
  int    v0_m_X2_8p;
  int    v0_Pm_X1_8p;
  int    v0_Pm_X2_8p;
  int    v0_Pm_Qp;
  int    v0_Pm_Hp;
  int    v0_MADm_windowSize;
  int    v0_m_windowSize;
  int    v0_m_Qc;
  int    v0_PPreHeader;
  int    v0_PrevLastQP; // QP of the second-to-last coded frame in the primary layer
  int    v0_CurrLastQP; // QP of the last coded frame in the primary layer
  int    v0_TotalFrameQP;
  int    v0_PAveHeaderBits1;
  int    v0_PAveHeaderBits2;
  int    v0_PAveHeaderBits3;
  int    v0_PAveFrameQP;
  int    v0_TotalNumberofBasicUnit;
  int    v0_NumberofCodedPFrame;
  int    v0_TotalQpforPPicture;
  int    v0_CurrentMAD_8p;
  int    v0_TotalBUMAD_12p;
  int    v0_PreviousMAD_8p;
  int    v0_PreviousWholeFrameMAD_8p;
  int    v0_DDquant;
  int    v0_QPLastPFrame;
  int    v0_QPLastGOP;
  int    v0_FrameQPBuffer;
  int    v0_FrameAveHeaderBits;
  int    v0_BUPFMAD_8p[70];// lhumod
  int    v0_BUCFMAD_8p[70];// lhumod
  int    v0_RCISliceActualBits;// lhugop
  
  int    v0_GOPOverdue;
  // rate control variables
  int    v0_Xp;
  int    v0_Xb;
  int    v0_Target;
  int    v0_Np;
  int    v0_UpperBound1;
  int    v0_UpperBound2;
  int    v0_LowerBound;
  int    v0_DeltaP;
  int    v0_TotalPFrame;
//// for feedback
  int    v0_aof_inc_qp;
  int    v0_fd_row_cnt;
  int    v0_fd_last_row;
  int    v0_fd_last_p;
  int    v0_fd_iframe;
  int    v0_fd_reset;  
  int    v0_fd_irq_en;
//int    fd_cpu_test;
//// for test
// int    v0_PrevImgType;// added by lhulhu
// int    v0_RCInitialQP;// added by lhulhu
// int    v0_NumberofBasicUnit;// added by lhumod
// int    v0_SceneChange_Simp2Comp;// added by lhusc
// int    v0_PrevFrmBits; // added by lhusc
// int    v0_TargetBitsPerFrm;// added by lhusc

// second view global parameters
  int    v1_enable;
  int    v1_rc_enable;
  int    v1_height;
  int    v1_width;
  int    v1_basicunit;
  int    v1_RCUpdateMode;
  int    v1_bit_rate;
  int    v1_framerate;
  int    v1_PMaxQpChange;
  int    v1_SeinitialQP;
  int    v1_size;
  int    v1_intra_period;
  int    v1_RCISliceBitRatio;
  int    v1_no_frm_base;
  int    v1_RCMinQP;
  int    v1_RCMaxQP;
  int    v1_FrameSizeInMbs;  
  int    v1_FrameHeightInMbs;
  int    v1_type;
  int    v1_BasicUnit;
  int    v1_MBPerRow;
  int    v1_qp;
  int    v1_RCIoverPRatio;
  int    v1_header_bits;
  int    v1_c1_over; //
  int    v1_re_bitrate;
  int    v1_new_bitrate;
  int    v1_cmadequ0;// lhumod
  int    v1_PrevBitRate;// lhumod
  int    v1_TotalFrameMAD;// lhumod
  int    v1_RCEnableAutoConfigGOP;// @lhu
  int    v1_RCEnableAutoConfigIOPRatio;// @lhu
  int    v1_prev_ac_br_index;// @lhu
//
  int    v1_gop_cnt;
  int    v1_frame_cnt;
  int    v1_bu_cnt;
  int    v1_mb_cnt;
  int    v1_rc_start;
  
  int    v1_frame_mad;
  int    v1_frame_tbits;
  int    v1_frame_hbits;
  
  int    v1_frame_bs;
  int    v1_slice_qp;
  int    v1_codedbu_cnt;
  int    v1_NumberofHeaderBits;
  int    v1_NumberofTextureBits;
  int    v1_NumberofBasicUnitHeaderBits;
  int    v1_NumberofBasicUnitTextureBits;
  int    v1_NumberofGOP;
  int    v1_TotalMADBasicUnit;
  int    v1_CurrentBufferFullness; //LIZG 25/10/2002
  int    v1_RemainingBits;
  int    v1_RCPSliceBits;
  int    v1_RCISliceBits;
  int    v1_NPSlice;
  int    v1_NISlice;
  int    v1_GAMMAP_1p;                //LIZG, JVT019r1
  int    v1_BETAP_1p;                 //LIZG, JVT019r1
  int    v1_TargetBufferLevel;     //LIZG 25/10/2002
  int    v1_MyInitialQp;
  int    v1_PAverageQp;
  int    v1_PreviousPictureMAD_8p;
  int    v1_MADPictureC1_12p;
  int    v1_MADPictureC2_12p;
  int    v1_PMADPictureC1_12p;
  int    v1_PMADPictureC2_12p;

  int    v1_PPictureMAD_8p;
  int    v1_PictureMAD_8p  [20];
  int    v1_ReferenceMAD_8p[20];
  int    v1_mad_tmp0 [20];
  int    v1_mad_tmp0_valid [20];
  int    v1_mad_tmp1 [20];
  int    v1_mad_tmp2 [20];
  int    v1_m_rgQp_8p [20];
  int    v1_m_rgRp_8p [20];
  int    v1_m_rgRp_8prr8 [20];
  int    v1_rc_tmp0 [20];
  int    v1_rc_tmp1 [20];
  int    v1_rc_tmp2 [20];
  int    v1_rc_tmp3 [20];
  int    v1_rc_tmp4 [20];
  
  int    v1_rc_hold;
  int    v1_mad_hold;
  
  char   v1_rc_rgRejected [20];
  char   v1_mad_rgRejected [20];
  int    v1_m_X1_8p;
  int    v1_m_X2_8p;
  int    v1_Pm_X1_8p;
  int    v1_Pm_X2_8p;
  int    v1_Pm_Qp;
  int    v1_Pm_Hp;
  int    v1_MADm_windowSize;
  int    v1_m_windowSize;
  int    v1_m_Qc;
  int    v1_PPreHeader;
  int    v1_PrevLastQP; // QP of the second-to-last coded frame in the primary layer
  int    v1_CurrLastQP; // QP of the last coded frame in the primary layer
  int    v1_TotalFrameQP;
  int    v1_PAveHeaderBits1;
  int    v1_PAveHeaderBits2;
  int    v1_PAveHeaderBits3;
  int    v1_PAveFrameQP;
  int    v1_TotalNumberofBasicUnit;
  int    v1_NumberofCodedPFrame;
  int    v1_TotalQpforPPicture;
  int    v1_CurrentMAD_8p;
  int    v1_TotalBUMAD_12p;
  int    v1_PreviousMAD_8p;
  int    v1_PreviousWholeFrameMAD_8p;
  int    v1_DDquant;
  int    v1_QPLastPFrame;
  int    v1_QPLastGOP;
  int    v1_FrameQPBuffer;
  int    v1_FrameAveHeaderBits;
  int    v1_BUPFMAD_8p[70];// lhumod
  int    v1_BUCFMAD_8p[70];// lhumod
  int    v1_RCISliceActualBits;// lhugop
  
  int    v1_GOPOverdue;
  // rate control variables
  int    v1_Xp;
  int    v1_Xb;
  int    v1_Target;
  int    v1_Np;
  int    v1_UpperBound1;
  int    v1_UpperBound2;
  int    v1_LowerBound;
  int    v1_DeltaP;
  int    v1_TotalPFrame;
//// for feedback
  int    v1_aof_inc_qp;
  int    v1_fd_row_cnt;
  int    v1_fd_last_row;
  int    v1_fd_last_p;
  int    v1_fd_iframe;
  int    v1_fd_reset;  
  int    v1_fd_irq_en;
//// for test
// int    v1_PrevImgType;// added by lhulhu
// int    v1_RCInitialQP;// added by lhulhu
// int    v1_NumberofBasicUnit;// added by lhumod
// int    v1_SceneChange_Simp2Comp;// added by lhusc
// int    v1_PrevFrmBits; // added by lhusc
// int    v1_TargetBitsPerFrm;// added by lhusc
} rca;

//s int *pp_BUPFMAD_8p;
//s int *pp_BUCFMAD_8p;
int v0_tbits_tmp,v0_hbits_tmp,v0_fbits_tmp,v0_mad_tmp;
int v1_tbits_tmp,v1_hbits_tmp,v1_fbits_tmp,v1_mad_tmp;
int qp;
int ymsel_tmp, ymseh_tmp; // @lhu
int64 v0_ymse_iframe,v0_ymse_last_p;
int64 v1_ymse_iframe,v1_ymse_last_p;
Boolean v0_last_p_gop_change;
Boolean v1_last_p_gop_change;
int v0_last_p_prev_gop;
int v1_last_p_prev_gop;
int v0_poweron_rc_params_set,v1_poweron_rc_params_set; //lhuemu

int aof_v0_frame;
int aof_v1_frame;

int  my_imin( );
int  my_iequmin( );
int  my_imax( );
int  my_iClip3( );
int  my_sqrt( );
int  Qstep2QP_8p( );
int  QP2Qstep_8p( );
int  RCAutoConfig_GOP( ); // @lhu
int  RCAutoConfig_IOverPRatio( ); // @lhu
void my_rc_ac_br(int); // @lhu

void my_v0_rc_params( );
void my_v0_rc_params_ac_gop( ); // @lhu
void my_v0_rc_params_ac_iopratio( ); // @lhu
void my_v0_rc_init_seq( );
void my_v0_rc_init_GOP( );
void my_v0_rc_init_pict( );
void my_v0_rc_update_pict( );
void my_v0_updatePparams( );
void my_v0_rc_update_pict_frame( );
void my_v0_updateRCModel( );
void my_v0_RCModelEstimator( );
void my_v0_updateMADModel( );
void my_v0_MADModelEstimator( );
void my_v0_updateQPNonPicAFF( );
int  my_v0_updateFirstP( );
int  my_v0_updateNegativeTarget( );
int  my_v0_updateFirstBU( );
void my_v0_updateLastBU( );
void my_v0_predictCurrPicMAD( );
void my_v0_updateModelQPBU( );
void my_v0_updateModelQPFrame( );
int  my_v0_rc_handle_mb( );
void my_v0_rc_update_bu_stats( );
void my_v0_rc_update_frame_stats( );
void my_v0_rc_init_gop_params( );
void my_v0_rc_init_frame( );
void my_v0_rc_store_header_bits( );
int  my_v0_updateQPRC1( );
int  my_v0_updateQPRC3( );
int  my_v0_updateQPRC0( );
void my_v0_initial_all( );
void my_v0_feedback( );
void my_v0_hold( );
int  (*my_v0_updateQP)( );

// second view sub-function
void my_v1_rc_params( );
void my_v1_rc_init_seq( );
void my_v1_rc_params_ac_gop( ); // @lhu
void my_v1_rc_params_ac_iopratio( ); // @lhu
void my_v1_rc_init_GOP( );
void my_v1_rc_init_pict( );
void my_v1_rc_update_pict( );
void my_v1_updatePparams( );
void my_v1_rc_update_pict_frame( );
void my_v1_updateRCModel( );
void my_v1_RCModelEstimator( );
void my_v1_updateMADModel( );
void my_v1_MADModelEstimator( );
void my_v1_updateQPNonPicAFF( );
int  my_v1_updateFirstP( );
int  my_v1_updateNegativeTarget( );
int  my_v1_updateFirstBU( );
void my_v1_updateLastBU( );
void my_v1_predictCurrPicMAD( );
void my_v1_updateModelQPBU( );
void my_v1_updateModelQPFrame( );
int  my_v1_rc_handle_mb( );
void my_v1_rc_update_bu_stats( );
void my_v1_rc_update_frame_stats( );
void my_v1_rc_init_gop_params( );
void my_v1_rc_init_frame( );
void my_v1_rc_store_header_bits( );
int  my_v1_updateQPRC1( );
int  my_v1_updateQPRC3( );
int  my_v1_updateQPRC0( );
void my_v1_initial_all( );
void my_v1_feedback( );
void my_v1_hold( );
int  (*my_v1_updateQP)( );

void update_aof_cycle();
void update_aof();



#ifdef ARMCM7_RC //###########
int VEBRC_IRQ_Handler( );
#endif


#endif
