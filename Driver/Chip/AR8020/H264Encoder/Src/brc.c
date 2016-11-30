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
int VEBRC_IRQ_Handler( )
{
    int i,run_case=0;

    my_v0_feedback( ); //// view0 irq
    if((rca.v0_fd_irq_en==1) && (rca.v0_rc_enable==1) && (rca.v0_enable==1)) {
        run_case=0;
        // only for gop change at last_p_frame. At this circumstance, Hardware use the updated gop for frame_cnt increment immediately,
        // but software use the un-updated one for its frame_cnt counting. Thus would iccur mismatch for both side.
        if (rca.v0_fd_last_p==1) {
            READ_WORD(V0_GOPFPS_ADDR,i); //read view0 gop
            if (rca.v0_fd_row_cnt==0) v0_last_p_prev_gop = (i>>24)&0xff;
            if (((i>>24)&0xff)!=v0_last_p_prev_gop) v0_last_p_gop_change = TRUE; // check view0's GOP change or not at last_p_frame
            v0_last_p_prev_gop = (i>>24)&0xff;
        }
    }
    else {
        my_v1_feedback( ); //// view1 irq
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
            if (rca.v1_fd_last_p==1) {
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
            if((rca.v0_fd_reset==1) && (rca.v0_fd_last_p==1) && (rca.v0_fd_last_row==1) && (v0_last_p_gop_change==FALSE) && (v0_poweron_rc_params_set==0)) {//restart!!!
                update_aof (); //lyu
                my_v0_initial_all( );
            }
            else if(rca.v0_rc_enable==1)
            {
                if ((rca.v0_fd_iframe==1) && (rca.v0_fd_row_cnt==0) && (v0_last_p_gop_change==TRUE)) {
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
                READ_WORD(V0_TBITS_ADDR,v0_tbits_tmp); //read tbits
                if(rca.v0_fd_last_row==1) {
                    READ_WORD(V0_ABITS_ADDR,v0_fbits_tmp); //read abits
                    if(rca.v0_fd_iframe==1) {
                        READ_WORD(V0_YMSEL_ADDR,ymsel_tmp); // read i frame's y-mse
                        READ_WORD(V0_YMSEH_ADDR,ymseh_tmp);
                        v0_ymse_iframe = (((int64)((ymseh_tmp>>24)&0xff)<<32) + ymsel_tmp);
                    }
                    if(rca.v0_fd_last_p==1) {
                        READ_WORD(V0_YMSEL_ADDR,ymsel_tmp); //read y-mse
                        READ_WORD(V0_YMSEH_ADDR,ymseh_tmp);
                        v0_ymse_last_p = (((int64)((ymseh_tmp>>24)&0xff)<<32) + ymsel_tmp);
                        if (ymseh_tmp&0x1) my_v0_rc_params_ac_gop( ); // @lhu, write ac_gop start symbol
                        if ((ymseh_tmp>>1)&0x1) my_v0_rc_params_ac_iopratio( ); // @lhu, write ac_iopratio start symbol
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
            if ((rca.v1_fd_reset==1) && (rca.v1_fd_last_p==1) && (rca.v1_fd_last_row==1) && (v1_last_p_gop_change==FALSE) && (v1_poweron_rc_params_set==0)) {//restart
                update_aof (); //lyu
                my_v1_initial_all( );
            }
            else if(rca.v1_rc_enable==1)
            {
                if ((rca.v1_fd_iframe==1) && (rca.v1_fd_row_cnt==0) && (v1_last_p_gop_change==TRUE)) {
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
                READ_WORD(V1_TBITS_ADDR,v1_tbits_tmp); //read tbits
                if(rca.v1_fd_last_row==1) {
                    READ_WORD(V1_ABITS_ADDR,v1_fbits_tmp); //read abits
                    if(rca.v1_fd_iframe==1) {
                        READ_WORD(V1_YMSEL_ADDR,ymsel_tmp); // read i frame's y-mse
                        READ_WORD(V1_YMSEH_ADDR,ymseh_tmp);
                        v1_ymse_iframe = (((int64)((ymseh_tmp>>24)&0xff)<<32) + ymsel_tmp);
                    }
                    if(rca.v1_fd_last_p==1) {
                        READ_WORD(V1_YMSEL_ADDR,ymsel_tmp); //read y-mse
                        READ_WORD(V1_YMSEH_ADDR,ymseh_tmp);
                        v1_ymse_last_p = (((int64)((ymseh_tmp>>24)&0xff)<<32) + ymsel_tmp);
                        if (ymseh_tmp&0x1) my_v1_rc_params_ac_gop( ); // @lhu, write ac_gop start symbol
                        if ((ymseh_tmp>>1)&0x1) my_v1_rc_params_ac_iopratio( ); //@ lhu, write ac_iopratio start symbol
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
            if ((rca.v1_fd_reset==1) && (rca.v1_fd_last_p==1) && (rca.v1_fd_last_row==1) && (v1_last_p_gop_change==FALSE) && (v1_poweron_rc_params_set==0)) {//restart
                update_aof (); //lyu
                my_v1_initial_all( );
            }
            else if(rca.v1_rc_enable==1)
            {
                if ((rca.v1_fd_iframe==1) && (rca.v1_fd_row_cnt==0) && (v1_last_p_gop_change==TRUE)) {
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
                READ_WORD(V0_TBITS_ADDR,v1_tbits_tmp); //read tbits
                if(rca.v1_fd_last_row==1) {
                    READ_WORD(V0_ABITS_ADDR,v1_fbits_tmp); //read abits
                    if(rca.v1_fd_iframe==1) {
                        READ_WORD(V0_YMSEL_ADDR,ymsel_tmp); // read i frame's y-mse
                        READ_WORD(V0_YMSEH_ADDR,ymseh_tmp);
                        v1_ymse_iframe = (((int64)((ymseh_tmp>>24)&0xff)<<32) + ymsel_tmp);
                    }
                    if(rca.v1_fd_last_p==1) {
                        READ_WORD(V0_YMSEL_ADDR,ymsel_tmp); //read y-mse
                        READ_WORD(V0_YMSEH_ADDR,ymseh_tmp);
                        v1_ymse_last_p = (((int64)((ymseh_tmp>>24)&0xff)<<32) + ymsel_tmp);
                        if (ymseh_tmp&0x1) my_v1_rc_params_ac_gop( ); // @lhu, write ac_gop start symbol
                        if ((ymseh_tmp>>1)&0x1) my_v1_rc_params_ac_iopratio( ); //@ lhu, write ac_iopratio start symbol
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
void my_v0_feedback( )
{
    int i;

    READ_WORD(V0_FEEDBACK_ADDR,i); //read feedback-data
    rca.v0_aof_inc_qp = (i>>16)&0xffff;
    rca.v0_fd_row_cnt = (i>>9)&0x7f;
    rca.v0_fd_last_row = (i>>8)&0x1;
    //rca.fd_cpu_test = (i>>7)&0x1;
    rca.v0_fd_irq_en = (i>>4)&0x1;
    //if(rca.v0_re_bitrate==0)
    rca.v0_re_bitrate = (i>>3)&0x1;
    //if(rca.v0_fd_reset==0)
    rca.v0_fd_reset = (i>>2)&0x1;
    rca.v0_fd_iframe = (i>>1)&0x1;
    rca.v0_fd_last_p = i&0x1;
    READ_WORD(V0_RCEN_BU_ADDR,i); //read rc_en, rc_mode & bu
        rca.v0_rc_enable = (i>>24)&0x1;
    READ_WORD(V0_ENABLE_ADDR,i); //read view enable
        rca.v0_enable = (i>>24)&0x1;
}

void my_v1_feedback( )
{
    int i;

    READ_WORD(V1_FEEDBACK_ADDR,i); //read feedback-data
    rca.v1_aof_inc_qp = (i>>16)&0xffff;
    rca.v1_fd_row_cnt = (i>>9)&0x7f;
    rca.v1_fd_last_row = (i>>8)&0x1;
    rca.v1_fd_irq_en = (i>>4)&0x1;
    //if(rca.v1_re_bitrate==0)
    rca.v1_re_bitrate = (i>>3)&0x1;
    //if(rca.v1_fd_reset==0)
    rca.v1_fd_reset = (i>>2)&0x1;
    rca.v1_fd_iframe = (i>>1)&0x1;
    rca.v1_fd_last_p = i&0x1;
    READ_WORD(V1_RCEN_BU_ADDR,i); //read rc_en, rc_mode & bu
        rca.v1_rc_enable = (i>>24)&0x1;
    READ_WORD(V1_ENABLE_ADDR,i); //read view enable
        rca.v1_enable = (i>>24)&0x1;
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
        rca.v0_RCEnableAutoConfigGOP = i&0x1;
        rca.v0_RCEnableAutoConfigIOPRatio = (i>>1)&0x1;
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
    int m;
    int v0_ac_br_index,v0_ac_br;
    int v1_ac_br_index,v1_ac_br;
    if (view==0) {
      READ_WORD(V0_RC_ACBR_ADDR,m);
      v0_ac_br_index = (m>>26)&0x1f;

      switch(v0_ac_br_index) {
        case 0 : v0_ac_br = 8000000 ; break; // 8Mbps
        case 1 : v0_ac_br = 1000000 ; break; // 1Mbps
        case 2 : v0_ac_br = 2000000 ; break; // 2Mbps
        case 3 : v0_ac_br = 3000000 ; break; // 3Mbps
        case 4 : v0_ac_br = 4000000 ; break; // 4Mbps
        case 5 : v0_ac_br = 5000000 ; break; // 5Mbps
        case 6 : v0_ac_br = 6000000 ; break; // 6Mbps
        case 7 : v0_ac_br = 7000000 ; break; // 7Mbps
        case 8 : v0_ac_br = 500000  ; break; // 500Kbps
        case 9 : v0_ac_br = 9000000 ; break; // 9Mbps
        case 10: v0_ac_br = 10000000; break; // 10Mbps
        case 11: v0_ac_br = 11000000; break; // 11Mbps
        case 12: v0_ac_br = 12000000; break; // 12Mbps
        case 13: v0_ac_br = 13000000; break; // 13Mbps
        case 14: v0_ac_br = 14000000; break; // 14Mbps
        case 15: v0_ac_br = 15000000; break; // 15Mbps
        case 16: v0_ac_br = 16000000; break; // 16Mbps
        case 17: v0_ac_br = 17000000; break; // 17Mbps
        case 18: v0_ac_br = 18000000; break; // 18Mbps
        case 19: v0_ac_br = 19000000; break; // 19Mbps
        case 20: v0_ac_br = 20000000; break; // 20Mbps
        case 21: v0_ac_br = 21000000; break; // 21Mbps
        case 22: v0_ac_br = 22000000; break; // 22Mbps
        case 23: v0_ac_br = 23000000; break; // 23Mbps
        case 24: v0_ac_br = 24000000; break; // 24Mbps
        case 25: v0_ac_br = 25000000; break; // 25Mbps
        case 26: v0_ac_br = 26000000; break; // 26Mbps
        case 27: v0_ac_br = 27000000; break; // 27Mbps
        case 28: v0_ac_br = 28000000; break; // 28Mbps
        case 29: v0_ac_br = 29000000; break; // 29Mbps
        case 30: v0_ac_br = 30000000; break; // 30Mbps
        default: v0_ac_br = 31000000; break; // 31Mbps
      }
      if (v0_ac_br_index != rca.v0_prev_ac_br_index)
        WRITE_WORD(V0_BR_ADDR, v0_ac_br);
      rca.v0_prev_ac_br_index = v0_ac_br_index;
    }
    else {
      READ_WORD(V1_RC_ACBR_ADDR,m);
      v1_ac_br_index = (m>>26)&0x1f;

      switch(v1_ac_br_index) {
        case 0 : v1_ac_br = 8000000 ; break; // 8Mbps
        case 1 : v1_ac_br = 1000000 ; break; // 1Mbps
        case 2 : v1_ac_br = 2000000 ; break; // 2Mbps
        case 3 : v1_ac_br = 3000000 ; break; // 3Mbps
        case 4 : v1_ac_br = 4000000 ; break; // 4Mbps
        case 5 : v1_ac_br = 5000000 ; break; // 5Mbps
        case 6 : v1_ac_br = 6000000 ; break; // 6Mbps
        case 7 : v1_ac_br = 7000000 ; break; // 7Mbps
        case 8 : v1_ac_br = 500000  ; break; // 500Kbps
        case 9 : v1_ac_br = 9000000 ; break; // 9Mbps
        case 10: v1_ac_br = 10000000; break; // 10Mbps
        case 11: v1_ac_br = 11000000; break; // 11Mbps
        case 12: v1_ac_br = 12000000; break; // 12Mbps
        case 13: v1_ac_br = 13000000; break; // 13Mbps
        case 14: v1_ac_br = 14000000; break; // 14Mbps
        case 15: v1_ac_br = 15000000; break; // 15Mbps
        case 16: v1_ac_br = 16000000; break; // 16Mbps
        case 17: v1_ac_br = 17000000; break; // 17Mbps
        case 18: v1_ac_br = 18000000; break; // 18Mbps
        case 19: v1_ac_br = 19000000; break; // 19Mbps
        case 20: v1_ac_br = 20000000; break; // 20Mbps
        case 21: v1_ac_br = 21000000; break; // 21Mbps
        case 22: v1_ac_br = 22000000; break; // 22Mbps
        case 23: v1_ac_br = 23000000; break; // 23Mbps
        case 24: v1_ac_br = 24000000; break; // 24Mbps
        case 25: v1_ac_br = 25000000; break; // 25Mbps
        case 26: v1_ac_br = 26000000; break; // 26Mbps
        case 27: v1_ac_br = 27000000; break; // 27Mbps
        case 28: v1_ac_br = 28000000; break; // 28Mbps
        case 29: v1_ac_br = 29000000; break; // 29Mbps
        case 30: v1_ac_br = 30000000; break; // 30Mbps
        default: v1_ac_br = 31000000; break; // 31Mbps
      }
      if (v1_ac_br_index != rca.v1_prev_ac_br_index)
        WRITE_WORD(V1_BR_ADDR, v1_ac_br);
      rca.v1_prev_ac_br_index = v1_ac_br_index;
    }
}
//===== Auto-config GOP based on sub-function of RCAutoConfig_GOP =====
void my_v0_rc_params_ac_gop( ) {
    int i,j,m,v0_autoconfig_gop;

    READ_WORD(V0_FRAME_XY_ADDR,m); //read frame-x & frame-y
        i=(m>>16)&0xffff;
        j=m&0xffff;
        rca.v0_width = i;
        rca.v0_height = j;

    READ_WORD(V0_RCEN_BU_ADDR,i); //read rc_en, rc_mode & bu
        rca.v0_rc_enable = (i>>24)&0x1;
        rca.v0_RCUpdateMode = (i>>16)&0x3;

    READ_WORD(V0_BR_ADDR,i); //read br
        rca.v0_bit_rate = i;

    if (rca.v0_rc_enable==1 && rca.v0_RCUpdateMode==RC_MODE_3) {
        v0_autoconfig_gop = RCAutoConfig_GOP(rca.v0_width, rca.v0_height, rca.v0_bit_rate, v0_ymse_last_p);
        rca.v0_intra_period = v0_autoconfig_gop;
        READ_WORD(V0_QP_ADDR, i);
        WRITE_WORD(V0_QP_ADDR, (((i>>8)<<8)+v0_autoconfig_gop));
    }
}
//===== Criteria for RC auto configure GOP ===> Depend on bitrate & frame's psnr (MSE) =====
int RCAutoConfig_GOP(int w, int h, int bit_rate, int64 mse) {
    int64 whm255square;
    int divider, rc_ac_gop;

    whm255square = (int64)(((w<<8)-w)*((h<<8)-h));
    divider = (int)(whm255square/mse);

    if (bit_rate>=5000000 && bit_rate <=8000000) { // 5Mbps ~ 8Mbps
        if      (divider>=10000)                 rc_ac_gop =  20;// psnr>=40db
        else if (divider>=6309 && divider<10000) rc_ac_gop =  20;// 10^4.0=10000  , 10^3.8=6309.57
        else if (divider>=3981 && divider<6309)  rc_ac_gop =  30;// 10^3.8=6309.57, 10^3.6=3981.07
        else if (divider>=3162 && divider<3981)  rc_ac_gop =  40;// 10^3.6=3981.07, 10^3.5=3162.27
        else if (divider>=2511 && divider<3162)  rc_ac_gop =  40;// 10^3.5=3162.27, 10^3.4=2511.88
        else if (divider>=1995 && divider<2511)  rc_ac_gop =  50;// 10^3.4=2511.88, 10^3.3=1995.26
        else if (divider>=1584 && divider<1995)  rc_ac_gop =  50;// 10^3.3=1995.26, 10^3.2=1584.89
        else if (divider>=1258 && divider<1584)  rc_ac_gop =  60;// 10^3.2=1584.89, 10^3.1=1258.92
        else if (divider>=1000 && divider<1258)  rc_ac_gop =  60;// 10^3.1=1258.92, 10^3.0=1000
        else if (divider>=794  && divider<1000)  rc_ac_gop =  70;// 10^3.0=1000   , 10^2.9=794.32
        else if (divider>=630  && divider<794 )  rc_ac_gop =  70;// 10^2.9=794.32 , 10^2.8=630.95
        else if (divider>=501  && divider<630 )  rc_ac_gop =  80;// 10^2.8=630.95 , 10^2.7=501.18
        else if (divider>=398  && divider<501 )  rc_ac_gop =  80;// 10^2.7=501.18 , 10^2.6=398.10
        else if (divider>=316  && divider<398 )  rc_ac_gop =  90;// 10^2.6=398.10 , 10^2.5=316.22
        else if (divider>=215  && divider<316 )  rc_ac_gop =  90;// 10^2.5=316.22 , 10^2.4=215.18
        else if (divider>=199  && divider<215 )  rc_ac_gop = 100;// 10^2.4=215.18 , 10^2.3=199.52
        else if (divider>=158  && divider<199 )  rc_ac_gop = 100;// 10^2.3=199.52 , 10^2.2=158.48
        else                                     rc_ac_gop = 110;// psnr<22db
    }
    else if (bit_rate>=3000000 && bit_rate <5000000) { // 3Mbps ~ 5Mbps
        if      (divider>=10000)                 rc_ac_gop =  30;// psnr>=40db
        else if (divider>=6309 && divider<10000) rc_ac_gop =  30;// 10^4.0=10000  , 10^3.8=6309.57
        else if (divider>=3981 && divider<6309)  rc_ac_gop =  40;// 10^3.8=6309.57, 10^3.6=3981.07
        else if (divider>=3162 && divider<3981)  rc_ac_gop =  50;// 10^3.6=3981.07, 10^3.5=3162.27
        else if (divider>=2511 && divider<3162)  rc_ac_gop =  50;// 10^3.5=3162.27, 10^3.4=2511.88
        else if (divider>=1995 && divider<2511)  rc_ac_gop =  60;// 10^3.4=2511.88, 10^3.3=1995.26
        else if (divider>=1584 && divider<1995)  rc_ac_gop =  60;// 10^3.3=1995.26, 10^3.2=1584.89
        else if (divider>=1258 && divider<1584)  rc_ac_gop =  70;// 10^3.2=1584.89, 10^3.1=1258.92
        else if (divider>=1000 && divider<1258)  rc_ac_gop =  70;// 10^3.1=1258.92, 10^3.0=1000
        else if (divider>=794  && divider<1000)  rc_ac_gop =  80;// 10^3.0=1000   , 10^2.9=794.32
        else if (divider>=630  && divider<794 )  rc_ac_gop =  80;// 10^2.9=794.32 , 10^2.8=630.95
        else if (divider>=501  && divider<630 )  rc_ac_gop =  90;// 10^2.8=630.95 , 10^2.7=501.18
        else if (divider>=398  && divider<501 )  rc_ac_gop =  90;// 10^2.7=501.18 , 10^2.6=398.10
        else if (divider>=316  && divider<398 )  rc_ac_gop = 100;// 10^2.6=398.10 , 10^2.5=316.22
        else if (divider>=215  && divider<316 )  rc_ac_gop = 100;// 10^2.5=316.22 , 10^2.4=215.18
        else if (divider>=199  && divider<215 )  rc_ac_gop = 110;// 10^2.4=215.18 , 10^2.3=199.52
        else if (divider>=158  && divider<199 )  rc_ac_gop = 110;// 10^2.3=199.52 , 10^2.2=158.48
        else                                     rc_ac_gop = 120;// psnr<22db
    }
    else if (bit_rate>=2000000 && bit_rate <3000000) { // 2Mbps ~ 3Mbps
        if      (divider>=10000)                 rc_ac_gop =  50;// psnr>=40db
        else if (divider>=6309 && divider<10000) rc_ac_gop =  50;// 10^4.0=10000  , 10^3.8=6309.57
        else if (divider>=3981 && divider<6309)  rc_ac_gop =  60;// 10^3.8=6309.57, 10^3.6=3981.07
        else if (divider>=3162 && divider<3981)  rc_ac_gop =  70;// 10^3.6=3981.07, 10^3.5=3162.27
        else if (divider>=2511 && divider<3162)  rc_ac_gop =  70;// 10^3.5=3162.27, 10^3.4=2511.88
        else if (divider>=1995 && divider<2511)  rc_ac_gop =  80;// 10^3.4=2511.88, 10^3.3=1995.26
        else if (divider>=1584 && divider<1995)  rc_ac_gop =  80;// 10^3.3=1995.26, 10^3.2=1584.89
        else if (divider>=1258 && divider<1584)  rc_ac_gop =  90;// 10^3.2=1584.89, 10^3.1=1258.92
        else if (divider>=1000 && divider<1258)  rc_ac_gop =  90;// 10^3.1=1258.92, 10^3.0=1000
        else if (divider>=794  && divider<1000)  rc_ac_gop = 100;// 10^3.0=1000   , 10^2.9=794.32
        else if (divider>=630  && divider<794 )  rc_ac_gop = 100;// 10^2.9=794.32 , 10^2.8=630.95
        else if (divider>=501  && divider<630 )  rc_ac_gop = 110;// 10^2.8=630.95 , 10^2.7=501.18
        else if (divider>=398  && divider<501 )  rc_ac_gop = 110;// 10^2.7=501.18 , 10^2.6=398.10
        else if (divider>=316  && divider<398 )  rc_ac_gop = 120;// 10^2.6=398.10 , 10^2.5=316.22
        else if (divider>=215  && divider<316 )  rc_ac_gop = 120;// 10^2.5=316.22 , 10^2.4=215.18
        else if (divider>=199  && divider<215 )  rc_ac_gop = 130;// 10^2.4=215.18 , 10^2.3=199.52
        else if (divider>=158  && divider<199 )  rc_ac_gop = 130;// 10^2.3=199.52 , 10^2.2=158.48
        else                                     rc_ac_gop = 140;// psnr<22db
    }
    else if (bit_rate>=1000000 && bit_rate <2000000) { // 1Mbps ~ 2Mbps
        if      (divider>=10000)                 rc_ac_gop =  60;// psnr>=40db
        else if (divider>=6309 && divider<10000) rc_ac_gop =  60;// 10^4.0=10000  , 10^3.8=6309.57
        else if (divider>=3981 && divider<6309)  rc_ac_gop =  70;// 10^3.8=6309.57, 10^3.6=3981.07
        else if (divider>=3162 && divider<3981)  rc_ac_gop =  80;// 10^3.6=3981.07, 10^3.5=3162.27
        else if (divider>=2511 && divider<3162)  rc_ac_gop =  80;// 10^3.5=3162.27, 10^3.4=2511.88
        else if (divider>=1995 && divider<2511)  rc_ac_gop =  90;// 10^3.4=2511.88, 10^3.3=1995.26
        else if (divider>=1584 && divider<1995)  rc_ac_gop =  90;// 10^3.3=1995.26, 10^3.2=1584.89
        else if (divider>=1258 && divider<1584)  rc_ac_gop = 100;// 10^3.2=1584.89, 10^3.1=1258.92
        else if (divider>=1000 && divider<1258)  rc_ac_gop = 100;// 10^3.1=1258.92, 10^3.0=1000
        else if (divider>=794  && divider<1000)  rc_ac_gop = 110;// 10^3.0=1000   , 10^2.9=794.32
        else if (divider>=630  && divider<794 )  rc_ac_gop = 110;// 10^2.9=794.32 , 10^2.8=630.95
        else if (divider>=501  && divider<630 )  rc_ac_gop = 120;// 10^2.8=630.95 , 10^2.7=501.18
        else if (divider>=398  && divider<501 )  rc_ac_gop = 120;// 10^2.7=501.18 , 10^2.6=398.10
        else if (divider>=316  && divider<398 )  rc_ac_gop = 130;// 10^2.6=398.10 , 10^2.5=316.22
        else if (divider>=215  && divider<316 )  rc_ac_gop = 130;// 10^2.5=316.22 , 10^2.4=215.18
        else if (divider>=199  && divider<215 )  rc_ac_gop = 140;// 10^2.4=215.18 , 10^2.3=199.52
        else if (divider>=158  && divider<199 )  rc_ac_gop = 140;// 10^2.3=199.52 , 10^2.2=158.48
        else                                     rc_ac_gop = 150;// psnr<22db
    }

    return rc_ac_gop;
}

//===== Auto-config IOverPRatio based on sub-function of RCAutoConfig_IOverPRatio =====
void my_v0_rc_params_ac_iopratio( ) {
    int i,j,m,v0_ac_iopratio;

    READ_WORD(V0_FRAME_XY_ADDR,m); //read frame-x & frame-y
        i=(m>>16)&0xffff;
        j=m&0xffff;
        rca.v0_width = i;
        rca.v0_height = j;

    READ_WORD(V0_RCEN_BU_ADDR,i); //read rc_en, rc_mode & bu
        rca.v0_rc_enable = (i>>24)&0x1;
        rca.v0_RCUpdateMode = (i>>16)&0x3;

    READ_WORD(V0_BR_ADDR,i); //read br
        rca.v0_bit_rate = i;

    if (rca.v0_rc_enable==1 && rca.v0_RCUpdateMode==RC_MODE_3) {
        v0_ac_iopratio = RCAutoConfig_IOverPRatio(rca.v0_width, rca.v0_height, rca.v0_bit_rate, rca.v0_RCIoverPRatio, v0_ymse_iframe, v0_ymse_last_p);
    rca.v0_RCIoverPRatio = v0_ac_iopratio;
        READ_WORD(V0_QP_ADDR, i);
        WRITE_WORD(V0_QP_ADDR, (((i>>16)<<16)+(v0_ac_iopratio<<8)+(i&0xff)));
    }
}

//===== Criteria for RC auto config IOverPRatio =====
// 1> Depend on bitrate & comparsion of I frame's psnr and last P frame's psnr in one GOP.
// 2> If I frame's psnr less or equal than last P frame's psnr, the next GOP would level-up iopratio till the I frams's psnr > last P frame's psnr.
// 3> If I frame's psnr bigger than last P frame's psnr, the next GOP would remain iopratio value of current GOP.
// 4> Finally use a high level threshold to clamp final output iopratio value.
int RCAutoConfig_IOverPRatio (int w, int h, int bit_rate, int iopratio, int64 imse, int64 pmse) {
    int64 whm255square;
    int i_divider, p_divider, rc_ac_iopratio;
    int ac_iopratio_high;

    whm255square = (int64)(((w<<8)-w)*((h<<8)-h));
    i_divider = (int)(whm255square/imse);
    p_divider = (int)(whm255square/pmse);

    if (bit_rate>=5000000 && bit_rate <=8000000) { // 5Mbps ~ 8Mbps
        if (i_divider <= p_divider) rc_ac_iopratio = iopratio + 1;
        else                        rc_ac_iopratio = iopratio;
        if      (p_divider>=10000)                   ac_iopratio_high =  2;// psnr>=40db
        else if (p_divider>=6309 && p_divider<10000) ac_iopratio_high =  2;// 10^4.0=10000  , 10^3.8=6309.57
        else if (p_divider>=3981 && p_divider<6309)  ac_iopratio_high =  3;// 10^3.8=6309.57, 10^3.6=3981.07
        else if (p_divider>=2511 && p_divider<3981)  ac_iopratio_high =  4;// 10^3.6=3981.07, 10^3.4=2511.88
        else if (p_divider>=1584 && p_divider<2511)  ac_iopratio_high =  5;// 10^3.4=2511.88, 10^3.2=1584.89
        else if (p_divider>=1000 && p_divider<1584)  ac_iopratio_high =  6;// 10^3.2=1584.89, 10^3.0=1000
        else if (p_divider>=630  && p_divider<1000)  ac_iopratio_high =  7;// 10^3.0=1000   , 10^2.8=630.95
        else if (p_divider>=398  && p_divider<630 )  ac_iopratio_high =  8;// 10^2.8=630.95 , 10^2.6=398.10
        else if (p_divider>=251  && p_divider<398 )  ac_iopratio_high =  9;// 10^2.6=398.10 , 10^2.4=251.18
        else if (p_divider>=158  && p_divider<251 )  ac_iopratio_high = 10;// 10^2.4=251.18 , 10^2.2=158.48
        else                                         ac_iopratio_high = 11;// psnr<22db

        rc_ac_iopratio = my_iequmin (rc_ac_iopratio, ac_iopratio_high);
    }
    else if (bit_rate>=3000000 && bit_rate <5000000) { // 3Mbps ~ 5Mbps
        if (i_divider <= p_divider) rc_ac_iopratio = iopratio + 1;
        else                        rc_ac_iopratio = iopratio;
        if      (p_divider>=10000)                   ac_iopratio_high =  3;// psnr>=40db
        else if (p_divider>=6309 && p_divider<10000) ac_iopratio_high =  3;// 10^4.0=10000  , 10^3.8=6309.57
        else if (p_divider>=3981 && p_divider<6309)  ac_iopratio_high =  4;// 10^3.8=6309.57, 10^3.6=3981.07
        else if (p_divider>=2511 && p_divider<3981)  ac_iopratio_high =  5;// 10^3.6=3981.07, 10^3.4=2511.88
        else if (p_divider>=1584 && p_divider<2511)  ac_iopratio_high =  6;// 10^3.4=2511.88, 10^3.2=1584.89
        else if (p_divider>=1000 && p_divider<1584)  ac_iopratio_high =  7;// 10^3.2=1584.89, 10^3.0=1000
        else if (p_divider>=630  && p_divider<1000)  ac_iopratio_high =  8;// 10^3.0=1000   , 10^2.8=630.95
        else if (p_divider>=398  && p_divider<630 )  ac_iopratio_high =  9;// 10^2.8=630.95 , 10^2.6=398.10
        else if (p_divider>=251  && p_divider<398 )  ac_iopratio_high = 10;// 10^2.6=398.10 , 10^2.4=251.18
        else if (p_divider>=158  && p_divider<251 )  ac_iopratio_high = 11;// 10^2.4=251.18 , 10^2.2=158.48
        else                                         ac_iopratio_high = 12;// psnr<22db

    rc_ac_iopratio = my_iequmin (rc_ac_iopratio, ac_iopratio_high);
    }
    else if (bit_rate>=2000000 && bit_rate <3000000) { // 2Mbps ~ 3Mbps
        if (i_divider <= p_divider) rc_ac_iopratio = iopratio + 1;
        else                        rc_ac_iopratio = iopratio;
        if      (p_divider>=10000)                   ac_iopratio_high =  4;// psnr>=40db
        else if (p_divider>=6309 && p_divider<10000) ac_iopratio_high =  4;// 10^4.0=10000  , 10^3.8=6309.57
        else if (p_divider>=3981 && p_divider<6309)  ac_iopratio_high =  5;// 10^3.8=6309.57, 10^3.6=3981.07
        else if (p_divider>=2511 && p_divider<3981)  ac_iopratio_high =  6;// 10^3.6=3981.07, 10^3.4=2511.88
        else if (p_divider>=1584 && p_divider<2511)  ac_iopratio_high =  7;// 10^3.4=2511.88, 10^3.2=1584.89
        else if (p_divider>=1000 && p_divider<1584)  ac_iopratio_high =  8;// 10^3.2=1584.89, 10^3.0=1000
        else if (p_divider>=630  && p_divider<1000)  ac_iopratio_high =  9;// 10^3.0=1000   , 10^2.8=630.95
        else if (p_divider>=398  && p_divider<630 )  ac_iopratio_high = 10;// 10^2.8=630.95 , 10^2.6=398.10
        else if (p_divider>=251  && p_divider<398 )  ac_iopratio_high = 11;// 11^2.6=398.10 , 10^2.4=251.18
        else if (p_divider>=158  && p_divider<251 )  ac_iopratio_high = 12;// 10^2.4=251.18 , 10^2.2=158.48
        else                                         ac_iopratio_high = 13;// psnr<22db

    rc_ac_iopratio = my_iequmin (rc_ac_iopratio, ac_iopratio_high);
    }
    else if (bit_rate>=1000000 && bit_rate <2000000) { // 1Mbps ~ 2Mbps
        if (i_divider <= p_divider) rc_ac_iopratio = iopratio + 1;
        else                        rc_ac_iopratio = iopratio;
        if      (p_divider>=10000)                   ac_iopratio_high =  5;// psnr>=40db
        else if (p_divider>=6309 && p_divider<10000) ac_iopratio_high =  5;// 10^4.0=10000  , 10^3.8=6309.57
        else if (p_divider>=3981 && p_divider<6309)  ac_iopratio_high =  6;// 10^3.8=6309.57, 10^3.6=3981.07
        else if (p_divider>=2511 && p_divider<3981)  ac_iopratio_high =  7;// 10^3.6=3981.07, 10^3.4=2511.88
        else if (p_divider>=1584 && p_divider<2511)  ac_iopratio_high =  8;// 10^3.4=2511.88, 10^3.2=1584.89
        else if (p_divider>=1000 && p_divider<1584)  ac_iopratio_high =  9;// 10^3.2=1584.89, 10^3.0=1000
        else if (p_divider>=630  && p_divider<1000)  ac_iopratio_high = 10;// 10^3.0=1000   , 10^2.8=630.95
        else if (p_divider>=398  && p_divider<630 )  ac_iopratio_high = 11;// 10^2.8=630.95 , 10^2.6=398.10
        else if (p_divider>=251  && p_divider<398 )  ac_iopratio_high = 12;// 10^2.6=398.10 , 10^2.4=251.18
        else if (p_divider>=158  && p_divider<251 )  ac_iopratio_high = 13;// 10^2.4=251.18 , 10^2.2=158.48
        else                                         ac_iopratio_high = 14;// psnr<22db

    rc_ac_iopratio = my_iequmin (rc_ac_iopratio, ac_iopratio_high);
    }

    return rc_ac_iopratio;
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

  if(rca.v0_intra_period==1)
    rca.v0_RCUpdateMode = RC_MODE_1;

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
     case RC_MODE_0: my_v0_updateQP = my_v0_updateQPRC0; break;
     case RC_MODE_1: my_v0_updateQP = my_v0_updateQPRC1; break;
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
  int OverBits,denom;
  int GOPDquant;
  int gop_bits;

    //if(rca.v0_RCUpdateMode != RC_MODE_0) {// lhugop
    //  my_v0_rc_init_seq( );
    //}
    // bit allocation for RC_MODE_3
    if(rca.v0_RCUpdateMode == RC_MODE_3) // running this only once !!!
    {
        // calculate allocated bits for each type of frame
        gop_bits = (!rca.v0_intra_period? 1:rca.v0_intra_period) * (rca.v0_bit_rate/rca.v0_framerate);
        denom = 1;
        
        if(rca.v0_intra_period>=1)
        {
            denom *= rca.v0_intra_period;
            denom += rca.v0_RCISliceBitRatio - 1;
        }
        // set bit targets for each type of frame
//18      rca.RCPSliceBits = (int)floor(gop_bits/denom + 0.5F);
        rca.v0_RCPSliceBits = gop_bits/denom ;
        rca.v0_RCISliceBits = (rca.v0_intra_period)? (rca.v0_RCISliceBitRatio * rca.v0_RCPSliceBits) : 0;

        //rca.v0_NISlice = (rca.v0_intra_period)? (rca.v0_no_frm_base/rca.v0_intra_period):0; // totoal I-frame number
        //rca.v0_NPSlice = rca.v0_no_frm_base - rca.v0_NISlice;
        rca.v0_NISlice = 1;//lhugop, only one I frame in one GOP
        rca.v0_NPSlice = rca.v0_intra_period - rca.v0_NISlice;//lhugop
    }

    // check if the last GOP over uses its budget. If yes, the initial QP of the I frame in
    // the coming  GOP will be increased.
    if(rca.v0_RemainingBits<0)
        Overum=TRUE;
    OverBits=-rca.v0_RemainingBits;

    //initialize the lower bound and the upper bound for the target bits of each frame, HRD consideration
    rca.v0_LowerBound  = rca.v0_RemainingBits + (rca.v0_bit_rate/rca.v0_framerate);
    rca.v0_UpperBound1 = rca.v0_RemainingBits + (rca.v0_bit_rate<<1); //2.048

    //compute the total number of bits for the current GOP
    gop_bits = (1+np)*(rca.v0_bit_rate/rca.v0_framerate);
    rca.v0_RemainingBits += gop_bits;
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

        // QP is constrained by QP of previous QP
        rca.v0_PAverageQp = my_iClip3(rca.v0_QPLastGOP-2, rca.v0_QPLastGOP+2, rca.v0_PAverageQp);
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
    if ( rca.v0_type==P_SLICE || ((rca.v0_RCUpdateMode==RC_MODE_1 || (rca.v0_type==I_SLICE&&rca.v0_RCUpdateMode==RC_MODE_3)) && (!(rca.v0_gop_cnt==0 && rca.v0_frame_cnt==0))) ) // lhulhu
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
       if(rca.v0_BasicUnit==rca.v0_FrameSizeInMbs || (rca.v0_RCUpdateMode==RC_MODE_3) )
       {
          if(rca.v0_NumberofCodedPFrame>0)
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
                    //rca.v0_Target = rca.v0_Target/(rca.v0_RCIoverPRatio<<2); //lhulhu
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
      if(rca.v0_RCUpdateMode!=RC_MODE_3 || rca.v0_type==P_SLICE)
        rca.v0_Target = my_iClip3(rca.v0_LowerBound, rca.v0_UpperBound2, rca.v0_Target);
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
  if( rca.v0_type==P_SLICE || ((rca.v0_RCUpdateMode==RC_MODE_1 || (rca.v0_type==I_SLICE&&rca.v0_RCUpdateMode==RC_MODE_3)) && (!(rca.v0_gop_cnt==0&&rca.v0_frame_cnt==0))) ) //lhulhu
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
    rca.v0_rc_tmp3[0] = (rca.v0_m_rgQp_8p[0]>>8)*(rca.v0_m_rgRp_8p[0]>>8);
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
        error_0p[i] = rca.v0_m_X1_8p/rca.v0_m_rgQp_8p[i] + (rca.v0_m_X2_8p/rca.v0_rc_tmp4[i]) - (rca.v0_m_rgRp_8p[i]>>8);
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

  // default RD model estimation results
  rca.v0_m_X1_8p = 0;
  rca.v0_m_X2_8p = 0;

  for(i=0;i<n_windowSize;i++) {// if all non-rejected Q are the same, take 1st order model
    if((rca.v0_m_rgQp_8p[i]!=rca.v0_m_rgQp_8p[0]) && !rc_rgRejected[i]) {
      estimateX2 = TRUE;
      if(n_realSize>=1)
        break;
    }
    if(!rc_rgRejected[i])
//a      rca.m_X1_8p += ((rca.m_rgQp_8p[i]>>4) * (rca.m_rgRp_8p[i]>>4)) / n_realSize;
      rca.v0_m_X1_8p += rca.v0_rc_tmp0[i] / n_realSize;
  }

  // take 2nd order model to estimate X1 and X2
  if ((n_realSize >= 1) && estimateX2)
  {
    a00_20p = n_realSize<<20;
    for (i = 0; i < n_windowSize; i++)
    {
      if (!rc_rgRejected[i])
      {
        a01_20p += rca.v0_rc_tmp1[i];
        a11_20p += rca.v0_rc_tmp2[i];
        b0_0p   += rca.v0_rc_tmp3[i];
        b1_0p   += rca.v0_m_rgRp_8p[i]>>8;
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
  if( rca.v0_type==P_SLICE || ((rca.v0_RCUpdateMode==RC_MODE_1 || (rca.v0_type==I_SLICE&&rca.v0_RCUpdateMode==RC_MODE_3)) && (!(rca.v0_gop_cnt==0&&rca.v0_frame_cnt==0))) ) //lhulhu
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
    if( rca.v0_type==P_SLICE || ((rca.v0_RCUpdateMode==RC_MODE_1 || (rca.v0_type==I_SLICE&&rca.v0_RCUpdateMode==RC_MODE_3)) && (!(rca.v0_gop_cnt==0&&rca.v0_frame_cnt==0))) ) {//lhulhu
      if (rca.v0_CurrentMAD_8p==0) rca.v0_PreviousMAD_8p=(1<<8);// lhumad, make fake for dividing by zero when PreviousMAD equal to 0
      else                         rca.v0_PreviousMAD_8p = rca.v0_CurrentMAD_8p;
    }

    // initial MAD model estimator
    my_v0_MADModelEstimator (n_windowSize, n_windowSize, rca.v0_mad_rgRejected);

    // remove outlier
    for (i = 0; i < n_windowSize; i++)
    {
      //error[i] = rca.v0_MADPictureC1 * rca.v0_ReferenceMAD[i] + rca.v0_MADPictureC2 - rca.v0_PictureMAD[i];
      error_8p[i] = ((rca.v0_MADPictureC1_12p*rca.v0_ReferenceMAD_8p[i])>>12) + (rca.v0_MADPictureC2_12p>>4) - rca.v0_PictureMAD_8p[i];
      std_16p += error_8p[i]*error_8p[i];
    }

    threshold_8p = (n_windowSize==2)? 0:my_sqrt64(std_16p/n_windowSize);

    n_realSize = n_windowSize;
    for(i=1; i<n_windowSize; i++)
    {
      if(abs(error_8p[i]) > threshold_8p)
        rca.v0_mad_rgRejected[i] = TRUE;
        n_realSize--;
    }

    // second MAD model estimator
    my_v0_MADModelEstimator(n_realSize, n_windowSize, rca.v0_mad_rgRejected);
  }
}


void my_v0_MADModelEstimator(int n_realSize, int n_windowSize, char *mad_rgRejected)
{
  int     i;
//2  int     oneSampleQ_8p=0;
  int     MatrixValue_4p;
  Boolean estimateX2=FALSE;
  unsigned int a00_8p=0,a01_8p=0,a11_8p=0,b0_8p=0,b1_8p=0; //b

    // default MAD model estimation results
    rca.v0_MADPictureC1_12p = 0;
    rca.v0_MADPictureC2_12p = 0;
    rca.v0_c1_over = 0;

    for(i=0;i<n_windowSize;i++) {// if all non-rejected MAD are the same, take 1st order model
        if((rca.v0_PictureMAD_8p[i]!=rca.v0_PictureMAD_8p[0]) && !mad_rgRejected[i])
        {
            estimateX2 = TRUE;
            if(n_realSize>=1)
                break;
        }
        if(!mad_rgRejected[i]) {
//b            rca.MADPictureC1_12p += ((rca.PictureMAD_8p[i]<<12) / rca.ReferenceMAD_8p[i]) /n_realSize;
            rca.v0_MADPictureC1_12p += rca.v0_mad_tmp0[i]/n_realSize;
            if(rca.v0_mad_tmp0_valid[i] == 0)
                rca.v0_c1_over = 1;
        }
    }

    // take 2nd order model to estimate X1 and X2
    if((n_realSize>=1) && estimateX2) {

        a00_8p = n_realSize<<8;
        for(i=0;i<n_windowSize;i++) {
            if(!mad_rgRejected[i]) {
                a01_8p += rca.v0_ReferenceMAD_8p[i];
                a11_8p += rca.v0_mad_tmp1[i];
                b0_8p  += rca.v0_PictureMAD_8p[i];
                b1_8p  += rca.v0_mad_tmp2[i];
           }
        }
        // solve the equation of AX = B
        MatrixValue_4p = ((long long)a00_8p*(long long)a11_8p - (long long)a01_8p*(long long)a01_8p + (1<<11))>>12;

        if(MatrixValue_4p != 0) { //if(fabs(MatrixValue) > 0.000001)
            rca.v0_MADPictureC2_12p = ((long long)b0_8p*(long long)a11_8p - (long long)b1_8p*(long long)a01_8p)/MatrixValue_4p;
            rca.v0_MADPictureC1_12p = ((long long)b1_8p*(long long)a00_8p - (long long)b0_8p*(long long)a01_8p)/MatrixValue_4p;
        }
        else {
            if (a01_8p==0) {// lhumad, make fake for dividing by zero when a01_8p equal to 0
                rca.v0_MADPictureC1_12p = (long long)b0_8p<<12;
                rca.v0_cmadequ0 = 1;
            }
            else {
                rca.v0_MADPictureC1_12p = ((long long)b0_8p<<12)/a01_8p;
                rca.v0_cmadequ0 = 0;
            }
            rca.v0_MADPictureC2_12p = 0;
        }
        rca.v0_c1_over = 0;
    }
    //if(rca.v0_type==P_SLICE)//g1 || (rca.v0_RCUpdateMode==RC_MODE_1 && (rca.v0_gop_cnt!=0 || rca.v0_frame_cnt!=0)))  //(rca.v0_number != 0)
    if( rca.v0_type==P_SLICE || ((rca.v0_RCUpdateMode==RC_MODE_1 || (rca.v0_type==I_SLICE&&rca.v0_RCUpdateMode==RC_MODE_3)) && (!(rca.v0_gop_cnt==0&&rca.v0_frame_cnt==0))) ) //lhulhu
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
//
//////////////////////////////////////////////////////////////////////////////////////
int my_v0_updateQPRC1( )
{
  int m_Bits;
  int SumofBasicUnit;
  int MaxQpChange, m_Qp, m_Hp;

  /* frame layer rate control */
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

        /* predict the MAD of current picture*/
//20        rca.v0_CurrentMAD=rca.v0_MADPictureC1*rca.v0_PreviousPictureMAD + rca.v0_MADPictureC2;
        rca.v0_CurrentMAD_8p = (rca.v0_MADPictureC1_12p*rca.v0_PreviousPictureMAD_8p)/(1<<12) + rca.v0_MADPictureC2_12p/(1<<4);

        /*compute the number of bits for the texture*/
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
  /*basic unit layer rate control*/
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

        /*the average QP of the previous frame is used to coded the first basic unit of the current frame or field*/
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
}


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
  if( rca.v0_BasicUnit == rca.v0_FrameSizeInMbs ) //lhulhu
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
        rca.v0_CurrentMAD_8p=(rca.v0_MADPictureC1_12p*rca.v0_PreviousPictureMAD_8p)/(1<<12) + rca.v0_MADPictureC2_12p/(1<<4);

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
    else if( rca.v0_type == P_SLICE || rca.v0_type == I_SLICE ) //lhulhu
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
            //if((rca.v0_bu_cnt==(rca.v0_TotalNumberofBasicUnit-1)) && rca.v0_type==P_SLICE)
            if((rca.v0_bu_cnt==(rca.v0_TotalNumberofBasicUnit-1)) && (rca.v0_type==P_SLICE || rca.v0_type==I_SLICE) ) //lhulhu
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
  if(rca.v0_basicunit>=rca.v0_MBPerRow)
    rca.v0_m_Qc = my_imin(rca.v0_m_Qc, rca.v0_PAveFrameQP+6);
  else
    rca.v0_m_Qc = my_imin(rca.v0_m_Qc, rca.v0_PAveFrameQP+3);

  rca.v0_TotalFrameQP +=rca.v0_m_Qc;
  if(rca.v0_bu_cnt==(rca.v0_TotalNumberofBasicUnit-1)) //(rca.v0_NumberofBasicUnit==0)
  {
//18    PAverageQP=(int)((double)rca.v0_TotalFrameQP/(double)rca.v0_TotalNumberofBasicUnit+0.5);
    PAverageQP=(rca.v0_TotalFrameQP+(rca.v0_TotalNumberofBasicUnit>>1))/rca.v0_TotalNumberofBasicUnit;
    if(rca.v0_frame_cnt==(rca.v0_intra_period-1)) //(rca.v0_NumberofPPicture == (rca.v0_intra_period - 2))
      rca.v0_QPLastPFrame = PAverageQP;
    if (rca.v0_type == P_SLICE) // not increase TotalQpforPPicture for I_SLICE, lhulhu
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
  if (rca.v0_type == P_SLICE) // not increase TotalQpforPPicture for I_SLICE, lhulhu
    rca.v0_TotalQpforPPicture +=PAverageQP;
  rca.v0_PAveFrameQP=PAverageQP;
  rca.v0_PAveHeaderBits3=rca.v0_PAveHeaderBits2;
}


void my_v0_predictCurrPicMAD( )
{
    int i,CurrentBUMAD_8p;

    //rca.CurrentMAD=rca.MADPictureC1*rca.BUPFMAD[rca.bu_cnt]+rca.MADPictureC2;
//s    rca.CurrentMAD_8p=((rca.MADPictureC1_12p>>4)*((*(pp_BUPFMAD_8p+rca.bu_cnt))>>8)) + (rca.MADPictureC2_12p>>4);
	rca.v0_CurrentMAD_8p=((rca.v0_MADPictureC1_12p>>4)*(rca.v0_BUPFMAD_8p[rca.v0_bu_cnt]>>8)) + (rca.v0_MADPictureC2_12p>>4);
    rca.v0_TotalBUMAD_12p=0;

    for(i=rca.v0_TotalNumberofBasicUnit-1; i>=rca.v0_bu_cnt; i--)
    {
      //CurrentBUMAD = rca.MADPictureC1*rca.BUPFMAD[i]+rca.MADPictureC2;
//s      CurrentBUMAD_8p = ((rca.MADPictureC1_12p>>4)*((*(pp_BUPFMAD_8p+i))>>8)) + (rca.MADPictureC2_12p>>4);
	  CurrentBUMAD_8p = ((rca.v0_MADPictureC1_12p>>4)*(rca.v0_BUPFMAD_8p[i]>>8)) + (rca.v0_MADPictureC2_12p>>4);
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
  rca.v0_m_Qc = my_imin(m_Qp+rca.v0_DDquant,  rca.v0_m_Qc); // control variation

  if(rca.v0_basicunit>=rca.v0_MBPerRow)
    rca.v0_m_Qc = my_imin(rca.v0_PAveFrameQP+6, rca.v0_m_Qc);
  else
    rca.v0_m_Qc = my_imin(rca.v0_PAveFrameQP+3, rca.v0_m_Qc);

  if(rca.v0_c1_over==1 || rca.v0_cmadequ0==1) // lhumad
    rca.v0_m_Qc = my_imin(m_Qp-rca.v0_DDquant, rca.v0_RCMaxQP); // clipping
  else
    rca.v0_m_Qc = my_iClip3(m_Qp-rca.v0_DDquant, rca.v0_RCMaxQP, rca.v0_m_Qc); // clipping

  if(rca.v0_basicunit>=rca.v0_MBPerRow)
    rca.v0_m_Qc = my_imax(rca.v0_PAveFrameQP-6, rca.v0_m_Qc);
  else
    rca.v0_m_Qc = my_imax(rca.v0_PAveFrameQP-3, rca.v0_m_Qc);

  rca.v0_m_Qc = my_imax(rca.v0_RCMinQP, rca.v0_m_Qc);
}


void my_v0_updateModelQPFrame( int m_Bits )
{
  long long dtmp_8p;
  int tmp_4p=0;
  int m_Qstep_8p;

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
    m_Qstep_8p = (2*((long long)rca.v0_m_X2_8p)*((long long)rca.v0_CurrentMAD_8p)) / ((tmp_4p<<4) - (rca.v0_m_X1_8p>>4)*(rca.v0_CurrentMAD_8p>>4));
  }

  rca.v0_m_Qc = Qstep2QP_8p(m_Qstep_8p);
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
    if(rca.v0_bu_cnt==0) { //frame over
        rca.v0_frame_mad   = 0;
        rca.v0_frame_tbits = 0;
        rca.v0_frame_hbits = 0;
    }
    else {
        rca.v0_frame_mad   += v0_mad_tmp;
        rca.v0_frame_tbits += v0_tbits_tmp;
        rca.v0_frame_hbits += v0_hbits_tmp;
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
    int np;

    if(rca.v0_RCUpdateMode==RC_MODE_1 || rca.v0_RCUpdateMode==RC_MODE_3)
    {
        if(rca.v0_frame_cnt==0 && rca.v0_bu_cnt==0) //
        {
            np = rca.v0_intra_period - 1;
            my_v0_rc_init_GOP( np );
        }
    }
    else if((rca.v0_RCUpdateMode==RC_MODE_0)|(rca.v0_RCUpdateMode==RC_MODE_2))
    {
        if(rca.v0_frame_cnt==0 && rca.v0_bu_cnt==0) // && rca.mb_cnt==0)
        {
            np = rca.v0_intra_period - 1;
            my_v0_rc_init_GOP( np );
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
        if(rca.v0_gop_cnt==0 && rca.v0_frame_cnt==0) //lhulhu, let non RC_MODE_1 can handle I_SLICE
        {
            rca.v0_qp = rca.v0_MyInitialQp;
        }
        //else if (rca.v0_type == P_SLICE) //g1 || rca.RCUpdateMode == RC_MODE_1 )
        else if (rca.v0_type == P_SLICE || rca.v0_RCUpdateMode==RC_MODE_1 || (rca.v0_type==I_SLICE && rca.v0_RCUpdateMode==RC_MODE_3)) //lhulhu
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


    if(rca.v0_basicunit < rca.v0_FrameSizeInMbs) // bu-level counter
    {
        if(rca.v0_bu_cnt==(rca.v0_TotalNumberofBasicUnit-1))
        {
            rca.v0_bu_cnt=0;
            if(rca.v0_frame_cnt==(rca.v0_intra_period-1))
            {
                rca.v0_frame_cnt=0;
                if(rca.v0_gop_cnt<=1000)
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
            if(rca.v0_gop_cnt<=1000)
                rca.v0_gop_cnt++;
        }
        else
            rca.v0_frame_cnt++;
    }


    my_v0_rc_update_frame_stats(); // computer frame parameters

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
        rca.v1_RCEnableAutoConfigGOP = i&0x1;
        rca.v1_RCEnableAutoConfigIOPRatio = (i>>1)&0x1;
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

#ifdef ARMCM7_RC  //###########
//===== Auto-config GOP based on sub-function of RCAutoConfig_GOP =====
void my_v1_rc_params_ac_gop( ) {
    int i,j,m,v1_autoconfig_gop;

    READ_WORD(V1_FRAME_XY_ADDR,m); //read frame-x & frame-y
        i=(m>>16)&0xffff;
        j=m&0xffff;
        rca.v1_width = i;
        rca.v1_height = j;

    READ_WORD(V1_RCEN_BU_ADDR,i); //read rc_en, rc_mode & bu
        rca.v1_rc_enable = (i>>24)&0x1;
        rca.v1_RCUpdateMode = (i>>16)&0x3;

    READ_WORD(V1_BR_ADDR,i); //read br
        rca.v1_bit_rate = i;

    if (rca.v1_rc_enable==1 && rca.v1_RCUpdateMode==RC_MODE_3) {
        v1_autoconfig_gop = RCAutoConfig_GOP(rca.v1_width, rca.v1_height, rca.v1_bit_rate, v1_ymse_last_p);
        rca.v1_intra_period = v1_autoconfig_gop;
        READ_WORD(V1_QP_ADDR, i);
        WRITE_WORD(V1_QP_ADDR, (((i>>8)<<8)+v1_autoconfig_gop));
    }
}

//===== Auto-config IOverPRatio based on sub-function of RCAutoConfig_IOverPRatio =====
void my_v1_rc_params_ac_iopratio( ) {
    int i,j,m,v1_ac_iopratio;

    READ_WORD(V1_FRAME_XY_ADDR,m); //read frame-x & frame-y
        i=(m>>16)&0xffff;
        j=m&0xffff;
        rca.v1_width = i;
        rca.v1_height = j;

    READ_WORD(V1_RCEN_BU_ADDR,i); //read rc_en, rc_mode & bu
        rca.v1_rc_enable = (i>>24)&0x1;
        rca.v1_RCUpdateMode = (i>>16)&0x3;

    READ_WORD(V1_BR_ADDR,i); //read br
        rca.v1_bit_rate = i;

    if (rca.v1_rc_enable==1 && rca.v1_RCUpdateMode==RC_MODE_3) {
        v1_ac_iopratio = RCAutoConfig_IOverPRatio(rca.v1_width, rca.v1_height, rca.v1_bit_rate, rca.v1_RCIoverPRatio, v1_ymse_iframe, v1_ymse_last_p);
    rca.v1_RCIoverPRatio = v1_ac_iopratio;
        READ_WORD(V1_QP_ADDR, i);
        WRITE_WORD(V1_QP_ADDR, (((i>>16)<<16)+(v1_ac_iopratio<<8)+(i&0xff)));
    }
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

  if(rca.v1_intra_period==1)
    rca.v1_RCUpdateMode = RC_MODE_1;

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
     case RC_MODE_0: my_v1_updateQP = my_v1_updateQPRC0; break;
     case RC_MODE_1: my_v1_updateQP = my_v1_updateQPRC1; break;
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
  int OverBits,denom;
  int GOPDquant;
  int gop_bits;

    //if(rca.v1_RCUpdateMode != RC_MODE_0) {// lhugop
    //  my_v1_rc_init_seq( );
    //}
    // bit allocation for RC_MODE_3
    if(rca.v1_RCUpdateMode == RC_MODE_3) // running this only once !!!
    {
        // calculate allocated bits for each type of frame
        gop_bits = (!rca.v1_intra_period? 1:rca.v1_intra_period)* (rca.v1_bit_rate/rca.v1_framerate);
        denom = 1;
        
        if(rca.v1_intra_period>=1)
        {
            denom *= rca.v1_intra_period;
            denom += rca.v1_RCISliceBitRatio - 1;
        }
        // set bit targets for each type of frame
//18      rca.RCPSliceBits = (int)floor(gop_bits/denom + 0.5F);
        rca.v1_RCPSliceBits = gop_bits/denom ;
        rca.v1_RCISliceBits = (rca.v1_intra_period)? (rca.v1_RCISliceBitRatio * rca.v1_RCPSliceBits) : 0;

        //rca.v1_NISlice = (rca.v1_intra_period)? (rca.v1_no_frm_base/rca.v1_intra_period):0; // totoal I-frame number
        //rca.v1_NPSlice = rca.v1_no_frm_base - rca.v1_NISlice;
        rca.v1_NISlice = 1;//lhugop, only one I frame in one GOP
        rca.v1_NPSlice = rca.v1_intra_period - rca.v1_NISlice;//lhugop
    }

    // check if the last GOP over uses its budget. If yes, the initial QP of the I frame in
    // the coming  GOP will be increased.
    if(rca.v1_RemainingBits<0)
        Overum=TRUE;
    OverBits=-rca.v1_RemainingBits;

    //initialize the lower bound and the upper bound for the target bits of each frame, HRD consideration
    rca.v1_LowerBound  = rca.v1_RemainingBits + (rca.v1_bit_rate/rca.v1_framerate);
    rca.v1_UpperBound1 = rca.v1_RemainingBits + (rca.v1_bit_rate<<1); //2.048

    //compute the total number of bits for the current GOP
    gop_bits = (1+np)*(rca.v1_bit_rate/rca.v1_framerate);
    rca.v1_RemainingBits += gop_bits;
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
        // QP is constrained by QP of previous QP
        rca.v1_PAverageQp = my_iClip3(rca.v1_QPLastGOP-2, rca.v1_QPLastGOP+2, rca.v1_PAverageQp);
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
    if ( rca.v1_type==P_SLICE || ((rca.v1_RCUpdateMode==RC_MODE_1 || (rca.v1_type==I_SLICE&&rca.v1_RCUpdateMode==RC_MODE_3)) && (!(rca.v1_gop_cnt==0 && rca.v1_frame_cnt==0))) ) // lhulhu
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
        if(rca.v1_BasicUnit==rca.v1_FrameSizeInMbs || (rca.v1_RCUpdateMode==RC_MODE_3) )
        {
          if(rca.v1_NumberofCodedPFrame>0)
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
                    //rca.v1_Target = rca.v1_Target/(rca.v1_RCIoverPRatio<<2);// lhulhu
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
      if(rca.v1_RCUpdateMode!=RC_MODE_3 || rca.v1_type==P_SLICE)
        rca.v1_Target = my_iClip3(rca.v1_LowerBound, rca.v1_UpperBound2, rca.v1_Target);
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
  if( rca.v1_type==P_SLICE || ((rca.v1_RCUpdateMode==RC_MODE_1 || (rca.v1_type==I_SLICE&&rca.v1_RCUpdateMode==RC_MODE_3)) && (!(rca.v1_gop_cnt==0&&rca.v1_frame_cnt==0))) ) //lhulhu
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
    rca.v1_rc_tmp3[0] = (rca.v1_m_rgQp_8p[0]>>8)*(rca.v1_m_rgRp_8p[0]>>8);
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
        error_0p[i] = rca.v1_m_X1_8p/rca.v1_m_rgQp_8p[i] + (rca.v1_m_X2_8p/rca.v1_rc_tmp4[i]) - (rca.v1_m_rgRp_8p[0]>>8);
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

  // default RD model estimation results
  rca.v1_m_X1_8p = 0;
  rca.v1_m_X2_8p = 0;

  for(i=0;i<n_windowSize;i++) {// if all non-rejected Q are the same, take 1st order model
    if((rca.v1_m_rgQp_8p[i]!=rca.v1_m_rgQp_8p[0]) && !rc_rgRejected[i]) {
      estimateX2 = TRUE;
      if(n_realSize>=1)
        break;
    }
    if(!rc_rgRejected[i])
//a      rca.m_X1_8p += ((rca.m_rgQp_8p[i]>>4) * (rca.m_rgRp_8p[i]>>4)) / n_realSize;
      rca.v1_m_X1_8p += rca.v1_rc_tmp0[i] / n_realSize;
  }

  // take 2nd order model to estimate X1 and X2
  if ((n_realSize >= 1) && estimateX2)
  {
    a00_20p = n_realSize<<20;
    for (i = 0; i < n_windowSize; i++)
    {
      if (!rc_rgRejected[i])
      {
        a01_20p += rca.v1_rc_tmp1[i];
        a11_20p += rca.v1_rc_tmp2[i];
        b0_0p   += rca.v1_rc_tmp3[i];
        b1_0p   += rca.v1_m_rgRp_8p[i]>>8;
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
  if( rca.v1_type==P_SLICE || ((rca.v1_RCUpdateMode==RC_MODE_1 || (rca.v1_type==I_SLICE&&rca.v1_RCUpdateMode==RC_MODE_3)) && (!(rca.v1_gop_cnt==0&&rca.v1_frame_cnt==0))) ) //lhulhu
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
    if( rca.v1_type==P_SLICE || ((rca.v1_RCUpdateMode==RC_MODE_1 || (rca.v1_type==I_SLICE&&rca.v1_RCUpdateMode==RC_MODE_3)) && (!(rca.v1_gop_cnt==0&&rca.v1_frame_cnt==0))) ) {//lhulhu
      if (rca.v1_CurrentMAD_8p==0) rca.v1_PreviousMAD_8p=(1<<8);// lhumad, make fake for dividing by zero when PreviousMAD equal to 0
      else                         rca.v1_PreviousMAD_8p = rca.v1_CurrentMAD_8p;
    }

    // initial MAD model estimator
    my_v1_MADModelEstimator (n_windowSize, n_windowSize, rca.v1_mad_rgRejected);

    // remove outlier
    for (i = 0; i < n_windowSize; i++)
    {
      //error[i] = rca.v1_MADPictureC1 * rca.v1_ReferenceMAD[i] + rca.v1_MADPictureC2 - rca.v1_PictureMAD[i];
      error_8p[i] = ((rca.v1_MADPictureC1_12p*rca.v1_ReferenceMAD_8p[i])>>12) + (rca.v1_MADPictureC2_12p>>4) - rca.v1_PictureMAD_8p[i];
      std_16p += error_8p[i]*error_8p[i];
    }

    threshold_8p = (n_windowSize==2)? 0:my_sqrt64(std_16p/n_windowSize);

    n_realSize = n_windowSize;
    for(i=1; i<n_windowSize; i++)
    {
      if(abs(error_8p[i]) > threshold_8p)
        rca.v1_mad_rgRejected[i] = TRUE;
        n_realSize--;
    }

    // second MAD model estimator
    my_v1_MADModelEstimator(n_realSize, n_windowSize, rca.v1_mad_rgRejected);
  }
}


void my_v1_MADModelEstimator(int n_realSize, int n_windowSize, char *mad_rgRejected)
{
  int     i;
  long long MatrixValue_4p;
  Boolean estimateX2=FALSE;
  unsigned int a00_8p=0,a01_8p=0,a11_8p=0,b0_8p=0,b1_8p=0; //b

    // default MAD model estimation results
    rca.v1_MADPictureC1_12p = 0;
    rca.v1_MADPictureC2_12p = 0;
    rca.v1_c1_over = 0;

    for(i=0;i<n_windowSize;i++) {// if all non-rejected MAD are the same, take 1st order model
        if((rca.v1_PictureMAD_8p[i]!=rca.v1_PictureMAD_8p[0]) && !mad_rgRejected[i])
        {
            estimateX2 = TRUE;
            if(n_realSize>=1)
                break;
        }
        if(!mad_rgRejected[i]) {
//b            rca.MADPictureC1_12p += ((rca.PictureMAD_8p[i]<<12) / rca.ReferenceMAD_8p[i]) /n_realSize;
            rca.v1_MADPictureC1_12p += rca.v1_mad_tmp0[i]/n_realSize;
            if(rca.v1_mad_tmp0_valid[i] == 0)
                rca.v1_c1_over = 1;
        }
    }

    // take 2nd order model to estimate X1 and X2
    if((n_realSize>=1) && estimateX2) {

        a00_8p = n_realSize<<8;
        for(i=0;i<n_windowSize;i++) {
            if(!mad_rgRejected[i]) {
                a01_8p += rca.v1_ReferenceMAD_8p[i];
                a11_8p += rca.v1_mad_tmp1[i];
                b0_8p  += rca.v1_PictureMAD_8p[i];
                b1_8p  += rca.v1_mad_tmp2[i];
           }
        }
        // solve the equation of AX = B
        MatrixValue_4p = ((long long)a00_8p*(long long)a11_8p - (long long)a01_8p*(long long)a01_8p + (1<<11))>>12;

        if(MatrixValue_4p != 0) { //if(fabs(MatrixValue) > 0.000001)
            rca.v1_MADPictureC2_12p = ((long long)b0_8p*(long long)a11_8p - (long long)b1_8p*(long long)a01_8p)/MatrixValue_4p;
            rca.v1_MADPictureC1_12p = ((long long)b1_8p*(long long)a00_8p - (long long)b0_8p*(long long)a01_8p)/MatrixValue_4p;

        }
        else {
            if (a01_8p==0) {// lhumad, make fake for dividing by zero when a01_8p equal to 0
                rca.v1_MADPictureC1_12p = (long long)b0_8p<<12;
                rca.v1_cmadequ0 = 1;
            }
            else {
                rca.v1_MADPictureC1_12p = (int)(((long long)b0_8p<<12)/(long long)a01_8p);
                rca.v1_cmadequ0 = 0;
            }
            rca.v1_MADPictureC2_12p = 0;
        }
        rca.v1_c1_over = 0;
    }
    //if(rca.v1_type==P_SLICE)//g1 || (rca.v1_RCUpdateMode==RC_MODE_1 && (rca.v1_gop_cnt!=0 || rca.v1_frame_cnt!=0)))  //(rca.v1_number != 0)
    if( rca.v1_type==P_SLICE || ((rca.v1_RCUpdateMode==RC_MODE_1 || (rca.v1_type==I_SLICE&&rca.v1_RCUpdateMode==RC_MODE_3)) && (!(rca.v1_gop_cnt==0&&rca.v1_frame_cnt==0))) ) //lhulhu
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

  /* frame layer rate control */
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

        /* predict the MAD of current picture*/
//20        rca.v1_CurrentMAD=rca.v1_MADPictureC1*rca.v1_PreviousPictureMAD + rca.v1_MADPictureC2;
        rca.v1_CurrentMAD_8p = (rca.v1_MADPictureC1_12p*rca.v1_PreviousPictureMAD_8p)/(1<<12) + rca.v1_MADPictureC2_12p/(1<<4);

        /*compute the number of bits for the texture*/
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
  /*basic unit layer rate control*/
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

        /*the average QP of the previous frame is used to coded the first basic unit of the current frame or field*/
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
}


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
  if( rca.v1_BasicUnit == rca.v1_FrameSizeInMbs ) //lhulhu
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
        rca.v1_CurrentMAD_8p=(rca.v1_MADPictureC1_12p*rca.v1_PreviousPictureMAD_8p)/(1<<12) + rca.v1_MADPictureC2_12p/(1<<4);

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
    else if( rca.v1_type == P_SLICE || rca.v1_type == I_SLICE ) //lhulhu
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
            //if((rca.v1_bu_cnt==(rca.v1_TotalNumberofBasicUnit-1)) && rca.v1_type==P_SLICE)
            if((rca.v1_bu_cnt==(rca.v1_TotalNumberofBasicUnit-1)) && (rca.v1_type==P_SLICE || rca.v1_type==I_SLICE) ) //lhulhu
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
  if(rca.v1_basicunit>=rca.v1_MBPerRow)
    rca.v1_m_Qc = my_imin(rca.v1_m_Qc, rca.v1_PAveFrameQP+6);
  else
    rca.v1_m_Qc = my_imin(rca.v1_m_Qc, rca.v1_PAveFrameQP+3);

  rca.v1_TotalFrameQP +=rca.v1_m_Qc;
  if(rca.v1_bu_cnt==(rca.v1_TotalNumberofBasicUnit-1)) //(rca.v1_NumberofBasicUnit==0)
  {
//18    PAverageQP=(int)((double)rca.v1_TotalFrameQP/(double)rca.v1_TotalNumberofBasicUnit+0.5);
    PAverageQP=(rca.v1_TotalFrameQP+(rca.v1_TotalNumberofBasicUnit>>1))/rca.v1_TotalNumberofBasicUnit;
    if(rca.v1_frame_cnt==(rca.v1_intra_period-1)) //(rca.v1_NumberofPPicture == (rca.v1_intra_period - 2))
      rca.v1_QPLastPFrame = PAverageQP;
    if (rca.v1_type == P_SLICE) // not increase TotalQpforPPicture for I_SLICE, lhulhu
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
  if (rca.v1_type == P_SLICE) // not increase TotalQpforPPicture for I_SLICE, lhulhu
    rca.v1_TotalQpforPPicture +=PAverageQP;
  rca.v1_PAveFrameQP=PAverageQP;
  rca.v1_PAveHeaderBits3=rca.v1_PAveHeaderBits2;
}



void my_v1_predictCurrPicMAD( )
{
    int i,CurrentBUMAD_8p;

    //rca.CurrentMAD=rca.MADPictureC1*rca.BUPFMAD[rca.bu_cnt]+rca.MADPictureC2;
//s    rca.CurrentMAD_8p=((rca.MADPictureC1_12p>>4)*((*(pp_BUPFMAD_8p+rca.bu_cnt))>>8)) + (rca.MADPictureC2_12p>>4);
	rca.v1_CurrentMAD_8p=((rca.v1_MADPictureC1_12p>>4)*(rca.v1_BUPFMAD_8p[rca.v1_bu_cnt]>>8)) + (rca.v1_MADPictureC2_12p>>4);

    rca.v1_TotalBUMAD_12p=0;

    for(i=rca.v1_TotalNumberofBasicUnit-1; i>=rca.v1_bu_cnt; i--)
    {
      //CurrentBUMAD = rca.MADPictureC1*rca.BUPFMAD[i]+rca.MADPictureC2;
//s      CurrentBUMAD_8p = ((rca.MADPictureC1_12p>>4)*((*(pp_BUPFMAD_8p+i))>>8)) + (rca.MADPictureC2_12p>>4);
	  CurrentBUMAD_8p = ((rca.v1_MADPictureC1_12p>>4)*(rca.v1_BUPFMAD_8p[i]>>8)) + (rca.v1_MADPictureC2_12p>>4);
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
  rca.v1_m_Qc = my_imin(m_Qp+rca.v1_DDquant,  rca.v1_m_Qc); // control variation

  if(rca.v1_basicunit>=rca.v1_MBPerRow)
    rca.v1_m_Qc = my_imin(rca.v1_PAveFrameQP+6, rca.v1_m_Qc);
  else
    rca.v1_m_Qc = my_imin(rca.v1_PAveFrameQP+3, rca.v1_m_Qc);

  if(rca.v1_c1_over==1 || rca.v1_cmadequ0==1) // lhumad
    rca.v1_m_Qc = my_imin(m_Qp-rca.v1_DDquant, rca.v1_RCMaxQP); // clipping
  else
    rca.v1_m_Qc = my_iClip3(m_Qp-rca.v1_DDquant, rca.v1_RCMaxQP, rca.v1_m_Qc); // clipping

  if(rca.v1_basicunit>=rca.v1_MBPerRow)
    rca.v1_m_Qc = my_imax(rca.v1_PAveFrameQP-6, rca.v1_m_Qc);
  else
    rca.v1_m_Qc = my_imax(rca.v1_PAveFrameQP-3, rca.v1_m_Qc);

  rca.v1_m_Qc = my_imax(rca.v1_RCMinQP, rca.v1_m_Qc);
}


void my_v1_updateModelQPFrame( int m_Bits )
{
  long long dtmp_8p;
  int tmp_4p=0;
  int m_Qstep_8p;

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
    m_Qstep_8p = (2*((long long)rca.v1_m_X2_8p)*((long long)rca.v1_CurrentMAD_8p)) / ((tmp_4p<<4) - (rca.v1_m_X1_8p>>4)*(rca.v1_CurrentMAD_8p>>4));
  }

  rca.v1_m_Qc = Qstep2QP_8p(m_Qstep_8p);
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
    if(rca.v1_bu_cnt==0) { //frame over
        rca.v1_frame_mad   = 0;
        rca.v1_frame_tbits = 0;
        rca.v1_frame_hbits = 0;
    }
    else {
        rca.v1_frame_mad   += v1_mad_tmp;
        rca.v1_frame_tbits += v1_tbits_tmp;
        rca.v1_frame_hbits += v1_hbits_tmp;
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
    int np;

    if(rca.v1_RCUpdateMode==RC_MODE_1 || rca.v1_RCUpdateMode==RC_MODE_3)
    {
        if(rca.v1_frame_cnt==0 && rca.v1_bu_cnt==0) //
        {
            np = rca.v1_intra_period - 1;
            my_v1_rc_init_GOP( np );
        }
    }
    else if((rca.v1_RCUpdateMode==RC_MODE_0)|(rca.v1_RCUpdateMode==RC_MODE_2))
    {
        if(rca.v1_frame_cnt==0 && rca.v1_bu_cnt==0) // && rca.mb_cnt==0)
        {
            np = rca.v1_intra_period - 1;
            my_v1_rc_init_GOP( np );
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
        if(rca.v1_gop_cnt==0 && rca.v1_frame_cnt==0) //lhulhu, let non RC_MODE_1 can handle I_SLICE
        {
            rca.v1_qp = rca.v1_MyInitialQp;
        }
        //else if (rca.v1_type == P_SLICE) //g1 || rca.RCUpdateMode == RC_MODE_1 )
        else if (rca.v1_type == P_SLICE || rca.v1_RCUpdateMode==RC_MODE_1 || (rca.v1_type==I_SLICE && rca.v1_RCUpdateMode==RC_MODE_3)) //lhulhu
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


    if(rca.v1_basicunit < rca.v1_FrameSizeInMbs) // bu-level counter
    {
        if(rca.v1_bu_cnt==(rca.v1_TotalNumberofBasicUnit-1))
        {
            rca.v1_bu_cnt=0;
            if(rca.v1_frame_cnt==(rca.v1_intra_period-1))
            {
                rca.v1_frame_cnt=0;
                if(rca.v1_gop_cnt<=1000)
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
            if(rca.v1_gop_cnt<=1000)
                rca.v1_gop_cnt++;
        }
        else
            rca.v1_frame_cnt++;
    }


    my_v1_rc_update_frame_stats(); // computer frame parameters

#ifndef ARMCM7_RC //#############
    my_hold( );
#endif //#############


    return rca.v1_qp;
}
