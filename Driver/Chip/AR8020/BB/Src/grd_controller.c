#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "config_functions_sel.h"
#include "sys_param.h"
#include "debuglog.h"
#include "interrupt.h"
#include "timer.h"
#include "reg_rw.h"
#include "config_baseband_register.h"
#include "BB_ctrl.h"
#include "grd_controller.h"
#include "grd_sweep.h"

typedef enum
{
    QAMUP,
    QAMDOWN
}QAMUPDONW;

#define SNR_STATIC_START_VALUE      (100)
#define SNR_STATIC_UP_THRESHOD      (5)
#define SNR_STATIC_DOWN_THRESHOD    (2)

static init_timer_st init_timer0_0;
static init_timer_st init_timer0_1;
static uint8_t Timer1_Delay1_Cnt = 0;
static uint8_t snr_static_count = SNR_STATIC_START_VALUE;

const SYS_PARAM default_sys_param =
{
    .usb_sel    = 0x00,
    .usb_cofig  = 0x00,
    .freq_band_sel=0x00,
    .it_mode    = 0x03,
    .qam_mode   = MOD_4QAM,
    .ldpc       = LDPC_1_2,
    .id_num      = 0x02,
    .test_enable = 0xff,
    .it_skip_freq_mode  = AUTO,
    .rc_skip_freq_mode  = AUTO,
    .search_id_enable   = 0xff,
    .freq_band = FREQ_24BAND_BIT_POS,
    .power  = 20,

    .qam_skip_mode = AUTO,

    //bpsk_ldpc12 ,qam4_ldpc12, qam4_ldpc23,    qam16_ldpc12,   qam64_ldpc12,   qam64_ldpc23
    //3.8db,       6.5db,       7.7db,          11.2db,         16.2db,         18db
    .qam_change_threshold = {0x009B, 0x011E, 0x0179, 0x03F6, 0x0AAA, 0x0FC6},
    .enable_freq_offset = DISABLE_FLAG,
};

void gen_qam_threshold_range(void)
{
    uint8_t i;
    for(i=0;i<QAM_CHANGE_THRESHOLD_COUNT;i++)
    {
        if(i < QAM_CHANGE_THRESHOLD_COUNT - 1)
        {
            context.qam_threshold_range[i][0] = context.qam_change_threshold[i];
            context.qam_threshold_range[i][1] = context.qam_change_threshold[i+1]- 1 ;
        }
        else
        {
            context.qam_threshold_range[i][0] = context.qam_change_threshold[i];
            context.qam_threshold_range[i][1] = 0xffff;
        }
    }
}

void Grd_Parm_Initial(void)
{
    Grd_Timer0_Init();
    Grd_Timer1_Init();
    
    context.it_manual_ch  = 0xff;
    context.qam_skip_mode = AUTO;
    context.it_skip_freq_mode = AUTO;
    context.rc_skip_freq_mode = AUTO;
    context.bw = BW_10M;
    
    //For QAM mode change
    memcpy(context.qam_change_threshold, default_sys_param.qam_change_threshold, sizeof(default_sys_param.qam_change_threshold));
    gen_qam_threshold_range();

    reg_IrqHandle(BB_TX_ENABLE_VECTOR_NUM, wimax_vsoc_tx_isr);
    INTR_NVIC_EnableIRQ(BB_TX_ENABLE_VECTOR_NUM);    
}


void BB_Grd_Id_Initial(void)
{
    BB_WriteReg(PAGE2, GRD_RC_ID_BIT39_32_REG, RC_ID_BIT39_32);
    BB_WriteReg(PAGE2, GRD_RC_ID_BIT31_24_REG, RC_ID_BIT31_24);
    BB_WriteReg(PAGE2, GRD_RC_ID_BIT23_16_REG, RC_ID_BIT23_16);
    BB_WriteReg(PAGE2, GRD_RC_ID_BIT15_08_REG, RC_ID_BIT15_08);
    BB_WriteReg(PAGE2, GRD_RC_ID_BIT07_00_REG, RC_ID_BIT07_00);
}

//---------------IT grd hop change--------------------------------
#define SNR_STATIC_START_VALUE      (100)
#define SNR_STATIC_UP_THRESHOD      (5)
#define SNR_STATIC_DOWN_THRESHOD    (2)

volatile DEVICE_STATE dev_state = INIT_DATA;

void grd_noise_sweep(void)
{
    grd_add_sweep_result();
    grd_set_next_sweep_freq();
    if(is_init_sne_average_and_fluct())
    {
        calu_sne_average_and_fluct(get_sweep_freq());
    }
}

void grd_fec_judge(void)
{
    if(dev_state == INIT_DATA)
    {
        if(is_it_sweep_finish())
        {
            init_sne_average_and_fluct();
            context.cur_ch = get_best_freq();
            dev_state = FEC_UNLOCK;
        }
    }
    else if(dev_state == CHECK_FEC_LOCK || dev_state == FEC_LOCK)
    {
        context.locked = grd_is_bb_fec_lock();
        if(context.locked)
        {
            dev_state = FEC_LOCK;
            /*if(context.first_freq_value == 0xff)
            {
                context.first_freq_value = context.cur_ch;
            } */               
            context.fec_unlock_cnt = 0;
        }
        else
        {
            context.fec_unlock_cnt++;
            if(context.fec_unlock_cnt > 64)
            {
                context.fec_unlock_cnt = 0;
                if(context.it_skip_freq_mode == AUTO)
                {
                    context.cur_ch = get_next_best_freq(context.cur_ch);
                    dev_state = FEC_UNLOCK;
                }
                
                context.qam_mode = MOD_BPSK;
                context.ldpc = LDPC_1_2;
                context.bw   = BW_10M;
                context.qam_ldpc = merge_qam_ldpc_to_index(context.qam_mode,context.ldpc);
                grd_set_txmsg_qam_change(context.qam_mode, context.bw, context.ldpc);
            }
        }
    }
    else if(dev_state == FEC_UNLOCK )
    {

        if(context.it_skip_freq_mode == AUTO)
        {
            grd_set_it_skip_freq(context.cur_ch);
            printf("CHSA=>%d\n", context.cur_ch);
        }
        dev_state = DELAY_14MS;
    }
    else if(dev_state == DELAY_14MS)
    {
        /*if(context.enable_freq_offset == ENABLE_FLAG)
        {
            bb_set_freq_offset(calu_it_skip_freq_delta(context.first_freq_value,context.cur_ch));
        }*/
        if(context.it_skip_freq_mode == MANUAL)
        {
            grd_set_it_work_freq(context.it_manual_ch);
            context.it_manual_ch = 0xff;
        }
        else
        {
            grd_set_it_work_freq(context.cur_ch);
        }
        reset_it_span_cnt();
        dev_state = CHECK_FEC_LOCK;
    }
    
    if(context.it_skip_freq_mode == MANUAL && context.it_manual_ch != 0xff)
    {
        grd_set_it_skip_freq(context.it_manual_ch);
        dev_state = DELAY_14MS;
        printf("CHSM=>%d\n", context.it_manual_ch);   //CHSM: channel switch Manual
    }
}

void grd_freq_skip_judge(void)
{
    if(dev_state != FEC_LOCK)
    {
        return;
    }

    if(!is_it_need_skip_freq(context.qam_ldpc))
        return;

    context.next_ch = get_next_best_freq(context.cur_ch);
    if(is_next_best_freq_pass(context.cur_ch,context.next_ch))
    {
       context.cur_ch = context.next_ch;
       dev_state = FEC_UNLOCK;
    }

}

uint32_t it_span_cnt = 0;
void reset_it_span_cnt(void)
{
    it_span_cnt = 0;
}

uint8_t is_retrans_cnt_pass(void)
{
    uint8_t Harqcnt;
    Harqcnt = BB_ReadReg(PAGE2, FEC_5_RD);

    if(((Harqcnt & 0xF0) >> 4) >=2 )
    {
        return 0;
    }

    return 1;
}
uint8_t span,retrans,snr_if;
uint16_t iMCS;
/*
  2.3G

0x7641a41a,//2306 + 0x05000000 (2.4g) 2406
0x76c4ec4e,//2316
0x775be5be,//2327.5
0x77df2df2,//2337.5
0x78762762,//2349
0x78f96f96,//2359
0x79906906,//2370.5
0x7a276276 //2382

2.4G

0x7b41a41a,//2403.5
0x7bc4ec4e,//2413.5
0x7c5be5be,//2425
0x7cdf2df2,//2435
0x7d762762,//2446.5
0x7df96f96,//2456.5
0x7e906906,//2468
0x7f276276 //2479.5
*/
const uint16_t snr_skip_threshold[6] = { 0x004e,0x0090,0x00be,0x01fd,0x055e,0x07ec};
uint8_t is_it_need_skip_freq(uint8_t qam_ldpc)
{
    it_span_cnt++;

    if(it_span_cnt < 100)
    {
        return 0;
    }
    retrans = is_retrans_cnt_pass();

    if(retrans)
    {
        return 0;
    }

    iMCS = snr_skip_threshold[qam_ldpc];
    snr_if = is_snr_ok(iMCS);
    if(snr_if)
    {
        return 0;
    }

    return 1;
}

void grd_set_it_skip_freq(uint8_t ch)
{
    BB_WriteReg(PAGE2, IT_FREQ_TX_0, 0xE0 + ch);
    BB_WriteReg(PAGE2, IT_FREQ_TX_1, 0xE0 + ch);
}

void grd_set_it_work_freq(uint8_t ch)
{
    BB_set_ITfrq(ch);
}

uint8_t grd_is_bb_fec_lock(void)
{
    static uint8_t status = 0xff;
    uint8_t data = BB_ReadReg(PAGE2, FEC_5_RD) & 0x01;
    if(status != data)
    {
        printf("ML:%d\r\n", data);
        status = data;
    }
    
    return data;
}

//--------------------------------------------------------
//---------------QAM change--------------------------------
EN_BB_QAM Grd_get_QAM(void)
{
    static uint8_t iqam = 0xff;

    EN_BB_QAM qam = (EN_BB_QAM)(BB_ReadReg(PAGE2, GRD_FEC_QAM_CR_TLV) & 0x03);
    if(iqam != qam)
    {
        iqam = qam;
        printf("-QAM:%d ",qam);
    }
    
    return qam;
}


void grd_set_txmsg_qam_change(EN_BB_QAM qam, EN_BB_BW bw, EN_BB_LDPC ldpc)
{
    uint8_t data = (qam << 6) | (bw << 3) | ldpc;
    printf("GMS =>0x%.2x \r\n", data);

    BB_WriteReg(PAGE2, QAM_CHANGE_0, data);
    BB_WriteReg(PAGE2, QAM_CHANGE_1, data+1);
}


uint8_t merge_qam_ldpc_to_index(EN_BB_QAM qam, EN_BB_LDPC ldpc)
{
    if(qam == MOD_BPSK && ldpc == LDPC_1_2)
    {
        return 0;
    }
    else if(qam == MOD_4QAM)
    {
        if(ldpc == LDPC_1_2)
        {
            return 1;
        }
        else if(ldpc == LDPC_2_3)
        {
            return 2;
        }
    }
    else if(qam == MOD_16QAM && ldpc == LDPC_1_2)
    {
        return 3;
    }
    else if(qam == MOD_64QAM)
    {
        if(ldpc == LDPC_1_2)
        {
            return 4;
        }
        else if(ldpc == LDPC_2_3)
        {
            return 5;
        }
    }

    return 0xff;

}

void split_index_to_qam_ldpc(uint8_t index, EN_BB_QAM *qam, EN_BB_LDPC *ldpc)
{
    if(index == 0)
    {
        *qam = MOD_BPSK;
        *ldpc = LDPC_1_2;
    }
    else if(index == 1)
    {
        *qam = MOD_4QAM;
        *ldpc = LDPC_1_2;
    }
    else if(index == 2)
    {
        *qam = MOD_4QAM;
        *ldpc = LDPC_2_3;

    }
    else if(index == 3)
    {
        *qam = MOD_16QAM;
        *ldpc = LDPC_1_2;

    }
    else if(index == 4)
    {
        *qam = MOD_64QAM;
        *ldpc = LDPC_1_2;

    }
    else if(index == 5)
    {
        *qam = MOD_64QAM;
        *ldpc = LDPC_2_3;
    }
    return;
}


void up_down_qamldpc(QAMUPDONW up_down)
{
    if(context.qam_skip_mode == MANUAL)
    {
        return;
    }

    if(QAMUP == up_down)
    {
        if(context.qam_ldpc == QAM_CHANGE_THRESHOLD_COUNT - 1)  //highest QAM EN_BB_LDPC mode
        {
            return;
        }
        context.qam_ldpc++;
    }
    else
    {
        if(context.qam_ldpc == 0)
        {
            return;
        }
        context.qam_ldpc--;
    }

    split_index_to_qam_ldpc(context.qam_ldpc,&(context.qam_mode),&(context.ldpc));
    grd_set_txmsg_qam_change(context.qam_mode, context.bw ,context.ldpc);
}


void grd_qam_change_judge(void)
{
    uint8_t snr_statice_value;

    if(!context.locked)
        return;

    snr_statice_value = snr_static_for_qam_change(context.qam_threshold_range[context.qam_ldpc][0],context.qam_threshold_range[context.qam_ldpc][1]);
    if(snr_statice_value == 0xff)
    {
        return;
    }
    else if(snr_statice_value == 0)
    {
        /*if(snr_static_count > SNR_STATIC_START_VALUE)
        {
            snr_static_count--;
        }
        else if(snr_static_count < SNR_STATIC_START_VALUE)
        {
            snr_static_count++;
        }*/
        snr_static_count = SNR_STATIC_START_VALUE;

        return;
    }
    else if(snr_statice_value == 1)
    {
        snr_static_count++;
    }
    else if(snr_statice_value == 2)
    {
        snr_static_count--;
    }

    if(snr_static_count >= SNR_STATIC_START_VALUE + SNR_STATIC_UP_THRESHOD)
    {
        up_down_qamldpc(QAMUP);
    }
    else if(snr_static_count <= SNR_STATIC_START_VALUE - SNR_STATIC_DOWN_THRESHOD)
    {
        up_down_qamldpc(QAMDOWN);
    }
    else
    {
        return;
    }

    snr_static_count = SNR_STATIC_START_VALUE;
    //qam_change_threshold[context.qam_ldpc] = (qam_change_threshold[context.qam_ldpc] + get_snr_qam_threshold()) / 2;

    return;
}

///////////////////////////////////////////////////////////////////////////////////

void wimax_vsoc_tx_isr(void)
{
    INTR_NVIC_DisableIRQ(BB_TX_ENABLE_VECTOR_NUM);
    TIM_StartTimer(init_timer0_0);
    INTR_NVIC_EnableIRQ(TIMER_INTR00_VECTOR_NUM);
}

void Grd_TIM0_IRQHandler(void)
{
    Reg_Read32(BASE_ADDR_TIMER0 + TMRNEOI_0);

    //Enable BB_TX intr
    INTR_NVIC_ClearPendingIRQ(BB_TX_ENABLE_VECTOR_NUM); //clear pending after TX Enable is LOW. MUST
    INTR_NVIC_EnableIRQ(BB_TX_ENABLE_VECTOR_NUM);

    //Disable TIM0 intr
    INTR_NVIC_DisableIRQ(TIMER_INTR00_VECTOR_NUM);
    TIM_StopTimer(init_timer0_0);
    
    //Enable TIM1 intr
    TIM_StartTimer(init_timer0_1);
    INTR_NVIC_EnableIRQ(TIMER_INTR01_VECTOR_NUM);   
}

void Grd_TIM1_IRQHandler(void)
{
    Reg_Read32(BASE_ADDR_TIMER0 + TMRNEOI_1); //disable the intr.      

    grd_add_snr_daq();
    switch (Timer1_Delay1_Cnt)
    {
        case 0:
            grd_noise_sweep();
            grd_fec_judge();
            Timer1_Delay1_Cnt++;
            break;

        case 1:
            Timer1_Delay1_Cnt++;
            break;

        case 2:
            if(context.rc_skip_freq_mode == AUTO)
            {
                grd_rc_hopfreq();
            }
            Timer1_Delay1_Cnt++;
            break;

        case 3:
            Timer1_Delay1_Cnt++;
            break;

        case 4:
            Timer1_Delay1_Cnt++;
            break;

        case 5:
            Timer1_Delay1_Cnt++;
            break;

        case 6:
            Timer1_Delay1_Cnt++;
            break;

        case 7:
            Timer1_Delay1_Cnt++;
            if(context.it_skip_freq_mode == AUTO)
            {
                grd_freq_skip_judge();
            }
            break;

        case 8:
            INTR_NVIC_DisableIRQ(TIMER_INTR01_VECTOR_NUM);                
            TIM_StopTimer(init_timer0_1);

            Timer1_Delay1_Cnt = 0;
            grd_qam_change_judge();
            break;

        default:
            Timer1_Delay1_Cnt = 0;
            dlog_error("Timer1_Delay1_Cnt error\n");
            break;
    }
}

void Grd_Timer1_Init(void)
{
    init_timer0_1.base_time_group = 0;
    init_timer0_1.time_num = 1;
    init_timer0_1.ctrl = 0;
    init_timer0_1.ctrl |= TIME_ENABLE | USER_DEFINED;
    TIM_RegisterTimer(init_timer0_1, 1200); //1.25ms
    reg_IrqHandle(TIMER_INTR01_VECTOR_NUM, Grd_TIM1_IRQHandler);
}

void Grd_Timer0_Init(void)
{
    init_timer0_0.base_time_group = 0;
    init_timer0_0.time_num = 0;
    init_timer0_0.ctrl = 0;
    init_timer0_0.ctrl |= TIME_ENABLE | USER_DEFINED;
    
    TIM_RegisterTimer(init_timer0_0, 2500); //2.5s
    reg_IrqHandle(TIMER_INTR00_VECTOR_NUM, Grd_TIM0_IRQHandler);
}


//=====================================Grd RC funcions =====
uint8_t grd_rc_channel = 0;
void grd_rc_hopfreq(void)
{
    grd_rc_channel++;

    if(grd_rc_channel >= MAX_RC_FRQ_SIZE)
    {
        grd_rc_channel = 0;
    }
    BB_set_Rcfrq(grd_rc_channel);    
}

//=====================================Grd Test interface=====
void grd_set_it_ch(uint8_t ch)
{
    context.it_manual_ch = ch;
}

void grd_set_it_skip_mode_ch(RUN_MODE mode, uint8_t ch)
{
    context.it_skip_freq_mode = MANUAL;
    grd_set_it_ch(ch);
}


