////////////////////////////////////////////////////////////////////////////
// brief: Rate Control algorithm for two video view
// author: Li Hu
// date: 2015-11-8
////////////////////////////////////////////////////////////////////////////
#include "data_type.h"
#include "brc.h"

#ifdef ARMCM7_RC  //###########
    #include "enc_internal.h"
#else  //###########
    #include <math.h>
    #include <limits.h>
    #include "global.h"
    #include "ratectl.h"
#endif    //###########

#ifdef ARMCM7_RC //###########
#define REG_BASE_ADDR 0xA0010000
#define V0_ENABLE_ADDR   (REG_BASE_ADDR+(0<<2))
#define V0_FRAME_XY_ADDR (REG_BASE_ADDR+(1<<2))
#define V0_GOPFPS_ADDR   (REG_BASE_ADDR+(2<<2))
#define V0_RCEN_BU_ADDR  (REG_BASE_ADDR+(5<<2))
#define V0_RCSET1_ADDR   (REG_BASE_ADDR+(6<<2))
#define V0_BR_ADDR       (REG_BASE_ADDR+(7<<2))
#define V0_RCSET2_ADDR   (REG_BASE_ADDR+(8<<2))
#define V0_FEEDBACK_ADDR (REG_BASE_ADDR+(9<<2))
#define V0_RC_ACBR_ADDR  (REG_BASE_ADDR+(10<<2))
#define V0_ZW_BSINFO_ADDR (REG_BASE_ADDR+(12<<2))

#define V0_QP_ADDR       (REG_BASE_ADDR+(18<<2))
#define V0_MAD_ADDR      (REG_BASE_ADDR+(19<<2))
#define V0_HBITS_ADDR    (REG_BASE_ADDR+(20<<2))
#define V0_TBITS_ADDR    (REG_BASE_ADDR+(21<<2))
#define V0_ABITS_ADDR    (REG_BASE_ADDR+(22<<2))
#define V0_YMSEL_ADDR    (REG_BASE_ADDR+(23<<2))
#define V0_YMSEH_ADDR    (REG_BASE_ADDR+(24<<2))

#define V1_REG_BASE_ADDR REG_BASE_ADDR+0x00000064
#define V1_ENABLE_ADDR   (V1_REG_BASE_ADDR+(0<<2))
#define V1_FRAME_XY_ADDR (V1_REG_BASE_ADDR+(1<<2))
#define V1_GOPFPS_ADDR   (V1_REG_BASE_ADDR+(2<<2))
#define V1_RCEN_BU_ADDR  (V1_REG_BASE_ADDR+(5<<2))
#define V1_RCSET1_ADDR   (V1_REG_BASE_ADDR+(6<<2))
#define V1_BR_ADDR       (V1_REG_BASE_ADDR+(7<<2))
#define V1_RCSET2_ADDR   (V1_REG_BASE_ADDR+(8<<2))
#define V1_FEEDBACK_ADDR (V1_REG_BASE_ADDR+(9<<2))
#define V1_RC_ACBR_ADDR  (V1_REG_BASE_ADDR+(10<<2))
#define V1_ZW_BSINFO_ADDR (V1_REG_BASE_ADDR+(12<<2))

#define V1_QP_ADDR       (V1_REG_BASE_ADDR+(18<<2))
#define V1_MAD_ADDR      (V1_REG_BASE_ADDR+(19<<2))
#define V1_HBITS_ADDR    (V1_REG_BASE_ADDR+(20<<2))
#define V1_TBITS_ADDR    (V1_REG_BASE_ADDR+(21<<2))
#define V1_ABITS_ADDR    (V1_REG_BASE_ADDR+(22<<2))
#define V1_YMSEL_ADDR    (V1_REG_BASE_ADDR+(23<<2))
#define V1_YMSEH_ADDR    (V1_REG_BASE_ADDR+(24<<2))
#endif    //###########




static const int OMEGA_4p = 0xe;
static const int MINVALUE = 4;

static const int QP2QSTEP_8p[6]={0x0a0,0x0b0,0x0d0,0x0e0,0x100,0x120};
int QP2Qstep_8p(int QP) {
    int i,Qstep_8p;

    Qstep_8p=QP2QSTEP_8p[QP%6];
    for(i=0;i<(QP/6);i++) Qstep_8p*=2;
    return Qstep_8p;
}

int Qstep2QP_8p(int Qstep_8p) {
    int tmp,q_per=0,q_rem=0;

    if     (Qstep_8p<QP2Qstep_8p(0 )) return 0;
    else if(Qstep_8p>QP2Qstep_8p(51)) return 51;

    tmp=QP2Qstep_8p(5);
    while(Qstep_8p>tmp){tmp=tmp<<1; q_per+=1;}

    if     (Qstep_8p<=(0x0a8<<q_per)) q_rem=0;
    else if(Qstep_8p<=(0x0c0<<q_per)) q_rem=1;
    else if(Qstep_8p<=(0x0d8<<q_per)) q_rem=2;
    else if(Qstep_8p<=(0x0f0<<q_per)) q_rem=3;
    else if(Qstep_8p<=(0x110<<q_per)) q_rem=4;
    else                              q_rem=5;

    return ((q_per*6)+q_rem);
}


int my_sqrt32(int x)
{
    int temp=0,v_bit=15,n=0,b=0x8000;

    if(x<=1)
        return x;
    else if(x<0x10000)
    {
        v_bit=7;
        b=0x80;
    }

    do{
        temp=((n<<1)+b)<<(v_bit--);
        if(x>=temp)
        {
            n+=b;
            x-=temp;
        }
    }while(b>>=1);

    return n;
}
int my_sqrt64(int64 x)
{
    int v_bit=31,nn = x>>32;
    int64 temp=0,n=0,b=0x80000000;

    if(x<=1)
        return ((int)x);

    if(nn==0)
        return(my_sqrt32((int)x));

    do{
        temp=((n<<1)+b)<<(v_bit--);
        if(x>=temp)
        {
            n+=b;
            x-=temp;
        }
    }while(b>>=1);

    return ((int)n);
}

int my_imin(int a, int b) { return((a<b)? a:b); }
int my_iequmin(int a, int b) { return((a<=b)? a:b); }
int my_imax(int a, int b) { return((a>b)? a:b); }
int my_iClip3(int low, int high, int x) {
  x = (x>low)? x:low;
  x = (x<high)? x:high;
  return x;
}




//==============================================================================
//==============================================================================
#ifdef ARMCM7_RC //###########
extern int v0_poweron_rc_params_set,v1_poweron_rc_params_set;
int VEBRC_IRQ_Handler(unsigned int view0_feedback, unsigned int view1_feedback)
{
    int i,run_case=0;

    my_v0_feedback( view0_feedback); //// view0 irq
    if((rca.v0_fd_irq_en==1) && (rca.v0_rc_enable==1) && (rca.v0_enable==1)) {
        run_case=0;
        // only for gop change at last_p_frame. At this circumstance, Hardware use the updated gop for frame_cnt increment immediately,
        // but software use the un-updated one for its frame_cnt counting. Thus would iccur mismatch for both side.
        if (rca.v0_fd_last_p==1 && rca.v0_fd_iframe!=1) { // fix the false decision when inserting OneIFrame
            READ_WORD(V0_GOPFPS_ADDR,i); //read view0 gop
            if (rca.v0_fd_row_cnt==0) v0_last_p_prev_gop = (i>>24)&0xff;
            if (((i>>24)&0xff)!=v0_last_p_prev_gop) v0_last_p_gop_change = TRUE; // check view0's GOP change or not at last_p_frame
            v0_last_p_prev_gop = (i>>24)&0xff;
        }
    }
    else {
        my_v1_feedback(view1_feedback ); //// view1 irq
        if((rca.v1_fd_irq_en==1) && (rca.v1_rc_enable==1) && (rca.v1_enable==1)) {
            READ_WORD(V1_FRAME_XY_ADDR,i); //check x/y first
            if( ((i&0xffff)>720) || (((i>>16)&0xffff)>1280) )
                //// only for 1080p input from view1 (encoder used view0)
                //1080P input view1 but encoder used view0,
                //for rc read parameter from view1 / use V1 compute / read enc-data from view0 / write QP to view0
                run_case=2;
            else
                run_case=1;
            // only for gop change at last_p_frame. At this circumstance, Hardware use the updated gop for frame_cnt increment immediately,
            // but software use the un-updated one for its frame_cnt counting. Thus would iccur mismatch for both side.
            if (rca.v1_fd_last_p==1 && rca.v1_fd_iframe!=1) { // fix the false decision when inserting OneIFrame
                READ_WORD(V1_GOPFPS_ADDR,i); //read view1 gop
                if (rca.v1_fd_row_cnt==0) v1_last_p_prev_gop = (i>>24)&0xff;
                if (((i>>24)&0xff)!=v1_last_p_prev_gop) v1_last_p_gop_change = TRUE; // check view1's GOP change or not at last_p_frame
                v1_last_p_prev_gop = (i>>24)&0xff;
            }
        }
        else
            run_case=3;
    }

    switch(run_case) {
        case 0:
            if((rca.v0_fd_reset==1) && (rca.v0_fd_last_p==1) && (rca.v0_fd_last_row==1) && (v0_last_p_gop_change==FALSE) && (v0_poweron_rc_params_set==0) && (rca.v0_gop_change_NotResetRC==0)) { // lhu, 2017/04/17
                update_aof (); //lyu
                my_v0_initial_all( );
            }
            else if(rca.v0_rc_enable==1)
            {
                if ((rca.v0_fd_iframe==1) && (rca.v0_fd_row_cnt==0) && (v0_last_p_gop_change==TRUE) && (rca.v0_gop_change_NotResetRC==0)) { // lhu, 2017/04/17
                	// release last_p_gop_change to FALSE and execute initial_all operation until the first bu of iframe reached!!!
                    update_aof (); //lyu
                    my_v0_initial_all( );
                    v0_last_p_gop_change=FALSE;
                }
                if ((rca.v0_gop_cnt!=0) && (rca.v0_fd_iframe==1) && (rca.v0_fd_last_row==1) && (v0_poweron_rc_params_set==1)) {
                    v0_poweron_rc_params_set = 0; //lhuemu
                }
                my_rc_ac_br(0);
                if(rca.v0_re_bitrate==1 && rca.v0_fd_last_row==1) // change bit rate
                    READ_WORD(V0_BR_ADDR,rca.v0_bit_rate); //read br

                READ_WORD(V0_MAD_ADDR,v0_mad_tmp); //read mad
                READ_WORD(V0_HBITS_ADDR,v0_hbits_tmp); //read hbits
                //READ_WORD(V0_TBITS_ADDR,v0_tbits_tmp); //read tbits
                //Fix the HeaderBits and TextureBits assignment according to lyu's statistics === begin
                if (rca.v0_type==I_SLICE) v0_hbits_tmp = v0_hbits_tmp - (rca.v0_MBPerRow*3/2); // decrease 1.5 bit every MB for I SLICE
                else                      v0_hbits_tmp = v0_hbits_tmp - 5;                     // decrease 5 bit every BU for P SLICE
                READ_WORD(V0_ABITS_ADDR,v0_fbits_tmp);
                v0_tbits_tmp = (v0_fbits_tmp - rca.v0_PrevFbits) - v0_hbits_tmp;
                if (rca.v0_bu_cnt==0) rca.v0_PrevFbits = 0;
                else                  rca.v0_PrevFbits = v0_fbits_tmp;
                //Fix the HeaderBits and TextureBits assignment according to lyu's statistics === end
                if(rca.v0_fd_last_row==1) {
                    READ_WORD(V0_ABITS_ADDR,v0_fbits_tmp); //read abits
                    READ_WORD(V0_YMSEL_ADDR,ymsel_tmp); // read frame's y-mse
                    READ_WORD(V0_YMSEH_ADDR,ymseh_tmp);
                    v0_ymse_frame = (((long long)((ymseh_tmp>>24)&0x3f)<<32) + ymsel_tmp);
                    if ((ymseh_tmp>>2)&0x1) rca.v0_wireless_screen=1; else rca.v0_wireless_screen=0;
                    if ((ymseh_tmp>>3)&0x1) rca.v0_changeToIFrame=1; else rca.v0_changeToIFrame=0; // lhu, 2017/03/09
                    if ((ymseh_tmp>>4)&0x1) rca.v0_insertOneIFrame=1; else rca.v0_insertOneIFrame=0; // lhu, 2017/03/09
                    rca.v0_frm_ymse[0]  = v0_ymse_frame; // lhu, 2017/03/27
                    rca.v0_frm_fbits[0] = v0_fbits_tmp;  // lhu, 2017/03/27
                    rca.v0_RCSliceBits = (rca.v0_type==P_SLICE)? rca.v0_RCPSliceBits:rca.v0_RCISliceBits; // lhu, 2017/03/27
                    rca.v0_frm_hbits[0] = rca.v0_frame_hbits; // lhu, 2017/03/27
                    rca.v0_frm_abits[0] = rca.v0_frame_abits; // lhu, 2017/03/27
                    if (rca.v0_fd_iframe==1) rca.v0_ifrm_ymse = v0_ymse_frame; // lhu, 2017/04/13
                    else {
                        if (rca.v0_fd_last_p==1) {
                            rca.v0_lastpfrm_ymse = v0_ymse_frame;
                            READ_WORD(V0_ZW_BSINFO_ADDR,i);
                            if ((i>>1)&0x1) my_ac_RCISliceBitRatio(rca.v0_RCISliceBitRatioMax,0);
                            else {READ_WORD(V0_RCSET2_ADDR,i); rca.v0_RCISliceBitRatio = (i>>24)&0xf;}
                        }
                    }
                }
                qp = my_v0_rc_handle_mb( ); // update QP once
                if(rca.v0_fd_last_row==1) //frame last
                    rca.v0_slice_qp = rca.v0_PAveFrameQP;

                READ_WORD(V0_QP_ADDR,i);
                WRITE_WORD(V0_QP_ADDR,((qp<<24)+(rca.v0_slice_qp<<16)+(i&0xffff))); //write qp & sqp, also maintain ac_gop and ac_iopratio
                my_v0_hold( );
            }
            return 1; //// view0 done

        case 1:
            if ((rca.v1_fd_reset==1) && (rca.v1_fd_last_p==1) && (rca.v1_fd_last_row==1) && (v1_last_p_gop_change==FALSE) && (v1_poweron_rc_params_set==0) && (rca.v1_gop_change_NotResetRC==0)) { // lhu, 2017/04/17
                update_aof (); //lyu
                my_v1_initial_all( );
            }
            else if(rca.v1_rc_enable==1)
            {
                if ((rca.v1_fd_iframe==1) && (rca.v1_fd_row_cnt==0) && (v1_last_p_gop_change==TRUE) && (rca.v1_gop_change_NotResetRC==0)) { // lhu, 2017/04/17
                    // release last_p_gop_change to FALSE and execute initial_all operation until the first bu of iframe reached!!!
                    update_aof (); //lyu
                    my_v1_initial_all( );
                    v1_last_p_gop_change=FALSE;
                }
                if ((rca.v1_gop_cnt!=0) && (rca.v1_fd_iframe==1) && (rca.v1_fd_last_row==1) && (v1_poweron_rc_params_set==1)) {
                    v1_poweron_rc_params_set = 0; //lhuemu
                }
                my_rc_ac_br(1);
                if(rca.v1_re_bitrate==1 && rca.v1_fd_last_row==1) // change bit rate
                    READ_WORD(V1_BR_ADDR,rca.v1_bit_rate); //read br

                READ_WORD(V1_MAD_ADDR,v1_mad_tmp); //read mad
                READ_WORD(V1_HBITS_ADDR,v1_hbits_tmp); //read hbits
                //READ_WORD(V1_TBITS_ADDR,v1_tbits_tmp); //read tbits
                //Fix the HeaderBits and TextureBits assignment according to lyu's statistics === begin
                if (rca.v1_type==I_SLICE) v1_hbits_tmp = v1_hbits_tmp - (rca.v1_MBPerRow*3/2); // decrease 1.5 bit every MB for I SLICE
                else                      v1_hbits_tmp = v1_hbits_tmp - 5;                     // decrease 5 bit every BU for P SLICE
                READ_WORD(V1_ABITS_ADDR,v1_fbits_tmp);
                v1_tbits_tmp = (v1_fbits_tmp - rca.v1_PrevFbits) - v1_hbits_tmp;
                if (rca.v1_bu_cnt==0) rca.v1_PrevFbits = 0;
                else                  rca.v1_PrevFbits = v1_fbits_tmp;
                //Fix the HeaderBits and TextureBits assignment according to lyu's statistics === end
                if(rca.v1_fd_last_row==1) {
                    READ_WORD(V1_ABITS_ADDR,v1_fbits_tmp); //read abits
                    READ_WORD(V1_YMSEL_ADDR,ymsel_tmp); // read frame's y-mse
                    READ_WORD(V1_YMSEH_ADDR,ymseh_tmp);
                    v1_ymse_frame = (((long long)((ymseh_tmp>>24)&0x3f)<<32) + ymsel_tmp);
                    if ((ymseh_tmp>>2)&0x1) rca.v1_wireless_screen=1; else rca.v1_wireless_screen=0;
                    if ((ymseh_tmp>>3)&0x1) rca.v1_changeToIFrame=1; else rca.v1_changeToIFrame=0; // lhu, 2017/03/09
                    if ((ymseh_tmp>>4)&0x1) rca.v1_insertOneIFrame=1; else rca.v1_insertOneIFrame=0; // lhu, 2017/03/09
                    rca.v1_frm_ymse[0]  = v1_ymse_frame; // lhu, 2017/03/27
                    rca.v1_frm_fbits[0] = v1_fbits_tmp;  // lhu, 2017/03/27
                    rca.v1_RCSliceBits = (rca.v1_type==P_SLICE)? rca.v1_RCPSliceBits:rca.v1_RCISliceBits; // lhu, 2017/03/27
                    rca.v1_frm_hbits[0] = rca.v1_frame_hbits; // lhu, 2017/03/27
                    rca.v1_frm_abits[0] = rca.v1_frame_abits; // lhu, 2017/03/27
                    if (rca.v1_fd_iframe==1) rca.v1_ifrm_ymse = v1_ymse_frame; // lhu, 2017/04/13
                    else {
                        if (rca.v1_fd_last_p==1) {
                            rca.v1_lastpfrm_ymse = v1_ymse_frame;
                            READ_WORD(V1_ZW_BSINFO_ADDR,i);
                            if ((i>>1)&0x1) my_ac_RCISliceBitRatio(rca.v1_RCISliceBitRatioMax,1);
                            else {READ_WORD(V1_RCSET2_ADDR,i); rca.v1_RCISliceBitRatio = (i>>24)&0xf;}
                        }
                    }
                }
                qp = my_v1_rc_handle_mb( ); // update QP once
                if(rca.v1_fd_last_row==1) //frame last
                    rca.v1_slice_qp = rca.v1_PAveFrameQP;

                READ_WORD(V1_QP_ADDR,i);
                WRITE_WORD(V1_QP_ADDR,((qp<<24)+(rca.v1_slice_qp<<16)+(i&0xffff))); //write qp & sqp, also maintain ac_gop and ac_iopratio
                my_v1_hold( );
            }
            return 1; //// view1 done

        case 2: // 1080p used view0
            if ((rca.v1_fd_reset==1) && (rca.v1_fd_last_p==1) && (rca.v1_fd_last_row==1) && (v1_last_p_gop_change==FALSE) && (v1_poweron_rc_params_set==0) && (rca.v1_gop_change_NotResetRC==0)) { // lhu, 2017/04/17
                update_aof (); //lyu
                my_v1_initial_all( );
            }
            else if(rca.v1_rc_enable==1)
            {
                if ((rca.v1_fd_iframe==1) && (rca.v1_fd_row_cnt==0) && (v1_last_p_gop_change==TRUE) && (rca.v1_gop_change_NotResetRC==0)) { // lhu, 2017/04/17
                    // release last_p_gop_change to FALSE and execute initial_all operation until the first bu of iframe reached!!!
                    update_aof (); //lyu
                    my_v1_initial_all( );
                    v1_last_p_gop_change=FALSE;
                }
                if ((rca.v1_gop_cnt!=0) && (rca.v1_fd_iframe==1) && (rca.v1_fd_last_row==1) && (v1_poweron_rc_params_set==1)) {
                    v1_poweron_rc_params_set = 0; //lhuemu
                }
                my_rc_ac_br(1);
                if(rca.v1_re_bitrate==1 && rca.v1_fd_last_row==1) // change bit rate
                    READ_WORD(V1_BR_ADDR,rca.v1_bit_rate); //read br

                READ_WORD(V0_MAD_ADDR,v1_mad_tmp); //read mad
                READ_WORD(V0_HBITS_ADDR,v1_hbits_tmp); //read hbits
                //READ_WORD(V0_TBITS_ADDR,v1_tbits_tmp); //read tbits
                //Fix the HeaderBits and TextureBits assignment according to lyu's statistics === begin
                if (rca.v1_type==I_SLICE) v1_hbits_tmp = v1_hbits_tmp - (rca.v1_MBPerRow*3/2); // decrease 1.5 bit every MB for I SLICE
                else                      v1_hbits_tmp = v1_hbits_tmp - 5;                     // decrease 5 bit every BU for P SLICE
                READ_WORD(V0_ABITS_ADDR,v1_fbits_tmp);
                v1_tbits_tmp = (v1_fbits_tmp - rca.v1_PrevFbits) - v1_hbits_tmp;
                if (rca.v1_bu_cnt==0) rca.v1_PrevFbits = 0;
                else                  rca.v1_PrevFbits = v1_fbits_tmp;
                //Fix the HeaderBits and TextureBits assignment according to lyu's statistics === end
                if(rca.v1_fd_last_row==1) {
                    READ_WORD(V0_ABITS_ADDR,v1_fbits_tmp); //read abits
                    READ_WORD(V0_YMSEL_ADDR,ymsel_tmp); // read frame's y-mse
                    READ_WORD(V0_YMSEH_ADDR,ymseh_tmp);
                    v1_ymse_frame = (((long long)((ymseh_tmp>>24)&0x3f)<<32) + ymsel_tmp);
                    if ((ymseh_tmp>>2)&0x1) rca.v1_wireless_screen=1; else rca.v1_wireless_screen=0;
                    if ((ymseh_tmp>>3)&0x1) rca.v1_changeToIFrame=1; else rca.v1_changeToIFrame=0; // lhu, 2017/03/09
                    if ((ymseh_tmp>>4)&0x1) rca.v1_insertOneIFrame=1; else rca.v1_insertOneIFrame=0; // lhu, 2017/03/09
                    rca.v1_frm_ymse[0]  = v1_ymse_frame; // lhu, 2017/03/27
                    rca.v1_frm_fbits[0] = v1_fbits_tmp;  // lhu, 2017/03/27
    	            rca.v1_RCSliceBits = (rca.v1_type==P_SLICE)? rca.v1_RCPSliceBits:rca.v1_RCISliceBits; // lhu, 2017/03/27
                    rca.v1_frm_hbits[0] = rca.v1_frame_hbits; // lhu, 2017/03/27
                    rca.v1_frm_abits[0] = rca.v1_frame_abits; // lhu, 2017/03/27
                    if (rca.v1_fd_iframe==1) rca.v1_ifrm_ymse = v1_ymse_frame; // lhu, 2017/04/13
                    else {
                        if (rca.v1_fd_last_p==1) {
                            rca.v1_lastpfrm_ymse = v1_ymse_frame;
                            READ_WORD(V1_ZW_BSINFO_ADDR,i);
                            if ((i>>1)&0x1) my_ac_RCISliceBitRatio(rca.v1_RCISliceBitRatioMax,1);
                            else {READ_WORD(V1_RCSET2_ADDR,i); rca.v1_RCISliceBitRatio = (i>>24)&0xf;}
                        }
                    }
                }
                qp = my_v1_rc_handle_mb( ); // update QP once
                if(rca.v1_fd_last_row==1) //frame last
                    rca.v1_slice_qp = rca.v1_PAveFrameQP;

                READ_WORD(V1_QP_ADDR,i);
                WRITE_WORD(V1_QP_ADDR,((qp<<24)+(rca.v1_slice_qp<<16)+(i&0xffff))); //write qp & sqp, also maintain ac_gop and ac_iopratio
                my_v1_hold( );
            }
            return 1; //// view1 done

        default: return 1;
    }
}
#endif   //###########
//==============================================================================
//==============================================================================


//-----------------------------------------------------------------------------
// add by bomb for aof target cycles
//-----------------------------------------------------------------------------
void update_aof_cycle(){
    unsigned int v0_on=0,v0_mbs=0, v1_on=0,v1_mbs=0, tmp=0,i=0;

    READ_WORD((REG_BASE_ADDR+(0x00<<2)),v0_on); //view0
    v0_on=(v0_on>>24)&0x1;
    if(v0_on==1){
        READ_WORD((REG_BASE_ADDR+(0x01<<2)),i);
        v0_mbs=((((i>>16)&0xffff)+15)/16)*(((i&0xffff)+15)/16);
        READ_WORD((REG_BASE_ADDR+(0x02<<2)),i);
        v0_mbs=v0_mbs*((i>>16)&0xff);
    }
    READ_WORD((REG_BASE_ADDR+(0x19<<2)),v1_on); //view1
    v1_on=(v1_on>>24)&0x1;
    if(v1_on==1){
        READ_WORD((REG_BASE_ADDR+(0x1a<<2)),i);
        v1_mbs=((((i>>16)&0xffff)+15)/16)*(((i&0xffff)+15)/16);
        READ_WORD((REG_BASE_ADDR+(0x1b<<2)),i);
        v1_mbs=v1_mbs*((i>>16)&0xff);
    }

    i=v0_mbs+v1_mbs;
    if(i>0){
        i=IN_CLK/i;
        if(i>0xff00) i=0xff00; else if(i<540) i=540;
        if(i>580) i=i-30;

        READ_WORD((REG_BASE_ADDR+(0x0a<<2)),tmp); //write back view0
        tmp=(tmp&0xff0000ff)|((i&0xffff)<<8);
        WRITE_WORD((REG_BASE_ADDR+(0x0a<<2)),tmp);
        READ_WORD((REG_BASE_ADDR+(0x23<<2)),tmp); //write back view1
        tmp=(tmp&0xff0000ff)|((i&0xffff)<<8);
        WRITE_WORD((REG_BASE_ADDR+(0x23<<2)),tmp);
    }
}
void update_aof(){
    if(aof_v0_frame!=rca.v0_frame_cnt){
        update_aof_cycle();
        aof_v0_frame=rca.v0_frame_cnt;
    }
    else if(aof_v1_frame!=rca.v1_frame_cnt){
        update_aof_cycle();
        aof_v1_frame=rca.v1_frame_cnt;
    }
    else ;
}
//-----------------------------------------------------------------------------


#ifdef ARMCM7_RC  //###########
//==============================================================================
void my_v0_initial_all( )
{
    int i;
    my_v0_rc_params( );
    //if(rca.v0_rc_enable==1) //lhuemu
    {
        qp = my_v0_rc_handle_mb( ); // update QP initial
        READ_WORD(V0_QP_ADDR,i);
        WRITE_WORD(V0_QP_ADDR,((qp<<24)+(rca.v0_slice_qp<<16)+(i&0xffff))); //after RC initial..
    }
}
void my_v1_initial_all( )
{
    int i;
    my_v1_rc_params( );
    //if(rca.v1_rc_enable==1) //lhuemu
    {
        qp = my_v1_rc_handle_mb( ); // update QP initial
        READ_WORD(V1_QP_ADDR,i);
        WRITE_WORD(V1_QP_ADDR,((qp<<24)+(rca.v1_slice_qp<<16)+(i&0xffff))); //after RC initial..
    }
}

//// read feedback data for restart rc and etc
void my_v0_feedback(unsigned int feedback)
{
    //READ_WORD(V0_FEEDBACK_ADDR,i); //read feedback-data
    rca.v0_aof_inc_qp = (feedback>>16)&0xffff;
    rca.v0_fd_row_cnt = (feedback>>9)&0x7f;
    rca.v0_fd_last_row = (feedback>>8)&0x1;
    //rca.fd_cpu_test = (feedback>>7)&0x1;
    rca.v0_fd_irq_en = (feedback>>4)&0x1;
    //if(rca.v0_re_bitrate==0)
    rca.v0_re_bitrate = (feedback>>3)&0x1;
    //if(rca.v0_fd_reset==0)
    rca.v0_fd_reset = (feedback>>2)&0x1;
    rca.v0_fd_iframe = (feedback>>1)&0x1;
    rca.v0_fd_last_p = feedback&0x1;
    READ_WORD(V0_RCEN_BU_ADDR,feedback); //read rc_en, rc_mode & bu
        rca.v0_rc_enable = (feedback>>24)&0x1;
    READ_WORD(V0_ENABLE_ADDR,feedback); //read view enable
        rca.v0_enable = (feedback>>24)&0x1;
}

void my_v1_feedback(unsigned int feedback)
{
    //READ_WORD(V1_FEEDBACK_ADDR,i); //read feedback-data
    rca.v1_aof_inc_qp = (feedback>>16)&0xffff;
    rca.v1_fd_row_cnt = (feedback>>9)&0x7f;
    rca.v1_fd_last_row = (feedback>>8)&0x1;
    rca.v1_fd_irq_en = (feedback>>4)&0x1;
    //if(rca.v1_re_bitrate==0)
    rca.v1_re_bitrate = (feedback>>3)&0x1;
    //if(rca.v1_fd_reset==0)
    rca.v1_fd_reset = (feedback>>2)&0x1;
    rca.v1_fd_iframe = (feedback>>1)&0x1;
    rca.v1_fd_last_p = feedback&0x1;
    READ_WORD(V1_RCEN_BU_ADDR,feedback); //read rc_en, rc_mode & bu
        rca.v1_rc_enable = (feedback>>24)&0x1;
    READ_WORD(V1_ENABLE_ADDR,feedback); //read view enable
        rca.v1_enable = (feedback>>24)&0x1;
}
#endif   //###########



//-----------------------------------------------------------------------------------
// \brief
//    Initialize rate control parameters
//-----------------------------------------------------------------------------------
#ifdef ARMCM7_RC  //###########
void my_v0_rc_params( ) {
    int i,j,m;

    v0_mad_tmp       = 0; // @lhu, initial value
    v0_hbits_tmp     = 0;
    v0_tbits_tmp     = 0;
    rca.v0_enable    = 0;
    rca.v0_gop_cnt   = 0;
    rca.v0_frame_cnt = 0;
    rca.v0_bu_cnt    = 0;
    rca.v0_mb_cnt    = 0;
    rca.v0_fd_reset  = 0; ////
    rca.v0_aof_inc_qp = 0;
    rca.v0_fd_last_row= 0;
    rca.v0_fd_row_cnt = 0;
    rca.v0_fd_last_p  = 0;
    rca.v0_fd_iframe  = 0;
    rca.v0_re_bitrate = 0;
    rca.v0_prev_ac_br_index = 0;
    rca.v0_wireless_screen = 0; // lhu, 2017/02/27
    rca.v0_changeToIFrame = 0; // lhu, 2017/03/09
    rca.v0_insertOneIFrame= 0; // lhu, 2017/03/09
    rca.v0_PrevFrmPSNRLow= 0; // lhu, 2017/03/15
    rca.v0_gop_change_NotResetRC = 0; // lhu, 2017/03/07
    rca.v0_nextPFgotoIF = 0; // lhu, 2017/03/07
    rca.v0_IFduration = 0; // lhu, 2017/03/07
    rca.v0_PrevFbits = 0; // lhu, 2017/04/05
    v0_last_p_gop_change = FALSE; // @lhu, initial value

    READ_WORD(V0_FRAME_XY_ADDR,m); //read frame-x & frame-y
        i=(m>>16)&0xffff;
        j=m&0xffff;
        rca.v0_MBPerRow = (i+15)/16;
        rca.v0_FrameSizeInMbs = rca.v0_MBPerRow*((j+15)/16);
        rca.v0_size = rca.v0_FrameSizeInMbs<<8;
        rca.v0_width = i;
        rca.v0_height = j;

    READ_WORD(V0_GOPFPS_ADDR,i); //read gop and fps
        i=(i>>16)&0xffff;
        rca.v0_intra_period = (i>>8)&0xff;
        rca.v0_framerate = i&0xff;

    READ_WORD(V0_RCEN_BU_ADDR,i); //read rc_en, rc_mode & bu
        rca.v0_rc_enable = (i>>24)&0x1;
        rca.v0_RCUpdateMode = (i>>16)&0x3;
        rca.v0_BasicUnit = i=i&0xffff;
        rca.v0_basicunit = i=i&0xffff;

    READ_WORD(V0_RCSET1_ADDR,i); //read rcset1
        rca.v0_PMaxQpChange = i&0x3f;
        rca.v0_RCMinQP = (i>>8)&0x3f;
        rca.v0_RCMaxQP = (i>>16)&0x3f;

    READ_WORD(V0_BR_ADDR,i); //read br
        rca.v0_bit_rate = i;

    READ_WORD(V0_RCSET2_ADDR,i); //read rcset2
        rca.v0_RCISliceBitRatioMax= (i>>8)&0x3f;
        rca.v0_RCIoverPRatio = (i>>16)&0xf;
        rca.v0_RCISliceBitRatio = (i>>24)&0xf;
}
#else  //###########
void my_rc_params( ) {
  rca.height           = img->height;
  rca.width            = img->width;
  rca.MBPerRow         = (rca.width+15)/16;
  rca.FrameSizeInMbs   = ((rca.height+15)/16)*rca.MBPerRow;
  rca.size             = rca.FrameSizeInMbs*256;
  rca.basicunit        = params->basicunit;
  rca.BasicUnit        = params->basicunit;
  rca.bit_rate         = params->bit_rate;
  rca.RCUpdateMode     = params->RCUpdateMode; ////
  rca.framerate        = params->source.frame_rate;
  rca.PMaxQpChange     = params->RCMaxQPChange;
  rca.RCMinQP          = img->RCMinQP;
  rca.RCMaxQP          = img->RCMaxQP;
  rca.intra_period     = params->intra_period;
  rca.rc_enable = 1;
  rca.RCIoverPRatio    = params->RCIoverPRatio; // 3.8
  rca.type             = I_SLICE;
  rca.RCISliceBitRatio = params->RCISliceBitRatio;
  rca.re_bitrate = 0;
}
#endif //###########

#ifdef ARMCM7_RC //###########
//===== Auto-config Bit-Rate =====
void my_rc_ac_br(int view) {
    int m,ac_br;
    unsigned char ac_br_index,v0_ac_br_index,v1_ac_br_index;
    if (view==0) {
        READ_WORD(V0_RC_ACBR_ADDR,m);
        v0_ac_br_index = (m>>26)&0x3f;
    } else {
        READ_WORD(V1_RC_ACBR_ADDR,m);
        v1_ac_br_index = (m>>26)&0x3f;
    }
    ac_br_index = (view==0)? v0_ac_br_index: v1_ac_br_index;

    switch(ac_br_index) {
        case 0 : ac_br = 8000000 ; break; // 8Mbps
        case 1 : ac_br = 600000  ; break; // 600kps
        case 2 : ac_br = 1200000 ; break; // 1.2Mbps
        case 3 : ac_br = 2400000 ; break; // 2.4Mbps
        case 4 : ac_br = 3000000 ; break; // 3Mbps
        case 5 : ac_br = 3500000 ; break; // 3.5Mbps
        case 6 : ac_br = 4000000 ; break; // 4Mbps
        case 7 : ac_br = 4800000 ; break; // 4.8Mbps
        case 8 : ac_br = 5000000 ; break; // 5Mbps
        case 9 : ac_br = 6000000 ; break; // 6Mbps
        case 10: ac_br = 7000000 ; break; // 7Mbps
        case 11: ac_br = 7500000 ; break; // 7.5Mbps
        case 12: ac_br = 9000000 ; break; // 9Mbps
        case 13: ac_br = 10000000; break; // 10Mbps
        case 14: ac_br = 11000000; break; // 11Mbps
        case 15: ac_br = 12000000; break; // 12Mbps
        case 16: ac_br = 13000000; break; // 13Mbps
        case 17: ac_br = 14000000; break; // 14Mbps
        case 18: ac_br = 15000000; break; // 15Mbps
        case 19: ac_br = 16000000; break; // 16Mbps
        case 20: ac_br = 17000000; break; // 17Mbps
        case 21: ac_br = 18000000; break; // 18Mbps
        case 22: ac_br = 19000000; break; // 19Mbps
        case 23: ac_br = 20000000; break; // 20Mbps
        case 24: ac_br = 21000000; break; // 21Mbps
        case 25: ac_br = 22000000; break; // 22Mbps
        case 26: ac_br = 23000000; break; // 23Mbps
        case 27: ac_br = 24000000; break; // 24Mbps
        case 28: ac_br = 25000000; break; // 25Mbps
        case 29: ac_br = 26000000; break; // 26Mbps
        case 30: ac_br = 27000000; break; // 27Mbps
        case 31: ac_br = 28000000; break; // 28Mbps
        case 32: ac_br = 29000000; break; // 29Mbps
        case 33: ac_br = 30000000; break; // 30Mbps
        default: ac_br = 8000000 ; break; // 8Mbps
    }
    if (view==0) {
        if (v0_ac_br_index != rca.v0_prev_ac_br_index)
            WRITE_WORD(V0_BR_ADDR, ac_br);
        rca.v0_prev_ac_br_index = v0_ac_br_index;
    } else {
        if (v1_ac_br_index != rca.v1_prev_ac_br_index)
            WRITE_WORD(V1_BR_ADDR, ac_br);
        rca.v1_prev_ac_br_index = v1_ac_br_index;
    }
}
unsigned short my_divider2psnr(int my_divider) {
    unsigned short my_psnr;
    if      (my_divider>=1000000)                      my_psnr = 60;// 10^6.0=10000
    else if (my_divider>=794328 && my_divider<1000000) my_psnr = 59;// 10^5.9=794328
    else if (my_divider>=630957 && my_divider<794328)  my_psnr = 58;// 10^5.8=630957
    else if (my_divider>=501187 && my_divider<630957)  my_psnr = 57;// 10^5.7=501187
    else if (my_divider>=398107 && my_divider<501187)  my_psnr = 56;// 10^5.6=398107
    else if (my_divider>=316227 && my_divider<398107)  my_psnr = 55;// 10^5.5=316227
    else if (my_divider>=251188 && my_divider<316227)  my_psnr = 54;// 10^5.4=251188
    else if (my_divider>=199526 && my_divider<251188)  my_psnr = 53;// 10^5.3=199526
    else if (my_divider>=158489 && my_divider<199526)  my_psnr = 52;// 10^5.2=158489
    else if (my_divider>=125892 && my_divider<158489)  my_psnr = 51;// 10^5.1=125892
    else if (my_divider>=100000 && my_divider<125892)  my_psnr = 50;// 10^5.0=100000
    else if (my_divider>=79432  && my_divider<100000)  my_psnr = 49;// 10^4.9=79432
    else if (my_divider>=63095  && my_divider<79432 )  my_psnr = 48;// 10^4.8=63095
    else if (my_divider>=50118  && my_divider<63095 )  my_psnr = 47;// 10^4.7=50118
    else if (my_divider>=39810  && my_divider<50118 )  my_psnr = 46;// 10^4.6=39810
    else if (my_divider>=31622  && my_divider<39810 )  my_psnr = 45;// 10^4.5=31622
    else if (my_divider>=25118  && my_divider<31622 )  my_psnr = 44;// 10^4.4=25118
    else if (my_divider>=19952  && my_divider<25118 )  my_psnr = 43;// 10^4.3=19952
    else if (my_divider>=15848  && my_divider<19952 )  my_psnr = 42;// 10^4.2=15848
    else if (my_divider>=12589  && my_divider<15848 )  my_psnr = 41;// 10^4.1=12589
    else if (my_divider>=10000  && my_divider<12589 )  my_psnr = 40;// 10^4.0=10000
    else if (my_divider>=7943   && my_divider<10000 )  my_psnr = 39;// 10^3.9=7943
    else if (my_divider>=6309   && my_divider<7943  )  my_psnr = 38;// 10^3.8=6309
    else if (my_divider>=5011   && my_divider<6309  )  my_psnr = 37;// 10^3.7=5011
    else if (my_divider>=3981   && my_divider<5011  )  my_psnr = 36;// 10^3.6=3981
    else if (my_divider>=3162   && my_divider<3981  )  my_psnr = 35;// 10^3.5=3162
    else if (my_divider>=2511   && my_divider<3162  )  my_psnr = 34;// 10^3.4=2511
    else if (my_divider>=1995   && my_divider<2511  )  my_psnr = 33;// 10^3.3=1995
    else if (my_divider>=1584   && my_divider<1995  )  my_psnr = 32;// 10^3.2=1584
    else if (my_divider>=1258   && my_divider<1584  )  my_psnr = 31;// 10^3.1=1258
    else if (my_divider>=1000   && my_divider<1258  )  my_psnr = 30;// 10^3.0=1000
    else if (my_divider>=794    && my_divider<1000  )  my_psnr = 29;// 10^2.9=794
    else if (my_divider>=630    && my_divider<794   )  my_psnr = 28;// 10^2.8=630
    else if (my_divider>=501    && my_divider<630   )  my_psnr = 27;// 10^2.7=501
    else if (my_divider>=398    && my_divider<501   )  my_psnr = 26;// 10^2.6=398
    else if (my_divider>=316    && my_divider<398   )  my_psnr = 25;// 10^2.5=316
    else if (my_divider>=251    && my_divider<316   )  my_psnr = 24;// 10^2.4=251
    else if (my_divider>=199    && my_divider<251   )  my_psnr = 23;// 10^2.3=199
    else if (my_divider>=158    && my_divider<199   )  my_psnr = 22;// 10^2.2=158
    else if (my_divider>=125    && my_divider<158   )  my_psnr = 21;// 10^2.1=125
    else if (my_divider>=100    && my_divider<125   )  my_psnr = 20;// 10^2.0=100
    else if (my_divider>=79     && my_divider<100   )  my_psnr = 19;// 10^1.9=79
    else if (my_divider>=63     && my_divider<79    )  my_psnr = 18;// 10^1.8=63
    else if (my_divider>=50     && my_divider<63    )  my_psnr = 17;// 10^1.7=50
    else if (my_divider>=39     && my_divider<50    )  my_psnr = 16;// 10^1.6=39
    else if (my_divider>=31     && my_divider<39    )  my_psnr = 15;// 10^1.5=31
    else if (my_divider>=25     && my_divider<31    )  my_psnr = 14;// 10^1.4=25
    else                                               my_psnr = 13;
    
    return my_psnr;
}
// compare two consecutive P frame's psnr, if it drop sharply and the degree of drop is greater than psnr_drop_level, I frame should be inserted afterward.
unsigned char my_trace_PSNRDropSharply(unsigned char psnr_drop_level, unsigned char view) {
    long long whm255square,m;
    int prev_frame_divider,curr_frame_divider;
    unsigned short prev_frame_psnr, curr_frame_psnr;
    if (view==0) {
        whm255square = (long long)(rca.v0_width*255)*(long long)(rca.v0_height*255);
        prev_frame_divider = (int)(whm255square/rca.v0_frm_ymse[1]);
        curr_frame_divider = (int)(whm255square/rca.v0_frm_ymse[0]);
    } else {
        whm255square = (long long)(rca.v1_width*255)*(long long)(rca.v1_height*255);
        prev_frame_divider = (int)(whm255square/rca.v1_frm_ymse[1]);
        curr_frame_divider = (int)(whm255square/rca.v1_frm_ymse[0]);
    }
    prev_frame_psnr = my_divider2psnr(prev_frame_divider);
    curr_frame_psnr = my_divider2psnr(curr_frame_divider);
    if ( (prev_frame_psnr>curr_frame_psnr) && ((prev_frame_psnr-curr_frame_psnr)>=psnr_drop_level) ) {
        if (view==0) rca.v0_PSNRDropSharply = 1;
        else         rca.v1_PSNRDropSharply = 1;
    } else {
        if (view==0) rca.v0_PSNRDropSharply = 0;
        else         rca.v1_PSNRDropSharply = 0;
    }
    if (view==0) return rca.v0_PSNRDropSharply;
    else         return rca.v1_PSNRDropSharply;
}
void my_criteria_decide_changeToIFrame(unsigned char HBitsRatioABits_level, unsigned char ABitsRatioTargetBits_level, unsigned char PSNRDrop_level, unsigned char view) {
    int v0_IntraPeriod, v0_NotResetRC, v1_IntraPeriod, v1_NotResetRC, i, RCSliceBits;
    Boolean v0_Condition1=FALSE,v0_Condition2=FALSE,v0_Condition3=FALSE,v1_Condition1=FALSE,v1_Condition2=FALSE,v1_Condition3=FALSE;
    if (view == 0) {
        // Condition1: Hbits/(Hbits+Tbits) bigger or equal than HBitsRatioABits_level.
        if ( (rca.v0_frm_hbits[0]*100/rca.v0_frm_abits[0])>=HBitsRatioABits_level ) v0_Condition1=TRUE;
        // Condition2: Abits/TargetBits of this frame is bigger or equal than ABitsRatioTargetBits_level.
        if ( (rca.v0_frm_fbits[0]*100/rca.v0_RCSliceBits)>=ABitsRatioTargetBits_level ) v0_Condition2=TRUE;
        // Condition3: PSNR drop sharply and degree of the drop is greater than PSNRDrop_level.
        if ( my_trace_PSNRDropSharply(PSNRDrop_level,0) ) v0_Condition3=TRUE;
        if ( v0_Condition1==TRUE && v0_Condition2==TRUE && v0_Condition3==TRUE ) {rca.v0_nextPFgotoIF=1; v0_IntraPeriod=1; v0_NotResetRC=1; rca.v0_IFduration=1;}
        else                                                                     {rca.v0_nextPFgotoIF=0; v0_NotResetRC=0; rca.v0_IFduration=0;}
    } else {
        if ( (rca.v1_frm_hbits[0]*100/rca.v1_frm_abits[0])>=HBitsRatioABits_level ) v1_Condition1=TRUE;
        if ( (rca.v1_frm_fbits[0]*100/rca.v1_RCSliceBits)>=ABitsRatioTargetBits_level ) v1_Condition2=TRUE;
        if ( my_trace_PSNRDropSharply(PSNRDrop_level,1) ) v1_Condition3=TRUE;
        if ( v1_Condition1==TRUE && v1_Condition2==TRUE && v1_Condition3==TRUE ) {rca.v1_nextPFgotoIF=1; v1_IntraPeriod=1; v1_NotResetRC= 1; rca.v1_IFduration=1;}
        else                                                                     {rca.v1_nextPFgotoIF=0; v1_NotResetRC= 0; rca.v1_IFduration=0;}
    }
	if (view == 0) {
        rca.v0_gop_change_NotResetRC = v0_NotResetRC;
        if (rca.v0_nextPFgotoIF==1) {
            READ_WORD(V0_GOPFPS_ADDR,i);
            rca.v0_PrevIntraPeriod = (i>>24)&0xff; // save previous GOP before writing new GOP to it.
            WRITE_WORD(V0_GOPFPS_ADDR,((v0_IntraPeriod<<24)+(i&0xffffff)));
            if (rca.v0_insertOneIFrame==1) {
                READ_WORD(V0_GOPFPS_ADDR,i);
                rca.v0_intra_period= (i>>24)&0xff;
            }
        }
    } else {
        rca.v1_gop_change_NotResetRC = v1_NotResetRC;
        if (rca.v1_nextPFgotoIF==1) {
            READ_WORD(V1_GOPFPS_ADDR,i);
            rca.v1_PrevIntraPeriod = (i>>24)&0xff; // save previous GOP before writing new GOP to it.
            WRITE_WORD(V1_GOPFPS_ADDR,((v1_IntraPeriod<<24)+(i&0xffffff)));
            if (rca.v1_insertOneIFrame==1) {
                READ_WORD(V1_GOPFPS_ADDR,i);
                rca.v1_intra_period= (i>>24)&0xff;
            }
        }
    }
}
// Go back to its normal GOP structure ===> After reach next GOP's I frame, release the IFduration.
void my_decide_backtoNormalGOP(int view) {
	int i;
    if (view == 0) {
        if ( (rca.v0_frame_cnt==0) && (rca.v0_IFduration==1) ) {
            READ_WORD(V0_GOPFPS_ADDR,i);
            WRITE_WORD(V0_GOPFPS_ADDR,((rca.v0_PrevIntraPeriod<<24)+(i&0xffffff)));
            rca.v0_IFduration=0;
            if (rca.v0_insertOneIFrame==1) {
                READ_WORD(V0_GOPFPS_ADDR,i);
                rca.v0_intra_period= (i>>24)&0xff; // go back to its normal IntraPeriod
            }
        }
    } else {
        if ( (rca.v1_frame_cnt==0) && (rca.v1_IFduration==1) ) {
            READ_WORD(V1_GOPFPS_ADDR,i);
            WRITE_WORD(V1_GOPFPS_ADDR,((rca.v1_PrevIntraPeriod<<24)+(i&0xffffff)));
            rca.v1_IFduration=0;
            if (rca.v1_insertOneIFrame==1) {
                READ_WORD(V1_GOPFPS_ADDR,i);
                rca.v1_intra_period= (i>>24)&0xff; // go back to its normal IntraPeriod
            }
        }
    }
}
/*===== Criteria for auto-config of RCISliceBitRatio=====
1> Depend on comparsion of I frame's psnr(Ipsnr) and last P frame's psnr(Ppsnr) in current GOP.
2> Ppsnr-Ipsnr and RCISliceBitRatio_nextGOP and RCISliceBitRatio_currGOP's relation:
    ||                       ||
    \/                       \/
    -5             RCISliceBitRatio_currGOP-4
    ...                      ...
    -2             RCISliceBitRatio_currGOP-1
    -1             RCISliceBitRatio_currGOP
     0             RCISliceBitRatio_currGOP
    +1             RCISliceBitRatio_currGOP+1
    +2             RCISliceBitRatio_currGOP+2
    ...                      ...
    +5             RCISliceBitRatio_currGOP+5
3> Finally use RCISliceBitRatioMax value to clamp final output RCISliceBitRatio value.*/
void my_ac_RCISliceBitRatio(unsigned char RCISliceBitRatioMax, int view) {
    long long whm255square,m;
    int i,iframe_divider,lastpframe_divider;
    unsigned short iframe_psnr, lastpframe_psnr;
    signed short diffpsnr_PI;
    unsigned char RCISliceBitRatio_currGOP, RCISliceBitRatio_nextGOP;
    if (view==0) {
        whm255square = (long long)(rca.v0_width*255)*(long long)(rca.v0_height*255);
        iframe_divider = (int)(whm255square/rca.v0_ifrm_ymse);
        lastpframe_divider = (int)(whm255square/rca.v0_lastpfrm_ymse);
    } else {
        whm255square = (long long)(rca.v1_width*255)*(long long)(rca.v1_height*255);
        iframe_divider = (int)(whm255square/rca.v1_ifrm_ymse);
        lastpframe_divider = (int)(whm255square/rca.v1_lastpfrm_ymse);
    }
    iframe_psnr = my_divider2psnr(iframe_divider);
    lastpframe_psnr = my_divider2psnr(lastpframe_divider);

    diffpsnr_PI = lastpframe_psnr - iframe_psnr;
    if (view==0) RCISliceBitRatio_currGOP = rca.v0_RCISliceBitRatio;
    else         RCISliceBitRatio_currGOP = rca.v1_RCISliceBitRatio;
    
    if (diffpsnr_PI>=1)                         RCISliceBitRatio_nextGOP = RCISliceBitRatio_currGOP+diffpsnr_PI;
    else if (diffpsnr_PI>=-1 && diffpsnr_PI<=0) RCISliceBitRatio_nextGOP = RCISliceBitRatio_currGOP;
    else                                        RCISliceBitRatio_nextGOP = RCISliceBitRatio_currGOP+diffpsnr_PI+1;
    RCISliceBitRatio_nextGOP = my_imax(RCISliceBitRatio_nextGOP, 1);
    RCISliceBitRatio_nextGOP = my_iequmin(RCISliceBitRatio_nextGOP, RCISliceBitRatioMax);
    
    if (view==0) {
        READ_WORD(V0_ZW_BSINFO_ADDR,i);
        WRITE_WORD(V0_ZW_BSINFO_ADDR,((i&0xffffff03)|((RCISliceBitRatio_nextGOP&0x3f)<<2)));
        rca.v0_RCISliceBitRatio = RCISliceBitRatio_nextGOP;
	} else {
        READ_WORD(V1_ZW_BSINFO_ADDR,i);
        WRITE_WORD(V1_ZW_BSINFO_ADDR,((i&0xffffff03)|((RCISliceBitRatio_nextGOP&0x3f)<<2)));
        rca.v1_RCISliceBitRatio = RCISliceBitRatio_nextGOP;
	}
}
#endif  //###########

void my_v0_rc_init_seq( )
{
//18  double L1,L2,L3;
  int bpp_p6,qp,i;

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

  rca.v0_type     = I_SLICE;
  rca.v0_qp       = 0;
  rca.v0_slice_qp = 0;
  rca.v0_c1_over  = 0;
  rca.v0_cmadequ0 = 0;// lhumad
  rca.v0_frame_mad   = 0;
  rca.v0_frame_tbits = 0;
  rca.v0_frame_hbits = 0;
  rca.v0_frame_abits = 0;

  //if(rca.v0_intra_period==1)
  //  rca.v0_RCUpdateMode = RC_MODE_1;

  if(rca.v0_RCUpdateMode!=RC_MODE_0)
  {
    if (rca.v0_RCUpdateMode==RC_MODE_1 && rca.v0_intra_period==1) {// make sure it execute only once!!! lhumod
      rca.v0_no_frm_base = rca.v0_intra_period*50; //!!!
      rca.v0_intra_period = rca.v0_no_frm_base;// make fake for frame_cnt increment, lhumod
    }
    else if (rca.v0_RCUpdateMode==RC_MODE_3) rca.v0_no_frm_base = rca.v0_intra_period*1; // lhugop
  }

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
  switch (rca.v0_RCUpdateMode )
  {
     //case RC_MODE_0: my_v0_updateQP = my_v0_updateQPRC0; break;
     //case RC_MODE_1: my_v0_updateQP = my_v0_updateQPRC1; break;
     case RC_MODE_3: my_v0_updateQP = my_v0_updateQPRC3; break;
     default: my_v0_updateQP = my_v0_updateQPRC3; break;
  }

  rca.v0_PreviousMAD_8p = (1<<8);
  rca.v0_CurrentMAD_8p  = (1<<8);
  rca.v0_Target        = 0;
  rca.v0_LowerBound    = 0;
  rca.v0_UpperBound1   = MAX_INT;
  rca.v0_UpperBound2   = MAX_INT;
  rca.v0_PAveFrameQP   = 0;
  rca.v0_m_Qc          = 0;
  rca.v0_PAverageQp    = 0;

  for(i=0;i<70;i++)
  {
    rca.v0_BUPFMAD_8p[i] = 0;
    rca.v0_BUCFMAD_8p[i] = 0;
  }

  for(i=0;i<2;i++) { // set ymse_pframe[i] to max value at begining of sequence, lhu, 2017/03/27
    rca.v0_frm_ymse[i] = rca.v0_height*rca.v0_width*((1<<8)-1)*((1<<8)-1);
  }
  rca.v0_PrevBitRate = rca.v0_bit_rate; //lhumod
  //compute the total number of MBs in a frame
  if(rca.v0_basicunit >= rca.v0_FrameSizeInMbs)
    rca.v0_basicunit = rca.v0_FrameSizeInMbs;

  if(rca.v0_basicunit < rca.v0_FrameSizeInMbs)
    rca.v0_TotalNumberofBasicUnit = rca.v0_FrameSizeInMbs/rca.v0_basicunit;
  else
    rca.v0_TotalNumberofBasicUnit = 1;

  //initialize the parameters of fluid flow traffic model
  rca.v0_CurrentBufferFullness = 0;
//  rca.GOPTargetBufferLevel = 0; //(double)rca.CurrentBufferFullness;

  //initialize the previous window size
  rca.v0_m_windowSize = 0;
  rca.v0_MADm_windowSize = 0;
  rca.v0_NumberofCodedPFrame = 0;
  rca.v0_NumberofGOP = 0;
  //remaining # of bits in GOP
  rca.v0_RemainingBits = 0;

  rca.v0_GAMMAP_1p=1;
  rca.v0_BETAP_1p=1;

  //quadratic rate-distortion model
  rca.v0_PPreHeader=0;

  rca.v0_Pm_X1_8p = rca.v0_bit_rate<<8;
  rca.v0_Pm_X2_8p = 0;
  // linear prediction model for P picture
  rca.v0_PMADPictureC1_12p = (1<<12);
  rca.v0_PMADPictureC2_12p = 0;
  rca.v0_MADPictureC1_12p = (1<<12);
  rca.v0_MADPictureC2_12p = 0;

  // Initialize values
  for(i=0;i<20;i++)
  {
    rca.v0_m_rgQp_8p[i] = 0;
    rca.v0_m_rgRp_8p[i] = 0;
    rca.v0_m_rgRp_8prr8[i] = 0;
    rca.v0_rc_tmp0[i] = 0;
    rca.v0_rc_tmp1[i] = 0;
    rca.v0_rc_tmp2[i] = 0;
    rca.v0_rc_tmp3[i] = 0;
    rca.v0_rc_tmp4[i] = 0;

    rca.v0_PictureMAD_8p[i]   = 0;
    rca.v0_ReferenceMAD_8p[i] = 0;
    rca.v0_mad_tmp0[i] = 0;
    rca.v0_mad_tmp0_valid[i] = 1;
    rca.v0_mad_tmp1[i] = 0;
    rca.v0_mad_tmp2[i] = 0;

    rca.v0_rc_rgRejected[i] = FALSE;
    rca.v0_mad_rgRejected[i] = FALSE;
  }

  rca.v0_rc_hold = 0;
  rca.v0_mad_hold = 0;

  rca.v0_PPictureMAD_8p = 0;
  //basic unit layer rate control
  rca.v0_PAveHeaderBits1 = 0;
  rca.v0_PAveHeaderBits3 = 0;
  rca.v0_DDquant = (rca.v0_TotalNumberofBasicUnit>=9? 1:2);

  rca.v0_frame_bs = rca.v0_bit_rate/rca.v0_framerate;

  bpp_p6=(rca.v0_frame_bs<<6)/rca.v0_size; //for test
/*if     (bpp_p6<=0x26) qp=35;
  else if(bpp_p6<=0x39) qp=25;
  else if(bpp_p6<=0x59) qp=20;
  else                  qp=10;*/// test for more initial_qp assignment, lhuemu
  if     (bpp_p6<=0x6 ) {if (rca.v0_height>=1080) qp=42; else if(rca.v0_height>=720) qp=40; else qp=38;}
  else if(bpp_p6<=0x16) {if (rca.v0_height>=1080) qp=39; else if(rca.v0_height>=720) qp=37; else qp=35;}
  else if(bpp_p6<=0x26) qp=35;
  else if(bpp_p6<=0x39) qp=25;
  else if(bpp_p6<=0x59) qp=20;
  else                  qp=10;

  rca.v0_MyInitialQp=qp;
}


void my_v0_rc_init_GOP(int np)
{
  Boolean Overum=FALSE;
  int OverBits,denom,i;
  int GOPDquant;
  int gop_bits;
  int v0_RCISliceBitsLow,v0_RCISliceBitsHigh,v0_RCISliceBitsLow2,v0_RCISliceBitsHigh2,v0_RCISliceBitsLow4,v0_RCISliceBitsHigh4,v0_RCISliceBitsLow8,v0_RCISliceBitsHigh8; // lhuqu1

    //if(rca.v0_RCUpdateMode != RC_MODE_0) {// lhugop
    //  my_v0_rc_init_seq( );
    //}
    // bit allocation for RC_MODE_3
    if(rca.v0_RCUpdateMode == RC_MODE_3) // running this only once !!!
    {
        // calculate allocated bits for each type of frame
        //69 gop_bits = rca.v0_no_frm_base * rca.v0_frame_bs;
        gop_bits = (!rca.v0_intra_period? 1:rca.v0_intra_period)*(rca.v0_bit_rate/rca.v0_framerate);
        //69 denom = 1;
        //69
        //69 if(rca.intra_period>=1)
        //69 {
        //69     denom *= rca.intra_period;
        //69     denom += rca.RCISliceBitRatio - 1;
        //69 }
        denom = (!rca.v0_intra_period? 1:rca.v0_intra_period) + rca.v0_RCISliceBitRatio - 1;

        // set bit targets for each type of frame
//18      rca.RCPSliceBits = (int)floor(gop_bits/denom + 0.5F);
        rca.v0_RCPSliceBits = gop_bits/denom ;
        rca.v0_RCISliceBits = (rca.v0_intra_period)? (rca.v0_RCISliceBitRatio * rca.v0_RCPSliceBits) : 0;

        rca.v0_NISlice = (rca.v0_intra_period)? (rca.v0_intra_period/rca.v0_intra_period):0; // totoal I-frame number
        rca.v0_NPSlice = rca.v0_intra_period - rca.v0_NISlice;
    }

    // check if the last GOP over uses its budget. If yes, the initial QP of the I frame in
    // the coming  GOP will be increased.
    if(rca.v0_RemainingBits<0)
        Overum=TRUE;
    OverBits=-rca.v0_RemainingBits;

    rca.v0_RemainingBits = 0; // set remainingbits as 0 at beginning of gop, lhu, 2017/02/08
    //initialize the lower bound and the upper bound for the target bits of each frame, HRD consideration
    rca.v0_LowerBound  = rca.v0_RemainingBits + (rca.v0_bit_rate/rca.v0_framerate);
    rca.v0_UpperBound1 = rca.v0_RemainingBits + (rca.v0_bit_rate<<1); //2.048
    rca.v0_UpperBound2  = ((OMEGA_4p*rca.v0_UpperBound1) >> 4); // lhu, 2017/03/13

    //compute the total number of bits for the current GOP
    if (rca.v0_IFduration!=1)
        gop_bits = (1+np)*(rca.v0_bit_rate/rca.v0_framerate);
    else {
        if (rca.v0_changeToIFrame==1)
            gop_bits = ((1+np)*(rca.v0_bit_rate/rca.v0_framerate)*14)/10; // expand whole GOP target by 40%, lhu, 2017/03/07
        else if (rca.v0_insertOneIFrame==1)
            gop_bits = (1+np)*(rca.v0_bit_rate/rca.v0_framerate); // maintain the original GOP target, lhu, 2017/03/09
    }
    rca.v0_RemainingBits+= gop_bits;
    rca.v0_Np = np;

    //  OverDuantQp=(int)(8 * OverBits/gop_bits+0.5);
    rca.v0_GOPOverdue=FALSE;

    //Compute InitialQp for each GOP
    rca.v0_TotalPFrame = np;
    if(rca.v0_gop_cnt==0)
    {
        rca.v0_QPLastGOP   = rca.v0_MyInitialQp;
        rca.v0_PAveFrameQP = rca.v0_MyInitialQp;
        rca.v0_PAverageQp  = rca.v0_MyInitialQp;
        rca.v0_m_Qc        = rca.v0_MyInitialQp;
    }
    else
    {
        //compute the average QP of P frames in the previous GOP
        rca.v0_PAverageQp=(rca.v0_TotalQpforPPicture+(np>>1))/np;// + 0.5);
        #ifdef JM_RC_DUMP
        #ifdef USE_MY_RC
        // rc-related debugging info dump, lhulhu
        {
          jm_rc_info_dump = fopen("jm_rc_info_dump.txt","a+");
          fprintf(jm_rc_info_dump, "(init_GOP_s1)PAverageQp:%-d \t", rca.PAverageQp);
          fclose (jm_rc_info_dump);
        }
        #endif
        #endif
        if     (np>=22) GOPDquant=2; // GOPDquant=(int)((1.0*(np+1)/15.0) + 0.5);
        else if(np>=7 ) GOPDquant=1; // if(GOPDquant>2)
        else            GOPDquant=0; // GOPDquant=2;

        rca.v0_PAverageQp -= GOPDquant;

        if(rca.v0_PAverageQp > (rca.v0_QPLastPFrame-2))
            rca.v0_PAverageQp--;

        if(rca.v0_RCUpdateMode == RC_MODE_3) {
            // lhuqu1, Determine the threshold windows for ISliceBits based on RCISliceBitRatio value
            rca.v0_RCISliceTargetBits = gop_bits * rca.v0_RCISliceBitRatio/(rca.v0_RCISliceBitRatio+(rca.v0_intra_period-1));
            v0_RCISliceBitsLow    = rca.v0_RCISliceTargetBits*9/10;
            v0_RCISliceBitsHigh   = rca.v0_RCISliceTargetBits*11/10;
            v0_RCISliceBitsLow2   = rca.v0_RCISliceTargetBits*8/10;
            v0_RCISliceBitsHigh2  = rca.v0_RCISliceTargetBits*12/10;
            v0_RCISliceBitsLow4   = rca.v0_RCISliceTargetBits*6/10;
            v0_RCISliceBitsHigh4  = rca.v0_RCISliceTargetBits*14/10;
            v0_RCISliceBitsLow8   = rca.v0_RCISliceTargetBits*2/10;
            v0_RCISliceBitsHigh8  = rca.v0_RCISliceTargetBits*18/10;
            if(rca.v0_RCISliceActualBits  <= v0_RCISliceBitsLow8)                                                              rca.v0_PAverageQp = rca.v0_QPLastGOP-6;
            else if((v0_RCISliceBitsLow8  < rca.v0_RCISliceActualBits) && (rca.v0_RCISliceActualBits <= v0_RCISliceBitsLow4))  rca.v0_PAverageQp = rca.v0_QPLastGOP-4;
            else if((v0_RCISliceBitsLow4  < rca.v0_RCISliceActualBits) && (rca.v0_RCISliceActualBits <= v0_RCISliceBitsLow2))  rca.v0_PAverageQp = rca.v0_QPLastGOP-2;
            else if((v0_RCISliceBitsLow2  < rca.v0_RCISliceActualBits) && (rca.v0_RCISliceActualBits <= v0_RCISliceBitsLow))   rca.v0_PAverageQp = rca.v0_QPLastGOP-1;
            else if((v0_RCISliceBitsLow   < rca.v0_RCISliceActualBits) && (rca.v0_RCISliceActualBits <= v0_RCISliceBitsHigh))  rca.v0_PAverageQp = rca.v0_QPLastGOP;
            else if((v0_RCISliceBitsHigh  < rca.v0_RCISliceActualBits) && (rca.v0_RCISliceActualBits <= v0_RCISliceBitsHigh2)) rca.v0_PAverageQp = rca.v0_QPLastGOP+1;
            else if((v0_RCISliceBitsHigh2 < rca.v0_RCISliceActualBits) && (rca.v0_RCISliceActualBits <= v0_RCISliceBitsHigh4)) rca.v0_PAverageQp = rca.v0_QPLastGOP+2;
            else if((v0_RCISliceBitsHigh4 < rca.v0_RCISliceActualBits) && (rca.v0_RCISliceActualBits <= v0_RCISliceBitsHigh8)) rca.v0_PAverageQp = rca.v0_QPLastGOP+4;
            else if(rca.v0_RCISliceActualBits > v0_RCISliceBitsHigh8)                                                          rca.v0_PAverageQp = rca.v0_QPLastGOP+6;
        } else {
            // QP is constrained by QP of previous QP
            rca.v0_PAverageQp = my_iClip3(rca.v0_QPLastGOP-2, rca.v0_QPLastGOP+2, rca.v0_PAverageQp);
        }
        #ifdef JM_RC_DUMP
        #ifdef USE_MY_RC
        // rc-related debugging info dump, lhulhu
        {
          jm_rc_info_dump = fopen("jm_rc_info_dump.txt","a+");
          fprintf(jm_rc_info_dump, "(init_GOP_s2)PAverageQp:%-d \n", rca.PAverageQp);
          fclose (jm_rc_info_dump);
        }
        #endif
        #endif
        // Also clipped within range.
        rca.v0_PAverageQp = my_iClip3(rca.v0_RCMinQP,  rca.v0_RCMaxQP,  rca.v0_PAverageQp);

        rca.v0_MyInitialQp = rca.v0_PAverageQp;
        rca.v0_Pm_Qp       = rca.v0_PAverageQp;
        rca.v0_PAveFrameQP = rca.v0_PAverageQp; //(13)
        rca.v0_QPLastGOP   = rca.v0_PAverageQp;
    }

    rca.v0_TotalQpforPPicture=0;//(13)
}


void my_v0_rc_init_pict(int mult)
{
  int i,tmp_T;

    //if ( rca.v0_type==P_SLICE ) //g1|| (rca.RCUpdateMode==RC_MODE_1 &&(rca.gop_cnt!=0 || rca.frame_cnt!=0)) ) // (rca.number !=0)
    if ( rca.v0_type==P_SLICE || ((rca.v0_RCUpdateMode==RC_MODE_1 || (rca.v0_type==I_SLICE&&rca.v0_RCUpdateMode==RC_MODE_3)) && (!(rca.v0_gop_cnt==0 && rca.v0_frame_cnt==0))) ) // lhuitune
    {
      //// for CBR ...
      if(rca.v0_PrevBitRate!=rca.v0_bit_rate)
        rca.v0_RemainingBits += (rca.v0_bit_rate - rca.v0_PrevBitRate)*rca.v0_Np/rca.v0_framerate;
      /*if(rca.v0_re_bitrate == 1)
      {
        rca.v0_re_bitrate = 0;
        rca.v0_RemainingBits += (rca.v0_new_bitrate - rca.v0_bit_rate)*rca.v0_Np/rca.v0_framerate;
        rca.v0_bit_rate = rca.v0_new_bitrate;
      }*/

      // Frame - Level
      if(rca.v0_BasicUnit >= rca.v0_FrameSizeInMbs)
      {
        if(rca.v0_frame_cnt==2) //(rca.NumberofPPicture==1)
        {
          rca.v0_TargetBufferLevel = rca.v0_CurrentBufferFullness;
//18          rca.DeltaP = (rca.CurrentBufferFullness - rca.GOPTargetBufferLevel) / (rca.TotalPFrame-1);
          rca.v0_DeltaP = rca.v0_CurrentBufferFullness/(rca.v0_TotalPFrame-1);
          rca.v0_TargetBufferLevel -= rca.v0_DeltaP;
        }
        else if(rca.v0_frame_cnt>2) //(rca.NumberofPPicture>1)
          rca.v0_TargetBufferLevel -= rca.v0_DeltaP;
      }
      // BU - Level
      else
      {
        if(rca.v0_NumberofCodedPFrame>0)
        {
          for(i=0;i<rca.v0_TotalNumberofBasicUnit;i++)
             rca.v0_BUPFMAD_8p[i] = rca.v0_BUCFMAD_8p[i];
        }

        if(rca.v0_gop_cnt==0) //(rca.NumberofGOP==1)
        {
          if(rca.v0_frame_cnt==2) //(rca.NumberofPPicture==1)
          {
            rca.v0_TargetBufferLevel = rca.v0_CurrentBufferFullness;
//18            rca.DeltaP = (rca.CurrentBufferFullness - rca.GOPTargetBufferLevel)/(rca.TotalPFrame-1);
            rca.v0_DeltaP = rca.v0_CurrentBufferFullness/(rca.v0_TotalPFrame-1);
            rca.v0_TargetBufferLevel -= rca.v0_DeltaP;
          }
          else if(rca.v0_frame_cnt>2) //(rca.NumberofPPicture>1)
            rca.v0_TargetBufferLevel -= rca.v0_DeltaP;
        }
        else if(rca.v0_gop_cnt>0) //(rca.NumberofGOP>1)
        {
          if(rca.v0_frame_cnt==1) //(rca.NumberofPPicture==0)
          {
            rca.v0_TargetBufferLevel = rca.v0_CurrentBufferFullness;
//18            rca.DeltaP = (rca.CurrentBufferFullness - rca.GOPTargetBufferLevel) / rca.TotalPFrame;
            rca.v0_DeltaP = rca.v0_CurrentBufferFullness/rca.v0_TotalPFrame;
            rca.v0_TargetBufferLevel -= rca.v0_DeltaP;
          }
          else if(rca.v0_frame_cnt>1) //(rca.NumberofPPicture>0)
            rca.v0_TargetBufferLevel -= rca.v0_DeltaP;
        }
      }
    }

    // Compute the target bit for each frame
    if(rca.v0_type==P_SLICE || ((rca.v0_gop_cnt!=0 || rca.v0_frame_cnt!=0) && (rca.v0_RCUpdateMode==RC_MODE_1 || rca.v0_RCUpdateMode==RC_MODE_3)))
    {
        // frame layer rate control
        if((rca.v0_BasicUnit>=rca.v0_FrameSizeInMbs || (rca.v0_RCUpdateMode==RC_MODE_3)) && (rca.v0_NumberofCodedPFrame>0))
        {
            if(rca.v0_RCUpdateMode == RC_MODE_3)
            {
                int bitrate = (rca.v0_type==P_SLICE)? rca.v0_RCPSliceBits:rca.v0_RCISliceBits;
                int denom = rca.v0_NISlice*rca.v0_RCISliceBits + rca.v0_NPSlice*rca.v0_RCPSliceBits;

                // target due to remaining bits
                rca.v0_Target = ((long long)bitrate*(long long)rca.v0_RemainingBits) / denom;

                // target given original taget rate and buffer considerations
//18            tmp_T = imax(0, (int)floor((double)bitrate - ((rca.CurrentBufferFullness-rca.TargetBufferLevel)/rca.GAMMAP) + 0.5) );
//s             tmp_T = imax(0, bitrate-((rca.CurrentBufferFullness-rca.TargetBufferLevel)/rca.GAMMAP_1p));
                tmp_T = my_imax(0, (bitrate-((rca.v0_CurrentBufferFullness-rca.v0_TargetBufferLevel)>>1)));

                if(rca.v0_type == I_SLICE) {
                    //rca.v0_Target = rca.v0_Target/(rca.v0_RCIoverPRatio); //lhulhu
                }
            }
            else
            {
//18              rca.Target = (int) floor( rca.RemainingBits / rca.Np + 0.5);
                rca.v0_Target = rca.v0_RemainingBits/rca.v0_Np;
                tmp_T=my_imax(0, ((rca.v0_bit_rate/rca.v0_framerate) - ((rca.v0_CurrentBufferFullness-rca.v0_TargetBufferLevel)>>1)));
//s              rca.Target = ((rca.Target-tmp_T)/rca.BETAP) + tmp_T;
                rca.v0_Target = (rca.v0_Target+tmp_T)>>1;
            }
        }
      // basic unit layer rate control
      else
      {
        if(((rca.v0_gop_cnt==0)&&(rca.v0_NumberofCodedPFrame>0)) || (rca.v0_gop_cnt>0))
        {
//18          rca.Target = (int)(floor(rca.RemainingBits/rca.Np + 0.5));
          rca.v0_Target = rca.v0_RemainingBits/rca.v0_Np;
          tmp_T = my_imax(0, ((rca.v0_bit_rate/rca.v0_framerate) - ((rca.v0_CurrentBufferFullness-rca.v0_TargetBufferLevel)>>1)));

//s          rca.Target = ((rca.Target-tmp_T)*rca.BETAP) + tmp_T;
          rca.v0_Target = ((rca.v0_Target+tmp_T)>>1);
        }
      }
      rca.v0_Target = mult * rca.v0_Target;

      // HRD consideration
      if(rca.v0_RCUpdateMode!=RC_MODE_3 || rca.v0_type==P_SLICE) {
        if (rca.v0_IFduration!=1)
          rca.v0_Target = my_iClip3(rca.v0_LowerBound, rca.v0_UpperBound2, rca.v0_Target);
        else {
          if (rca.v0_changeToIFrame==1)
            rca.v0_Target = (rca.v0_Target*14)/10; // expand P frame target by 40%, lhu, 2017/03/07
        }
      }
    }

    #ifdef JM_RC_DUMP
    #ifdef USE_MY_RC
    // rc-related debugging info dump, lhulhu
    {
      jm_rc_info_dump = fopen("jm_rc_info_dump.txt","a+");
      fprintf(jm_rc_info_dump, "Target(init_pict):%-d \t", rca.Target);
      fclose (jm_rc_info_dump);
    }
    #endif
    #endif
    // frame layer rate control
    rca.v0_NumberofHeaderBits  = 0;
    rca.v0_NumberofTextureBits = 0;
    rca.v0_TotalFrameMAD = 0;// lhumod
    // basic unit layer rate control
    if(rca.v0_BasicUnit < rca.v0_FrameSizeInMbs)
    {
      rca.v0_TotalFrameQP = 0;
      rca.v0_NumberofBasicUnitHeaderBits  = 0;
      rca.v0_NumberofBasicUnitTextureBits = 0;
      rca.v0_TotalMADBasicUnit = 0;
    }
    rca.v0_PrevBitRate = rca.v0_bit_rate; // lhumod
    rca.v0_PrevRCMinQP = rca.v0_RCMinQP; // lhupsnr
}


void my_v0_rc_update_pict(int nbits) // after frame running once
{
  int delta_bits;
/////////////////////////////////////////////////////  my_rc_update_pict_frame( );
  if((rca.v0_RCUpdateMode==RC_MODE_0) || (rca.v0_RCUpdateMode==RC_MODE_2)){
    if(rca.v0_type==P_SLICE)
      my_v0_updatePparams( );
  }
  else if(rca.v0_RCUpdateMode==RC_MODE_1){
    if(rca.v0_type==P_SLICE) //g1   (rca.gop_cnt!=0 || rca.frame_cnt!=0) //( rca.number != 0 )
      my_v0_updatePparams( );
  }
  else if(rca.v0_RCUpdateMode==RC_MODE_3){
    if(rca.v0_type==I_SLICE && (rca.v0_gop_cnt!=0 || rca.v0_frame_cnt!=0)) //(rca.number != 0)
      rca.v0_NISlice--;
    if(rca.v0_type==P_SLICE)
    {
      my_v0_updatePparams( );
      rca.v0_NPSlice--;
    }
  }
/////////////////////////////////////////////////////
  if (rca.v0_RCUpdateMode==RC_MODE_3 && rca.v0_type==I_SLICE) { // lhugop, save bits number for I_SLICE every gop
    rca.v0_RCISliceActualBits = nbits;
  }

  delta_bits=nbits - (rca.v0_bit_rate/rca.v0_framerate);
  // remaining # of bits in GOP
  rca.v0_RemainingBits -= nbits;
  rca.v0_CurrentBufferFullness += delta_bits;

  // update the lower bound and the upper bound for the target bits of each frame, HRD consideration
  rca.v0_LowerBound  -= delta_bits;
  rca.v0_UpperBound1 -= delta_bits;
  rca.v0_UpperBound2  = ((OMEGA_4p*rca.v0_UpperBound1) >> 4);

  // update the parameters of quadratic R-D model
  if(rca.v0_type==P_SLICE || (rca.v0_RCUpdateMode==RC_MODE_1 && (rca.v0_gop_cnt!=0 || rca.v0_frame_cnt!=0)))
  {
    my_v0_updateRCModel( );
    if(rca.v0_RCUpdateMode == RC_MODE_3)
        rca.v0_PreviousWholeFrameMAD_8p = rca.v0_frame_mad; // my_ComputeFrameMAD( ) * (1<<8);
//21      rca.PreviousWholeFrameMAD = my_ComputeFrameMAD( ); ////!!!!
  }
}

void my_v0_updatePparams( )
{
  rca.v0_Np--;
  if(rca.v0_NumberofCodedPFrame<=1000)
    rca.v0_NumberofCodedPFrame++;
}



void my_v0_updateRCModel ( )
{
  int n_windowSize;
  int i,n_realSize;
  int m_Nc = rca.v0_NumberofCodedPFrame;
  Boolean MADModelFlag = FALSE;
//1  static Boolean m_rgRejected[RC_MODEL_HISTORY];
  int error_0p[RC_MODEL_HISTORY];
  unsigned int std_0p=0, threshold_0p;

  if(rca.v0_bu_cnt==0)
    rca.v0_codedbu_cnt = rca.v0_TotalNumberofBasicUnit;
  else
    rca.v0_codedbu_cnt = rca.v0_bu_cnt;

  //if(rca.v0_type==P_SLICE)//g1 || (rca.v0_RCUpdateMode==RC_MODE_1 && (rca.v0_gop_cnt!=0 || rca.v0_frame_cnt!=0)) ) //(rca.v0_number != 0)
  if( rca.v0_type==P_SLICE || ((rca.v0_RCUpdateMode==RC_MODE_1 || (rca.v0_type==I_SLICE&&rca.v0_RCUpdateMode==RC_MODE_3)) && (!(rca.v0_gop_cnt==0&&rca.v0_frame_cnt==0))) ) //lhuitune
  {
    //frame layer rate control
    if(rca.v0_BasicUnit >= rca.v0_FrameSizeInMbs)
    {
        rca.v0_CurrentMAD_8p = rca.v0_frame_mad; //my_ComputeFrameMAD() * (1<<8);
        m_Nc=rca.v0_NumberofCodedPFrame;
    }
    //basic unit layer rate control
    else
    {
        //compute the MAD of the current bu
        rca.v0_CurrentMAD_8p = rca.v0_TotalMADBasicUnit/rca.v0_BasicUnit;
        rca.v0_TotalMADBasicUnit=0;

        // compute the average number of header bits
        rca.v0_PAveHeaderBits1=(rca.v0_PAveHeaderBits1*(rca.v0_codedbu_cnt-1) + rca.v0_NumberofBasicUnitHeaderBits)/rca.v0_codedbu_cnt;
        if(rca.v0_PAveHeaderBits3 == 0)
            rca.v0_PAveHeaderBits2 = rca.v0_PAveHeaderBits1;
        else
        {
            rca.v0_PAveHeaderBits2 = (rca.v0_PAveHeaderBits1*rca.v0_codedbu_cnt +
                rca.v0_PAveHeaderBits3*(rca.v0_TotalNumberofBasicUnit-rca.v0_codedbu_cnt))/rca.v0_TotalNumberofBasicUnit;
        }

//s        *(pp_BUCFMAD_8p+rca.v0_codedbu_cnt-1) = rca.v0_CurrentMAD_8p;
        rca.v0_BUCFMAD_8p[rca.v0_codedbu_cnt-1] = rca.v0_CurrentMAD_8p;

        if(rca.v0_codedbu_cnt >= rca.v0_TotalNumberofBasicUnit)
            m_Nc = rca.v0_NumberofCodedPFrame * rca.v0_TotalNumberofBasicUnit;
        else
            m_Nc = rca.v0_NumberofCodedPFrame * rca.v0_TotalNumberofBasicUnit + rca.v0_codedbu_cnt;
    }

    if(m_Nc > 1)
      MADModelFlag=TRUE;

    rca.v0_PPreHeader = rca.v0_NumberofHeaderBits;

    // hold to over
    rca.v0_rc_hold = 1;

    rca.v0_m_rgQp_8p[0] = QP2Qstep_8p(rca.v0_m_Qc); //*1.0/prc->CurrentMAD;

    if(rca.v0_BasicUnit >= rca.v0_FrameSizeInMbs) {//frame layer rate control
        if(rca.v0_CurrentMAD_8p==0) {// added by lhumad
            rca.v0_cmadequ0 = 1;
            rca.v0_m_rgRp_8p[0] = (long long)rca.v0_NumberofTextureBits<<16;
        }
        else {
            rca.v0_cmadequ0 = 0;
            rca.v0_m_rgRp_8p[0] = ((long long)rca.v0_NumberofTextureBits<<16)/rca.v0_CurrentMAD_8p;
        }
    }
    else {//basic unit layer rate control
        if(rca.v0_CurrentMAD_8p==0) {// added by lhumad
            rca.v0_cmadequ0 = 1;
            rca.v0_m_rgRp_8p[0] = (long long)rca.v0_NumberofBasicUnitTextureBits<<16;
        }
        else {
            rca.v0_cmadequ0 = 0;
            //rca.v0_Pm_rgRp[0] = rca.v0_NumberofBasicUnitTextureBits*1.0/rca.v0_CurrentMAD;
            rca.v0_m_rgRp_8p[0] = ((long long)rca.v0_NumberofBasicUnitTextureBits<<16)/rca.v0_CurrentMAD_8p;
        }
    }

    rca.v0_rc_tmp0[0] = (rca.v0_m_rgQp_8p[0]>>4)*(rca.v0_m_rgRp_8p[0]>>4);
    rca.v0_rc_tmp1[0] = (1<<24)/(rca.v0_m_rgQp_8p[0]>>4);
    rca.v0_rc_tmp4[0] = (rca.v0_m_rgQp_8p[0]>>4)*(rca.v0_m_rgQp_8p[0]>>4);
    rca.v0_rc_tmp2[0] = (1<<28)/rca.v0_rc_tmp4[0];
    rca.v0_m_rgRp_8prr8[0] = rca.v0_m_rgRp_8p[0]>>8;
    rca.v0_rc_tmp3[0] = (rca.v0_m_rgQp_8p[0]>>8)*rca.v0_m_rgRp_8prr8[0];;
    rca.v0_m_X1_8p = rca.v0_Pm_X1_8p;
    rca.v0_m_X2_8p = rca.v0_Pm_X2_8p;

    //compute the size of window
    //n_windowSize = (rca.v0_CurrentMAD>rca.v0_PreviousMAD)? (int)(rca.v0_PreviousMAD/rca.v0_CurrentMAD * (RC_MODEL_HISTORY-1))
    //    :(int)(rca.v0_CurrentMAD/rca.v0_PreviousMAD * (RC_MODEL_HISTORY-1));
    n_windowSize = (rca.v0_CurrentMAD_8p>rca.v0_PreviousMAD_8p)? ((rca.v0_PreviousMAD_8p*20)/rca.v0_CurrentMAD_8p):
        ((rca.v0_CurrentMAD_8p*20)/rca.v0_PreviousMAD_8p);

    n_windowSize=my_iClip3(1, m_Nc, n_windowSize);
    n_windowSize=my_imin(n_windowSize,rca.v0_m_windowSize+1); // m_windowSize:: previous_windowsize
    n_windowSize=my_imin(n_windowSize,20);

    //update the previous window size
    rca.v0_m_windowSize = n_windowSize;
    n_realSize = n_windowSize;

    // initial RD model estimator
    my_v0_RCModelEstimator(n_windowSize, n_windowSize, rca.v0_rc_rgRejected);

    n_windowSize = rca.v0_m_windowSize;
    // remove outlier

    for(i=0; i<n_windowSize; i++)
    {
//a     error_4p[i] = rca.v0_m_X1_8p/rca.v0_m_rgQp_8p[i] + (rca.v0_m_X2_8p)/((rca.v0_m_rgQp_8p[i]>>4)*(rca.v0_m_rgQp_8p[i]>>4)) - (rca.v0_m_rgRp_8p[i]>>8);
        error_0p[i] = rca.v0_m_X1_8p/rca.v0_m_rgQp_8p[i] + (rca.v0_m_X2_8p/rca.v0_rc_tmp4[i]) - rca.v0_m_rgRp_8prr8[i];
        std_0p += error_0p[i]*error_0p[i];
    }

    threshold_0p = (n_windowSize==2)? 0:my_sqrt32(std_0p/n_windowSize);

    for(i=1;i<n_windowSize;i++)
    {
      if(abs(error_0p[i]) > threshold_0p)
      {
        rca.v0_rc_rgRejected[i] = TRUE;
        n_realSize--;
      }
    }
    // always include the last data point
//1    rca.v0_rc_rgRejected[0] = FALSE;

    // second RD model estimator
    my_v0_RCModelEstimator(n_realSize, n_windowSize, rca.v0_rc_rgRejected);

    if( MADModelFlag )
      my_v0_updateMADModel( );
    else if(rca.v0_type==P_SLICE)//g1 || (rca.v0_RCUpdateMode==RC_MODE_1 && (rca.v0_gop_cnt!=0 || rca.v0_frame_cnt!=0)) ) //(rca.v0_number != 0)
      rca.v0_PPictureMAD_8p = rca.v0_CurrentMAD_8p;
  }
}


void my_v0_RCModelEstimator (int n_realSize, int n_windowSize, char *rc_rgRejected)
{
  int i;
  Boolean estimateX2 = FALSE;
  unsigned int  a00_20p=0,a01_20p=0,a11_20p=0,b0_0p=0,b1_0p=0;
  long long  MatrixValue_20p;
  int sum_rc_tmp0=0;

    // default RD model estimation results
    rca.v0_m_X1_8p = 0;
    rca.v0_m_X2_8p = 0;

    for(i=0;i<n_windowSize;i++) // if all non-rejected Q are the same, take 1st order model
    {
        if(!rc_rgRejected[i])
        {
            if((rca.v0_m_rgQp_8p[i]!=rca.v0_m_rgQp_8p[0]))
            {
                estimateX2 = TRUE;
                break;
            }
            sum_rc_tmp0 += rca.v0_rc_tmp0[i]; // ((rca.v0_m_rgQp_8p[i]>>4) * (rca.v0_m_rgRp_8p[i]>>4));
        }
    }
    if(estimateX2==FALSE)
        rca.v0_m_X1_8p = sum_rc_tmp0/n_realSize;


  // take 2nd order model to estimate X1 and X2
  if(estimateX2)
  {
    a00_20p = n_realSize<<20;
    for (i = 0; i < n_windowSize; i++)
    {
      if (!rc_rgRejected[i])
      {
        a01_20p += rca.v0_rc_tmp1[i];
        a11_20p += rca.v0_rc_tmp2[i];
        b0_0p   += rca.v0_rc_tmp3[i];
        b1_0p   += rca.v0_m_rgRp_8prr8[i];
      }
    }
    MatrixValue_20p = (((long long)a00_20p*(long long)a11_20p)-((long long)a01_20p*(long long)a01_20p)+(1<<19))>>20;
    if(MatrixValue_20p > 1)
    {
      rca.v0_m_X1_8p = (((long long)b0_0p*(long long)a11_20p - (long long)b1_0p*(long long)a01_20p)<<8)/MatrixValue_20p;
      rca.v0_m_X2_8p = (((long long)b1_0p*(long long)a00_20p - (long long)b0_0p*(long long)a01_20p)<<8)/MatrixValue_20p;
    }
    else
    {
      rca.v0_m_X1_8p = (b0_0p<<8)/(a00_20p>>20);
      rca.v0_m_X2_8p = 0;
    }
  }

  //if(rca.v0_type==P_SLICE)//g1 || (rca.v0_RCUpdateMode==RC_MODE_1 && (rca.v0_gop_cnt!=0 || rca.v0_frame_cnt!=0))) //(rca.v0_number != 0)
  if( rca.v0_type==P_SLICE || ((rca.v0_RCUpdateMode==RC_MODE_1 || (rca.v0_type==I_SLICE&&rca.v0_RCUpdateMode==RC_MODE_3)) && (!(rca.v0_gop_cnt==0&&rca.v0_frame_cnt==0))) ) //lhuitune
  {
    rca.v0_Pm_X1_8p = rca.v0_m_X1_8p;
    rca.v0_Pm_X2_8p = rca.v0_m_X2_8p;
  }
}


void my_v0_updateMADModel( )
{
  int    n_windowSize;
  int    i, n_realSize;
  int    m_Nc = rca.v0_NumberofCodedPFrame;
  static int error_8p[RC_MODEL_HISTORY];
  long long std_16p=0;
  int threshold_8p;
  int MADPictureC2_12prr4;

  if(rca.v0_NumberofCodedPFrame>0)
  {
    //frame layer rate control
    if(rca.v0_BasicUnit >= rca.v0_FrameSizeInMbs)
      m_Nc = rca.v0_NumberofCodedPFrame;
    else // basic unit layer rate control
      m_Nc=rca.v0_NumberofCodedPFrame*rca.v0_TotalNumberofBasicUnit+rca.v0_codedbu_cnt; //rca.v0_CodedBasicUnit;

    // hold to over
    rca.v0_mad_hold=1;

    rca.v0_PPictureMAD_8p = rca.v0_CurrentMAD_8p;
    rca.v0_PictureMAD_8p[0]  = rca.v0_PPictureMAD_8p;

    if(rca.v0_BasicUnit >= rca.v0_FrameSizeInMbs)
        rca.v0_ReferenceMAD_8p[0]=rca.v0_PictureMAD_8p[1];
    else
        rca.v0_ReferenceMAD_8p[0]=rca.v0_BUPFMAD_8p[rca.v0_codedbu_cnt-1];
//s        rca.v0_ReferenceMAD_8p[0] = *(pp_BUPFMAD_8p+rca.v0_codedbu_cnt-1);

    if(rca.v0_ReferenceMAD_8p[0] == 0)
    {
        rca.v0_mad_tmp0_valid[0] = 0;
        rca.v0_mad_tmp0[0] = 0;
    }
    else
    {
        rca.v0_mad_tmp0_valid[0] = 1;
        rca.v0_mad_tmp0[0] = (rca.v0_PictureMAD_8p[0]<<12)/rca.v0_ReferenceMAD_8p[0];
    }
    rca.v0_mad_tmp1[0] = (rca.v0_ReferenceMAD_8p[0]>>4)*(rca.v0_ReferenceMAD_8p[0]>>4);
    rca.v0_mad_tmp2[0] = (rca.v0_PictureMAD_8p[0]>>4)*(rca.v0_ReferenceMAD_8p[0]>>4);


    rca.v0_MADPictureC1_12p = rca.v0_PMADPictureC1_12p;
    rca.v0_MADPictureC2_12p = rca.v0_PMADPictureC2_12p;

    //compute the size of window
    //n_windowSize = (rca.v0_CurrentMAD>rca.v0_PreviousMAD)? (int)((float)(RC_MODEL_HISTORY-1) * rca.v0_PreviousMAD/rca.v0_CurrentMAD)
    //    :(int)((float)(RC_MODEL_HISTORY-1) * rca.v0_CurrentMAD/rca.v0_PreviousMAD);
    n_windowSize = (rca.v0_CurrentMAD_8p>rca.v0_PreviousMAD_8p)? ((20*rca.v0_PreviousMAD_8p)/rca.v0_CurrentMAD_8p)
        :((20*rca.v0_CurrentMAD_8p)/rca.v0_PreviousMAD_8p);

    n_windowSize = my_iClip3(1, (m_Nc-1), n_windowSize);
    n_windowSize = my_imin(n_windowSize, my_imin(20, rca.v0_MADm_windowSize+1));

    //update the previous window size
    rca.v0_MADm_windowSize=n_windowSize;


    //update the MAD for the previous frame
    //if(rca.v0_type==P_SLICE) {//g1 || (rca.v0_RCUpdateMode==RC_MODE_1 && (rca.v0_gop_cnt!=0 || rca.v0_frame_cnt!=0)))//(rca.v0_number != 0)
    if( rca.v0_type==P_SLICE || ((rca.v0_RCUpdateMode==RC_MODE_1 || (rca.v0_type==I_SLICE&&rca.v0_RCUpdateMode==RC_MODE_3)) && (!(rca.v0_gop_cnt==0&&rca.v0_frame_cnt==0))) ) {//lhuitune
      if (rca.v0_CurrentMAD_8p==0) rca.v0_PreviousMAD_8p=1;// lhumad, make fake for dividing by zero when PreviousMAD equal to 0
      else                         rca.v0_PreviousMAD_8p = rca.v0_CurrentMAD_8p;
    }

    // initial MAD model estimator
    my_v0_MADModelEstimator (n_windowSize, n_windowSize, rca.v0_mad_rgRejected);

    MADPictureC2_12prr4 = rca.v0_MADPictureC2_12p>>4;
    // remove outlier
    for (i = 0; i < n_windowSize; i++)
    {
      //error[i] = rca.v0_MADPictureC1 * rca.v0_ReferenceMAD[i] + rca.v0_MADPictureC2 - rca.v0_PictureMAD[i];
      error_8p[i] = ((rca.v0_MADPictureC1_12p*rca.v0_ReferenceMAD_8p[i])>>12) + MADPictureC2_12prr4 - rca.v0_PictureMAD_8p[i];
      std_16p += error_8p[i]*error_8p[i];
    }

    threshold_8p = (n_windowSize==2)? 0:my_sqrt64(std_16p/n_windowSize);

    n_realSize = n_windowSize;
    for(i=1; i<n_windowSize; i++)
    {
      if(abs(error_8p[i]) > threshold_8p)
      {
        rca.v0_mad_rgRejected[i] = TRUE;
        n_realSize--;
      }
    }

    // second MAD model estimator
    my_v0_MADModelEstimator(n_realSize, n_windowSize, rca.v0_mad_rgRejected);
  }
}


void my_v0_MADModelEstimator(int n_realSize, int n_windowSize, char *mad_rgRejected)
{
  int     i;
  long long MatrixValue_20p; // change 4p to 20p, lhu, 2017/02/23
  Boolean estimateX2=FALSE;
  unsigned int a00_20p=0,a01_20p=0,a11_20p=0,b0_8p=0,b1_8p=0; // change 8p to 20p, lhu, 2017/02/23

    // default MAD model estimation results
    rca.v0_MADPictureC1_12p = 0;
    rca.v0_MADPictureC2_12p = 0;
    rca.v0_c1_over = 0;

    for(i=0;i<n_windowSize;i++) // if all non-rejected MAD are the same, take 1st order model
    {
        if(!mad_rgRejected[i])
        {
            if(rca.v0_PictureMAD_8p[i]!=rca.v0_PictureMAD_8p[0])
            {
                estimateX2 = TRUE;
                    break;
            }
            rca.v0_MADPictureC1_12p += rca.v0_mad_tmp0[i]; // ((rca.v0_PictureMAD_8p[i]<<12) / rca.v0_ReferenceMAD_8p[i]) /n_realSize;
            if(rca.v0_mad_tmp0_valid[i] == 0)
                rca.v0_c1_over = 1;
        }
    }
    if(estimateX2==FALSE)
        rca.v0_MADPictureC1_12p = rca.v0_MADPictureC1_12p/n_realSize;

    // take 2nd order model to estimate X1 and X2
    if(estimateX2)
    {
        a00_20p = n_realSize<<20; // change 8 to 20, lhu, 2017/02/23
        for(i=0;i<n_windowSize;i++)
        {
            if(!mad_rgRejected[i])
            {
                a01_20p += (rca.v0_ReferenceMAD_8p[i]<<12); // change 8p to 20p, lhu, 2017/02/23
                a11_20p += (rca.v0_mad_tmp1[i]<<12); // change 8p to 20p, lhu, 2017/02/23
                b0_8p  += rca.v0_PictureMAD_8p[i];
                b1_8p  += rca.v0_mad_tmp2[i]; // (rca.v0_PictureMAD_8p[i]>>4)*(rca.v0_ReferenceMAD_8p[i]>>4);
            }
        }
        // solve the equation of AX = B
        MatrixValue_20p = ((long long)a00_20p*(long long)a11_20p - (long long)a01_20p*(long long)a01_20p + (1<<19))>>20; // change 4p to 20p, lhu, 2017/02/23

        //if(MatrixValue_4p != 0)  //if(fabs(MatrixValue) > 0.000001)
        if(abs(MatrixValue_20p) > 1)  // change 4p to 20p, lhu, 2017/02/23
        {
            rca.v0_MADPictureC2_12p = (((long long)b0_8p*(long long)a11_20p - (long long)b1_8p*(long long)a01_20p)<<4)/MatrixValue_20p;
            rca.v0_MADPictureC1_12p = (((long long)b1_8p*(long long)a00_20p - (long long)b0_8p*(long long)a01_20p)<<4)/MatrixValue_20p;
        }
        else
        {
            if (a01_20p==0) {// lhumad, make fake for dividing by zero when a01_20p equal to 0
                rca.v0_MADPictureC1_12p = ((long long)b0_8p)<<4;
                rca.v0_cmadequ0 = 1;
            }
            else {
                rca.v0_MADPictureC1_12p = (((long long)b0_8p)<<24)/(long long)a01_20p; // lhu, 2017/02/23
                rca.v0_cmadequ0 = 0;
            }
            rca.v0_MADPictureC2_12p = 0;
        }
        rca.v0_c1_over = 0;
    }
    //if(rca.v0_type==P_SLICE)//g1 || (rca.v0_RCUpdateMode==RC_MODE_1 && (rca.v0_gop_cnt!=0 || rca.v0_frame_cnt!=0)))  //(rca.v0_number != 0)
    if( rca.v0_type==P_SLICE || ((rca.v0_RCUpdateMode==RC_MODE_1 || (rca.v0_type==I_SLICE&&rca.v0_RCUpdateMode==RC_MODE_3)) && (!(rca.v0_gop_cnt==0&&rca.v0_frame_cnt==0))) ) //lhuitune
    {
        rca.v0_PMADPictureC1_12p = rca.v0_MADPictureC1_12p;
        rca.v0_PMADPictureC2_12p = rca.v0_MADPictureC2_12p;
    }
}


void my_v0_hold( )
{
    int i;
    if(rca.v0_rc_hold==1)
    {
        for(i=(RC_MODEL_HISTORY-2); i>0; i--)
        {// update the history
            rca.v0_m_rgQp_8p[i] = rca.v0_m_rgQp_8p[i-1];
            rca.v0_m_rgRp_8p[i] = rca.v0_m_rgRp_8p[i-1];
            rca.v0_rc_tmp0[i] = rca.v0_rc_tmp0[i-1];
            rca.v0_rc_tmp1[i] = rca.v0_rc_tmp1[i-1];
            rca.v0_rc_tmp2[i] = rca.v0_rc_tmp2[i-1];
            rca.v0_rc_tmp3[i] = rca.v0_rc_tmp3[i-1];
            rca.v0_rc_tmp4[i] = rca.v0_rc_tmp4[i-1];
            rca.v0_m_rgRp_8prr8[i] = rca.v0_m_rgRp_8prr8[i-1];
        }
        for(i=0; i<(RC_MODEL_HISTORY-1); i++)
            rca.v0_rc_rgRejected[i] = FALSE;

        rca.v0_rc_hold=0;
    }

    if(rca.v0_mad_hold==1)
    {
        for(i=(RC_MODEL_HISTORY-2);i>0;i--)
        {// update the history
            rca.v0_PictureMAD_8p[i] = rca.v0_PictureMAD_8p[i-1];
            rca.v0_ReferenceMAD_8p[i] = rca.v0_ReferenceMAD_8p[i-1];
            rca.v0_mad_tmp0[i] = rca.v0_mad_tmp0[i-1];
            rca.v0_mad_tmp0_valid[i] = rca.v0_mad_tmp0_valid[i-1];
            rca.v0_mad_tmp1[i] = rca.v0_mad_tmp1[i-1];
            rca.v0_mad_tmp2[i] = rca.v0_mad_tmp2[i-1];
        }
        for(i=0; i<(RC_MODEL_HISTORY-1); i++)
            rca.v0_mad_rgRejected[i] = FALSE;

        rca.v0_mad_hold=0;
    }
}

/*
int my_v0_updateQPRC0( )
{
  int m_Bits;
  int MaxQpChange, m_Qp, m_Hp;

  // frame layer rate control
  if(rca.v0_BasicUnit>=rca.v0_FrameSizeInMbs)
  {
      if (rca.v0_type==I_SLICE)
      {
        rca.v0_m_Qc = rca.v0_MyInitialQp;
        return rca.v0_m_Qc;
      }
      else if(rca.v0_type==P_SLICE && rca.v0_frame_cnt==1) //rca.NumberofPPicture==0
      {
        rca.v0_m_Qc=rca.v0_MyInitialQp;
        my_v0_updateQPNonPicAFF( );
        return rca.v0_m_Qc;
      }
      else
      {
        rca.v0_m_X1_8p = rca.v0_Pm_X1_8p;
        rca.v0_m_X2_8p = rca.v0_Pm_X2_8p;
        rca.v0_MADPictureC1_12p = rca.v0_PMADPictureC1_12p;
        rca.v0_MADPictureC2_12p = rca.v0_PMADPictureC2_12p;
        rca.v0_PreviousPictureMAD_8p = rca.v0_PPictureMAD_8p;

        MaxQpChange = rca.v0_PMaxQpChange;
        m_Qp = rca.v0_Pm_Qp;
        m_Hp = rca.v0_PPreHeader;

        // predict the MAD of current picture
        rca.v0_CurrentMAD_8p = ((rca.v0_MADPictureC1_12p>>8)*(rca.v0_PreviousPictureMAD_8p>>4)) + (rca.v0_MADPictureC2_12p>>4);

        // compute the number of bits for the texture
        if(rca.v0_Target<0)
        {
          rca.v0_m_Qc=m_Qp+MaxQpChange;
          rca.v0_m_Qc = my_iClip3(rca.v0_RCMinQP, rca.v0_RCMaxQP, rca.v0_m_Qc); // Clipping
        }
        else
        {
          m_Bits = rca.v0_Target-m_Hp;
          m_Bits = my_imax(m_Bits, ((rca.v0_bit_rate/rca.v0_framerate)/MINVALUE));

          my_v0_updateModelQPFrame( m_Bits );

          rca.v0_m_Qc = my_iClip3(rca.v0_RCMinQP, rca.v0_RCMaxQP, rca.v0_m_Qc); // clipping
          rca.v0_m_Qc = my_iClip3(m_Qp-MaxQpChange, m_Qp+MaxQpChange, rca.v0_m_Qc); // control variation
        }

        my_v0_updateQPNonPicAFF( );

        return rca.v0_m_Qc;
      }
  }
  // basic unit layer rate control
  else
  {
    if (rca.v0_type == I_SLICE) //top field of I frame
    {
      rca.v0_m_Qc = rca.v0_MyInitialQp;
      return rca.v0_m_Qc;
    }
    else if( rca.v0_type == P_SLICE )
    {
      if(rca.v0_gop_cnt==0 && rca.v0_frame_cnt==1) //((rca.v0_NumberofGOP==1) && (rca.v0_NumberofPPicture==0)) first P_Frame
      {
          return my_v0_updateFirstP( );
      }
      else
      {
        rca.v0_m_X1_8p = rca.v0_Pm_X1_8p;
        rca.v0_m_X2_8p = rca.v0_Pm_X2_8p;
        rca.v0_MADPictureC1_12p = rca.v0_PMADPictureC1_12p;
        rca.v0_MADPictureC2_12p = rca.v0_PMADPictureC2_12p;

        m_Qp=rca.v0_Pm_Qp;

        //the average QP of the previous frame is used to coded the first basic unit of the current frame
        if(rca.v0_bu_cnt==0) //(rca.v0_NumberofBasicUnit==SumofBasicUnit)
          return my_v0_updateFirstBU( );
        else
        {
          //compute the number of remaining bits
          rca.v0_Target -= (rca.v0_NumberofBasicUnitHeaderBits + rca.v0_NumberofBasicUnitTextureBits);
          rca.v0_NumberofBasicUnitHeaderBits  = 0;
          rca.v0_NumberofBasicUnitTextureBits = 0;
          if(rca.v0_Target<0)
            return my_v0_updateNegativeTarget( m_Qp );
          else
          {
            //predict the MAD of current picture
            my_v0_predictCurrPicMAD( );

            //compute the total number of bits for the current basic unit
            my_v0_updateModelQPBU( m_Qp );

            rca.v0_TotalFrameQP +=rca.v0_m_Qc;
            rca.v0_Pm_Qp=rca.v0_m_Qc;
            if(rca.v0_type==P_SLICE && (rca.v0_bu_cnt==(rca.v0_TotalNumberofBasicUnit-1))) //(rca.v0_NumberofBasicUnit == 0 && rca.v0_type == P_SLICE )
              my_v0_updateLastBU( );

            return rca.v0_m_Qc;
          }
        }
      }
    }
  }
  return rca.v0_m_Qc;
}


//////////////////////////////////////////////////////////////////////////////////////
// \brief
//    compute a  quantization parameter for each frame
//////////////////////////////////////////////////////////////////////////////////////
int my_v0_updateQPRC1( )
{
  int m_Bits;
  int SumofBasicUnit;
  int MaxQpChange, m_Qp, m_Hp;

  // frame layer rate control
  if(rca.v0_BasicUnit >= rca.v0_FrameSizeInMbs )
  {
    {
      if(rca.v0_gop_cnt==0 && rca.v0_frame_cnt==0) //(rca.v0_number == 0)
      {
        rca.v0_m_Qc = rca.v0_MyInitialQp;
        return rca.v0_m_Qc;
      }
      else if(rca.v0_frame_cnt==1) //( rca.v0_NumberofPPicture == 0 && (rca.v0_number != 0))
      {
        rca.v0_m_Qc=rca.v0_MyInitialQp;
        my_v0_updateQPNonPicAFF(  );
        return rca.v0_m_Qc;
      }
      else
      {
        rca.v0_m_X1_8p = rca.v0_Pm_X1_8p;
        rca.v0_m_X2_8p = rca.v0_Pm_X2_8p;
        rca.v0_MADPictureC1_12p = rca.v0_PMADPictureC1_12p;
        rca.v0_MADPictureC2_12p = rca.v0_PMADPictureC2_12p;
//22        rca.v0_PreviousPictureMAD = rca.v0_PPictureMAD[0];
//22        rca.v0_PreviousPictureMAD_5p = rca.v0_PPictureMAD[0]*(1<<5);
//<27>        rca.v0_PreviousPictureMAD_8p = rca.v0_PPictureMAD_8p[0];
            rca.v0_PreviousPictureMAD_8p = rca.v0_PPictureMAD_8p;

        MaxQpChange = rca.v0_PMaxQpChange;
        m_Qp = rca.v0_Pm_Qp;
        m_Hp = rca.v0_PPreHeader;

        // predict the MAD of current picture
//20        rca.v0_CurrentMAD=rca.v0_MADPictureC1*rca.v0_PreviousPictureMAD + rca.v0_MADPictureC2;
        rca.v0_CurrentMAD_8p = (rca.v0_MADPictureC1_12p*rca.v0_PreviousPictureMAD_8p)/(1<<12) + rca.v0_MADPictureC2_12p/(1<<4);

        //compute the number of bits for the texture
        if(rca.v0_Target < 0)
        {
          rca.v0_m_Qc=m_Qp+MaxQpChange;
          rca.v0_m_Qc = my_iClip3(rca.v0_RCMinQP, rca.v0_RCMaxQP, rca.v0_m_Qc); // Clipping
        }
        else
        {
          m_Bits = rca.v0_Target-m_Hp;
          m_Bits = my_imax(m_Bits, (int)(rca.v0_bit_rate/(MINVALUE*rca.v0_framerate)));

          my_v0_updateModelQPFrame( m_Bits );

          rca.v0_m_Qc = my_iClip3(rca.v0_RCMinQP, rca.v0_RCMaxQP, rca.v0_m_Qc); // clipping
          rca.v0_m_Qc = my_iClip3(m_Qp-MaxQpChange, m_Qp+MaxQpChange, rca.v0_m_Qc); // control variation
        }

          my_v0_updateQPNonPicAFF( );

        return rca.v0_m_Qc;
      }
    }
  }
  //basic unit layer rate control
  else
  {
    if(rca.v0_gop_cnt==0 && rca.v0_frame_cnt==0) // (rca.v0_number == 0)
    {
      rca.v0_m_Qc = rca.v0_MyInitialQp;
      return rca.v0_m_Qc;
    }
    else
    {
      if(rca.v0_frame_cnt==1) //((rca.v0_NumberofGOP==1)&&(rca.v0_NumberofPPicture==0))  //every gop frist p-frame
      {
          return my_v0_updateFirstP( );
      }
      else
      {
        rca.v0_m_X1_8p = rca.v0_Pm_X1_8p;
        rca.v0_m_X2_8p = rca.v0_Pm_X2_8p;
        rca.v0_MADPictureC1_12p=rca.v0_PMADPictureC1_12p;
        rca.v0_MADPictureC2_12p=rca.v0_PMADPictureC2_12p;

        m_Qp=rca.v0_Pm_Qp;

        SumofBasicUnit=rca.v0_TotalNumberofBasicUnit;

        //the average QP of the previous frame is used to coded the first basic unit of the current frame or field
        if(rca.v0_bu_cnt==0)
          return my_v0_updateFirstBU( );
        else
        {
          //compute the number of remaining bits
          rca.v0_Target -= (rca.v0_NumberofBasicUnitHeaderBits + rca.v0_NumberofBasicUnitTextureBits);
          rca.v0_NumberofBasicUnitHeaderBits  = 0;
          rca.v0_NumberofBasicUnitTextureBits = 0;
          if(rca.v0_Target<0)
            return my_v0_updateNegativeTarget( m_Qp );
          else
          {
            //predict the MAD of current picture
            my_v0_predictCurrPicMAD( );

            //compute the total number of bits for the current basic unit
            my_v0_updateModelQPBU( m_Qp );

            rca.v0_TotalFrameQP += rca.v0_m_Qc;
            rca.v0_Pm_Qp=rca.v0_m_Qc;
            if(rca.v0_bu_cnt==(rca.v0_TotalNumberofBasicUnit-1) && rca.v0_type==P_SLICE)//g1(rca.v0_gop_cnt!=0 || rca.v0_frame_cnt!=0)) //(rca.v0_number != 0))
              my_v0_updateLastBU( );

            return rca.v0_m_Qc;
          }
        }
      }
    }
  }
  return rca.v0_m_Qc;
}*/


//////////////////////////////////////////////////////////////////////////////////////
// \brief
//    compute a  quantization parameter for each frame
//////////////////////////////////////////////////////////////////////////////////////
int my_v0_updateQPRC3( )
{
  int m_Bits;
  int SumofBasicUnit;
  int MaxQpChange, m_Qp, m_Hp;

  /* frame layer rate control */
  //if(rca.v0_BasicUnit == rca.v0_FrameSizeInMbs || rca.v0_type != P_SLICE )
  if( rca.v0_BasicUnit == rca.v0_FrameSizeInMbs ) //lhuitune
  {
      if(rca.v0_gop_cnt==0 && rca.v0_frame_cnt==0) // (rca.v0_number == 0)
      {
        rca.v0_m_Qc = rca.v0_MyInitialQp;
        return rca.v0_m_Qc;
      }
      else if(rca.v0_type==P_SLICE &&  rca.v0_frame_cnt==0) // rca.v0_NumberofPPicture == 0 )
      {
        rca.v0_m_Qc = rca.v0_MyInitialQp;
        my_v0_updateQPNonPicAFF( );
        return rca.v0_m_Qc;
      }
      else
      {
        rca.v0_m_X1_8p = rca.v0_Pm_X1_8p;
        rca.v0_m_X2_8p = rca.v0_Pm_X2_8p;
        rca.v0_MADPictureC1_12p = rca.v0_PMADPictureC1_12p;
        rca.v0_MADPictureC2_12p = rca.v0_PMADPictureC2_12p;
//22        rca.v0_PreviousPictureMAD = rca.v0_PPictureMAD[0];
            rca.v0_PreviousPictureMAD_8p = rca.v0_PPictureMAD_8p;

        MaxQpChange = rca.v0_PMaxQpChange;
        m_Qp = rca.v0_Pm_Qp;
        m_Hp = rca.v0_PPreHeader;

        if (rca.v0_BasicUnit < rca.v0_FrameSizeInMbs && rca.v0_type != P_SLICE )
        {
          // when RC_MODE_3 is set and basic unit is smaller than a frame, note that:
          // the linear MAD model and the quadratic QP model operate on small units and not on a whole frame;
          // we therefore have to account for this
            rca.v0_PreviousPictureMAD_8p = rca.v0_PreviousWholeFrameMAD_8p;
        }
        if (rca.v0_type == I_SLICE )
          m_Hp = 0; // it is usually a very small portion of the total I_SLICE bit budget

        /* predict the MAD of current picture*/
//20        rca.v0_CurrentMAD=rca.v0_MADPictureC1*rca.v0_PreviousPictureMAD + rca.v0_MADPictureC2;
//30        rca.v0_CurrentMAD_8p=(rca.v0_MADPictureC1_12p*rca.v0_PreviousPictureMAD_8p)/(1<<12) + rca.v0_MADPictureC2_12p/(1<<4);
        rca.v0_CurrentMAD_8p=(rca.v0_MADPictureC1_12p>>8)*(rca.v0_PreviousPictureMAD_8p>>4) + (rca.v0_MADPictureC2_12p>>4);

        /*compute the number of bits for the texture*/
        if(rca.v0_Target < 0)
        {
          rca.v0_m_Qc=m_Qp+MaxQpChange;
          rca.v0_m_Qc = my_iClip3(rca.v0_RCMinQP, rca.v0_RCMaxQP, rca.v0_m_Qc); // Clipping
        }
        else
        {
          if (rca.v0_type != P_SLICE )
          {
            if (rca.v0_BasicUnit < rca.v0_FrameSizeInMbs )
              m_Bits =(rca.v0_Target-m_Hp)/rca.v0_TotalNumberofBasicUnit;
            else
              m_Bits =rca.v0_Target-m_Hp;
          }
          else {
            m_Bits = rca.v0_Target-m_Hp;
            m_Bits = my_imax(m_Bits, (int)(rca.v0_bit_rate/(MINVALUE*rca.v0_framerate)));
          }
          my_v0_updateModelQPFrame( m_Bits );
          rca.v0_m_Qc = my_iClip3(rca.v0_RCMinQP, rca.v0_RCMaxQP, rca.v0_m_Qc); // clipping
          if (rca.v0_type == P_SLICE )
            rca.v0_m_Qc = my_iClip3(m_Qp-MaxQpChange, m_Qp+MaxQpChange, rca.v0_m_Qc); // control variation
        }

        if(rca.v0_type == P_SLICE)  // && rca.v0_FieldControl == 0
          my_v0_updateQPNonPicAFF( );

        return rca.v0_m_Qc;
      }
  }
  //// basic unit layer rate control
  else
  {
    if(rca.v0_gop_cnt==0 && rca.v0_frame_cnt==0) // (rca.v0_number == 0)
    {
      rca.v0_m_Qc = rca.v0_MyInitialQp;
      return rca.v0_m_Qc;
    }
    //else if( rca.v0_type == P_SLICE )
    else if( rca.v0_type == P_SLICE || rca.v0_type == I_SLICE ) //lhuitune
    {
      if(rca.v0_gop_cnt==0 && rca.v0_frame_cnt==1) // ((rca.v0_NumberofGOP==1)&&(rca.v0_NumberofPPicture==0)) // gop==0; frameP==0
      {
          return my_v0_updateFirstP(  );
      }
      else
      {
        rca.v0_m_X1_8p = rca.v0_Pm_X1_8p;
        rca.v0_m_X2_8p = rca.v0_Pm_X2_8p;
        rca.v0_MADPictureC1_12p=rca.v0_PMADPictureC1_12p;
        rca.v0_MADPictureC2_12p=rca.v0_PMADPictureC2_12p;

        m_Qp=rca.v0_Pm_Qp;

        SumofBasicUnit=rca.v0_TotalNumberofBasicUnit;

        if(rca.v0_bu_cnt==0) //(rca.v0_NumberofBasicUnit==SumofBasicUnit)
          return my_v0_updateFirstBU( );
        else
        {
          /*compute the number of remaining bits*/
          rca.v0_Target -= (rca.v0_NumberofBasicUnitHeaderBits + rca.v0_NumberofBasicUnitTextureBits);
          rca.v0_NumberofBasicUnitHeaderBits  = 0;
          rca.v0_NumberofBasicUnitTextureBits = 0;
          #ifdef JM_RC_DUMP
          #ifdef USE_MY_RC
          // jm rc-related debugging info dump, lhulhu
          {
            jm_rc_info_dump = fopen("jm_rc_info_dump.txt","a+");
            fprintf(jm_rc_info_dump, "Target(BU):%-d \t", rca.v0_Target);
            fclose (jm_rc_info_dump);
          }
          #endif
          #endif
          if(rca.v0_Target<0)
            return my_v0_updateNegativeTarget( m_Qp );
          else
          {
            /*predict the MAD of current picture*/
            my_v0_predictCurrPicMAD( );

            /*compute the total number of bits for the current basic unit*/
            my_v0_updateModelQPBU( m_Qp );

            rca.v0_TotalFrameQP +=rca.v0_m_Qc;
            rca.v0_Pm_Qp=rca.v0_m_Qc;
            if((rca.v0_bu_cnt==(rca.v0_TotalNumberofBasicUnit-1)) && rca.v0_type==P_SLICE) // lhu, 2017/03/23
            //if((rca.v0_bu_cnt==(rca.v0_TotalNumberofBasicUnit-1)) && (rca.v0_type==P_SLICE || rca.v0_type==I_SLICE) ) //lhuitune
              my_v0_updateLastBU( );

            return rca.v0_m_Qc;
          }
        }
      }
    }
  }
  return rca.v0_m_Qc;
}


void my_v0_updateQPNonPicAFF( )
{
    rca.v0_TotalQpforPPicture +=rca.v0_m_Qc;
    rca.v0_Pm_Qp=rca.v0_m_Qc;
}


int my_v0_updateFirstP( )
{
  //top field of the first P frame
  rca.v0_m_Qc=rca.v0_MyInitialQp;
  rca.v0_NumberofBasicUnitHeaderBits=0;
  rca.v0_NumberofBasicUnitTextureBits=0;
  //bottom field of the first P frame
  if(rca.v0_bu_cnt==(rca.v0_TotalNumberofBasicUnit-1)) //(rca.v0_NumberofBasicUnit==0)
  {
    rca.v0_TotalQpforPPicture +=rca.v0_m_Qc;
    rca.v0_PAveFrameQP=rca.v0_m_Qc;
    rca.v0_PAveHeaderBits3=rca.v0_PAveHeaderBits2;
  }
  rca.v0_Pm_Qp = rca.v0_m_Qc;
  rca.v0_TotalFrameQP += rca.v0_m_Qc;
  return rca.v0_m_Qc;
}


int my_v0_updateNegativeTarget( int m_Qp )
{
  int PAverageQP;

  if(rca.v0_GOPOverdue==TRUE)
    rca.v0_m_Qc=m_Qp+2;
  else
    rca.v0_m_Qc=m_Qp+rca.v0_DDquant;//2

  rca.v0_m_Qc = my_imin(rca.v0_m_Qc, rca.v0_RCMaxQP);  // clipping
  if(rca.v0_basicunit>=rca.v0_MBPerRow) {
    if (rca.v0_wireless_screen!=1) { // added by lhu, 2017/02/27
      if (rca.v0_type == P_SLICE) rca.v0_m_Qc = my_imin(rca.v0_PAveFrameQP+6, rca.v0_m_Qc); // change +6 to +10, lhu, 2017/01/26
      else                        rca.v0_m_Qc = my_imin(rca.v0_PAveFrameQP+5, rca.v0_m_Qc); // lower QP change range for I slice, lhu, 2017/02/07
    } else {
      if (rca.v0_type == P_SLICE) rca.v0_m_Qc = my_imin(rca.v0_PAveFrameQP+3, rca.v0_m_Qc); // change +6 to +3, lhu, 2017/04/25
      else                        rca.v0_m_Qc = my_imin(rca.v0_PAveFrameQP+2, rca.v0_m_Qc); // change +6 to +2 for I slice, lhu, 2017/04/25
    }
  } else
    rca.v0_m_Qc = my_imin(rca.v0_m_Qc, rca.v0_PAveFrameQP+3);

  rca.v0_TotalFrameQP +=rca.v0_m_Qc;
  if(rca.v0_bu_cnt==(rca.v0_TotalNumberofBasicUnit-1)) //(rca.v0_NumberofBasicUnit==0)
  {
//18    PAverageQP=(int)((double)rca.v0_TotalFrameQP/(double)rca.v0_TotalNumberofBasicUnit+0.5);
    PAverageQP=(rca.v0_TotalFrameQP+(rca.v0_TotalNumberofBasicUnit>>1))/rca.v0_TotalNumberofBasicUnit;
    if(rca.v0_frame_cnt==(rca.v0_intra_period-1)) //(rca.v0_NumberofPPicture == (rca.v0_intra_period - 2))
      rca.v0_QPLastPFrame = PAverageQP;
    if (rca.v0_type == P_SLICE) // not increase TotalQpforPPicture for I_SLICE, lhuitune
      rca.v0_TotalQpforPPicture +=PAverageQP;
    rca.v0_PAveFrameQP=PAverageQP;
    rca.v0_PAveHeaderBits3=rca.v0_PAveHeaderBits2;
  }
  if(rca.v0_GOPOverdue==TRUE)
    rca.v0_Pm_Qp=rca.v0_PAveFrameQP;
  else
    rca.v0_Pm_Qp=rca.v0_m_Qc;

  return rca.v0_m_Qc;
}


int my_v0_updateFirstBU( )
{
  if(rca.v0_frame_cnt==1) rca.v0_PAveFrameQP = rca.v0_QPLastPFrame; // first P frame's initial QP value equals to LastPFrame's average QP, lhu, 2017/03/23
  else                    rca.v0_PAveFrameQP = rca.v0_PAveFrameQP;
  if(rca.v0_Target<=0)
  {
    rca.v0_m_Qc = rca.v0_PAveFrameQP + 2;
    if(rca.v0_m_Qc > rca.v0_RCMaxQP)
      rca.v0_m_Qc = rca.v0_RCMaxQP;

    rca.v0_GOPOverdue=TRUE;
  }
  else
  {
    rca.v0_m_Qc=rca.v0_PAveFrameQP;
  }
  rca.v0_TotalFrameQP +=rca.v0_m_Qc;
  rca.v0_Pm_Qp = rca.v0_PAveFrameQP;

  return rca.v0_m_Qc;
}


void my_v0_updateLastBU( )
{
  int PAverageQP;

//18  PAverageQP=(int)((double)rca.v0_TotalFrameQP/(double)rca.v0_TotalNumberofBasicUnit+0.5);
  PAverageQP=(rca.v0_TotalFrameQP+(rca.v0_TotalNumberofBasicUnit>>1))/rca.v0_TotalNumberofBasicUnit;
  if(rca.v0_frame_cnt==(rca.v0_intra_period-1)) // (rca.v0_NumberofPPicture == (rca.v0_intra_period - 2))  last P_FRAME in gop
    rca.v0_QPLastPFrame = PAverageQP;
  if (rca.v0_type == P_SLICE) // not increase TotalQpforPPicture for I_SLICE, lhuitune
    rca.v0_TotalQpforPPicture +=PAverageQP;
  rca.v0_PAveFrameQP=PAverageQP;
  rca.v0_PAveHeaderBits3=rca.v0_PAveHeaderBits2;
}


void my_v0_updateModelQPFrame( int m_Bits )
{
  long long dtmp_8p, qstep_tmp;
  int tmp_4p=0;
  int m_Qstep_8p;

  //dtmp_8p = (rca.v0_CurrentMAD_8p>>6)*(rca.v0_m_X1_8p>>6)*(rca.v0_CurrentMAD_8p>>6)*(rca.v0_m_X1_8p>>6) + \
  //    4*(rca.v0_m_X2_8p>>4)*(rca.v0_CurrentMAD_8p>>4)*m_Bits;
  dtmp_8p = ((long long)rca.v0_CurrentMAD_8p>>6)*((long long)rca.v0_CurrentMAD_8p>>6)*((long long)rca.v0_m_X1_8p>>6)*((long long)rca.v0_m_X1_8p>>6) + \
      4*((long long)rca.v0_m_X2_8p>>4)*((long long)rca.v0_CurrentMAD_8p>>4)*m_Bits;

  if(dtmp_8p>0)
      tmp_4p = my_sqrt64(dtmp_8p);

  if((rca.v0_m_X2_8p==0) || (dtmp_8p<0) || ((tmp_4p-((rca.v0_m_X1_8p>>6)*(rca.v0_CurrentMAD_8p>>6)))<=0))
  {
    //m_Qstep = (float)((rca.v0_m_X1*rca.v0_CurrentMAD) / (double) m_Bits);
    m_Qstep_8p = ((rca.v0_m_X1_8p>>4)*(rca.v0_CurrentMAD_8p>>4)) / m_Bits;
  }
  else // 2nd order mode
  {
    //m_Qstep = (float)((2*rca.v0_m_X2_8p*rca.v0_CurrentMAD_8p)/(sqrt(dtmp)*(1<<16) - rca.v0_m_X1_8p*rca.v0_CurrentMAD_8p));
    qstep_tmp = (2*((long long)rca.v0_m_X2_8p)*((long long)rca.v0_CurrentMAD_8p)) / ((tmp_4p<<4) - (rca.v0_m_X1_8p>>4)*(rca.v0_CurrentMAD_8p>>4));
    m_Qstep_8p = qstep_tmp;
  }

  rca.v0_m_Qc = Qstep2QP_8p(m_Qstep_8p);
}


void my_v0_predictCurrPicMAD( )
{
    int i,CurrentBUMAD_8p,MADPictureC1_12prr4,MADPictureC2_12prr4;

    MADPictureC1_12prr4 = rca.v0_MADPictureC1_12p>>4;
    MADPictureC2_12prr4 = rca.v0_MADPictureC2_12p>>4;

    //rca.v0_CurrentMAD=rca.v0_MADPictureC1*rca.v0_BUPFMAD[rca.v0_bu_cnt]+rca.v0_MADPictureC2;
    rca.v0_CurrentMAD_8p=(MADPictureC1_12prr4*(rca.v0_BUPFMAD_8p[rca.v0_bu_cnt]>>8)) + MADPictureC2_12prr4;
    rca.v0_TotalBUMAD_12p=0;

    for(i=rca.v0_TotalNumberofBasicUnit-1; i>=rca.v0_bu_cnt; i--)
    {
        //CurrentBUMAD = rca.v0_MADPictureC1*rca.v0_BUPFMAD[i]+rca.v0_MADPictureC2;
        CurrentBUMAD_8p = (MADPictureC1_12prr4*(rca.v0_BUPFMAD_8p[i]>>8)) + MADPictureC2_12prr4;
        rca.v0_TotalBUMAD_12p += (CurrentBUMAD_8p*CurrentBUMAD_8p)>>4;
    }
}


void my_v0_updateModelQPBU( int m_Qp )
{
  int m_Bits;
  long long dtmp_8p,qstep_tmp;
  int tmp_4p=0;
  int m_Qstep_8p;

  //compute the total number of bits for the current basic unit
  //m_Bits =(int)(rca.v0_Target * rca.v0_CurrentMAD * rca.v0_CurrentMAD / rca.v0_TotalBUMAD);
  if((rca.v0_TotalBUMAD_12p>>8) == 0)
    m_Bits = rca.v0_Target;
  else
    m_Bits =(rca.v0_Target*(rca.v0_CurrentMAD_8p>>6)*(rca.v0_CurrentMAD_8p>>6)) / (rca.v0_TotalBUMAD_12p>>8);

  //compute the number of texture bits
  m_Bits -=rca.v0_PAveHeaderBits2;

  m_Bits=my_imax(m_Bits,((rca.v0_bit_rate/rca.v0_framerate)/(MINVALUE*rca.v0_TotalNumberofBasicUnit)));

  //dtmp = rca.v0_CurrentMAD*rca.v0_CurrentMAD*rca.v0_m_X1*rca.v0_m_X1 + 4*rca.v0_m_X2*rca.v0_CurrentMAD*m_Bits;
  dtmp_8p = ((long long)rca.v0_CurrentMAD_8p>>6)*((long long)rca.v0_CurrentMAD_8p>>6)*((long long)rca.v0_m_X1_8p>>6)*((long long)rca.v0_m_X1_8p>>6) + \
      4*((long long)rca.v0_m_X2_8p>>4)*((long long)rca.v0_CurrentMAD_8p>>4)*m_Bits;

  if(dtmp_8p>0)
    tmp_4p = my_sqrt64(dtmp_8p);

  //if((rca.v0_m_X2==0) || (dtmp<0) || ((sqrt(dtmp)-(rca.v0_m_X1*rca.v0_CurrentMAD))<=0))  // fall back 1st order mode
  if((rca.v0_m_X2_8p==0) || (dtmp_8p<0) || ((tmp_4p-((rca.v0_m_X1_8p>>6)*(rca.v0_CurrentMAD_8p>>6)))<=0))
  {
    //m_Qstep = (float)((rca.v0_m_X1*rca.v0_CurrentMAD) / (double) m_Bits);
    m_Qstep_8p = ((rca.v0_m_X1_8p>>4)*(rca.v0_CurrentMAD_8p>>4)) / m_Bits;
  }
  else // 2nd order mode
  {
      //m_Qstep = (float)((2*rca.v0_m_X2_8p*rca.v0_CurrentMAD_8p)/(sqrt(dtmp)*(1<<16) - rca.v0_m_X1_8p*rca.v0_CurrentMAD_8p));
      qstep_tmp = (2*((long long)rca.v0_m_X2_8p)*((long long)rca.v0_CurrentMAD_8p)) / ((tmp_4p<<4) - (rca.v0_m_X1_8p>>4)*(rca.v0_CurrentMAD_8p>>4));
      m_Qstep_8p = qstep_tmp;
  }

  rca.v0_m_Qc = Qstep2QP_8p(m_Qstep_8p);
  //use the Qc by R-D model when non-wireless-screen application, lhu, 2017/02/27
  if (rca.v0_wireless_screen==1) // added by lhu, 2017/02/27
    rca.v0_m_Qc = my_imin(m_Qp+rca.v0_DDquant,  rca.v0_m_Qc); // control variation
  
  if(rca.v0_basicunit>=rca.v0_MBPerRow) {
  	if (rca.v0_wireless_screen!=1) { // added by lhu, 2017/02/27
      if (rca.v0_type == P_SLICE) rca.v0_m_Qc = my_imin(rca.v0_PAveFrameQP+6, rca.v0_m_Qc); // change +6 to +10, lhu, 2017/01/24
      else                        rca.v0_m_Qc = my_imin(rca.v0_PAveFrameQP+5, rca.v0_m_Qc); // lower QP change range for I slice, lhu, 2017/02/07
    } else {
      if (rca.v0_type == P_SLICE) rca.v0_m_Qc = my_imin(rca.v0_PAveFrameQP+3, rca.v0_m_Qc); // change +6 to +3, lhu, 2017/04/25
      else                        rca.v0_m_Qc = my_imin(rca.v0_PAveFrameQP+2, rca.v0_m_Qc); // change +6 to +2 for I slice, lhu, 2017/04/25
    }
  } else
    rca.v0_m_Qc = my_imin(rca.v0_PAveFrameQP+3, rca.v0_m_Qc);

  if(rca.v0_c1_over==1)
    //rca.v0_m_Qc = my_imin(m_Qp-rca.v0_DDquant, rca.v0_RCMaxQP); // clipping
    rca.v0_m_Qc = my_imin(m_Qp+rca.v0_DDquant, rca.v0_RCMaxQP-10); // not letting QP decrease when MAD equal 0, 2017/02/21
  else
    rca.v0_m_Qc = my_iClip3(m_Qp-rca.v0_DDquant, rca.v0_RCMaxQP, rca.v0_m_Qc); // clipping

  if(rca.v0_basicunit>=rca.v0_MBPerRow) {
  	if (rca.v0_wireless_screen!=1) { // added by lhu, 2017/04/18
      if (rca.v0_type == P_SLICE) rca.v0_m_Qc = my_imax(rca.v0_PAveFrameQP-6, rca.v0_m_Qc); // lhu, 2017/04/18
      else                        rca.v0_m_Qc = my_imax(rca.v0_PAveFrameQP-5, rca.v0_m_Qc); // lhu, 2017/04/18
    } else {
      if (rca.v0_type == P_SLICE) rca.v0_m_Qc = my_imax(rca.v0_PAveFrameQP-3, rca.v0_m_Qc); // lhu, 2017/04/25
      else                        rca.v0_m_Qc = my_imax(rca.v0_PAveFrameQP-2, rca.v0_m_Qc); // lhu, 2017/04/25
    }
  } else
    rca.v0_m_Qc = my_imax(rca.v0_PAveFrameQP-3, rca.v0_m_Qc);

  rca.v0_m_Qc = my_imax(rca.v0_RCMinQP, rca.v0_m_Qc);
}


#ifdef ARMCM7_RC //#########
void my_v0_rc_update_bu_stats( ) {
    rca.v0_NumberofHeaderBits  = rca.v0_frame_hbits;
    rca.v0_NumberofTextureBits = rca.v0_frame_tbits;
    // basic unit layer rate control
    if(rca.v0_BasicUnit < rca.v0_FrameSizeInMbs) {
        rca.v0_NumberofBasicUnitHeaderBits  = v0_hbits_tmp;  // add slice_header
        rca.v0_NumberofBasicUnitTextureBits = v0_tbits_tmp;
    }
}
void my_v0_rc_update_frame_stats( ) {
    rca.v0_frame_mad   += v0_mad_tmp;
    rca.v0_frame_tbits += v0_tbits_tmp;
    rca.v0_frame_hbits += v0_hbits_tmp;
    rca.v0_frame_abits = rca.v0_frame_tbits+rca.v0_frame_hbits;
    if(rca.v0_bu_cnt==0) { //after calculate frame's status reset related status to zero, lhu, 2017/03/06
        rca.v0_frame_mad   = 0;
        rca.v0_frame_tbits = 0;
        rca.v0_frame_hbits = 0;
        rca.v0_frame_abits = 0;
    }
}
#else //#########
void my_rc_update_bu_stats( ) {
    rca.NumberofHeaderBits  = global_frame_hbits;
    rca.NumberofTextureBits = global_frame_tbits;
    // basic unit layer rate control
    if(rca.BasicUnit < rca.FrameSizeInMbs) {
        rca.NumberofBasicUnitHeaderBits  = global_bu_hbits;  // add slice_header
        rca.NumberofBasicUnitTextureBits = global_bu_tbits;
    }
  #ifdef JM_RC_DUMP
  #ifdef USE_MY_RC
  // jm rc-related debugging info dump, lhulhu
  {
    jm_rc_info_dump=fopen("jm_rc_info_dump.txt","a+");
    fprintf(jm_rc_info_dump, "BUHBits:%-d BUTBits:%-d\n", global_bu_hbits, global_bu_tbits);
    fclose(jm_rc_info_dump);
  }
  #endif
  #endif
}
void my_rc_update_frame_stats( )
{
    if(rca.bu_cnt==0) //frame over
    {
        rca.frame_mad   = 0;
        rca.frame_tbits = 0;
        rca.frame_hbits = 0;
    }
    else
    {
        rca.frame_mad   += global_bu_mad;
        rca.frame_tbits += global_bu_tbits;  //
        rca.frame_hbits += global_bu_hbits;
    }
}
#endif //#########



void my_v0_rc_init_gop_params( )
{
    if(rca.v0_RCUpdateMode==RC_MODE_1)
    {
        //if((rca.gop_cnt==0 && rca.frame_cnt==0) || ((rca.gop_cnt*rca.intra_period)==rca.no_frm_base))
        if(rca.v0_frame_cnt==0 && rca.v0_bu_cnt==0)
            my_v0_rc_init_GOP( rca.v0_no_frm_base - 1 );
    }
    else if((rca.v0_RCUpdateMode==RC_MODE_0)|(rca.v0_RCUpdateMode==RC_MODE_2)|(rca.v0_RCUpdateMode==RC_MODE_3))
    {
        if(rca.v0_frame_cnt==0 && rca.v0_bu_cnt==0) {
            if (rca.v0_IFduration==1 && rca.v0_insertOneIFrame==1) {
                rca.v0_intra_period = rca.v0_PrevIntraPeriod; // use previous intra_period to calculate GOP TargetBits, lhu, 2017/03/13
                rca.v0_RCISliceBitRatio = 3; // fix the Ipratio when decide insert Intra Frame, lhu, 2017/04/26
            }
            my_v0_rc_init_GOP( rca.v0_intra_period - 1 );
        }
    }
}


int my_v0_rc_handle_mb( )
{
    //// first update, update MB_state
    if(rca.v0_gop_cnt!=0 || rca.v0_frame_cnt!=0 || rca.v0_bu_cnt!=0)// || rca.mb_cnt!=0
    {
        my_v0_rc_update_bu_stats( ); //my_rc_update_mb_stats();
#ifdef ARMCM7_RC //#############
        rca.v0_TotalMADBasicUnit = v0_mad_tmp;
        rca.v0_TotalFrameMAD    += v0_mad_tmp;// lhumod
#else //#############
        rca.TotalMADBasicUnit = global_bu_mad;
        rca.TotalFrameMAD    += global_bu_mad;// lhumod
#endif //#############
    }


    if((rca.v0_gop_cnt>0 || rca.v0_frame_cnt>0) && rca.v0_bu_cnt==0) {// && rca.mb_cnt==0))
        rca.v0_frame_mad = rca.v0_TotalFrameMAD/rca.v0_FrameSizeInMbs;// lhumod, calculate the average MB's mad value of previous encoded frame.
#ifdef ARMCM7_RC //#############
        my_v0_rc_update_pict( v0_fbits_tmp );  // should put it to the frame-last
#else //#############
        //my_rc_update_pict( global_frame_abits );  // should put it to the frame-last
        my_rc_update_pict( global_fbits );// lhumod, it actually get buffer related info(remainingbits,currentbufferfullness) after previous frame encoded.
#endif //#############
    }

    //// initial sequence (only once)
    if((rca.v0_gop_cnt==0)&(rca.v0_frame_cnt==0)&(rca.v0_bu_cnt==0)) //&(rca.mb_cnt==0))
    {
        my_v0_rc_params( );
        rca.v0_type = I_SLICE;
        my_v0_rc_init_seq( ); //// initial seq params
        my_v0_rc_init_gop_params( );
        my_v0_rc_init_pict(1);
        rca.v0_qp = my_v0_updateQP( );
        rca.v0_slice_qp = rca.v0_qp;
    }
    else if((rca.v0_frame_cnt==0)&(rca.v0_bu_cnt==0)) //&(rca.mb_cnt==0)) //// initial GOP
    {
        rca.v0_type = I_SLICE;
        my_v0_rc_init_gop_params( );
        my_v0_rc_init_pict(1);
        rca.v0_qp = my_v0_updateQP( );
        rca.v0_slice_qp = rca.v0_qp;
    }
    else if(rca.v0_bu_cnt==0) //&(rca.mb_cnt==0)) //// initial frame
    {
        rca.v0_type = P_SLICE;
        my_v0_rc_init_pict(1);
        rca.v0_qp = my_v0_updateQP( );
        rca.v0_slice_qp = rca.v0_qp;
    }

    // frame layer rate control //// BU-Level
    if (rca.v0_basicunit < rca.v0_FrameSizeInMbs)
    {
        // each I or B frame has only one QP
        //if(rca.v0_type==I_SLICE)//g1 && rca.RCUpdateMode!=RC_MODE_1) || (rca.gop_cnt==0 && rca.frame_cnt==0)) //!(rca.number)
        if(rca.v0_gop_cnt==0 && rca.v0_frame_cnt==0) //lhuitune
        {
            rca.v0_qp = rca.v0_MyInitialQp;
        }
        //else if (rca.v0_type == P_SLICE) //g1 || rca.RCUpdateMode == RC_MODE_1 )
        else if (rca.v0_type == P_SLICE || rca.v0_RCUpdateMode==RC_MODE_1 || (rca.v0_type==I_SLICE && rca.v0_RCUpdateMode==RC_MODE_3)) //lhuitune
        {
            // compute the quantization parameter for each basic unit of P frame
            if(rca.v0_bu_cnt!=0) // && rca.mb_cnt==0)
            {
              my_v0_updateRCModel( );
              rca.v0_qp = my_v0_updateQP( );
            }
        }
    }

    rca.v0_qp = my_iClip3(rca.v0_RCMinQP, rca.v0_RCMaxQP, rca.v0_qp); // -rca.bitdepth_luma_qp_scale

    my_v0_rc_update_frame_stats(); // computer frame parameters
    if( rca.v0_bu_cnt==(rca.v0_TotalNumberofBasicUnit-1) ) {
        if (rca.v0_changeToIFrame==1 || rca.v0_insertOneIFrame==1) {
            if (rca.v0_type==P_SLICE) my_criteria_decide_changeToIFrame((ymseh_tmp>>16)&0xff,(ymseh_tmp>>8)&0xff,(ymseh_tmp>>5)&0x7,0); // lhu, 2017/03/24
            else                      my_decide_backtoNormalGOP(0); // lhu, 2017/03/24
        } else { // lhu, 2017/04/10
            rca.v0_gop_change_NotResetRC=0; rca.v0_IFduration=0;
        }
        rca.v0_frm_ymse[1]  = rca.v0_frm_ymse[0]; // lhu, 2017/03/27
        // renew the QPLastPFrame after intra_period updated, lhu, 2017/03/28
        if ( (rca.v0_type==P_SLICE) && (rca.v0_frame_cnt>=(rca.v0_intra_period-1)) ) {
            rca.v0_QPLastPFrame = (rca.v0_TotalFrameQP+(rca.v0_TotalNumberofBasicUnit>>1))/rca.v0_TotalNumberofBasicUnit;
        }
    }

    if(rca.v0_basicunit < rca.v0_FrameSizeInMbs) // bu-level counter
    {
        if(rca.v0_bu_cnt==(rca.v0_TotalNumberofBasicUnit-1))
        {
            rca.v0_bu_cnt=0;
            if(rca.v0_frame_cnt>=(rca.v0_intra_period-1)) // change "==" to ">=", lhu, 2017/03/09
            {
                rca.v0_frame_cnt=0;
                //if(rca.v0_gop_cnt<=1000)
                rca.v0_gop_cnt++;
            }
            else
                rca.v0_frame_cnt++;
        }
        else
            rca.v0_bu_cnt++;
    }
    else // frame-level counter
    {
        if(rca.v0_frame_cnt==(rca.v0_intra_period-1))
        {
            rca.v0_frame_cnt=0;
            //if(rca.v0_gop_cnt<=1000)
            rca.v0_gop_cnt++;
        }
        else
            rca.v0_frame_cnt++;
    }

#ifndef ARMCM7_RC //#############
    my_hold( );
#endif //#############


    return rca.v0_qp;
}



//==============================================================================
// C model of the second encode video path for rate control
//==============================================================================
//------------------------------------------------------------------------------
// \brief
//    Initialize rate control parameters
//------------------------------------------------------------------------------
#ifdef ARMCM7_RC  //###########
void my_v1_rc_params( ) {
    int i,j,m;

    v1_mad_tmp       = 0; // @lhu, initial value
    v1_hbits_tmp     = 0;
    v1_tbits_tmp     = 0;
    rca.v1_enable    = 0;
    rca.v1_gop_cnt   = 0;
    rca.v1_frame_cnt = 0;
    rca.v1_bu_cnt    = 0;
    rca.v1_mb_cnt    = 0;
    rca.v1_fd_reset  = 0; ////
    rca.v1_aof_inc_qp = 0;
    rca.v1_fd_last_row  = 0;
    rca.v1_fd_row_cnt   = 0;
    rca.v1_fd_last_p    = 0;
    rca.v1_fd_iframe    = 0;
    rca.v1_re_bitrate = 0;
    rca.v1_prev_ac_br_index = 0;
    rca.v1_wireless_screen = 0; // lhu, 2017/02/27
    rca.v1_changeToIFrame = 0; // lhu, 2017/03/09
    rca.v1_insertOneIFrame= 0; // lhu, 2017/03/09
    rca.v1_PrevFrmPSNRLow= 0; // lhu, 2017/03/15
    rca.v1_gop_change_NotResetRC = 0; // lhu, 2017/03/07
    rca.v1_nextPFgotoIF = 0; // lhu, 2017/03/07
    rca.v1_IFduration = 0; // lhu, 2017/03/07
    rca.v1_PrevFbits = 0; // lhu, 2017/04/05
    v1_last_p_gop_change = FALSE; //@lhu, initial value

    READ_WORD(V1_FRAME_XY_ADDR,m); //read frame-x & frame-y
        i=(m>>16)&0xffff;
        j=m&0xffff;
        rca.v1_MBPerRow = (i+15)/16;
        rca.v1_FrameSizeInMbs = rca.v1_MBPerRow*((j+15)/16);
        rca.v1_size = rca.v1_FrameSizeInMbs<<8;
        rca.v1_width = i;
        rca.v1_height = j;

    READ_WORD(V1_GOPFPS_ADDR,i); //read gop and fps
        i=(i>>16)&0xffff;
        rca.v1_intra_period = (i>>8)&0xff;
        rca.v1_framerate = i&0xff;

    READ_WORD(V1_RCEN_BU_ADDR,i); //read rc_en, rc_mode & bu
        rca.v1_rc_enable = (i>>24)&0x1;
        rca.v1_RCUpdateMode = (i>>16)&0x3;
        rca.v1_BasicUnit = i=i&0xffff;
        rca.v1_basicunit = i=i&0xffff;

    READ_WORD(V1_RCSET1_ADDR,i); //read rcset1
        rca.v1_PMaxQpChange = i&0x3f;
        rca.v1_RCMinQP = (i>>8)&0x3f;
        rca.v1_RCMaxQP = (i>>16)&0x3f;

    READ_WORD(V1_BR_ADDR,i); //read br
        rca.v1_bit_rate = i;

    READ_WORD(V1_RCSET2_ADDR,i); //read rcset2
        rca.v1_RCISliceBitRatioMax = (i>>8)&0x3f;
        rca.v1_RCIoverPRatio = (i>>16)&0xf;
        rca.v1_RCISliceBitRatio = (i>>24)&0xf;
}
#else  //###########
void my_rc_params( ) {
  rca.height           = img->height;
  rca.width            = img->width;
  rca.MBPerRow         = (rca.width+15)/16;
  rca.FrameSizeInMbs   = ((rca.height+15)/16)*rca.MBPerRow;
  rca.size             = rca.FrameSizeInMbs*256;
  rca.basicunit        = params->basicunit;
  rca.BasicUnit        = params->basicunit;
  rca.bit_rate         = params->bit_rate;
  rca.RCUpdateMode     = params->RCUpdateMode; ////
  rca.framerate        = params->source.frame_rate;
  rca.PMaxQpChange     = params->RCMaxQPChange;
  rca.RCMinQP          = img->RCMinQP;
  rca.RCMaxQP          = img->RCMaxQP;
  rca.intra_period     = params->intra_period;
  rca.rc_enable = 1;
  rca.RCIoverPRatio    = params->RCIoverPRatio; // 3.8
  rca.type             = I_SLICE;
  rca.RCISliceBitRatio = params->RCISliceBitRatio;
  rca.re_bitrate = 0;
}
#endif //###########

void my_v1_rc_init_seq( )
{
//18  double L1,L2,L3;
  int bpp_p6,qp,i;

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////

  rca.v1_type     = I_SLICE;
  rca.v1_qp       = 0;
  rca.v1_slice_qp = 0;
  rca.v1_c1_over  = 0;
  rca.v1_cmadequ0 = 0; // lhumad
  rca.v1_frame_mad   = 0;
  rca.v1_frame_tbits = 0;
  rca.v1_frame_hbits = 0;

  //if(rca.v1_intra_period==1)
  //  rca.v1_RCUpdateMode = RC_MODE_1;

  if(rca.v1_RCUpdateMode!=RC_MODE_0)
  {
    if (rca.v1_RCUpdateMode==RC_MODE_1 && rca.v1_intra_period==1) {// make sure it execute only once!!! lhumod
      rca.v1_no_frm_base = rca.v1_intra_period*50; //!!!
      rca.v1_intra_period = rca.v1_no_frm_base;// make fake for frame_cnt increment, lhumod
    }
    else if (rca.v1_RCUpdateMode==RC_MODE_3) rca.v1_no_frm_base = rca.v1_intra_period*1; // lhugop
  }

////////////////////////////////////////////////////////
////////////////////////////////////////////////////////
  switch (rca.v1_RCUpdateMode )
  {
     //case RC_MODE_0: my_v1_updateQP = my_v1_updateQPRC0; break;
     //case RC_MODE_1: my_v1_updateQP = my_v1_updateQPRC1; break;
     case RC_MODE_3: my_v1_updateQP = my_v1_updateQPRC3; break;
     default: my_v1_updateQP = my_v1_updateQPRC3; break;
  }

  rca.v1_PreviousMAD_8p = (1<<8);
  rca.v1_CurrentMAD_8p  = (1<<8);
  rca.v1_Target        = 0;
  rca.v1_LowerBound    = 0;
  rca.v1_UpperBound1   = MAX_INT;
  rca.v1_UpperBound2   = MAX_INT;
  rca.v1_PAveFrameQP   = 0;
  rca.v1_m_Qc          = 0;
  rca.v1_PAverageQp    = 0;

  for(i=0;i<70;i++)
  {
    rca.v1_BUPFMAD_8p[i] = 0;
    rca.v1_BUCFMAD_8p[i] = 0;
  }

  for(i=0;i<2;i++) { // set ymse_pframe[i] to max value at begining of sequence, lhu, 2017/03/27
    rca.v1_frm_ymse[i] = rca.v1_height*rca.v1_width*((1<<8)-1)*((1<<8)-1);
  }
  rca.v1_PrevBitRate = rca.v1_bit_rate; //lhumod
  //compute the total number of MBs in a frame
  if(rca.v1_basicunit >= rca.v1_FrameSizeInMbs)
    rca.v1_basicunit = rca.v1_FrameSizeInMbs;

  if(rca.v1_basicunit < rca.v1_FrameSizeInMbs)
    rca.v1_TotalNumberofBasicUnit = rca.v1_FrameSizeInMbs/rca.v1_basicunit;
  else
    rca.v1_TotalNumberofBasicUnit = 1;

  //initialize the parameters of fluid flow traffic model
  rca.v1_CurrentBufferFullness = 0;
//  rca.GOPTargetBufferLevel = 0; //(double)rca.CurrentBufferFullness;

  //initialize the previous window size
  rca.v1_m_windowSize = 0;
  rca.v1_MADm_windowSize = 0;
  rca.v1_NumberofCodedPFrame = 0;
  rca.v1_NumberofGOP = 0;
  //remaining # of bits in GOP
  rca.v1_RemainingBits = 0;

  rca.v1_GAMMAP_1p=1;
  rca.v1_BETAP_1p=1;

  //quadratic rate-distortion model
  rca.v1_PPreHeader=0;

  rca.v1_Pm_X1_8p = rca.v1_bit_rate<<8;
  rca.v1_Pm_X2_8p = 0;
  // linear prediction model for P picture
  rca.v1_PMADPictureC1_12p = (1<<12);
  rca.v1_PMADPictureC2_12p = 0;
  rca.v1_MADPictureC1_12p = (1<<12);
  rca.v1_MADPictureC2_12p = 0;

  // Initialize values
  for(i=0;i<20;i++)
  {
    rca.v1_m_rgQp_8p[i] = 0;
    rca.v1_m_rgRp_8p[i] = 0;
    rca.v1_m_rgRp_8prr8[i] = 0;
    rca.v1_rc_tmp0[i] = 0;
    rca.v1_rc_tmp1[i] = 0;
    rca.v1_rc_tmp2[i] = 0;
    rca.v1_rc_tmp3[i] = 0;
    rca.v1_rc_tmp4[i] = 0;

    rca.v1_PictureMAD_8p[i]   = 0;
    rca.v1_ReferenceMAD_8p[i] = 0;
    rca.v1_mad_tmp0[i] = 0;
    rca.v1_mad_tmp0_valid[i] = 1;
    rca.v1_mad_tmp1[i] = 0;
    rca.v1_mad_tmp2[i] = 0;

    rca.v1_rc_rgRejected[i] = FALSE;
    rca.v1_mad_rgRejected[i] = FALSE;
  }

  rca.v1_rc_hold = 0;
  rca.v1_mad_hold = 0;

  rca.v1_PPictureMAD_8p = 0;
  //basic unit layer rate control
  rca.v1_PAveHeaderBits1 = 0;
  rca.v1_PAveHeaderBits3 = 0;
  rca.v1_DDquant = (rca.v1_TotalNumberofBasicUnit>=9? 1:2);

  rca.v1_frame_bs = rca.v1_bit_rate/rca.v1_framerate;

  bpp_p6=(rca.v1_frame_bs<<6)/rca.v1_size; //for test
/*if     (bpp_p6<=0x26) qp=35;
  else if(bpp_p6<=0x39) qp=25;
  else if(bpp_p6<=0x59) qp=20;
  else                  qp=10;*/// test for more initial_qp assignment, lhuemu
  if     (bpp_p6<=0x6 ) {if (rca.v1_height>=1080) qp=42; else if(rca.v1_height>=720) qp=40; else qp=38;}
  else if(bpp_p6<=0x16) {if (rca.v1_height>=1080) qp=39; else if(rca.v1_height>=720) qp=37; else qp=35;}
  else if(bpp_p6<=0x26) qp=35;
  else if(bpp_p6<=0x39) qp=25;
  else if(bpp_p6<=0x59) qp=20;
  else                  qp=10;

  rca.v1_MyInitialQp=qp;
}


void my_v1_rc_init_GOP(int np)
{
  Boolean Overum=FALSE;
  int OverBits,denom,i;
  int GOPDquant;
  int gop_bits;
  int v1_RCISliceBitsLow,v1_RCISliceBitsHigh,v1_RCISliceBitsLow2,v1_RCISliceBitsHigh2,v1_RCISliceBitsLow4,v1_RCISliceBitsHigh4,v1_RCISliceBitsLow8,v1_RCISliceBitsHigh8; // lhuqu1

    //if(rca.v1_RCUpdateMode != RC_MODE_0) {// lhugop
    //  my_v1_rc_init_seq( );
    //}
    // bit allocation for RC_MODE_3
    if(rca.v1_RCUpdateMode == RC_MODE_3) // running this only once !!!
    {
        // calculate allocated bits for each type of frame
        //69 gop_bits = rca.v1_no_frm_base * rca.v1_frame_bs;
        gop_bits = (!rca.v1_intra_period? 1:rca.v1_intra_period)*(rca.v1_bit_rate/rca.v1_framerate);
        //69 denom = 1;
        //69
        //69 if(rca.intra_period>=1)
        //69 {
        //69     denom *= rca.intra_period;
        //69     denom += rca.RCISliceBitRatio - 1;
        //69 }
        denom = (!rca.v1_intra_period? 1:rca.v1_intra_period) + rca.v1_RCISliceBitRatio - 1;

        // set bit targets for each type of frame
//18      rca.RCPSliceBits = (int)floor(gop_bits/denom + 0.5F);
        rca.v1_RCPSliceBits = gop_bits/denom ;
        rca.v1_RCISliceBits = (rca.v1_intra_period)? (rca.v1_RCISliceBitRatio * rca.v1_RCPSliceBits) : 0;

        rca.v1_NISlice = (rca.v1_intra_period)? (rca.v1_intra_period/rca.v1_intra_period):0; // totoal I-frame number
        rca.v1_NPSlice = rca.v1_intra_period - rca.v1_NISlice;
    }

    // check if the last GOP over uses its budget. If yes, the initial QP of the I frame in
    // the coming  GOP will be increased.
    if(rca.v1_RemainingBits<0)
        Overum=TRUE;
    OverBits=-rca.v1_RemainingBits;

    rca.v1_RemainingBits = 0; // set remainingbits as 0 at beginning of gop, lhu, 2017/02/08
    //initialize the lower bound and the upper bound for the target bits of each frame, HRD consideration
    rca.v1_LowerBound  = rca.v1_RemainingBits + (rca.v1_bit_rate/rca.v1_framerate);
    rca.v1_UpperBound1 = rca.v1_RemainingBits + (rca.v1_bit_rate<<1); //2.048
    rca.v1_UpperBound2  = ((OMEGA_4p*rca.v1_UpperBound1) >> 4); // lhu, 2017/03/13

    //compute the total number of bits for the current GOP
    if (rca.v1_IFduration!=1)
        gop_bits = (1+np)*(rca.v1_bit_rate/rca.v1_framerate);
    else {
        if (rca.v1_changeToIFrame==1)
            gop_bits = ((1+np)*(rca.v1_bit_rate/rca.v1_framerate)*14)/10; // expand whole GOP target by 40%, lhu, 2017/03/07
        else if (rca.v1_insertOneIFrame==1)
            gop_bits = (1+np)*(rca.v1_bit_rate/rca.v1_framerate); // maintain the original GOP target, lhu, 2017/03/09
    }
    rca.v1_RemainingBits+= gop_bits;
    rca.v1_Np = np;

    //  OverDuantQp=(int)(8 * OverBits/gop_bits+0.5);
    rca.v1_GOPOverdue=FALSE;

    //Compute InitialQp for each GOP
    rca.v1_TotalPFrame = np;
    if(rca.v1_gop_cnt==0)
    {
        rca.v1_QPLastGOP   = rca.v1_MyInitialQp;
        rca.v1_PAveFrameQP = rca.v1_MyInitialQp;
        rca.v1_PAverageQp  = rca.v1_MyInitialQp;
        rca.v1_m_Qc        = rca.v1_MyInitialQp;
    }
    else
    {
        //compute the average QP of P frames in the previous GOP
        rca.v1_PAverageQp=(rca.v1_TotalQpforPPicture+(np>>1))/np;// + 0.5);
        #ifdef JM_RC_DUMP
        #ifdef USE_MY_RC
        // rc-related debugging info dump, lhulhu
        {
          jm_rc_info_dump = fopen("jm_rc_info_dump.txt","a+");
          fprintf(jm_rc_info_dump, "(init_GOP_s1)PAverageQp:%-d \t", rca.v1_PAverageQp);
          fclose (jm_rc_info_dump);
        }
        #endif
        #endif
        if     (np>=22) GOPDquant=2; // GOPDquant=(int)((1.0*(np+1)/15.0) + 0.5);
        else if(np>=7 ) GOPDquant=1; // if(GOPDquant>2)
        else            GOPDquant=0; // GOPDquant=2;

        rca.v1_PAverageQp -= GOPDquant;

        if(rca.v1_PAverageQp > (rca.v1_QPLastPFrame-2))
            rca.v1_PAverageQp--;

        if(rca.v1_RCUpdateMode == RC_MODE_3) {
            // lhuqu1, Determine the threshold windows for ISliceBits based on RCISliceBitRatio value
            rca.v1_RCISliceTargetBits = gop_bits * rca.v1_RCISliceBitRatio/(rca.v1_RCISliceBitRatio+(rca.v1_intra_period-1));
            v1_RCISliceBitsLow    = rca.v1_RCISliceTargetBits*9/10;
            v1_RCISliceBitsHigh   = rca.v1_RCISliceTargetBits*11/10;
            v1_RCISliceBitsLow2   = rca.v1_RCISliceTargetBits*8/10;
            v1_RCISliceBitsHigh2  = rca.v1_RCISliceTargetBits*12/10;
            v1_RCISliceBitsLow4   = rca.v1_RCISliceTargetBits*6/10;
            v1_RCISliceBitsHigh4  = rca.v1_RCISliceTargetBits*14/10;
            v1_RCISliceBitsLow8   = rca.v1_RCISliceTargetBits*2/10;
            v1_RCISliceBitsHigh8  = rca.v1_RCISliceTargetBits*18/10;
            if(rca.v1_RCISliceActualBits  <= v1_RCISliceBitsLow8)                                                              rca.v1_PAverageQp = rca.v1_QPLastGOP-6;
            else if((v1_RCISliceBitsLow8  < rca.v1_RCISliceActualBits) && (rca.v1_RCISliceActualBits <= v1_RCISliceBitsLow4))  rca.v1_PAverageQp = rca.v1_QPLastGOP-4;
            else if((v1_RCISliceBitsLow4  < rca.v1_RCISliceActualBits) && (rca.v1_RCISliceActualBits <= v1_RCISliceBitsLow2))  rca.v1_PAverageQp = rca.v1_QPLastGOP-2;
            else if((v1_RCISliceBitsLow2  < rca.v1_RCISliceActualBits) && (rca.v1_RCISliceActualBits <= v1_RCISliceBitsLow))   rca.v1_PAverageQp = rca.v1_QPLastGOP-1;
            else if((v1_RCISliceBitsLow   < rca.v1_RCISliceActualBits) && (rca.v1_RCISliceActualBits <= v1_RCISliceBitsHigh))  rca.v1_PAverageQp = rca.v1_QPLastGOP;
            else if((v1_RCISliceBitsHigh  < rca.v1_RCISliceActualBits) && (rca.v1_RCISliceActualBits <= v1_RCISliceBitsHigh2)) rca.v1_PAverageQp = rca.v1_QPLastGOP+1;
            else if((v1_RCISliceBitsHigh2 < rca.v1_RCISliceActualBits) && (rca.v1_RCISliceActualBits <= v1_RCISliceBitsHigh4)) rca.v1_PAverageQp = rca.v1_QPLastGOP+2;
            else if((v1_RCISliceBitsHigh4 < rca.v1_RCISliceActualBits) && (rca.v1_RCISliceActualBits <= v1_RCISliceBitsHigh8)) rca.v1_PAverageQp = rca.v1_QPLastGOP+4;
            else if(rca.v1_RCISliceActualBits > v1_RCISliceBitsHigh8)                                                          rca.v1_PAverageQp = rca.v1_QPLastGOP+6;
        } else {
            // QP is constrained by QP of previous QP
            rca.v1_PAverageQp = my_iClip3(rca.v1_QPLastGOP-2, rca.v1_QPLastGOP+2, rca.v1_PAverageQp);
        }
        #ifdef JM_RC_DUMP
        #ifdef USE_MY_RC
        // rc-related debugging info dump, lhulhu
        {
          jm_rc_info_dump = fopen("jm_rc_info_dump.txt","a+");
          fprintf(jm_rc_info_dump, "(init_GOP_s2)PAverageQp:%-d \n", rca.v1_PAverageQp);
          fclose (jm_rc_info_dump);
        }
        #endif
        #endif
        // Also clipped within range.
        rca.v1_PAverageQp = my_iClip3(rca.v1_RCMinQP,  rca.v1_RCMaxQP,  rca.v1_PAverageQp);

        rca.v1_MyInitialQp = rca.v1_PAverageQp;
        rca.v1_Pm_Qp       = rca.v1_PAverageQp;
        rca.v1_PAveFrameQP = rca.v1_PAverageQp; //(13)
        rca.v1_QPLastGOP   = rca.v1_PAverageQp;
    }

    rca.v1_TotalQpforPPicture=0;//(13)
}


void my_v1_rc_init_pict(int mult)
{
  int i,tmp_T;

    //if ( rca.v1_type==P_SLICE ) //g1|| (rca.RCUpdateMode==RC_MODE_1 &&(rca.gop_cnt!=0 || rca.frame_cnt!=0)) ) // (rca.number !=0)
    if ( rca.v1_type==P_SLICE || ((rca.v1_RCUpdateMode==RC_MODE_1 || (rca.v1_type==I_SLICE&&rca.v1_RCUpdateMode==RC_MODE_3)) && (!(rca.v1_gop_cnt==0 && rca.v1_frame_cnt==0))) ) // lhuitune
    {
      //// for CBR ...
      if(rca.v1_PrevBitRate!=rca.v1_bit_rate)
        rca.v1_RemainingBits += (rca.v1_bit_rate - rca.v1_PrevBitRate)*rca.v1_Np/rca.v1_framerate;
      /*if(rca.v1_re_bitrate == 1)
      {
        rca.v1_re_bitrate = 0;
        rca.v1_RemainingBits += (rca.v1_new_bitrate - rca.v1_bit_rate)*rca.v1_Np/rca.v1_framerate;
        rca.v1_bit_rate = rca.v1_new_bitrate;
      }*/

      // Frame - Level
      if(rca.v1_BasicUnit >= rca.v1_FrameSizeInMbs)
      {
        if(rca.v1_frame_cnt==2) //(rca.NumberofPPicture==1)
        {
          rca.v1_TargetBufferLevel = rca.v1_CurrentBufferFullness;
//18          rca.DeltaP = (rca.CurrentBufferFullness - rca.GOPTargetBufferLevel) / (rca.TotalPFrame-1);
          rca.v1_DeltaP = rca.v1_CurrentBufferFullness/(rca.v1_TotalPFrame-1);
          rca.v1_TargetBufferLevel -= rca.v1_DeltaP;
        }
        else if(rca.v1_frame_cnt>2) //(rca.NumberofPPicture>1)
          rca.v1_TargetBufferLevel -= rca.v1_DeltaP;
      }
      // BU - Level
      else
      {
        if(rca.v1_NumberofCodedPFrame>0)
        {
          for(i=0;i<rca.v1_TotalNumberofBasicUnit;i++)
             rca.v1_BUPFMAD_8p[i] = rca.v1_BUCFMAD_8p[i];
        }

        if(rca.v1_gop_cnt==0) //(rca.NumberofGOP==1)
        {
          if(rca.v1_frame_cnt==2) //(rca.NumberofPPicture==1)
          {
            rca.v1_TargetBufferLevel = rca.v1_CurrentBufferFullness;
//18            rca.DeltaP = (rca.CurrentBufferFullness - rca.GOPTargetBufferLevel)/(rca.TotalPFrame-1);
            rca.v1_DeltaP = rca.v1_CurrentBufferFullness/(rca.v1_TotalPFrame-1);
            rca.v1_TargetBufferLevel -= rca.v1_DeltaP;
          }
          else if(rca.v1_frame_cnt>2) //(rca.NumberofPPicture>1)
            rca.v1_TargetBufferLevel -= rca.v1_DeltaP;
        }
        else if(rca.v1_gop_cnt>0) //(rca.NumberofGOP>1)
        {
          if(rca.v1_frame_cnt==1) //(rca.NumberofPPicture==0)
          {
            rca.v1_TargetBufferLevel = rca.v1_CurrentBufferFullness;
//18            rca.DeltaP = (rca.CurrentBufferFullness - rca.GOPTargetBufferLevel) / rca.TotalPFrame;
            rca.v1_DeltaP = rca.v1_CurrentBufferFullness/rca.v1_TotalPFrame;
            rca.v1_TargetBufferLevel -= rca.v1_DeltaP;
          }
          else if(rca.v1_frame_cnt>1) //(rca.NumberofPPicture>0)
            rca.v1_TargetBufferLevel -= rca.v1_DeltaP;
        }
      }
    }

    // Compute the target bit for each frame
    if(rca.v1_type==P_SLICE || ((rca.v1_gop_cnt!=0 || rca.v1_frame_cnt!=0) && (rca.v1_RCUpdateMode==RC_MODE_1 || rca.v1_RCUpdateMode==RC_MODE_3))) //(rca.number != 0)
    {
        // frame layer rate control
        if((rca.v1_BasicUnit>=rca.v1_FrameSizeInMbs || (rca.v1_RCUpdateMode==RC_MODE_3)) && (rca.v1_NumberofCodedPFrame>0))
        {
            if(rca.v1_RCUpdateMode == RC_MODE_3)
            {
                int bitrate = (rca.v1_type==P_SLICE)? rca.v1_RCPSliceBits:rca.v1_RCISliceBits;
                int denom = rca.v1_NISlice*rca.v1_RCISliceBits + rca.v1_NPSlice*rca.v1_RCPSliceBits;

                // target due to remaining bits
                rca.v1_Target = ((long long)bitrate*(long long)rca.v1_RemainingBits) / denom;

                // target given original taget rate and buffer considerations
//18            tmp_T = imax(0, (int)floor((double)bitrate - ((rca.CurrentBufferFullness-rca.TargetBufferLevel)/rca.GAMMAP) + 0.5) );
//s             tmp_T = imax(0, bitrate-((rca.CurrentBufferFullness-rca.TargetBufferLevel)/rca.GAMMAP_1p));
                tmp_T = my_imax(0, (bitrate-((rca.v1_CurrentBufferFullness-rca.v1_TargetBufferLevel)>>1)));

                if(rca.v1_type == I_SLICE) {
                    //rca.v1_Target = rca.v1_Target/(rca.v1_RCIoverPRatio);// lhulhu
                }
            }
            else
            {
//18              rca.Target = (int) floor( rca.RemainingBits / rca.Np + 0.5);
                rca.v1_Target = rca.v1_RemainingBits/rca.v1_Np;
                tmp_T=my_imax(0, ((rca.v1_bit_rate/rca.v1_framerate) - ((rca.v1_CurrentBufferFullness-rca.v1_TargetBufferLevel)>>1)));
//s              rca.Target = ((rca.Target-tmp_T)/rca.BETAP) + tmp_T;
                rca.v1_Target = (rca.v1_Target+tmp_T)>>1;
            }
        }
      // basic unit layer rate control
      else
      {
        if(((rca.v1_gop_cnt==0)&&(rca.v1_NumberofCodedPFrame>0)) || (rca.v1_gop_cnt>0))
        {
//18          rca.Target = (int)(floor(rca.RemainingBits/rca.Np + 0.5));

          rca.v1_Target = rca.v1_RemainingBits/rca.v1_Np;
          tmp_T = my_imax(0, ((rca.v1_bit_rate/rca.v1_framerate) - ((rca.v1_CurrentBufferFullness-rca.v1_TargetBufferLevel)>>1)));
//s          rca.Target = ((rca.Target-tmp_T)*rca.BETAP) + tmp_T;
          rca.v1_Target = ((rca.v1_Target+tmp_T)>>1);
        }
      }
      rca.v1_Target = mult * rca.v1_Target;

      // HRD consideration
      if(rca.v1_RCUpdateMode!=RC_MODE_3 || rca.v1_type==P_SLICE) {
        if (rca.v1_IFduration!=1)
          rca.v1_Target = my_iClip3(rca.v1_LowerBound, rca.v1_UpperBound2, rca.v1_Target);
        else {
          if (rca.v1_changeToIFrame==1)
            rca.v1_Target = (rca.v1_Target*14)/10; // expand P frame target by 40%, lhu, 2017/03/07
        }
      }
    }

    #ifdef JM_RC_DUMP
    #ifdef USE_MY_RC
    // rc-related debugging info dump, lhulhu
    {
      jm_rc_info_dump = fopen("jm_rc_info_dump.txt","a+");
      fprintf(jm_rc_info_dump, "Target(init_pict):%-d \t", rca.Target);
      fclose (jm_rc_info_dump);
    }
    #endif
    #endif
    // frame layer rate control
    rca.v1_NumberofHeaderBits  = 0;
    rca.v1_NumberofTextureBits = 0;
    rca.v1_TotalFrameMAD = 0;// lhumod
    // basic unit layer rate control
    if(rca.v1_BasicUnit < rca.v1_FrameSizeInMbs)
    {
      rca.v1_TotalFrameQP = 0;
      rca.v1_NumberofBasicUnitHeaderBits  = 0;
      rca.v1_NumberofBasicUnitTextureBits = 0;
      rca.v1_TotalMADBasicUnit = 0;
    }
    rca.v1_PrevBitRate = rca.v1_bit_rate; //lhumod
    rca.v1_PrevRCMinQP = rca.v1_RCMinQP; // lhupsnr
}


void my_v1_rc_update_pict(int nbits) // after frame running once
{
  int delta_bits;
/////////////////////////////////////////////////////  my_rc_update_pict_frame( );
  if((rca.v1_RCUpdateMode==RC_MODE_0) || (rca.v1_RCUpdateMode==RC_MODE_2)){
    if(rca.v1_type==P_SLICE)
      my_v1_updatePparams( );
  }
  else if(rca.v1_RCUpdateMode==RC_MODE_1){
    if(rca.v1_type==P_SLICE) //g1   (rca.gop_cnt!=0 || rca.frame_cnt!=0) //( rca.number != 0 )
      my_v1_updatePparams( );
  }
  else if(rca.v1_RCUpdateMode==RC_MODE_3){
    if(rca.v1_type==I_SLICE && (rca.v1_gop_cnt!=0 || rca.v1_frame_cnt!=0)) //(rca.number != 0)
      rca.v1_NISlice--;
    if(rca.v1_type==P_SLICE)
    {
      my_v1_updatePparams( );
      rca.v1_NPSlice--;
    }
  }
/////////////////////////////////////////////////////
  if (rca.v1_RCUpdateMode==RC_MODE_3 && rca.v1_type==I_SLICE) { // lhugop, save bits number for I_SLICE every gop
    rca.v1_RCISliceActualBits = nbits;
  }

  delta_bits=nbits - (rca.v1_bit_rate/rca.v1_framerate);
  // remaining # of bits in GOP
  rca.v1_RemainingBits -= nbits;
  rca.v1_CurrentBufferFullness += delta_bits;

  // update the lower bound and the upper bound for the target bits of each frame, HRD consideration
  rca.v1_LowerBound  -= delta_bits;
  rca.v1_UpperBound1 -= delta_bits;
  rca.v1_UpperBound2  = ((OMEGA_4p*rca.v1_UpperBound1) >> 4);

  // update the parameters of quadratic R-D model
  if(rca.v1_type==P_SLICE || (rca.v1_RCUpdateMode==RC_MODE_1 && (rca.v1_gop_cnt!=0 || rca.v1_frame_cnt!=0)))
  {
    my_v1_updateRCModel( );
    if(rca.v1_RCUpdateMode == RC_MODE_3)
        rca.v1_PreviousWholeFrameMAD_8p = rca.v1_frame_mad; // my_ComputeFrameMAD( ) * (1<<8);
//21      rca.PreviousWholeFrameMAD = my_ComputeFrameMAD( ); ////!!!!
  }
}

void my_v1_updatePparams( )
{
  rca.v1_Np--;
  if(rca.v1_NumberofCodedPFrame<=1000)
    rca.v1_NumberofCodedPFrame++;
}



void my_v1_updateRCModel ( )
{
  int n_windowSize;
  int i,n_realSize;
  int m_Nc = rca.v1_NumberofCodedPFrame;
  Boolean MADModelFlag = FALSE;
//1  static Boolean m_rgRejected[RC_MODEL_HISTORY];
  int error_0p[RC_MODEL_HISTORY];
  unsigned int std_0p=0, threshold_0p;

  if(rca.v1_bu_cnt==0)
    rca.v1_codedbu_cnt = rca.v1_TotalNumberofBasicUnit;
  else
    rca.v1_codedbu_cnt = rca.v1_bu_cnt;

  //if(rca.v1_type==P_SLICE)//g1 || (rca.v1_RCUpdateMode==RC_MODE_1 && (rca.v1_gop_cnt!=0 || rca.v1_frame_cnt!=0)) ) //(rca.v1_number != 0)
  if( rca.v1_type==P_SLICE || ((rca.v1_RCUpdateMode==RC_MODE_1 || (rca.v1_type==I_SLICE&&rca.v1_RCUpdateMode==RC_MODE_3)) && (!(rca.v1_gop_cnt==0&&rca.v1_frame_cnt==0))) ) //lhuitune
  {
    //frame layer rate control
    if(rca.v1_BasicUnit >= rca.v1_FrameSizeInMbs)
    {
        rca.v1_CurrentMAD_8p = rca.v1_frame_mad; //my_ComputeFrameMAD() * (1<<8);
        m_Nc=rca.v1_NumberofCodedPFrame;
    }
    //basic unit layer rate control
    else
    {
        //compute the MAD of the current bu
        rca.v1_CurrentMAD_8p = rca.v1_TotalMADBasicUnit/rca.v1_BasicUnit;
        rca.v1_TotalMADBasicUnit=0;

        // compute the average number of header bits
        rca.v1_PAveHeaderBits1=(rca.v1_PAveHeaderBits1*(rca.v1_codedbu_cnt-1) + rca.v1_NumberofBasicUnitHeaderBits)/rca.v1_codedbu_cnt;
        if(rca.v1_PAveHeaderBits3 == 0)
            rca.v1_PAveHeaderBits2 = rca.v1_PAveHeaderBits1;
        else
        {
            rca.v1_PAveHeaderBits2 = (rca.v1_PAveHeaderBits1*rca.v1_codedbu_cnt +
                rca.v1_PAveHeaderBits3*(rca.v1_TotalNumberofBasicUnit-rca.v1_codedbu_cnt))/rca.v1_TotalNumberofBasicUnit;
        }

//s        *(pp_BUCFMAD_8p+rca.v1_codedbu_cnt-1) = rca.v1_CurrentMAD_8p;
        rca.v1_BUCFMAD_8p[rca.v1_codedbu_cnt-1] = rca.v1_CurrentMAD_8p;

        if(rca.v1_codedbu_cnt >= rca.v1_TotalNumberofBasicUnit)
            m_Nc = rca.v1_NumberofCodedPFrame * rca.v1_TotalNumberofBasicUnit;
        else
            m_Nc = rca.v1_NumberofCodedPFrame * rca.v1_TotalNumberofBasicUnit + rca.v1_codedbu_cnt;
    }

    if(m_Nc > 1)
      MADModelFlag=TRUE;

    rca.v1_PPreHeader = rca.v1_NumberofHeaderBits;

    // hold to over
    rca.v1_rc_hold = 1;

    rca.v1_m_rgQp_8p[0] = QP2Qstep_8p(rca.v1_m_Qc); //*1.0/prc->CurrentMAD;

    if(rca.v1_BasicUnit >= rca.v1_FrameSizeInMbs) {//frame layer rate control
        if(rca.v1_CurrentMAD_8p==0) {// added by lhumad
            rca.v1_cmadequ0 = 1;
            rca.v1_m_rgRp_8p[0] = (long long)rca.v1_NumberofTextureBits<<16;
        }
        else {
            rca.v1_cmadequ0 = 0;
            rca.v1_m_rgRp_8p[0] = ((long long)rca.v1_NumberofTextureBits<<16)/rca.v1_CurrentMAD_8p;
        }
    }
    else {//basic unit layer rate control
        if(rca.v1_CurrentMAD_8p==0) {// added by lhumad
            rca.v1_cmadequ0 = 1;
            rca.v1_m_rgRp_8p[0] = (long long)rca.v1_NumberofBasicUnitTextureBits<<16;
        }
        else {
            rca.v1_cmadequ0 = 0;
            //rca.v1_Pm_rgRp[0] = rca.v1_NumberofBasicUnitTextureBits*1.0/rca.v1_CurrentMAD;
            rca.v1_m_rgRp_8p[0] = ((long long)rca.v1_NumberofBasicUnitTextureBits<<16)/rca.v1_CurrentMAD_8p;
        }
    }

    rca.v1_rc_tmp0[0] = (rca.v1_m_rgQp_8p[0]>>4)*(rca.v1_m_rgRp_8p[0]>>4);
    rca.v1_rc_tmp1[0] = (1<<24)/(rca.v1_m_rgQp_8p[0]>>4);
    rca.v1_rc_tmp4[0] = (rca.v1_m_rgQp_8p[0]>>4)*(rca.v1_m_rgQp_8p[0]>>4);
    rca.v1_rc_tmp2[0] = (1<<28)/rca.v1_rc_tmp4[0];
    rca.v1_m_rgRp_8prr8[0] = rca.v1_m_rgRp_8p[0]>>8;
    rca.v1_rc_tmp3[0] = (rca.v1_m_rgQp_8p[0]>>8)*rca.v1_m_rgRp_8prr8[0];;
    rca.v1_m_X1_8p = rca.v1_Pm_X1_8p;
    rca.v1_m_X2_8p = rca.v1_Pm_X2_8p;

    //compute the size of window
    //n_windowSize = (rca.v1_CurrentMAD>rca.v1_PreviousMAD)? (int)(rca.v1_PreviousMAD/rca.v1_CurrentMAD * (RC_MODEL_HISTORY-1))
    //    :(int)(rca.v1_CurrentMAD/rca.v1_PreviousMAD * (RC_MODEL_HISTORY-1));
    n_windowSize = (rca.v1_CurrentMAD_8p>rca.v1_PreviousMAD_8p)? ((rca.v1_PreviousMAD_8p*20)/rca.v1_CurrentMAD_8p):
        ((rca.v1_CurrentMAD_8p*20)/rca.v1_PreviousMAD_8p);

    n_windowSize=my_iClip3(1, m_Nc, n_windowSize);
    n_windowSize=my_imin(n_windowSize,rca.v1_m_windowSize+1); // m_windowSize:: previous_windowsize
    n_windowSize=my_imin(n_windowSize,20);

    //update the previous window size
    rca.v1_m_windowSize = n_windowSize;
    n_realSize = n_windowSize;

    // initial RD model estimator
    my_v1_RCModelEstimator(n_windowSize, n_windowSize, rca.v1_rc_rgRejected);

    n_windowSize = rca.v1_m_windowSize;
    // remove outlier

    for(i=0; i<n_windowSize; i++)
    {
//a     error_4p[i] = rca.v1_m_X1_8p/rca.v1_m_rgQp_8p[i] + (rca.v1_m_X2_8p)/((rca.v1_m_rgQp_8p[i]>>4)*(rca.v1_m_rgQp_8p[i]>>4)) - (rca.v1_m_rgRp_8p[i]>>8);
        error_0p[i] = rca.v1_m_X1_8p/rca.v1_m_rgQp_8p[i] + (rca.v1_m_X2_8p/rca.v1_rc_tmp4[i]) - rca.v1_m_rgRp_8prr8[i];
        std_0p += error_0p[i]*error_0p[i];
    }

    threshold_0p = (n_windowSize==2)? 0:my_sqrt32(std_0p/n_windowSize);

    for(i=1;i<n_windowSize;i++)
    {
      if(abs(error_0p[i]) > threshold_0p)
      {
        rca.v1_rc_rgRejected[i] = TRUE;
        n_realSize--;
      }
    }
    // always include the last data point
//1    rca.v1_rc_rgRejected[0] = FALSE;

    // second RD model estimator
    my_v1_RCModelEstimator(n_realSize, n_windowSize, rca.v1_rc_rgRejected);

    if( MADModelFlag )
      my_v1_updateMADModel( );
    else if(rca.v1_type==P_SLICE)//g1 || (rca.v1_RCUpdateMode==RC_MODE_1 && (rca.v1_gop_cnt!=0 || rca.v1_frame_cnt!=0)) ) //(rca.v1_number != 0)
      rca.v1_PPictureMAD_8p = rca.v1_CurrentMAD_8p;
  }
}


void my_v1_RCModelEstimator (int n_realSize, int n_windowSize, char *rc_rgRejected)
{
  int i;
  Boolean estimateX2 = FALSE;
  unsigned int  a00_20p=0,a01_20p=0,a11_20p=0,b0_0p=0,b1_0p=0;
  long long  MatrixValue_20p;
  int sum_rc_tmp0=0;

    // default RD model estimation results
    rca.v1_m_X1_8p = 0;
    rca.v1_m_X2_8p = 0;

    for(i=0;i<n_windowSize;i++) // if all non-rejected Q are the same, take 1st order model
    {
        if(!rc_rgRejected[i])
        {
            if((rca.v1_m_rgQp_8p[i]!=rca.v1_m_rgQp_8p[0]))
            {
                estimateX2 = TRUE;
                break;
            }
            sum_rc_tmp0 += rca.v1_rc_tmp0[i]; // ((rca.v1_m_rgQp_8p[i]>>4) * (rca.v1_m_rgRp_8p[i]>>4));
        }
    }
    if(estimateX2==FALSE)
        rca.v1_m_X1_8p = sum_rc_tmp0/n_realSize;


  // take 2nd order model to estimate X1 and X2
  if(estimateX2)
  {
    a00_20p = n_realSize<<20;
    for (i = 0; i < n_windowSize; i++)
    {
      if (!rc_rgRejected[i])
      {
        a01_20p += rca.v1_rc_tmp1[i];
        a11_20p += rca.v1_rc_tmp2[i];
        b0_0p   += rca.v1_rc_tmp3[i];
        b1_0p   += rca.v1_m_rgRp_8prr8[i];
      }
    }
    MatrixValue_20p = (((long long)a00_20p*(long long)a11_20p)-((long long)a01_20p*(long long)a01_20p)+(1<<19))>>20;
    if(MatrixValue_20p > 1)
    {
      rca.v1_m_X1_8p = (((long long)b0_0p*(long long)a11_20p - (long long)b1_0p*(long long)a01_20p)<<8)/MatrixValue_20p;
      rca.v1_m_X2_8p = (((long long)b1_0p*(long long)a00_20p - (long long)b0_0p*(long long)a01_20p)<<8)/MatrixValue_20p;
    }
    else
    {
      rca.v1_m_X1_8p = (b0_0p<<8)/(a00_20p>>20);
      rca.v1_m_X2_8p = 0;
    }
  }

  //if(rca.v1_type==P_SLICE)//g1 || (rca.v1_RCUpdateMode==RC_MODE_1 && (rca.v1_gop_cnt!=0 || rca.v1_frame_cnt!=0))) //(rca.v1_number != 0)
  if( rca.v1_type==P_SLICE || ((rca.v1_RCUpdateMode==RC_MODE_1 || (rca.v1_type==I_SLICE&&rca.v1_RCUpdateMode==RC_MODE_3)) && (!(rca.v1_gop_cnt==0&&rca.v1_frame_cnt==0))) ) //lhuitune
  {
    rca.v1_Pm_X1_8p = rca.v1_m_X1_8p;
    rca.v1_Pm_X2_8p = rca.v1_m_X2_8p;
  }
}


void my_v1_updateMADModel( )
{
  int    n_windowSize;
  int    i, n_realSize;
  int    m_Nc = rca.v1_NumberofCodedPFrame;
  static int error_8p[RC_MODEL_HISTORY];
  long long std_16p=0;
  int threshold_8p;
  int MADPictureC2_12prr4;

  if(rca.v1_NumberofCodedPFrame>0)
  {
    //frame layer rate control
    if(rca.v1_BasicUnit >= rca.v1_FrameSizeInMbs)
      m_Nc = rca.v1_NumberofCodedPFrame;
    else // basic unit layer rate control
      m_Nc=rca.v1_NumberofCodedPFrame*rca.v1_TotalNumberofBasicUnit+rca.v1_codedbu_cnt; //rca.v1_CodedBasicUnit;

    // hold to over
    rca.v1_mad_hold=1;

    rca.v1_PPictureMAD_8p = rca.v1_CurrentMAD_8p;
    rca.v1_PictureMAD_8p[0]  = rca.v1_PPictureMAD_8p;

    if(rca.v1_BasicUnit >= rca.v1_FrameSizeInMbs)
        rca.v1_ReferenceMAD_8p[0]=rca.v1_PictureMAD_8p[1];
    else
        rca.v1_ReferenceMAD_8p[0]=rca.v1_BUPFMAD_8p[rca.v1_codedbu_cnt-1];
//s        rca.v1_ReferenceMAD_8p[0] = *(pp_BUPFMAD_8p+rca.v1_codedbu_cnt-1);

    if(rca.v1_ReferenceMAD_8p[0] == 0)
    {
        rca.v1_mad_tmp0_valid[0] = 0;
        rca.v1_mad_tmp0[0] = 0;
    }
    else
    {
        rca.v1_mad_tmp0_valid[0] = 1;
        rca.v1_mad_tmp0[0] = (rca.v1_PictureMAD_8p[0]<<12)/rca.v1_ReferenceMAD_8p[0];
    }
    rca.v1_mad_tmp1[0] = (rca.v1_ReferenceMAD_8p[0]>>4)*(rca.v1_ReferenceMAD_8p[0]>>4);
    rca.v1_mad_tmp2[0] = (rca.v1_PictureMAD_8p[0]>>4)*(rca.v1_ReferenceMAD_8p[0]>>4);


    rca.v1_MADPictureC1_12p = rca.v1_PMADPictureC1_12p;
    rca.v1_MADPictureC2_12p = rca.v1_PMADPictureC2_12p;

    //compute the size of window
    //n_windowSize = (rca.v1_CurrentMAD>rca.v1_PreviousMAD)? (int)((float)(RC_MODEL_HISTORY-1) * rca.v1_PreviousMAD/rca.v1_CurrentMAD)
    //    :(int)((float)(RC_MODEL_HISTORY-1) * rca.v1_CurrentMAD/rca.v1_PreviousMAD);
    n_windowSize = (rca.v1_CurrentMAD_8p>rca.v1_PreviousMAD_8p)? ((20*rca.v1_PreviousMAD_8p)/rca.v1_CurrentMAD_8p)
        :((20*rca.v1_CurrentMAD_8p)/rca.v1_PreviousMAD_8p);

    n_windowSize = my_iClip3(1, (m_Nc-1), n_windowSize);
    n_windowSize = my_imin(n_windowSize, my_imin(20, rca.v1_MADm_windowSize+1));

    //update the previous window size
    rca.v1_MADm_windowSize=n_windowSize;


    //update the MAD for the previous frame
    //if(rca.v1_type==P_SLICE) {//g1 || (rca.v1_RCUpdateMode==RC_MODE_1 && (rca.v1_gop_cnt!=0 || rca.v1_frame_cnt!=0)))//(rca.v1_number != 0)
    if( rca.v1_type==P_SLICE || ((rca.v1_RCUpdateMode==RC_MODE_1 || (rca.v1_type==I_SLICE&&rca.v1_RCUpdateMode==RC_MODE_3)) && (!(rca.v1_gop_cnt==0&&rca.v1_frame_cnt==0))) ) {//lhuitune
      if (rca.v1_CurrentMAD_8p==0) rca.v1_PreviousMAD_8p=1;// lhumad, make fake for dividing by zero when PreviousMAD equal to 0
      else                         rca.v1_PreviousMAD_8p = rca.v1_CurrentMAD_8p;
    }

    // initial MAD model estimator
    my_v1_MADModelEstimator (n_windowSize, n_windowSize, rca.v1_mad_rgRejected);

    MADPictureC2_12prr4 = rca.v1_MADPictureC2_12p>>4;
    // remove outlier
    for (i = 0; i < n_windowSize; i++)
    {
      //error[i] = rca.v1_MADPictureC1 * rca.v1_ReferenceMAD[i] + rca.v1_MADPictureC2 - rca.v1_PictureMAD[i];
      error_8p[i] = ((rca.v1_MADPictureC1_12p*rca.v1_ReferenceMAD_8p[i])>>12) + MADPictureC2_12prr4 - rca.v1_PictureMAD_8p[i];
      std_16p += error_8p[i]*error_8p[i];
    }

    threshold_8p = (n_windowSize==2)? 0:my_sqrt64(std_16p/n_windowSize);

    n_realSize = n_windowSize;
    for(i=1; i<n_windowSize; i++)
    {
      if(abs(error_8p[i]) > threshold_8p)
      {
        rca.v1_mad_rgRejected[i] = TRUE;
        n_realSize--;
      }
    }

    // second MAD model estimator
    my_v1_MADModelEstimator(n_realSize, n_windowSize, rca.v1_mad_rgRejected);
  }
}


void my_v1_MADModelEstimator(int n_realSize, int n_windowSize, char *mad_rgRejected)
{
  int     i;
  long long MatrixValue_20p; // change 4p to 20p, lhu, 2017/02/23
  Boolean estimateX2=FALSE;
  unsigned int a00_20p=0,a01_20p=0,a11_20p=0,b0_8p=0,b1_8p=0; // change 8p to 20p, lhu, 2017/02/23

    // default MAD model estimation results
    rca.v1_MADPictureC1_12p = 0;
    rca.v1_MADPictureC2_12p = 0;
    rca.v1_c1_over = 0;

    for(i=0;i<n_windowSize;i++) // if all non-rejected MAD are the same, take 1st order model
    {
        if(!mad_rgRejected[i])
        {
            if(rca.v1_PictureMAD_8p[i]!=rca.v1_PictureMAD_8p[0])
            {
                estimateX2 = TRUE;
                    break;
            }
            rca.v1_MADPictureC1_12p += rca.v1_mad_tmp0[i]; // ((rca.v1_PictureMAD_8p[i]<<12) / rca.v1_ReferenceMAD_8p[i]) /n_realSize;
            if(rca.v1_mad_tmp0_valid[i] == 0)
                rca.v1_c1_over = 1;
        }
    }
    if(estimateX2==FALSE)
        rca.v1_MADPictureC1_12p = rca.v1_MADPictureC1_12p/n_realSize;

    // take 2nd order model to estimate X1 and X2
    if(estimateX2)
    {
        a00_20p = n_realSize<<20; // change 8 to 20, lhu, 2017/02/23
        for(i=0;i<n_windowSize;i++)
        {
            if(!mad_rgRejected[i])
            {
                a01_20p += (rca.v1_ReferenceMAD_8p[i]<<12); // change 8p to 20p, lhu, 2017/02/23
                a11_20p += (rca.v1_mad_tmp1[i]<<12); // change 8p to 20p, lhu, 2017/02/23
                b0_8p  += rca.v1_PictureMAD_8p[i];
                b1_8p  += rca.v1_mad_tmp2[i]; // (rca.v1_PictureMAD_8p[i]>>4)*(rca.v1_ReferenceMAD_8p[i]>>4);
            }
        }
        // solve the equation of AX = B
        MatrixValue_20p = ((long long)a00_20p*(long long)a11_20p - (long long)a01_20p*(long long)a01_20p + (1<<19))>>20; // change 8p to 20p, lhu, 2017/02/23

        //if(MatrixValue_4p != 0)  //if(fabs(MatrixValue) > 0.000001)
        if(abs(MatrixValue_20p) > 1) // change 4p to 20p, lhu, 2017/02/23
        {
            rca.v1_MADPictureC2_12p = (((long long)b0_8p*(long long)a11_20p - (long long)b1_8p*(long long)a01_20p)<<4)/MatrixValue_20p;
            rca.v1_MADPictureC1_12p = (((long long)b1_8p*(long long)a00_20p - (long long)b0_8p*(long long)a01_20p)<<4)/MatrixValue_20p;
        }
        else
        {
            if (a01_20p==0) {// lhumad, make fake for dividing by zero when a01_20p equal to 0
                rca.v1_MADPictureC1_12p = ((long long)b0_8p)<<4;
                rca.v1_cmadequ0 = 1;
            }
            else {
                rca.v1_MADPictureC1_12p = (((long long)b0_8p)<<24)/(long long)a01_20p; // lhu, 2017/02/23
                rca.v1_cmadequ0 = 0;
            }
            rca.v1_MADPictureC2_12p = 0;
        }
        rca.v1_c1_over = 0;
    }
    //if(rca.v1_type==P_SLICE)//g1 || (rca.v1_RCUpdateMode==RC_MODE_1 && (rca.v1_gop_cnt!=0 || rca.v1_frame_cnt!=0)))  //(rca.v1_number != 0)
    if( rca.v1_type==P_SLICE || ((rca.v1_RCUpdateMode==RC_MODE_1 || (rca.v1_type==I_SLICE&&rca.v1_RCUpdateMode==RC_MODE_3)) && (!(rca.v1_gop_cnt==0&&rca.v1_frame_cnt==0))) ) //lhuitune
    {
        rca.v1_PMADPictureC1_12p = rca.v1_MADPictureC1_12p;
        rca.v1_PMADPictureC2_12p = rca.v1_MADPictureC2_12p;
    }
}


void my_v1_hold( )
{
    int i;
    if(rca.v1_rc_hold==1)
    {
        for(i=(RC_MODEL_HISTORY-2); i>0; i--)
        {// update the history
            rca.v1_m_rgQp_8p[i] = rca.v1_m_rgQp_8p[i-1];
            rca.v1_m_rgRp_8p[i] = rca.v1_m_rgRp_8p[i-1];
            rca.v1_rc_tmp0[i] = rca.v1_rc_tmp0[i-1];
            rca.v1_rc_tmp1[i] = rca.v1_rc_tmp1[i-1];
            rca.v1_rc_tmp2[i] = rca.v1_rc_tmp2[i-1];
            rca.v1_rc_tmp3[i] = rca.v1_rc_tmp3[i-1];
            rca.v1_rc_tmp4[i] = rca.v1_rc_tmp4[i-1];
            rca.v1_m_rgRp_8prr8[i] = rca.v1_m_rgRp_8prr8[i-1];
        }
        for(i=0; i<(RC_MODEL_HISTORY-1); i++)
            rca.v1_rc_rgRejected[i] = FALSE;

        rca.v1_rc_hold=0;
    }

    if(rca.v1_mad_hold==1)
    {
        for(i=(RC_MODEL_HISTORY-2);i>0;i--)
        {// update the history
            rca.v1_PictureMAD_8p[i] = rca.v1_PictureMAD_8p[i-1];
            rca.v1_ReferenceMAD_8p[i] = rca.v1_ReferenceMAD_8p[i-1];
            rca.v1_mad_tmp0[i] = rca.v1_mad_tmp0[i-1];
            rca.v1_mad_tmp0_valid[i] = rca.v1_mad_tmp0_valid[i-1];
            rca.v1_mad_tmp1[i] = rca.v1_mad_tmp1[i-1];
            rca.v1_mad_tmp2[i] = rca.v1_mad_tmp2[i-1];
        }
        for(i=0; i<(RC_MODEL_HISTORY-1); i++)
            rca.v1_mad_rgRejected[i] = FALSE;

        rca.v1_mad_hold=0;
    }
}

/*
int my_v1_updateQPRC0( )
{
  int m_Bits;
  int MaxQpChange, m_Qp, m_Hp;

  // frame layer rate control
  if(rca.v1_BasicUnit>=rca.v1_FrameSizeInMbs)
  {
      if (rca.v1_type==I_SLICE)
      {
        rca.v1_m_Qc = rca.v1_MyInitialQp;
        return rca.v1_m_Qc;
      }
      else if(rca.v1_type==P_SLICE && rca.v1_frame_cnt==1) //rca.NumberofPPicture==0
      {
        rca.v1_m_Qc=rca.v1_MyInitialQp;
        my_v1_updateQPNonPicAFF( );
        return rca.v1_m_Qc;
      }
      else
      {
        rca.v1_m_X1_8p = rca.v1_Pm_X1_8p;
        rca.v1_m_X2_8p = rca.v1_Pm_X2_8p;
        rca.v1_MADPictureC1_12p = rca.v1_PMADPictureC1_12p;
        rca.v1_MADPictureC2_12p = rca.v1_PMADPictureC2_12p;
        rca.v1_PreviousPictureMAD_8p = rca.v1_PPictureMAD_8p;

        MaxQpChange = rca.v1_PMaxQpChange;
        m_Qp = rca.v1_Pm_Qp;
        m_Hp = rca.v1_PPreHeader;

        // predict the MAD of current picture
        rca.v1_CurrentMAD_8p = ((rca.v1_MADPictureC1_12p>>8)*(rca.v1_PreviousPictureMAD_8p>>4)) + (rca.v1_MADPictureC2_12p>>4);

        // compute the number of bits for the texture
        if(rca.v1_Target<0)
        {
          rca.v1_m_Qc=m_Qp+MaxQpChange;
          rca.v1_m_Qc = my_iClip3(rca.v1_RCMinQP, rca.v1_RCMaxQP, rca.v1_m_Qc); // Clipping
        }
        else
        {
          m_Bits = rca.v1_Target-m_Hp;
          m_Bits = my_imax(m_Bits, ((rca.v1_bit_rate/rca.v1_framerate)/MINVALUE));

          my_v1_updateModelQPFrame( m_Bits );

          rca.v1_m_Qc = my_iClip3(rca.v1_RCMinQP, rca.v1_RCMaxQP, rca.v1_m_Qc); // clipping
          rca.v1_m_Qc = my_iClip3(m_Qp-MaxQpChange, m_Qp+MaxQpChange, rca.v1_m_Qc); // control variation
        }

        my_v1_updateQPNonPicAFF( );

        return rca.v1_m_Qc;
      }
  }
  // basic unit layer rate control
  else
  {
    if (rca.v1_type == I_SLICE) //top field of I frame
    {
      rca.v1_m_Qc = rca.v1_MyInitialQp;
      return rca.v1_m_Qc;
    }
    else if( rca.v1_type == P_SLICE )
    {
      if(rca.v1_gop_cnt==0 && rca.v1_frame_cnt==1) //((rca.v1_NumberofGOP==1) && (rca.v1_NumberofPPicture==0)) first P_Frame
      {
          return my_v1_updateFirstP( );
      }
      else
      {
        rca.v1_m_X1_8p = rca.v1_Pm_X1_8p;
        rca.v1_m_X2_8p = rca.v1_Pm_X2_8p;
        rca.v1_MADPictureC1_12p = rca.v1_PMADPictureC1_12p;
        rca.v1_MADPictureC2_12p = rca.v1_PMADPictureC2_12p;

        m_Qp=rca.v1_Pm_Qp;

        //the average QP of the previous frame is used to coded the first basic unit of the current frame
        if(rca.v1_bu_cnt==0) //(rca.v1_NumberofBasicUnit==SumofBasicUnit)
          return my_v1_updateFirstBU( );
        else
        {
          //compute the number of remaining bits
          rca.v1_Target -= (rca.v1_NumberofBasicUnitHeaderBits + rca.v1_NumberofBasicUnitTextureBits);
          rca.v1_NumberofBasicUnitHeaderBits  = 0;
          rca.v1_NumberofBasicUnitTextureBits = 0;
          if(rca.v1_Target<0)
            return my_v1_updateNegativeTarget( m_Qp );
          else
          {
            //predict the MAD of current picture
            my_v1_predictCurrPicMAD( );

            //compute the total number of bits for the current basic unit
            my_v1_updateModelQPBU( m_Qp );

            rca.v1_TotalFrameQP +=rca.v1_m_Qc;
            rca.v1_Pm_Qp=rca.v1_m_Qc;
            if(rca.v1_type==P_SLICE && (rca.v1_bu_cnt==(rca.v1_TotalNumberofBasicUnit-1))) //(rca.v1_NumberofBasicUnit == 0 && rca.v1_type == P_SLICE )
              my_v1_updateLastBU( );

            return rca.v1_m_Qc;
          }
        }
      }
    }
  }
  return rca.v1_m_Qc;
}


//////////////////////////////////////////////////////////////////////////////////////
// \brief
//    compute a  quantization parameter for each frame
//////////////////////////////////////////////////////////////////////////////////////
int my_v1_updateQPRC1( )
{
  int m_Bits;
  int SumofBasicUnit;
  int MaxQpChange, m_Qp, m_Hp;

  // frame layer rate control
  if(rca.v1_BasicUnit >= rca.v1_FrameSizeInMbs )
  {
    {
      if(rca.v1_gop_cnt==0 && rca.v1_frame_cnt==0) //(rca.v1_number == 0)
      {
        rca.v1_m_Qc = rca.v1_MyInitialQp;
        return rca.v1_m_Qc;
      }
      else if(rca.v1_frame_cnt==1) //( rca.v1_NumberofPPicture == 0 && (rca.v1_number != 0))
      {
        rca.v1_m_Qc=rca.v1_MyInitialQp;
        my_v1_updateQPNonPicAFF(  );
        return rca.v1_m_Qc;
      }
      else
      {
        rca.v1_m_X1_8p = rca.v1_Pm_X1_8p;
        rca.v1_m_X2_8p = rca.v1_Pm_X2_8p;
        rca.v1_MADPictureC1_12p = rca.v1_PMADPictureC1_12p;
        rca.v1_MADPictureC2_12p = rca.v1_PMADPictureC2_12p;
//22        rca.v1_PreviousPictureMAD = rca.v1_PPictureMAD[0];
//22        rca.v1_PreviousPictureMAD_5p = rca.v1_PPictureMAD[0]*(1<<5);
//<27>        rca.v1_PreviousPictureMAD_8p = rca.v1_PPictureMAD_8p[0];
            rca.v1_PreviousPictureMAD_8p = rca.v1_PPictureMAD_8p;

        MaxQpChange = rca.v1_PMaxQpChange;
        m_Qp = rca.v1_Pm_Qp;
        m_Hp = rca.v1_PPreHeader;

        // predict the MAD of current picture
//20        rca.v1_CurrentMAD=rca.v1_MADPictureC1*rca.v1_PreviousPictureMAD + rca.v1_MADPictureC2;
        rca.v1_CurrentMAD_8p = (rca.v1_MADPictureC1_12p*rca.v1_PreviousPictureMAD_8p)/(1<<12) + rca.v1_MADPictureC2_12p/(1<<4);

        //compute the number of bits for the texture
        if(rca.v1_Target < 0)
        {
          rca.v1_m_Qc=m_Qp+MaxQpChange;
          rca.v1_m_Qc = my_iClip3(rca.v1_RCMinQP, rca.v1_RCMaxQP, rca.v1_m_Qc); // Clipping
        }
        else
        {
          m_Bits = rca.v1_Target-m_Hp;
          m_Bits = my_imax(m_Bits, (int)(rca.v1_bit_rate/(MINVALUE*rca.v1_framerate)));

          my_v1_updateModelQPFrame( m_Bits );

          rca.v1_m_Qc = my_iClip3(rca.v1_RCMinQP, rca.v1_RCMaxQP, rca.v1_m_Qc); // clipping
          rca.v1_m_Qc = my_iClip3(m_Qp-MaxQpChange, m_Qp+MaxQpChange, rca.v1_m_Qc); // control variation
        }

          my_v1_updateQPNonPicAFF( );

        return rca.v1_m_Qc;
      }
    }
  }
  //basic unit layer rate control
  else
  {
    if(rca.v1_gop_cnt==0 && rca.v1_frame_cnt==0) // (rca.v1_number == 0)
    {
      rca.v1_m_Qc = rca.v1_MyInitialQp;
      return rca.v1_m_Qc;
    }
    else
    {
      if(rca.v1_frame_cnt==1) //((rca.v1_NumberofGOP==1)&&(rca.v1_NumberofPPicture==0))  //every gop frist p-frame
      {
          return my_v1_updateFirstP( );
      }
      else
      {
        rca.v1_m_X1_8p = rca.v1_Pm_X1_8p;
        rca.v1_m_X2_8p = rca.v1_Pm_X2_8p;
        rca.v1_MADPictureC1_12p=rca.v1_PMADPictureC1_12p;
        rca.v1_MADPictureC2_12p=rca.v1_PMADPictureC2_12p;

        m_Qp=rca.v1_Pm_Qp;

        SumofBasicUnit=rca.v1_TotalNumberofBasicUnit;

        //the average QP of the previous frame is used to coded the first basic unit of the current frame or field
        if(rca.v1_bu_cnt==0)
          return my_v1_updateFirstBU( );
        else
        {
          //compute the number of remaining bits
          rca.v1_Target -= (rca.v1_NumberofBasicUnitHeaderBits + rca.v1_NumberofBasicUnitTextureBits);
          rca.v1_NumberofBasicUnitHeaderBits  = 0;
          rca.v1_NumberofBasicUnitTextureBits = 0;
          if(rca.v1_Target<0)
            return my_v1_updateNegativeTarget( m_Qp );
          else
          {
            //predict the MAD of current picture
            my_v1_predictCurrPicMAD( );

            //compute the total number of bits for the current basic unit
            my_v1_updateModelQPBU( m_Qp );

            rca.v1_TotalFrameQP += rca.v1_m_Qc;
            rca.v1_Pm_Qp=rca.v1_m_Qc;
            if(rca.v1_bu_cnt==(rca.v1_TotalNumberofBasicUnit-1) && rca.v1_type==P_SLICE)//g1(rca.v1_gop_cnt!=0 || rca.v1_frame_cnt!=0)) //(rca.v1_number != 0))
              my_v1_updateLastBU( );

            return rca.v1_m_Qc;
          }
        }
      }
    }
  }
  return rca.v1_m_Qc;
}*/


//////////////////////////////////////////////////////////////////////////////////////
// \brief
//    compute a  quantization parameter for each frame
//////////////////////////////////////////////////////////////////////////////////////
int my_v1_updateQPRC3( )
{
  int m_Bits;
  int SumofBasicUnit;
  int MaxQpChange, m_Qp, m_Hp;

  /* frame layer rate control */
  //if(rca.v1_BasicUnit == rca.v1_FrameSizeInMbs || rca.v1_type != P_SLICE )
  if( rca.v1_BasicUnit == rca.v1_FrameSizeInMbs ) //lhuitune
  {
      if(rca.v1_gop_cnt==0 && rca.v1_frame_cnt==0) // (rca.v1_number == 0)
      {
        rca.v1_m_Qc = rca.v1_MyInitialQp;
        return rca.v1_m_Qc;
      }
      else if(rca.v1_type==P_SLICE &&  rca.v1_frame_cnt==0) // rca.v1_NumberofPPicture == 0 )
      {
        rca.v1_m_Qc = rca.v1_MyInitialQp;
        my_v1_updateQPNonPicAFF( );
        return rca.v1_m_Qc;
      }
      else
      {
        rca.v1_m_X1_8p = rca.v1_Pm_X1_8p;
        rca.v1_m_X2_8p = rca.v1_Pm_X2_8p;
        rca.v1_MADPictureC1_12p = rca.v1_PMADPictureC1_12p;
        rca.v1_MADPictureC2_12p = rca.v1_PMADPictureC2_12p;
//22        rca.v1_PreviousPictureMAD = rca.v1_PPictureMAD[0];
            rca.v1_PreviousPictureMAD_8p = rca.v1_PPictureMAD_8p;

        MaxQpChange = rca.v1_PMaxQpChange;
        m_Qp = rca.v1_Pm_Qp;
        m_Hp = rca.v1_PPreHeader;

        if (rca.v1_BasicUnit < rca.v1_FrameSizeInMbs && rca.v1_type != P_SLICE )
        {
          // when RC_MODE_3 is set and basic unit is smaller than a frame, note that:
          // the linear MAD model and the quadratic QP model operate on small units and not on a whole frame;
          // we therefore have to account for this
            rca.v1_PreviousPictureMAD_8p = rca.v1_PreviousWholeFrameMAD_8p;
        }
        if (rca.v1_type == I_SLICE )
          m_Hp = 0; // it is usually a very small portion of the total I_SLICE bit budget

        /* predict the MAD of current picture*/
//20        rca.v1_CurrentMAD=rca.v1_MADPictureC1*rca.v1_PreviousPictureMAD + rca.v1_MADPictureC2;
//30        rca.v1_CurrentMAD_8p=(rca.v1_MADPictureC1_12p*rca.v1_PreviousPictureMAD_8p)/(1<<12) + rca.v1_MADPictureC2_12p/(1<<4);
        rca.v1_CurrentMAD_8p=(rca.v1_MADPictureC1_12p>>8)*(rca.v1_PreviousPictureMAD_8p>>4) + (rca.v1_MADPictureC2_12p>>4);

        /*compute the number of bits for the texture*/
        if(rca.v1_Target < 0)
        {
          rca.v1_m_Qc=m_Qp+MaxQpChange;
          rca.v1_m_Qc = my_iClip3(rca.v1_RCMinQP, rca.v1_RCMaxQP, rca.v1_m_Qc); // Clipping
        }
        else
        {
          if (rca.v1_type != P_SLICE )
          {
            if (rca.v1_BasicUnit < rca.v1_FrameSizeInMbs )
              m_Bits =(rca.v1_Target-m_Hp)/rca.v1_TotalNumberofBasicUnit;
            else
              m_Bits =rca.v1_Target-m_Hp;
          }
          else {
            m_Bits = rca.v1_Target-m_Hp;
            m_Bits = my_imax(m_Bits, (int)(rca.v1_bit_rate/(MINVALUE*rca.v1_framerate)));
          }
          my_v1_updateModelQPFrame( m_Bits );

          rca.v1_m_Qc = my_iClip3(rca.v1_RCMinQP, rca.v1_RCMaxQP, rca.v1_m_Qc); // clipping
          if (rca.v1_type == P_SLICE )
            rca.v1_m_Qc = my_iClip3(m_Qp-MaxQpChange, m_Qp+MaxQpChange, rca.v1_m_Qc); // control variation
        }

        if(rca.v1_type == P_SLICE)  // && rca.v1_FieldControl == 0
          my_v1_updateQPNonPicAFF( );

        return rca.v1_m_Qc;
      }
  }
  //// basic unit layer rate control
  else
  {
    if(rca.v1_gop_cnt==0 && rca.v1_frame_cnt==0) // (rca.v1_number == 0)
    {
      rca.v1_m_Qc = rca.v1_MyInitialQp;
      return rca.v1_m_Qc;
    }
    //else if( rca.v1_type == P_SLICE )
    else if( rca.v1_type == P_SLICE || rca.v1_type == I_SLICE ) //lhuitune
    {
      if(rca.v1_gop_cnt==0 && rca.v1_frame_cnt==1) // ((rca.v1_NumberofGOP==1)&&(rca.v1_NumberofPPicture==0)) // gop==0; frameP==0
      {
          return my_v1_updateFirstP(  );
      }
      else
      {
        rca.v1_m_X1_8p = rca.v1_Pm_X1_8p;
        rca.v1_m_X2_8p = rca.v1_Pm_X2_8p;
        rca.v1_MADPictureC1_12p=rca.v1_PMADPictureC1_12p;
        rca.v1_MADPictureC2_12p=rca.v1_PMADPictureC2_12p;

        m_Qp=rca.v1_Pm_Qp;

        SumofBasicUnit=rca.v1_TotalNumberofBasicUnit;

        if(rca.v1_bu_cnt==0) //(rca.v1_NumberofBasicUnit==SumofBasicUnit)
          return my_v1_updateFirstBU( );
        else
        {
          /*compute the number of remaining bits*/
          rca.v1_Target -= (rca.v1_NumberofBasicUnitHeaderBits + rca.v1_NumberofBasicUnitTextureBits);
          rca.v1_NumberofBasicUnitHeaderBits  = 0;
          rca.v1_NumberofBasicUnitTextureBits = 0;
          #ifdef JM_RC_DUMP
          #ifdef USE_MY_RC
          // jm rc-related debugging info dump, lhulhu
          {
            jm_rc_info_dump = fopen("jm_rc_info_dump.txt","a+");
            fprintf(jm_rc_info_dump, "Target(BU):%-d \t", rca.v1_Target);
            fclose (jm_rc_info_dump);
          }
          #endif
          #endif
          if(rca.v1_Target<0)
            return my_v1_updateNegativeTarget( m_Qp );
          else
          {
            /*predict the MAD of current picture*/
            my_v1_predictCurrPicMAD( );

            /*compute the total number of bits for the current basic unit*/
            my_v1_updateModelQPBU( m_Qp );

            rca.v1_TotalFrameQP +=rca.v1_m_Qc;
            rca.v1_Pm_Qp=rca.v1_m_Qc;
            if((rca.v1_bu_cnt==(rca.v1_TotalNumberofBasicUnit-1)) && rca.v1_type==P_SLICE) // lhu, 2017/03/23
            //if((rca.v1_bu_cnt==(rca.v1_TotalNumberofBasicUnit-1)) && (rca.v1_type==P_SLICE || rca.v1_type==I_SLICE) ) //lhuitune
              my_v1_updateLastBU( );

            return rca.v1_m_Qc;
          }
        }
      }
    }
  }
  return rca.v1_m_Qc;
}


void my_v1_updateQPNonPicAFF( )
{
    rca.v1_TotalQpforPPicture +=rca.v1_m_Qc;
    rca.v1_Pm_Qp=rca.v1_m_Qc;
}


int my_v1_updateFirstP( )
{
  //top field of the first P frame
  rca.v1_m_Qc=rca.v1_MyInitialQp;
  rca.v1_NumberofBasicUnitHeaderBits=0;
  rca.v1_NumberofBasicUnitTextureBits=0;
  //bottom field of the first P frame
  if(rca.v1_bu_cnt==(rca.v1_TotalNumberofBasicUnit-1)) //(rca.v1_NumberofBasicUnit==0)
  {
    rca.v1_TotalQpforPPicture +=rca.v1_m_Qc;
    rca.v1_PAveFrameQP=rca.v1_m_Qc;
    rca.v1_PAveHeaderBits3=rca.v1_PAveHeaderBits2;
  }
  rca.v1_Pm_Qp = rca.v1_m_Qc;
  rca.v1_TotalFrameQP += rca.v1_m_Qc;
  return rca.v1_m_Qc;
}


int my_v1_updateNegativeTarget( int m_Qp )
{
  int PAverageQP;

  if(rca.v1_GOPOverdue==TRUE)
    rca.v1_m_Qc=m_Qp+2;
  else
    rca.v1_m_Qc=m_Qp+rca.v1_DDquant;//2

  rca.v1_m_Qc = my_imin(rca.v1_m_Qc, rca.v1_RCMaxQP);  // clipping
  if(rca.v1_basicunit>=rca.v1_MBPerRow) {
  	if (rca.v1_wireless_screen!=1) { // added by lhu, 2017/02/27
      if (rca.v1_type == P_SLICE) rca.v1_m_Qc = my_imin(rca.v1_PAveFrameQP+6, rca.v1_m_Qc); // change +6 to +10, lhu, 2017/01/26
      else                        rca.v1_m_Qc = my_imin(rca.v1_PAveFrameQP+5, rca.v1_m_Qc); // lower QP change range for I slice, lhu, 2017/02/07
    } else {
      if (rca.v1_type == P_SLICE) rca.v1_m_Qc = my_imin(rca.v1_PAveFrameQP+3, rca.v1_m_Qc); // change +6 to +3, lhu, 2017/04/25
      else                        rca.v1_m_Qc = my_imin(rca.v1_PAveFrameQP+2, rca.v1_m_Qc); // change +6 to +2 for I slice, lhu, 2017/04/25
    }
  } else
    rca.v1_m_Qc = my_imin(rca.v1_m_Qc, rca.v1_PAveFrameQP+3);

  rca.v1_TotalFrameQP +=rca.v1_m_Qc;
  if(rca.v1_bu_cnt==(rca.v1_TotalNumberofBasicUnit-1)) //(rca.v1_NumberofBasicUnit==0)
  {
//18    PAverageQP=(int)((double)rca.v1_TotalFrameQP/(double)rca.v1_TotalNumberofBasicUnit+0.5);
    PAverageQP=(rca.v1_TotalFrameQP+(rca.v1_TotalNumberofBasicUnit>>1))/rca.v1_TotalNumberofBasicUnit;
    if(rca.v1_frame_cnt==(rca.v1_intra_period-1)) //(rca.v1_NumberofPPicture == (rca.v1_intra_period - 2))
      rca.v1_QPLastPFrame = PAverageQP;
    if (rca.v1_type == P_SLICE) // not increase TotalQpforPPicture for I_SLICE, lhuitune
      rca.v1_TotalQpforPPicture +=PAverageQP;
    rca.v1_PAveFrameQP=PAverageQP;
    rca.v1_PAveHeaderBits3=rca.v1_PAveHeaderBits2;
  }
  if(rca.v1_GOPOverdue==TRUE)
    rca.v1_Pm_Qp=rca.v1_PAveFrameQP;
  else
    rca.v1_Pm_Qp=rca.v1_m_Qc;

  return rca.v1_m_Qc;
}


int my_v1_updateFirstBU( )
{
  if(rca.v1_frame_cnt==1) rca.v1_PAveFrameQP = rca.v1_QPLastPFrame; // first P frame's initial QP value equals to LastPFrame's average QP, lhu, 2017/03/23
  else                    rca.v1_PAveFrameQP = rca.v1_PAveFrameQP;
  if(rca.v1_Target<=0)
  {
    rca.v1_m_Qc = rca.v1_PAveFrameQP + 2;
    if(rca.v1_m_Qc > rca.v1_RCMaxQP)
      rca.v1_m_Qc = rca.v1_RCMaxQP;

    rca.v1_GOPOverdue=TRUE;
  }
  else
  {
    rca.v1_m_Qc=rca.v1_PAveFrameQP;
  }
  rca.v1_TotalFrameQP +=rca.v1_m_Qc;
  rca.v1_Pm_Qp = rca.v1_PAveFrameQP;

  return rca.v1_m_Qc;
}


void my_v1_updateLastBU( )
{
  int PAverageQP;

//18  PAverageQP=(int)((double)rca.v1_TotalFrameQP/(double)rca.v1_TotalNumberofBasicUnit+0.5);
  PAverageQP=(rca.v1_TotalFrameQP+(rca.v1_TotalNumberofBasicUnit>>1))/rca.v1_TotalNumberofBasicUnit;
  if(rca.v1_frame_cnt==(rca.v1_intra_period-1)) // (rca.v1_NumberofPPicture == (rca.v1_intra_period - 2))  last P_FRAME in gop
    rca.v1_QPLastPFrame = PAverageQP;
  if (rca.v1_type == P_SLICE) // not increase TotalQpforPPicture for I_SLICE, lhuitune
    rca.v1_TotalQpforPPicture +=PAverageQP;
  rca.v1_PAveFrameQP=PAverageQP;
  rca.v1_PAveHeaderBits3=rca.v1_PAveHeaderBits2;
}


void my_v1_updateModelQPFrame( int m_Bits )
{
  long long dtmp_8p, qstep_tmp;
  int tmp_4p=0;
  int m_Qstep_8p;

  //dtmp_8p = (rca.v1_CurrentMAD_8p>>6)*(rca.v1_m_X1_8p>>6)*(rca.v1_CurrentMAD_8p>>6)*(rca.v1_m_X1_8p>>6) + \
  //    4*(rca.v1_m_X2_8p>>4)*(rca.v1_CurrentMAD_8p>>4)*m_Bits;
  dtmp_8p = ((long long)rca.v1_CurrentMAD_8p>>6)*((long long)rca.v1_CurrentMAD_8p>>6)*((long long)rca.v1_m_X1_8p>>6)*((long long)rca.v1_m_X1_8p>>6) + \
      4*((long long)rca.v1_m_X2_8p>>4)*((long long)rca.v1_CurrentMAD_8p>>4)*m_Bits;

  if(dtmp_8p>0)
      tmp_4p = my_sqrt64(dtmp_8p);

  if((rca.v1_m_X2_8p==0) || (dtmp_8p<0) || ((tmp_4p-((rca.v1_m_X1_8p>>6)*(rca.v1_CurrentMAD_8p>>6)))<=0))
  {
    //m_Qstep = (float)((rca.v1_m_X1*rca.v1_CurrentMAD) / (double) m_Bits);
    m_Qstep_8p = ((rca.v1_m_X1_8p>>4)*(rca.v1_CurrentMAD_8p>>4)) / m_Bits;
  }
  else // 2nd order mode
  {
    //m_Qstep = (float)((2*rca.v1_m_X2_8p*rca.v1_CurrentMAD_8p)/(sqrt(dtmp)*(1<<16) - rca.v1_m_X1_8p*rca.v1_CurrentMAD_8p));
    qstep_tmp = (2*((long long)rca.v1_m_X2_8p)*((long long)rca.v1_CurrentMAD_8p)) / ((tmp_4p<<4) - (rca.v1_m_X1_8p>>4)*(rca.v1_CurrentMAD_8p>>4));
    m_Qstep_8p = qstep_tmp;
  }

  rca.v1_m_Qc = Qstep2QP_8p(m_Qstep_8p);
}


void my_v1_predictCurrPicMAD( )
{
    int i,CurrentBUMAD_8p,MADPictureC1_12prr4,MADPictureC2_12prr4;

    MADPictureC1_12prr4 = rca.v1_MADPictureC1_12p>>4;
    MADPictureC2_12prr4 = rca.v1_MADPictureC2_12p>>4;

    //rca.v1_CurrentMAD=rca.v1_MADPictureC1*rca.v1_BUPFMAD[rca.v1_bu_cnt]+rca.v1_MADPictureC2;
    rca.v1_CurrentMAD_8p=(MADPictureC1_12prr4*(rca.v1_BUPFMAD_8p[rca.v1_bu_cnt]>>8)) + MADPictureC2_12prr4;
    rca.v1_TotalBUMAD_12p=0;

    for(i=rca.v1_TotalNumberofBasicUnit-1; i>=rca.v1_bu_cnt; i--)
    {
        //CurrentBUMAD = rca.v1_MADPictureC1*rca.v1_BUPFMAD[i]+rca.v1_MADPictureC2;
        CurrentBUMAD_8p = (MADPictureC1_12prr4*(rca.v1_BUPFMAD_8p[i]>>8)) + MADPictureC2_12prr4;
        rca.v1_TotalBUMAD_12p += (CurrentBUMAD_8p*CurrentBUMAD_8p)>>4;
    }
}


void my_v1_updateModelQPBU( int m_Qp )
{
  int m_Bits;
  long long dtmp_8p,qstep_tmp;
  int tmp_4p=0;
  int m_Qstep_8p;

  //compute the total number of bits for the current basic unit
  //m_Bits =(int)(rca.v1_Target * rca.v1_CurrentMAD * rca.v1_CurrentMAD / rca.v1_TotalBUMAD);
  if((rca.v1_TotalBUMAD_12p>>8) == 0)
    m_Bits = rca.v1_Target;
  else
    m_Bits =(rca.v1_Target*(rca.v1_CurrentMAD_8p>>6)*(rca.v1_CurrentMAD_8p>>6)) / (rca.v1_TotalBUMAD_12p>>8);

  //compute the number of texture bits
  m_Bits -=rca.v1_PAveHeaderBits2;

  m_Bits=my_imax(m_Bits,((rca.v1_bit_rate/rca.v1_framerate)/(MINVALUE*rca.v1_TotalNumberofBasicUnit)));

  //dtmp = rca.v1_CurrentMAD*rca.v1_CurrentMAD*rca.v1_m_X1*rca.v1_m_X1 + 4*rca.v1_m_X2*rca.v1_CurrentMAD*m_Bits;
  dtmp_8p = ((long long)rca.v1_CurrentMAD_8p>>6)*((long long)rca.v1_CurrentMAD_8p>>6)*((long long)rca.v1_m_X1_8p>>6)*((long long)rca.v1_m_X1_8p>>6) + \
      4*((long long)rca.v1_m_X2_8p>>4)*((long long)rca.v1_CurrentMAD_8p>>4)*m_Bits;

  if(dtmp_8p>0)
    tmp_4p = my_sqrt64(dtmp_8p);

  //if((rca.v1_m_X2==0) || (dtmp<0) || ((sqrt(dtmp)-(rca.v1_m_X1*rca.v1_CurrentMAD))<=0))  // fall back 1st order mode
  if((rca.v1_m_X2_8p==0) || (dtmp_8p<0) || ((tmp_4p-((rca.v1_m_X1_8p>>6)*(rca.v1_CurrentMAD_8p>>6)))<=0))
  {
    //m_Qstep = (float)((rca.v1_m_X1*rca.v1_CurrentMAD) / (double) m_Bits);
    m_Qstep_8p = ((rca.v1_m_X1_8p>>4)*(rca.v1_CurrentMAD_8p>>4)) / m_Bits;
  }
  else // 2nd order mode
  {
      //m_Qstep = (float)((2*rca.v1_m_X2_8p*rca.v1_CurrentMAD_8p)/(sqrt(dtmp)*(1<<16) - rca.v1_m_X1_8p*rca.v1_CurrentMAD_8p));
      qstep_tmp = (2*((long long)rca.v1_m_X2_8p)*((long long)rca.v1_CurrentMAD_8p)) / ((tmp_4p<<4) - (rca.v1_m_X1_8p>>4)*(rca.v1_CurrentMAD_8p>>4));
      m_Qstep_8p = qstep_tmp;
  }

  rca.v1_m_Qc = Qstep2QP_8p(m_Qstep_8p);
  //use the Qc by R-D model when non-wireless-screen application, lhu, 2017/02/27
  if (rca.v1_wireless_screen==1) // added by lhu, 2017/02/27
    rca.v1_m_Qc = my_imin(m_Qp+rca.v1_DDquant,  rca.v1_m_Qc); // control variation

  if(rca.v1_basicunit>=rca.v1_MBPerRow) {
  	if (rca.v1_wireless_screen!=1) {// added by lhu, 2017/02/27
      if (rca.v1_type == P_SLICE) rca.v1_m_Qc = my_imin(rca.v1_PAveFrameQP+6, rca.v1_m_Qc); // change +6 to +10, lhu, 2017/01/24
      else                        rca.v1_m_Qc = my_imin(rca.v1_PAveFrameQP+5, rca.v1_m_Qc); // lower QP change range for I slice, lhu, 2017/02/07
    } else {
      if (rca.v1_type == P_SLICE) rca.v1_m_Qc = my_imin(rca.v1_PAveFrameQP+3, rca.v1_m_Qc); // change +6 to +3, lhu, 2017/04/25
      else                        rca.v1_m_Qc = my_imin(rca.v1_PAveFrameQP+2, rca.v1_m_Qc); // change +6 to +2 for I slice, lhu, 2017/04/25
    }
  } else
    rca.v1_m_Qc = my_imin(rca.v1_PAveFrameQP+3, rca.v1_m_Qc);

  if(rca.v1_c1_over==1)
    //rca.v1_m_Qc = my_imin(m_Qp-rca.v1_DDquant, rca.v1_RCMaxQP); // clipping
    rca.v1_m_Qc = my_imin(m_Qp+rca.v1_DDquant, rca.v1_RCMaxQP-10); // not letting QP decrease when MAD equal 0, 2017/02/21
  else
    rca.v1_m_Qc = my_iClip3(m_Qp-rca.v1_DDquant, rca.v1_RCMaxQP, rca.v1_m_Qc); // clipping

  if(rca.v1_basicunit>=rca.v1_MBPerRow) {
  	if (rca.v1_wireless_screen!=1) {// added by lhu, 2017/04/18
      if (rca.v1_type == P_SLICE) rca.v1_m_Qc = my_imax(rca.v1_PAveFrameQP-6, rca.v1_m_Qc); // lhu, 2017/04/18
      else                        rca.v1_m_Qc = my_imax(rca.v1_PAveFrameQP-5, rca.v1_m_Qc); // lhu, 2017/04/18
    } else {
      if (rca.v1_type == P_SLICE) rca.v1_m_Qc = my_imax(rca.v1_PAveFrameQP-3, rca.v1_m_Qc); // lhu, 2017/04/25
      else                        rca.v1_m_Qc = my_imax(rca.v1_PAveFrameQP-2, rca.v1_m_Qc); // lhu, 2017/04/25
    }
  } else
    rca.v1_m_Qc = my_imax(rca.v1_PAveFrameQP-3, rca.v1_m_Qc);

  rca.v1_m_Qc = my_imax(rca.v1_RCMinQP, rca.v1_m_Qc);
}


#ifdef ARMCM7_RC //#########
void my_v1_rc_update_bu_stats( ) {
    rca.v1_NumberofHeaderBits  = rca.v1_frame_hbits;
    rca.v1_NumberofTextureBits = rca.v1_frame_tbits;
    // basic unit layer rate control
    if(rca.v1_BasicUnit < rca.v1_FrameSizeInMbs) {
        rca.v1_NumberofBasicUnitHeaderBits  = v1_hbits_tmp;  // add slice_header
        rca.v1_NumberofBasicUnitTextureBits = v1_tbits_tmp;
    }
}
void my_v1_rc_update_frame_stats( ) {
    rca.v1_frame_mad   += v1_mad_tmp;
    rca.v1_frame_tbits += v1_tbits_tmp;
    rca.v1_frame_hbits += v1_hbits_tmp;
    rca.v1_frame_abits = rca.v1_frame_tbits+rca.v1_frame_hbits;
    if(rca.v1_bu_cnt==0) { //after calculate frame's status reset related status to zero, lhu, 2017/03/06
        rca.v1_frame_mad   = 0;
        rca.v1_frame_tbits = 0;
        rca.v1_frame_hbits = 0;
        rca.v1_frame_abits = 0;
    }
}
#else //#########
void my_rc_update_bu_stats( ) {
    rca.NumberofHeaderBits  = global_frame_hbits;
    rca.NumberofTextureBits = global_frame_tbits;
    // basic unit layer rate control
    if(rca.BasicUnit < rca.FrameSizeInMbs) {
        rca.NumberofBasicUnitHeaderBits  = global_bu_hbits;  // add slice_header
        rca.NumberofBasicUnitTextureBits = global_bu_tbits;
    }
  #ifdef JM_RC_DUMP
  #ifdef USE_MY_RC
  // jm rc-related debugging info dump, lhulhu
  {
    jm_rc_info_dump=fopen("jm_rc_info_dump.txt","a+");
    fprintf(jm_rc_info_dump, "BUHBits:%-d BUTBits:%-d\n", global_bu_hbits, global_bu_tbits);
    fclose(jm_rc_info_dump);
  }
  #endif
  #endif
}
void my_rc_update_frame_stats( )
{
    if(rca.bu_cnt==0) //frame over
    {
        rca.frame_mad   = 0;
        rca.frame_tbits = 0;
        rca.frame_hbits = 0;
    }
    else
    {
        rca.frame_mad   += global_bu_mad;
        rca.frame_tbits += global_bu_tbits;  //
        rca.frame_hbits += global_bu_hbits;
    }
}
#endif //#########


void my_v1_rc_init_gop_params( )
{
    if(rca.v1_RCUpdateMode==RC_MODE_1)
    {
        //if((rca.gop_cnt==0 && rca.frame_cnt==0) || ((rca.gop_cnt*rca.intra_period)==rca.no_frm_base))
        if(rca.v1_frame_cnt==0 && rca.v1_bu_cnt==0)
            my_v1_rc_init_GOP( rca.v1_no_frm_base - 1 );
    }
    else if((rca.v1_RCUpdateMode==RC_MODE_0)|(rca.v1_RCUpdateMode==RC_MODE_2)|(rca.v1_RCUpdateMode==RC_MODE_3))
    {
        if(rca.v1_frame_cnt==0 && rca.v1_bu_cnt==0) {
            if (rca.v1_IFduration==1 && rca.v1_insertOneIFrame==1) {
                rca.v1_intra_period = rca.v1_PrevIntraPeriod; // use previous intra_period to calculate GOP TargetBits, lhu, 2017/03/13
                rca.v1_RCISliceBitRatio = 3; // fix the Ipratio when decide insert Intra Frame, lhu, 2017/04/26
            }
            my_v1_rc_init_GOP( rca.v1_intra_period - 1 );
        }
    }
}


int my_v1_rc_handle_mb( )
{
    //// first update, update MB_state
    if(rca.v1_gop_cnt!=0 || rca.v1_frame_cnt!=0 || rca.v1_bu_cnt!=0)// || rca.mb_cnt!=0
    {
        my_v1_rc_update_bu_stats( ); //my_rc_update_mb_stats();
#ifdef ARMCM7_RC //#############
        rca.v1_TotalMADBasicUnit = v1_mad_tmp;
        rca.v1_TotalFrameMAD    += v1_mad_tmp;// lhumod
#else //#############
        rca.TotalMADBasicUnit = global_bu_mad;
        rca.TotalFrameMAD    += global_bu_mad;// lhumod
#endif //#############
    }


    if((rca.v1_gop_cnt>0 || rca.v1_frame_cnt>0) && rca.v1_bu_cnt==0) {// && rca.mb_cnt==0))
        rca.v1_frame_mad = rca.v1_TotalFrameMAD/rca.v1_FrameSizeInMbs;// lhumod, calculate the average MB's mad value of previous encoded frame.
#ifdef ARMCM7_RC //#############
        my_v1_rc_update_pict( v1_fbits_tmp );  // should put it to the frame-last
#else //#############
        //my_rc_update_pict( global_frame_abits );  // should put it to the frame-last
        my_rc_update_pict( global_fbits );// lhumod, it actually get buffer related info(remainingbits,currentbufferfullness) after previous frame encoded.
#endif //#############
    }

    //// initial sequence (only once)
    if((rca.v1_gop_cnt==0)&(rca.v1_frame_cnt==0)&(rca.v1_bu_cnt==0)) //&(rca.mb_cnt==0))
    {
        my_v1_rc_params( );
        rca.v1_type = I_SLICE;
        my_v1_rc_init_seq( ); //// initial seq params
        my_v1_rc_init_gop_params( );
        my_v1_rc_init_pict(1);
        rca.v1_qp = my_v1_updateQP( );
        rca.v1_slice_qp = rca.v1_qp;
    }
    else if((rca.v1_frame_cnt==0)&(rca.v1_bu_cnt==0)) //&(rca.mb_cnt==0)) //// initial GOP
    {
        rca.v1_type = I_SLICE;
        my_v1_rc_init_gop_params( );
        my_v1_rc_init_pict(1);
        rca.v1_qp = my_v1_updateQP( );
        rca.v1_slice_qp = rca.v1_qp;
    }
    else if(rca.v1_bu_cnt==0) //&(rca.mb_cnt==0)) //// initial frame
    {
        rca.v1_type = P_SLICE;
        my_v1_rc_init_pict(1);
        rca.v1_qp = my_v1_updateQP( );
        rca.v1_slice_qp = rca.v1_qp;
    }

    // frame layer rate control //// BU-Level
    if (rca.v1_basicunit < rca.v1_FrameSizeInMbs)
    {
        // each I or B frame has only one QP
        //if(rca.v1_type==I_SLICE)//g1 && rca.RCUpdateMode!=RC_MODE_1) || (rca.gop_cnt==0 && rca.frame_cnt==0)) //!(rca.number)
        if(rca.v1_gop_cnt==0 && rca.v1_frame_cnt==0) //lhuitune
        {
            rca.v1_qp = rca.v1_MyInitialQp;
        }
        //else if (rca.v1_type == P_SLICE) //g1 || rca.RCUpdateMode == RC_MODE_1 )
        else if (rca.v1_type == P_SLICE || rca.v1_RCUpdateMode==RC_MODE_1 || (rca.v1_type==I_SLICE && rca.v1_RCUpdateMode==RC_MODE_3)) //lhuitune
        {
            // compute the quantization parameter for each basic unit of P frame
            if(rca.v1_bu_cnt!=0) // && rca.mb_cnt==0)
            {
              my_v1_updateRCModel( );
              rca.v1_qp = my_v1_updateQP( );
            }
        }
    }

    rca.v1_qp = my_iClip3(rca.v1_RCMinQP, rca.v1_RCMaxQP, rca.v1_qp); // -rca.bitdepth_luma_qp_scale

    my_v1_rc_update_frame_stats(); // computer frame parameters
    if( rca.v1_bu_cnt==(rca.v1_TotalNumberofBasicUnit-1) ) {
        if (rca.v1_changeToIFrame==1 || rca.v1_insertOneIFrame==1) {
            if (rca.v1_type==P_SLICE) my_criteria_decide_changeToIFrame((ymseh_tmp>>16)&0xff,(ymseh_tmp>>8)&0xff,(ymseh_tmp>>5)&0x7,1); // lhu, 2017/03/24
            else                      my_decide_backtoNormalGOP(1); // lhu, 2017/03/24
        } else { // lhu, 2017/04/10
            rca.v1_gop_change_NotResetRC=0; rca.v1_IFduration=0;
        }
        rca.v1_frm_ymse[1]  = rca.v1_frm_ymse[0]; // lhu, 2017/03/27
        // renew the QPLastPFrame after intra_period updated, lhu, 2017/03/28
        if ( (rca.v1_type==P_SLICE) && (rca.v1_frame_cnt>=(rca.v1_intra_period-1)) ) {
            rca.v1_QPLastPFrame = (rca.v1_TotalFrameQP+(rca.v1_TotalNumberofBasicUnit>>1))/rca.v1_TotalNumberofBasicUnit;
        }
    }

    if(rca.v1_basicunit < rca.v1_FrameSizeInMbs) // bu-level counter
    {
        if(rca.v1_bu_cnt==(rca.v1_TotalNumberofBasicUnit-1))
        {
            rca.v1_bu_cnt=0;
            if(rca.v1_frame_cnt>=(rca.v1_intra_period-1)) // change "==" to ">=", lhu, 2017/03/09
            {
                rca.v1_frame_cnt=0;
                //if(rca.v1_gop_cnt<=1000)
                rca.v1_gop_cnt++;
            }
            else
                rca.v1_frame_cnt++;
        }
        else
            rca.v1_bu_cnt++;
    }
    else // frame-level counter
    {
        if(rca.v1_frame_cnt==(rca.v1_intra_period-1))
        {
            rca.v1_frame_cnt=0;
            //if(rca.v1_gop_cnt<=1000)
            rca.v1_gop_cnt++;
        }
        else
            rca.v1_frame_cnt++;
    }

#ifndef ARMCM7_RC //#############
    my_hold( );
#endif //#############


    return rca.v1_qp;
}
