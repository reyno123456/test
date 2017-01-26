#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "bb_sys_param.h"
#include "debuglog.h"
#include "interrupt.h"
#include "timer.h"
#include "reg_rw.h"
#include "bb_regs.h"
#include "bb_ctrl_internal.h"
#include "bb_snr_service.h"
#include "bb_grd_ctrl.h"
#include "bb_grd_sweep.h"
#include "gpio.h"


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
static DebugMode g_stGrdDebugMode = 
{
	.u8_enterDebugModeCnt 	= 0,
	.bl_enterDebugModeFlag 	= 0,
	.bl_isDebugMode 		= 0,
};

static void grd_enter_debug_mode(void);
static void grd_handle_debug_mode_cmd(uint8_t flag);
void BB_GRD_start(void)
{
    context.dev_state = INIT_DATA;
    context.qam_ldpc = 0;

    grd_set_txmsg_mcs_change(context.qam_ldpc);

    GPIO_SetMode(RED_LED_GPIO, GPIO_MODE_1);
    GPIO_SetPinDirect(RED_LED_GPIO, GPIO_DATA_DIRECT_OUTPUT);

    GPIO_SetMode(BLUE_LED_GPIO, GPIO_MODE_1);
    GPIO_SetPinDirect(BLUE_LED_GPIO, GPIO_DATA_DIRECT_OUTPUT);
    
    GPIO_SetPin(RED_LED_GPIO, 0);   //RED LED ON
    GPIO_SetPin(BLUE_LED_GPIO, 1);  //BLUE LED OFF
    
    BB_Grd_SetRCId(context.u8_flashId);
    Grd_Timer0_Init();
    Grd_Timer1_Init();

    BB_set_Rcfrq(context.freq_band, 0);
    grd_set_it_skip_freq(context.cur_IT_ch);
    grd_set_it_work_freq(context.freq_band, context.cur_IT_ch);

    grd_sweep_freq_init();

    reg_IrqHandle(BB_TX_ENABLE_VECTOR_NUM, wimax_vsoc_tx_isr, NULL);
	INTR_NVIC_SetIRQPriority(BB_TX_ENABLE_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_BB_TX,0));
    INTR_NVIC_EnableIRQ(BB_TX_ENABLE_VECTOR_NUM);
}


void BB_Grd_SetRCId(uint8_t *pu8_id)
{
    BB_WriteReg(PAGE2, GRD_RC_ID_BIT39_32_REG, pu8_id[0]);
    BB_WriteReg(PAGE2, GRD_RC_ID_BIT31_24_REG, pu8_id[1]);
    BB_WriteReg(PAGE2, GRD_RC_ID_BIT23_16_REG, pu8_id[2]);
    BB_WriteReg(PAGE2, GRD_RC_ID_BIT15_08_REG, pu8_id[3]);
    BB_WriteReg(PAGE2, GRD_RC_ID_BIT07_00_REG, pu8_id[4]);
    
    dlog_info("id[0~4]:0x%x 0x%x 0x%x 0x%x 0x%x",
               pu8_id[0], 
               pu8_id[1], 
               pu8_id[2], 
               pu8_id[3], 
               pu8_id[4]);
}

//---------------IT grd hop change--------------------------------

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
    if(context.dev_state == INIT_DATA)
    {
        if(is_it_sweep_finish())
        {
            init_sne_average_and_fluct();
            context.cur_IT_ch = get_best_freq();
            context.dev_state = FEC_UNLOCK;
        }
    }
    else if(context.dev_state == CHECK_FEC_LOCK || context.dev_state == FEC_LOCK)
    {
        context.locked = grd_is_bb_fec_lock();
        if(context.locked)
        {
            context.dev_state = FEC_LOCK;
            GPIO_SetPin(BLUE_LED_GPIO, 0);  //BLUE LED ON
            GPIO_SetPin(RED_LED_GPIO, 1);   //RED LED OFF
            
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
                GPIO_SetPin(BLUE_LED_GPIO, 1);  //BLUE LED OFF
                GPIO_SetPin(RED_LED_GPIO, 0);   //RED LED ON
                
                context.fec_unlock_cnt = 0;
                if(context.it_skip_freq_mode == AUTO)
                {
                    context.cur_IT_ch = get_next_best_freq(context.cur_IT_ch);
                    context.dev_state = FEC_UNLOCK;
                }

                if(context.qam_skip_mode == AUTO)
                {
                    context.qam_mode= MOD_BPSK;
                    context.ldpc    = LDPC_1_2;
                    context.CH_bandwidth = BW_10M;

                    context.qam_ldpc = 0;
                    grd_set_txmsg_mcs_change(context.qam_ldpc);
                }
            }
        }
    }
    else if(context.dev_state == FEC_UNLOCK )
    {
        if(context.it_skip_freq_mode == AUTO && context.freq_band == RF_2G) //for test 5G only
        {
            grd_set_it_skip_freq(context.cur_IT_ch);
            dlog_info("Ch=>%d\n", context.cur_IT_ch);
        }
        context.dev_state = DELAY_14MS;
    }
    else if(context.dev_state == DELAY_14MS)
    {
        #if 0
        if(context.enable_freq_offset == ENABLE_FLAG)
        {
            bb_set_freq_offset(calu_it_skip_freq_delta(context.first_freq_value,context.cur_IT_ch));
        }
        #endif
        if(context.it_manual_ch != 0xff)
        {
            grd_set_it_work_freq(context.freq_band, context.it_manual_ch);
            context.it_manual_ch = 0xff;
        }
        else
        {
            grd_set_it_work_freq(context.freq_band, context.cur_IT_ch);
        }

        if(context.it_manual_rf_band != 0xff && context.freq_band!= context.it_manual_rf_band)
        {
            BB_set_RF_Band(BB_GRD_MODE, context.it_manual_rf_band);
            context.it_manual_rf_band = 0xff;
            context.freq_band = context.it_manual_rf_band;
        }
        reset_it_span_cnt();
        context.dev_state = CHECK_FEC_LOCK;
    }
    
    if(context.it_manual_ch != 0xff)
    {
        grd_set_it_skip_freq(context.it_manual_ch);
        context.dev_state = DELAY_14MS;
        dlog_info("ch switch Manual=>%d\n", context.it_manual_ch);
    }
    
    if(context.freq_band != context.it_manual_rf_band && context.it_manual_rf_band != 0xff)
    {
        dlog_info("To switch band: %d %d\r\n", context.freq_band, context.it_manual_rf_band);
        context.dev_state = DELAY_14MS;
    }
}

void grd_freq_skip_judge(void)
{    
    if(context.dev_state != FEC_LOCK)
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
       context.dev_state = FEC_UNLOCK;
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

const uint16_t snr_skip_threshold[7] = {0x0021, 0x004e,0x0090,0x00be,0x01fd,0x055e,0x07ec};
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

    #if 0
    iMCS = snr_skip_threshold[qam_ldpc];
    snr_if = is_snr_ok(iMCS);
    if(snr_if)
    {
        return 0;
    }
    #endif

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
        dlog_info("ML:%d\r\n", data);
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
        dlog_info("-QAM:%d ",qam);
    }

    return qam;
}


void grd_set_txmsg_qam_change(ENUM_BB_QAM qam, ENUM_CH_BW bw, ENUM_BB_LDPC ldpc)
{
    uint8_t data = (qam << 6) | (bw << 3) | ldpc;
    dlog_info("MCS1=>%d\n", data);

    BB_WriteReg(PAGE2, QAM_CHANGE_0, data);
    BB_WriteReg(PAGE2, QAM_CHANGE_1, data+1);
}


void grd_set_txmsg_mcs_change(uint8_t index )
{
    BB_WriteReg(PAGE2, MCS_INDEX_MODE_0, index);
    BB_WriteReg(PAGE2, MCS_INDEX_MODE_1, index +1);

    dlog_info("MCS2=>%d\n", index);
}


void up_down_qamldpc(QAMUPDONW up_down)
{
    if (context.qam_skip_mode == MANUAL)
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

    grd_set_txmsg_mcs_change( context.qam_ldpc );
}


void grd_qam_change_judge(void)
{
    uint8_t snr_statice_value;

    if(!context.locked)
    {
        return;
    }

    snr_statice_value = snr_static_for_qam_change(context.qam_threshold_range[context.qam_ldpc][0],
                                                  context.qam_threshold_range[context.qam_ldpc][1]);
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

    return;
}

///////////////////////////////////////////////////////////////////////////////////

void wimax_vsoc_tx_isr(uint32_t u32_vectorNum)
{
    INTR_NVIC_DisableIRQ(BB_TX_ENABLE_VECTOR_NUM);
    STRU_WIRELESS_INFO_DISPLAY *osdptr = (STRU_WIRELESS_INFO_DISPLAY *)(SRAM_BB_STATUS_SHARE_MEMORY_ST_ADDR);
    grd_enter_debug_mode();
    
    TIM_StartTimer(init_timer0_0);
    INTR_NVIC_EnableIRQ(TIMER_INTR00_VECTOR_NUM);
}

void Grd_TIM0_IRQHandler(uint32_t u32_vectorNum)
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

void Grd_TIM1_IRQHandler(uint32_t u32_vectorNum)
{
    Reg_Read32(BASE_ADDR_TIMER0 + TMRNEOI_1); //disable the intr.
    grd_handle_all_cmds();
    
    if(1 == (g_stGrdDebugMode.bl_isDebugMode))
    {
        INTR_NVIC_DisableIRQ(TIMER_INTR01_VECTOR_NUM);                
        TIM_StopTimer(init_timer0_1);
    }
    else
    {
        grd_add_snr_daq();
        switch (Timer1_Delay1_Cnt)
        {
            case 0:
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
                BB_grd_GatherOSDInfo();
                break;

            case 7:
                Timer1_Delay1_Cnt++;
                if(context.it_skip_freq_mode == AUTO && context.freq_band == RF_2G)
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
}

void Grd_Timer1_Init(void)
{
    init_timer0_1.base_time_group = 0;
    init_timer0_1.time_num = 1;
    init_timer0_1.ctrl = 0;
    init_timer0_1.ctrl |= TIME_ENABLE | USER_DEFINED;
    TIM_RegisterTimer(init_timer0_1, 1200); //1.25ms
    reg_IrqHandle(TIMER_INTR01_VECTOR_NUM, Grd_TIM1_IRQHandler, NULL);
	INTR_NVIC_SetIRQPriority(TIMER_INTR01_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_TIMER01,0));
}

void Grd_Timer0_Init(void)
{
    init_timer0_0.base_time_group = 0;
    init_timer0_0.time_num = 0;
    init_timer0_0.ctrl = 0;
    init_timer0_0.ctrl |= TIME_ENABLE | USER_DEFINED;
    
    TIM_RegisterTimer(init_timer0_0, 2500); //2.5s
    reg_IrqHandle(TIMER_INTR00_VECTOR_NUM, Grd_TIM0_IRQHandler, NULL);
	INTR_NVIC_SetIRQPriority(TIMER_INTR00_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_TIMER00,0));
}


//=====================================Grd RC funcions =====
uint8_t grd_rc_channel = 0;
void grd_rc_hopfreq(void)
{
	uint8_t max_ch_size = (context.freq_band == RF_2G) ? (MAX_2G_RC_FRQ_SIZE):(MAX_5G_RC_FRQ_SIZE);

    grd_rc_channel++;
    if(grd_rc_channel >= max_ch_size)
    {
        grd_rc_channel = 0;
    }
	BB_set_Rcfrq(context.freq_band, grd_rc_channel);
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


void grd_handle_IT_mode_cmd(ENUM_RUN_MODE mode)
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
static void grd_handle_RC_mode_cmd(ENUM_RUN_MODE mode)
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
	BB_set_Rcfrq(context.freq_band, ch);

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
    if(context.freq_band != rf_band)
    {
        BB_WriteReg(PAGE2, RF_BAND_CHANGE_0, 0xc0 + (uint8_t)rf_band);
        BB_WriteReg(PAGE2, RF_BAND_CHANGE_1, 0xc0 + (uint8_t)rf_band + 1);

        context.it_manual_rf_band = rf_band;
        dlog_info("To rf_band %d %d\r\n", context.freq_band, rf_band);
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


static void grd_handle_MCS_mode_cmd(ENUM_RUN_MODE mode)
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
static void grd_handle_brc_mode_cmd(ENUM_RUN_MODE mode)
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


void grd_handle_one_cmd(STRU_WIRELESS_CONFIG_CHANGE* pcmd)
{
    uint8_t class  = pcmd->u8_configClass;
    uint8_t item   = pcmd->u8_configItem;
    uint32_t value = pcmd->u32_configValue;

    dlog_info("class item value %d %d 0x%0.8x \r\n", class, item, value);
    if(class == WIRELESS_FREQ_CHANGE)
    {
        switch(item)
        {
            case FREQ_BAND_MODE:
            {
                //band mode: AUTO MANUAL, only suppor the Manual mode
                break;
            }

            case FREQ_BAND_SELECT:
            {
                grd_handle_RF_band_cmd((ENUM_RF_BAND)value);
                break;
            }

            case FREQ_CHANNEL_MODE: //auto manual
            {
                grd_handle_IT_mode_cmd((ENUM_RUN_MODE)value);
                break;
            }
            
            case FREQ_CHANNEL_SELECT:
            {
                grd_handle_IT_CH_cmd((uint8_t)value);
                break;
            }

            case RC_CHANNEL_MODE:
            {
                grd_handle_RC_mode_cmd( (ENUM_RUN_MODE)value);
                break;
            }

            case RC_CHANNEL_SELECT:
            {
                grd_handle_RC_mode_cmd( (ENUM_RUN_MODE)MANUAL);
                grd_handle_RC_CH_cmd((uint8_t)value);
                break;
            }
            
            case RC_CHANNEL_FREQ:
            {
                grd_handle_RC_mode_cmd( (ENUM_RUN_MODE)MANUAL);
                BB_write_RcRegs(value);
                dlog_info("RC_CHANNEL_FREQ %x\r\n", value);                
                break;
            }

            case IT_CHANNEL_FREQ:
            {
                grd_handle_IT_mode_cmd( (ENUM_RUN_MODE)MANUAL);
                BB_write_ItRegs(value);
                dlog_info("IT_CHANNEL_FREQ %x\r\n", value);                
                break;
            }

            default:
            {
                dlog_error("%s\r\n", "unknown WIRELESS_FREQ_CHANGE command");
                break;
            }
        }
    }

    if(class == WIRELESS_MCS_CHANGE)
    {
        switch(item)
        {
            case MCS_MODE_SELECT:
                grd_handle_MCS_mode_cmd((ENUM_RUN_MODE)value);
                break;

            case MCS_MODULATION_SELECT:
                {
                    #if 0
                    ENUM_BB_LDPC ldpc = (ENUM_BB_LDPC)(value&0x0f);
                    ENUM_BB_QAM  qam  = (ENUM_BB_QAM)((value >> 4)&0x0f);
                    grd_handle_MCS_cmd(qam, ldpc);
                    #endif
                    grd_set_txmsg_mcs_change(value);
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
                grd_handle_brc_mode_cmd( (ENUM_RUN_MODE)value);
                break;

            case ENCODER_DYNAMIC_BIT_RATE_SELECT:
                grd_handle_brc_bitrate_cmd( (uint8_t)value);
                break;

            default:
                dlog_error("%s\r\n", "unknown WIRELESS_ENCODER_CHANGE command");
                break;                
        }
    }
	
	if(class == WIRELESS_DEBUG_CHANGE)
    {
        switch(item)
        {
            case 0:
                grd_handle_debug_mode_cmd( (uint8_t)value);
                break;

            default:
                dlog_error("%s\r\n", "unknown WIRELESS_DEBUG_CHANGE command");
                break;                
        }
    }

    if(class == WIRELESS_MISC)
    {
        BB_handle_misc_cmds(pcmd);
    }
}


static void grd_handle_all_cmds(void)
{   
    int ret = 0;
    int cnt = 0;
    STRU_WIRELESS_CONFIG_CHANGE cfg;
    while( (cnt++ < 5) && (1 == BB_GetCmd(&cfg)))
    {
        grd_handle_one_cmd( &cfg );
    }
}

/*
 *
*/
static void BB_grd_GatherOSDInfo(void)
{
    STRU_WIRELESS_INFO_DISPLAY *osdptr = (STRU_WIRELESS_INFO_DISPLAY *)(SRAM_BB_STATUS_SHARE_MEMORY_ST_ADDR);

    osdptr->messageId = 0x33;
    osdptr->head = 0xff; //starting writing
    osdptr->tail = 0x00;

    osdptr->IT_channel = context.cur_IT_ch;

    osdptr->agc_value[0] = BB_ReadReg(PAGE2, AAGC_2_RD);
    osdptr->agc_value[1] = BB_ReadReg(PAGE2, AAGC_3_RD);
    osdptr->agc_value[2] = BB_ReadReg(PAGE2, FEC_5_RD);

    //osdptr->agc_value[2] = BB_ReadReg(PAGE2, RX3_GAIN_ALL_R);
    osdptr->agc_value[3] = BB_ReadReg(PAGE2, RX4_GAIN_ALL_R);
    osdptr->lock_status  = BB_ReadReg(PAGE2, FEC_5_RD);
    osdptr->snr_vlaue[0] = get_snr_average(0);
    osdptr->snr_vlaue[1] = get_snr_average(1);
    osdptr->snr_vlaue[2] = get_snr_average(2);
    osdptr->snr_vlaue[3] = get_snr_average(3);

    osdptr->ldpc_error = (((uint16_t)BB_ReadReg(PAGE2, LDPC_ERR_HIGH_8)) << 8) | BB_ReadReg(PAGE2, LDPC_ERR_LOW_8);
    osdptr->harq_count = (BB_ReadReg(PAGE2, FEC_5_RD) >> 4);
    uint8_t tmp = BB_ReadReg(PAGE2, 0xdd);
    #if 0
    if(osdptr->ldpc_error > 0 || tmp > 0)
    {
        dlog_info("err:%x harq:%x lost:%x\n", osdptr->ldpc_error, osdptr->harq_count, tmp);
    }
    #endif
    osdptr->modulation_mode = grd_get_IT_QAM();
    osdptr->code_rate       = grd_get_IT_LDPC();
    osdptr->ch_bandwidth    = context.CH_bandwidth;         
	osdptr->in_debug        = (uint8_t)(g_stGrdDebugMode.bl_isDebugMode);
    osdptr->lock_status     = BB_ReadReg(PAGE2, FEC_5_RD);
    memset(osdptr->sweep_energy, 0, sizeof(osdptr->sweep_energy));
    grd_get_sweep_noise(0, osdptr->sweep_energy);
    
    if(context.brc_mode == AUTO)
    {
        osdptr->encoder_bitrate = context.qam_ldpc;
    }
    else
    {
        osdptr->encoder_bitrate = context.brc_bps;
    }

    osdptr->head = 0x00;
    osdptr->tail = 0xff;    //end of the writing
}

/*
  * flag:enter/out debug mode flag
 */
static void grd_handle_debug_mode_cmd(uint8_t flag)
{
	if(!flag)//enter debug mode
	{
		//notification sky enter/out debug mode
		BB_WriteReg(PAGE2, NTF_TEST_MODE_0, flag);
		BB_WriteReg(PAGE2, NTF_TEST_MODE_1, flag+1);
		//command_TestGpioNormal2(64,1);	
		if(0 == (g_stGrdDebugMode.bl_isDebugMode))
		{
			g_stGrdDebugMode.u8_enterDebugModeCnt = 0;
			//start cnt
			g_stGrdDebugMode.bl_enterDebugModeFlag = 1;
			dlog_info("g_stGrdDebugMode.bl_enterDebugModeFlag = 1");
		}
		else
		{
			//already debug mode,do nothing.	
		}
	}
	else	//out debug mode
	{
		g_stGrdDebugMode.u8_enterDebugModeCnt = 0;
		g_stGrdDebugMode.bl_enterDebugModeFlag = 0;
		g_stGrdDebugMode.bl_isDebugMode	= 0;
	}
	
   
   	dlog_info("flag =%d\r\n", flag);    
}

static void grd_enter_debug_mode(void)
{
    STRU_WIRELESS_INFO_DISPLAY *osdptr = (STRU_WIRELESS_INFO_DISPLAY *)(SRAM_BB_STATUS_SHARE_MEMORY_ST_ADDR);
	if(1 == (g_stGrdDebugMode.bl_enterDebugModeFlag))
	{
		if((g_stGrdDebugMode.u8_enterDebugModeCnt) < 30)	
		{
			(g_stGrdDebugMode.u8_enterDebugModeCnt) += 1;
		}
		else
		{
			g_stGrdDebugMode.u8_enterDebugModeCnt = 0;
			g_stGrdDebugMode.bl_enterDebugModeFlag = 0;
			//now grd really enter test mode
			g_stGrdDebugMode.bl_isDebugMode	= 1;
			dlog_info("g_stGrdDebugMode.bl_isDebugMode	= 1");
            osdptr->in_debug          = (uint8_t)(g_stGrdDebugMode.bl_isDebugMode);
            context.rc_skip_freq_mode = MANUAL;
            context.it_skip_freq_mode = MANUAL;
            context.qam_skip_mode     = MANUAL;
		}
	}
	else
	{
		//do nothing
	}	
}
