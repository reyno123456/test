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
#include "BB_snr_service.h"
#include "grd_controller.h"
#include "grd_sweep.h"
#include "wireless_interface.h"
#include "sys_event.h"


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
    .usb_sel        = 0x00,
    .usb_cofig      = 0x00,
    .freq_band_sel  = 0x00,
    .it_mode    =  0x03,
    .qam_mode   =  MOD_4QAM,
    .ldpc       =  LDPC_1_2,
    .id_num     =  0x02,
    .test_enable = 0xff,
    .it_skip_freq_mode  = AUTO,
    .rc_skip_freq_mode  = AUTO,
    .search_id_enable   = 0xff,
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
    context.CH_bandwidth      = BW_10M;
    context.it_manual_rf_band = 0xff;
    context.trx_ctrl          = IT_RC_MODE;
    
    grd_sweep_freq_init();

    //For QAM mode change
    memcpy(context.qam_change_threshold, default_sys_param.qam_change_threshold, sizeof(default_sys_param.qam_change_threshold));
    gen_qam_threshold_range();

    grd_set_it_skip_freq(context.cur_IT_ch);
    grd_set_it_work_freq(context.RF_band, context.cur_IT_ch);
    
    reg_IrqHandle(BB_TX_ENABLE_VECTOR_NUM, wimax_vsoc_tx_isr);
    INTR_NVIC_EnableIRQ(BB_TX_ENABLE_VECTOR_NUM);

    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_USER_CFG_CHANGE_LOCAL, grd_handle_events_callback);
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
    int8_t result = grd_add_sweep_result(context.CH_bandwidth);
    if(result == 1)
    {
        grd_set_next_sweep_freq();
        if(is_init_sne_average_and_fluct())
        {
            calu_sne_average_and_fluct(get_sweep_freq());
        }
    }
}

void grd_fec_judge(void)
{
    if(dev_state == INIT_DATA)
    {
        if(is_it_sweep_finish())
        {
            init_sne_average_and_fluct();
            context.cur_IT_ch = get_best_freq();
            dev_state = FEC_UNLOCK;
        }
    }
    else if(dev_state == CHECK_FEC_LOCK || dev_state == FEC_LOCK)
    {
        context.locked = grd_is_bb_fec_lock();
        if(context.locked)
        {
            dev_state = FEC_LOCK;

            #if 0
            if(context.first_freq_value == 0xff)
            { 
                context.first_freq_value = context.cur_IT_ch;
            }   
            #endif
            
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
                    context.cur_IT_ch = get_next_best_freq(context.cur_IT_ch);
                    dev_state = FEC_UNLOCK;
                }

                if(context.qam_skip_mode == AUTO)
                {
                    context.qam_mode= MOD_BPSK;
                    context.ldpc    = LDPC_1_2;
                    context.CH_bandwidth = BW_10M;

                    context.qam_ldpc = merge_qam_ldpc_to_index(context.qam_mode, context.ldpc);
                    grd_set_txmsg_qam_change(context.qam_mode, context.CH_bandwidth, context.ldpc);
                }
            }
        }
    }
    else if(dev_state == FEC_UNLOCK )
    {
        if(context.it_skip_freq_mode == AUTO && context.RF_band == RF_2G) //for test 5G only
        {
            grd_set_it_skip_freq(context.cur_IT_ch);
            printf("CHSA=>%d\n", context.cur_IT_ch);
        }
        dev_state = DELAY_14MS;
    }
    else if(dev_state == DELAY_14MS)
    {
        #if 0
        if(context.enable_freq_offset == ENABLE_FLAG)
        {
            bb_set_freq_offset(calu_it_skip_freq_delta(context.first_freq_value,context.cur_IT_ch));
        }
        #endif
        if(context.it_manual_ch != 0xff)
        {
            grd_set_it_work_freq(context.RF_band, context.it_manual_ch);
            context.it_manual_ch = 0xff;
        }
        else
        {
            grd_set_it_work_freq(context.RF_band, context.cur_IT_ch);
        }

        if(context.it_manual_rf_band != 0xff && context.RF_band!= context.it_manual_rf_band)
        {
            BB_set_RF_Band(BB_GRD_MODE, context.it_manual_rf_band);
            context.it_manual_rf_band = 0xff;
            context.RF_band = context.it_manual_rf_band;
        }
        reset_it_span_cnt();
        dev_state = CHECK_FEC_LOCK;
    }
    
    if(context.it_manual_ch != 0xff)
    {
        grd_set_it_skip_freq(context.it_manual_ch);
        dev_state = DELAY_14MS;
        dlog_info("channel switch Manual=>%d\n", context.it_manual_ch);
    }
    
    if(context.RF_band != context.it_manual_rf_band && context.it_manual_rf_band != 0xff)
    {
        dlog_info("To switch band: %d %d\r\n", context.RF_band, context.it_manual_rf_band);
        dev_state = DELAY_14MS;
    }
}

void grd_freq_skip_judge(void)
{    
    if(dev_state != FEC_LOCK)
    {
        return;
    }

    if(!is_it_need_skip_freq(context.qam_ldpc))
    {
        return;
    }

    context.next_IT_ch = get_next_best_freq(context.cur_IT_ch);
    if(is_next_best_freq_pass(context.cur_IT_ch,context.next_IT_ch))
    {
       context.cur_IT_ch = context.next_IT_ch;
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
    uint8_t Harqcnt = BB_ReadReg(PAGE2, FEC_5_RD);
    if(((Harqcnt & 0xF0) >> 4) >=2)
    {
        return 0;
    }

    return 1;
}

uint8_t span,retrans,snr_if;
uint16_t iMCS;

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
    BB_WriteReg(PAGE2, IT_FREQ_TX_1, 0xE0 + ch + 1);
}

void grd_set_it_work_freq(ENUM_RF_BAND rf_band, uint8_t ch)
{
    BB_set_ITfrq(rf_band, ch);
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

//---------------QAM change--------------------------------
ENUM_BB_QAM Grd_get_QAM(void)
{
    static uint8_t iqam = 0xff;

    ENUM_BB_QAM qam = (ENUM_BB_QAM)(BB_ReadReg(PAGE2, GRD_FEC_QAM_CR_TLV) & 0x03);
    if(iqam != qam)
    {
        iqam = qam;
        printf("-QAM:%d ",qam);
    }

    return qam;
}


void grd_set_txmsg_qam_change(ENUM_BB_QAM qam, ENUM_CH_BW bw, ENUM_BB_LDPC ldpc)
{
    uint8_t data = (qam << 6) | (bw << 3) | ldpc;
    printf("GMS =>0x%.2x \r\n", data);

    BB_WriteReg(PAGE2, QAM_CHANGE_0, data);
    BB_WriteReg(PAGE2, QAM_CHANGE_1, data+1);
}


uint8_t merge_qam_ldpc_to_index(ENUM_BB_QAM qam, ENUM_BB_LDPC ldpc)
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

void split_index_to_qam_ldpc(uint8_t index, ENUM_BB_QAM *qam, ENUM_BB_LDPC *ldpc)
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
        if(context.qam_ldpc == QAM_CHANGE_THRESHOLD_COUNT - 1)  //highest QAM ENUM_BB_LDPC mode
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
    grd_set_txmsg_qam_change(context.qam_mode, context.CH_bandwidth ,context.ldpc);
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
            grd_handle_all_cmds();
            grd_noise_sweep();
            Timer1_Delay1_Cnt++;
            break;

        case 1:
            grd_fec_judge();
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
            grd_get_osd_info();
            break;

        case 7:
            Timer1_Delay1_Cnt++;
            if(context.it_skip_freq_mode == AUTO && context.RF_band == RF_2G)
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
	uint8_t max_ch_size = (context.RF_band == RF_2G) ? (MAX_2G_RC_FRQ_SIZE):(MAX_5G_RC_FRQ_SIZE);

    grd_rc_channel++;
    if(grd_rc_channel >= max_ch_size)
    {
        grd_rc_channel = 0;
    }
	BB_set_Rcfrq(context.RF_band, grd_rc_channel);
}


///////////////////////////////////////////////////////////////////////////////////

/*
 * ground get the sky IT QAM mode
*/
ENUM_BB_QAM grd_get_IT_QAM(void)
{
    return (ENUM_BB_QAM)(BB_ReadReg(PAGE2, GRD_FEC_QAM_CR_TLV) & 0x03);
}

/*
 * ground get the sky IT LDPC mode
*/
ENUM_BB_LDPC grd_get_IT_LDPC(void)
{
    return (ENUM_BB_LDPC)( (BB_ReadReg(PAGE2, GRD_FEC_QAM_CR_TLV) >>2) & 0x07);
}


void grd_handle_IT_mode_cmd(RUN_MODE mode)
{
    context.it_skip_freq_mode = mode;
    dlog_info("mode= %d\r\n", mode);
}


/*
  *   only set value to context, the request will be handle in 14ms interrupt.
 */
void grd_handle_IT_CH_cmd(uint8_t ch)
{
    context.it_manual_ch = ch;
    dlog_info("ch= %d\r\n", ch);    
}


/*
  * mode: set RC to auto or Manual
  * ch: the requested channel from  
  * 
 */
static void grd_handle_RC_mode_cmd(RUN_MODE mode)
{
    context.rc_skip_freq_mode = mode;

    BB_WriteReg(PAGE2, RC_CH_MODE_0, 0x80+mode);
    BB_WriteReg(PAGE2, RC_CH_MODE_1, 0x80+mode+1); //manual command will send together with the RC channel
    
    if(mode == MANUAL)
    {
        BB_WriteReg(PAGE2, RC_CH_CHANGE_0, 0xFE);
        BB_WriteReg(PAGE2, RC_CH_CHANGE_1, 0xFF);
        dlog_info("RC manual, wait ch \r\n");
    }
    dlog_info("mode =%d\r\n", mode);    
}


static void grd_handle_RC_CH_cmd(uint8_t ch)
{
	BB_set_Rcfrq(context.RF_band, ch);

    BB_WriteReg(PAGE2, RC_CH_CHANGE_0, 0x80+ch);
    BB_WriteReg(PAGE2, RC_CH_CHANGE_1, 0x80+ch + 1);

    dlog_info("ch =%d\r\n", ch);     
}

/*
 * switch between 2.5G and 5G.
 */
static void grd_handle_RF_band_cmd(ENUM_RF_BAND rf_band)
{
    //notify sky
    if(context.RF_band != rf_band)
    {
        BB_WriteReg(PAGE2, RF_BAND_CHANGE_0, 0xc0 + (uint8_t)rf_band);
        BB_WriteReg(PAGE2, RF_BAND_CHANGE_1, 0xc0 + (uint8_t)rf_band + 1);

        context.it_manual_rf_band = rf_band;
        dlog_info("To rf_band %d %d\r\n", context.RF_band, rf_band);
    }    
}


/*
 *  handle command for 10M, 20M
*/
static void grd_handle_CH_bandwitdh_cmd(ENUM_CH_BW bw)
{
    //set and soft-rest
    if(context.CH_bandwidth != bw)
    {
        BB_set_RF_bandwitdh(BB_GRD_MODE, bw);

        BB_WriteReg(PAGE2, RF_CH_BW_CHANGE_0, 0xc0 | (uint8_t)bw);
        BB_WriteReg(PAGE2, RF_CH_BW_CHANGE_1, 0xc0 | (uint8_t)bw + 1);        
    }

    dlog_info("CH_bandwidth =%d\r\n", context.CH_bandwidth);    
}


static void grd_handle_MCS_mode_cmd(RUN_MODE mode)
{
	context.qam_skip_mode = mode;
    dlog_info("qam_skip_mode = %d \r\n", context.qam_skip_mode);
}


/*
  * handle set MCS mode: QAM, LDPC, encoder rate
*/
static void grd_handle_MCS_cmd(ENUM_BB_QAM qam, ENUM_BB_LDPC ldpc)
{
    grd_set_txmsg_qam_change(qam, context.CH_bandwidth, ldpc);
    dlog_info("qam, ldpc =%d %d\r\n", qam, ldpc);
}


/*
 * handle H264 encoder brc 
*/
static void grd_handle_brc_mode_cmd(RUN_MODE mode)
{
    context.brc_mode = mode;

    BB_WriteReg(PAGE2, ENCODER_BRC_MODE_0, 0xe0+mode);
    BB_WriteReg(PAGE2, ENCODER_BRC_MODE_1, 0xe0+mode+1);

    dlog_info("brc mode =%d\r\n", mode);
}


/*
  * handle H264 encoder brc 
 */
static void grd_handle_brc_bitrate_cmd(uint8_t brc_coderate)
{
    BB_WriteReg(PAGE2, ENCODER_BRC_CHAGE_0, (0xc0 | brc_coderate));
    BB_WriteReg(PAGE2, ENCODER_BRC_CHAGE_1, (0xc0 | brc_coderate)+1);
    context.brc_bps = brc_coderate;

    dlog_info("brc_coderate = %d \r\n", brc_coderate);
}


typedef struct _STRU_grd_cmds
{
    uint8_t avail; /*command is using*/
    STRU_WIRELESS_CONFIG_CHANGE config;
}STRU_grd_cmds;


static void grd_handle_one_cmd(STRU_grd_cmds* p)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd = p->config;

    uint8_t class = cmd.configClass;
    uint8_t item  = cmd.configItem;
    uint8_t value = cmd.configValue;

    dlog_info("class item value %d %d %d \r\n", class, item, value);
    if(class == WIRELESS_FREQ_CHANGE)
    {
        switch(item)
        {
            case FREQ_BAND_MODE:
                //band mode: AUTO MANUAL, only suppor the Manual mode
                break;

            case FREQ_BAND_SELECT:    
                grd_handle_RF_band_cmd((ENUM_RF_BAND)value);
                break;

            case FREQ_CHANNEL_MODE: //auto manual
                grd_handle_IT_mode_cmd((RUN_MODE)value);
                break;

            case FREQ_CHANNEL_SELECT:
                grd_handle_IT_CH_cmd((uint8_t)value);
                break;

            default:
                dlog_error("%s\r\n", "unknown WIRELESS_FREQ_CHANGE command");
                break;
        }
    }

    if(class == WIRELESS_MCS_CHANGE)
    {
        switch(item)
        {
            case MCS_MODE_SELECT:
                grd_handle_MCS_mode_cmd((RUN_MODE)value);
                break;

            case MCS_MODULATION_SELECT:
                {
                    ENUM_BB_LDPC ldpc = (ENUM_BB_LDPC)(value&0x0f);
                    ENUM_BB_QAM  qam  = (ENUM_BB_QAM)((value >> 4)&0x0f);
                    
                    grd_handle_MCS_cmd(qam, ldpc);
                }
                break;

            default:
                dlog_error("%s\r\n", "unknown WIRELESS_MCS_CHANGE command");
                break;
        }        
    }

    if(class == WIRELESS_ENCODER_CHANGE)
    {
        switch(item)
        {
            case ENCODER_DYNAMIC_BIT_RATE_MODE:
                grd_handle_brc_mode_cmd( (RUN_MODE)value);
                break;

            case ENCODER_DYNAMIC_BIT_RATE_SELECT:
                grd_handle_brc_bitrate_cmd( (uint8_t)value);
                break;

            default:
                dlog_error("%s\r\n", "unknown WIRELESS_ENCODER_CHANGE command");
                break;                
        }
    }    
}

static STRU_grd_cmds grd_cmds_buf[10];

void grd_handle_all_cmds(void)
{
    uint8_t i;
    for(i = 0; i < sizeof(grd_cmds_buf)/sizeof(grd_cmds_buf[0]); i++)
    {
        if(grd_cmds_buf[i].avail == 1)
        {
            grd_handle_one_cmd( grd_cmds_buf + i);
            grd_cmds_buf[i].avail = 0;
        }
    }
}


void grd_handle_events_callback(void *p)
{
    uint8_t i;
    uint8_t found;
    STRU_WIRELESS_CONFIG_CHANGE *pcmd = (STRU_WIRELESS_CONFIG_CHANGE *)p;

    dlog_info("Get Message: %d %d %d\r\n", pcmd->configClass, pcmd->configItem, pcmd->configValue);

    found = 0;
    for(i = 0; i < sizeof(grd_cmds_buf)/sizeof(grd_cmds_buf[0]); i++)
    {
        if(grd_cmds_buf[i].avail == 0)
        {
            memcpy((void *)(&grd_cmds_buf[i].config), p, sizeof(grd_cmds_buf[0]));
            grd_cmds_buf[i].avail = 1;
            found = 1;
            break;
        }
    }

    if(!found)
    {
        dlog_error("ERROR: %s\r\n", "insert events to grd");
    }
}

void grd_add_spi_cmds(uint32_t type, uint32_t value)
{
    uint8_t i;

    dlog_info("%d %d \r\n", type, value);
    
    STRU_WIRELESS_CONFIG_CHANGE cmd;
    if(type == 0)
    {
        cmd.configClass = WIRELESS_FREQ_CHANGE;
        cmd.configItem  = FREQ_BAND_WIDTH_SELECT;
        cmd.configValue = value; //BW_10M; BW_20M
    }
 
    if(type == 2)
    {
        cmd.configClass = WIRELESS_FREQ_CHANGE;
        cmd.configItem  = FREQ_BAND_MODE;
        cmd.configValue = value; //AUTO, Manual
    }

    if(type == 3)
    {    
        cmd.configClass = WIRELESS_FREQ_CHANGE;
        cmd.configItem  = FREQ_BAND_SELECT; 
        cmd.configValue = value;    //0: 2G   1: 5G
    }

    if(type == 4)
    {    
        cmd.configClass = WIRELESS_FREQ_CHANGE;
        cmd.configItem  = FREQ_CHANNEL_MODE;
        cmd.configValue = value;   //AUTO,  Manual
    }

    if(type == 5)
    {    
        cmd.configClass = WIRELESS_FREQ_CHANGE;
        cmd.configItem  = FREQ_CHANNEL_SELECT;
        cmd.configValue = value;
    }

    if(type == 6)
    {
        cmd.configClass  = WIRELESS_MCS_CHANGE;
        cmd.configItem   = MCS_MODE_SELECT;
        cmd.configValue  = value; //AUTO, Manual
    }

    if(type == 7)
    {
        cmd.configClass  = WIRELESS_MCS_CHANGE;
        cmd.configItem   = MCS_MODULATION_SELECT;
        cmd.configValue  = value; //
    }

    if(type == 8)
    {
        cmd.configClass  = WIRELESS_ENCODER_CHANGE;
        cmd.configItem   = ENCODER_DYNAMIC_BIT_RATE_MODE;
        cmd.configValue  = value; //auto, manual
    }

    if(type == 9)
    {
        cmd.configClass  = WIRELESS_ENCODER_CHANGE;
        cmd.configItem   = ENCODER_DYNAMIC_BIT_RATE_SELECT;
        cmd.configValue  = value; //brc
    }

    dlog_info("%d %d  %d\r\n", cmd.configClass, cmd.configItem, cmd.configValue);

    uint8_t found = 0;    
    for(i = 0; i < sizeof(grd_cmds_buf)/sizeof(grd_cmds_buf[0]); i++)
    {
        if(grd_cmds_buf[i].avail == 0)
        {
            memcpy((void *)(&grd_cmds_buf[i].config), (void *)&cmd, sizeof(grd_cmds_buf[0]));
            grd_cmds_buf[i].avail = 1;
            found = 1;
            break;
        }
    }

    if(!found)
    {
        dlog_error("%s \r\n", "insert events to grd");
    }   
}


void grd_get_osd_info(void)
{
    STRU_WIRELESS_INFO_DISPLAY *osdptr = (STRU_WIRELESS_INFO_DISPLAY *)(OSD_STATUS_SHM_ADDR);

    static int osd_cnt = 0;
    static int ch = 0;
    if(osd_cnt++ > 2000)
    {
        osd_cnt = 0;
        osdptr->messageId = 0x33;
        osdptr->head = 0xff; //starting writing
        osdptr->tail = 0x00;

        osdptr->IT_channel = context.cur_IT_ch;

        osdptr->agc_value[0] = BB_ReadReg(PAGE2, AAGC_2_RD);
        osdptr->agc_value[1] = BB_ReadReg(PAGE2, AAGC_3_RD);

        osdptr->agc_value[2] = BB_ReadReg(PAGE2, RX3_GAIN_ALL_R);
        osdptr->agc_value[3] = BB_ReadReg(PAGE2, RX4_GAIN_ALL_R);

        osdptr->snr_vlaue[0] = get_snr_average(0);
        osdptr->snr_vlaue[1] = get_snr_average(1);
        osdptr->snr_vlaue[2] = get_snr_average(2);
        osdptr->snr_vlaue[3] = get_snr_average(3);

        osdptr->ldpc_error = (((uint16_t)BB_ReadReg(PAGE2, LDPC_ERR_HIGH_8)) << 8) | BB_ReadReg(PAGE2, LDPC_ERR_LOW_8);
        osdptr->harq_count = (BB_ReadReg(PAGE2, FEC_5_RD) >> 4);

        osdptr->modulation_mode = grd_get_IT_QAM();
        osdptr->code_rate       = grd_get_IT_LDPC();
        osdptr->ch_bandwidth    = context.CH_bandwidth; 
        
        memset(osdptr->sweep_energy, 0, sizeof(osdptr->sweep_energy));
        grd_get_sweep_noise(0, osdptr->sweep_energy);
        
        if(context.brc_mode == AUTO)
        {
            osdptr->encoder_bitrate = BB_map_modulation_to_br( (uint8_t)((osdptr->modulation_mode)<<6) | ((osdptr->ch_bandwidth) << 3) | (osdptr->code_rate));
        }
        else
        {
            osdptr->encoder_bitrate = context.brc_bps;
        }

        osdptr->head = 0x00;
        osdptr->tail = 0xff;    //end of the writing

        dlog_info("%d %d %d %d %d %d %d %d %d %d %d %d %d %d %d\r\n",
                    osdptr->IT_channel, 
                    osdptr->agc_value[0], osdptr->agc_value[1], osdptr->agc_value[2], osdptr->agc_value[3],
                    osdptr->snr_vlaue[0], osdptr->snr_vlaue[1], osdptr->snr_vlaue[2], osdptr->snr_vlaue[3],
                    osdptr->ldpc_error, osdptr->harq_count, osdptr->encoder_bitrate, context.brc_mode,
                    osdptr->modulation_mode, osdptr->code_rate);

        dlog_info("ch: %d %d %d %d %d %d %d %d %d \r\n", 
                   ch, 
                   osdptr->sweep_energy[ch*8],   osdptr->sweep_energy[ch*8+1], osdptr->sweep_energy[ch*8+2], osdptr->sweep_energy[ch*8+3],
                   osdptr->sweep_energy[ch*8+4], osdptr->sweep_energy[ch*8+5], osdptr->sweep_energy[ch*8+6], osdptr->sweep_energy[ch*8+7]
                 );
        ch++;
        if(ch >= 21)
        {
            ch = 0;
        }
    }
}
