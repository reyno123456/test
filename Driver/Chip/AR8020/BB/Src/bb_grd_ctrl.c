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
#include "bb_uart_com.h"


#define SNR_STATIC_START_VALUE      (100)
#define SNR_STATIC_UP_THRESHOD      (5)
#define SNR_STATIC_DOWN_THRESHOD    (2)

static init_timer_st grd_timer2_6;
static init_timer_st grd_timer2_7;
static uint8_t Timer1_Delay1_Cnt = 0;
static uint8_t snr_static_count = SNR_STATIC_START_VALUE;
static uint8_t hop_count = 0;
static uint8_t flag_itFreqskip = 0;
static uint8_t flag_snrPostCheck;
static uint64_t s_u64_rcMask = 0;
static uint8_t s_u8_rcMaskEnable = 0;

static void BB_grd_uartDataHandler(void *p);
static void grd_calc_dist(void);
static uint32_t grd_calc_dist_get_avg_value(uint32_t *u32_dist);
static void grd_init_rc_frq_mask_func(void);
static void grd_write_mask_code_to_sky(uint8_t enable, uint64_t *mask);


STRU_CALC_DIST_DATA s_st_calcDistData = 
{
    .e_status = INVALID,
    .u32_calcDistValue = 0,
    .u32_calcDistZero = 0,
    .u32_cnt = 0,
    .u32_lockCnt = 0
};


void BB_GRD_start(void)
{
    context.dev_state = INIT_DATA;
    context.qam_ldpc = 0;
    context.flag_mrs = 0;

    grd_set_txmsg_mcs_change(context.qam_ldpc);

    GPIO_SetMode(RED_LED_GPIO, GPIO_MODE_2);
    GPIO_SetPinDirect(RED_LED_GPIO, GPIO_DATA_DIRECT_OUTPUT);

    GPIO_SetMode(BLUE_LED_GPIO, GPIO_MODE_2);
    GPIO_SetPinDirect(BLUE_LED_GPIO, GPIO_DATA_DIRECT_OUTPUT);
    
    GPIO_SetPin(RED_LED_GPIO, 0);   //RED LED ON
    GPIO_SetPin(BLUE_LED_GPIO, 1);  //BLUE LED OFF
    
    BB_Grd_SetRCId((uint8_t*)(context.u8_flashId));
    Grd_Timer2_6_Init();
    Grd_Timer2_7_Init();

    BB_set_Rcfrq(context.freq_band, 0);

    //do not notify sky until sweep end and get the best channel from sweep result
    context.cur_IT_ch = 0;
    BB_set_ITfrq(context.freq_band, 0);
    BB_grd_notify_it_skip_freq(context.freq_band, 1); 

    BB_SweepStart(context.freq_band, context.CH_bandwidth);

    reg_IrqHandle(BB_TX_ENABLE_VECTOR_NUM, wimax_vsoc_tx_isr, NULL);
    INTR_NVIC_SetIRQPriority(BB_TX_ENABLE_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_BB_TX,0));
    INTR_NVIC_EnableIRQ(BB_TX_ENABLE_VECTOR_NUM);

    BB_UARTComInit( BB_grd_uartDataHandler ); 
    BB_GetDevInfo();

    grd_init_rc_frq_mask_func();
}


static void BB_grd_uartDataHandler(void *p)
{
    STRU_SysEventSkyStatus  status;
    uint64_t tmpMaskCode = 0;

    uint32_t u32_rcvLen = BB_UARTComReceiveMsg(BB_UART_COM_SESSION_0,
                          (uint8_t *)&status,
                          sizeof(STRU_SysEventSkyStatus));

    if ( u32_rcvLen  >= sizeof(STRU_SysEventSkyStatus))
    {
        if (RC_LOCK_CNT == status.pid)
        {
            dlog_info("rclock:%d  nrlock:%d", status.par.rcLockCnt.u8_rcCrcLockCnt, status.par.rcLockCnt.u8_rcNrLockCnt);
        }
        else if (RC_MASK_CODE == status.pid)
        {
            tmpMaskCode = s_u64_rcMask;
            s_u64_rcMask = status.par.u64_rcMask;
            grd_write_mask_code_to_sky(s_u8_rcMaskEnable, &s_u64_rcMask);
            if (tmpMaskCode != s_u64_rcMask)
            {
                dlog_info("rcv_sky_calc_mask_code:0x%x,0x%x", (uint32_t)((s_u64_rcMask)>>32), (uint32_t)(s_u64_rcMask));
            }
        }
        else
        {

        }
    }
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

uint32_t it_span_cnt = 0;
void reset_it_span_cnt(void)
{
    it_span_cnt = 0;
}

void grd_fec_judge(void)
{
    if( context.flag_mrs == 1 )
    {
        context.flag_mrs = 2;
        BB_WriteRegMask(PAGE1, 0x83, 0x01, 0x01); 
        dlog_info("Disable %d\n", context.cycle_count);        
    }
    else if( context.flag_mrs == 2 )
    {
        context.flag_mrs = 0;
        BB_WriteRegMask(PAGE1, 0x83, 0x00, 0x01);   
        dlog_info("Enable %d\n", context.cycle_count);        
    }

    if( context.dev_state == INIT_DATA )
    {
        context.locked = grd_is_bb_fec_lock();
        if( !context.locked )
        {
            if (context.fec_unlock_cnt ++ > 20 )
            {
                if( context.it_skip_freq_mode == AUTO )
                {
                    uint8_t mainch, optch;
                    if (BB_selectBestCh(SELECT_MAIN_OPT, &mainch, &optch, (uint8_t *)NULL, 0))
                    {
                        context.cur_IT_ch  = mainch;
                        BB_Sweep_updateCh(mainch);
                        BB_set_ITfrq(context.freq_band, mainch);
                        //dlog_info("Select %d %d", mainch, optch);
                        BB_grd_notify_it_skip_freq(context.freq_band, context.cur_IT_ch);                    
                    }
                }
                context.fec_unlock_cnt = 0;
            }
        }
        else
        {
            context.dev_state = CHECK_FEC_LOCK;
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
                    uint8_t mainch, optch;
                    BB_selectBestCh(SELECT_MAIN_OPT, &mainch, &optch, NULL, 0);

                    context.cur_IT_ch  = mainch;
                    BB_Sweep_updateCh(mainch);
                    BB_set_ITfrq(context.freq_band, mainch);
                    BB_grd_notify_it_skip_freq(context.freq_band, context.cur_IT_ch);
                    dlog_info("unlock: select channel %d %d", mainch, optch);
                }

                if(context.qam_skip_mode == AUTO)
                {
                    context.qam_mode= MOD_BPSK;
                    context.ldpc    = LDPC_1_2;

                    context.qam_ldpc = 0;
                    grd_set_txmsg_mcs_change(context.qam_ldpc);
                }
            }
        }
    }
    else if(context.dev_state == FEC_UNLOCK )
    {
        if(context.it_skip_freq_mode == AUTO )
        {
            BB_grd_notify_it_skip_freq(context.freq_band, context.cur_IT_ch);
            dlog_info("Set Ch:%d %d %d %x %x %x\n", context.cur_IT_ch, context.cycle_count,  ((BB_ReadReg(PAGE2, FEC_5_RD)& 0xF0) >> 4), grd_get_it_snr(),
                        (((uint16_t)BB_ReadReg(PAGE2, 0xd7)) << 8) | BB_ReadReg(PAGE2, 0xd8)
                     );
        }
        context.dev_state = DELAY_14MS;
    }
    else if(context.dev_state == DELAY_14MS)
    {
        if( context.it_skip_freq_mode == AUTO )
        {
            if(context.it_manual_ch != 0xff)
            {
                BB_set_ITfrq(context.freq_band, context.it_manual_ch);
                context.it_manual_ch = 0xff;
                context.flag_mrs = 1;
            }
            else
            {
                BB_set_ITfrq(context.freq_band, context.cur_IT_ch);
                dlog_info("Hop:%d Harq:%d\n", context.cycle_count, ((BB_ReadReg(PAGE2, FEC_5_RD)& 0xF0) >> 4));
                context.flag_mrs = 1; 
            }
        }

        if(context.it_manual_rf_band != 0xff && context.freq_band!= context.it_manual_rf_band)
        {
            BB_set_RF_Band(BB_GRD_MODE, context.it_manual_rf_band);
            context.freq_band = context.it_manual_rf_band;
            context.it_manual_rf_band = 0xff;
        }
        reset_it_span_cnt();
        context.dev_state = CHECK_FEC_LOCK;
    }

    if(context.freq_band != context.it_manual_rf_band && context.it_manual_rf_band != 0xff)
    {
        dlog_info("To band: %d %d", context.freq_band, context.it_manual_rf_band);
        context.dev_state = DELAY_14MS;
    }
}






const uint16_t snr_skip_threshold[] = { 0x23,    //bpsk 1/2
                                        0x2d,    //bpsk 1/2
                                        0x6c,    //qpsk 1/2
                                        0x181,   //16QAM 1/2
                                        0x492,   //64QAM 1/2
                                        0x52c};  //64QAM 2/3

int grd_freq_skip_pre_judge(void)
{
    uint8_t flag = 0;
    int16_t aver, fluct;
    uint8_t bestch, optch;    
    
    if ( context.dev_state != FEC_LOCK )
    {
        return 0;
    }
    
    if( it_span_cnt < 16 )
    {
        it_span_cnt ++;
        return 0;
    }

    uint8_t Harqcnt  = ((BB_ReadReg(PAGE2, FEC_5_RD)& 0xF0) >> 4);
    uint8_t Harqcnt1 = ((BB_ReadReg(PAGE2, 0xd1)& 0xF0) >> 4);
    //if(Harqcnt > 0)
    //{
    //    dlog_info("Harq %d:%d %d snr:%x %x %d ret=%d", Harqcnt, Harqcnt1, context.cycle_count, grd_get_it_snr(), 
    //                                        (((uint16_t)BB_ReadReg(PAGE2, 0xc2)) << 8) | BB_ReadReg(PAGE2, 0xc3),
    //                                        (((uint16_t)BB_ReadReg(PAGE2, 0xd7)) << 8) | BB_ReadReg(PAGE2, 0xd8));
    //}

    if (Harqcnt >= 7 && Harqcnt1 >= 7)      //check LDPC error
    {
        dlog_info("Harq: %d %d", Harqcnt, Harqcnt1);
        flag = 0;
    }
    else if (Harqcnt >= 3 && Harqcnt1 >= 3) //check snr.
    {
        flag = grd_check_piecewiseSnrPass(1, snr_skip_threshold[context.qam_ldpc]);
        if ( 1 == flag ) //snr pass
        {
            return 0;
        }
    }
    else
    {
        return 0;
    }

    optch = get_opt_channel();
    int16_t cmp = compare_chNoisePower(optch, context.cur_IT_ch, &aver, &fluct, context.freq_band, 0);
    //dlog_info("cmp:(%d:%d)result:(%d %d)", context.cur_IT_ch, optch, aver, fluct);
    if ( ( aver >= 20 ) || ( aver >= 10 && fluct > 0 )  )
    {
        if( 0 == flag ) //already Fail
        {
            reset_it_span_cnt( );
            context.cur_IT_ch  = optch;
            BB_Sweep_updateCh( context.cur_IT_ch );

            BB_grd_notify_it_skip_freq(context.freq_band, context.cur_IT_ch);
            
            dlog_info("Set Ch0:%d %d %d %x\n", 
                       context.cur_IT_ch, context.cycle_count, ((BB_ReadReg(PAGE2, FEC_5_RD)& 0xF0) >> 4), grd_get_it_snr());
            context.dev_state = DELAY_14MS;
			return 0;
        }
        else
        {
            context.next_IT_ch = optch;
            return 1;  //need to check in the next 1.25ms
        }
    }
    else
    {
        return 0;
    }
}


void grd_freq_skip_post_judge(void)
{
    if(context.dev_state != FEC_LOCK )
    {
        return;
    }

    if ( 0 == grd_check_piecewiseSnrPass( 0, snr_skip_threshold[context.qam_ldpc] ) ) //Fail, need to hop
    {
        if( context.cur_IT_ch != context.next_IT_ch )
        {
            reset_it_span_cnt( );
            context.cur_IT_ch = context.next_IT_ch;
            BB_Sweep_updateCh( context.cur_IT_ch );
            BB_grd_notify_it_skip_freq(context.freq_band, context.cur_IT_ch);
            dlog_info("Set Ch1:%d %d %d %x\n", context.cur_IT_ch, context.cycle_count, 
                                             ((BB_ReadReg(PAGE2, FEC_5_RD)& 0xF0) >> 4), grd_get_it_snr());
        }

        context.dev_state = DELAY_14MS;        
    }
}


uint8_t grd_is_bb_fec_lock(void)
{
    static uint8_t status = 0xff;
    uint8_t data = BB_ReadReg(PAGE2, FEC_5_RD) & 0x01;
    if(status != data)
    {
        dlog_info("ML:%d", data);
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
    dlog_info("MCS1=>%d", data);

    BB_WriteReg(PAGE2, QAM_CHANGE_0, data);
    BB_WriteReg(PAGE2, QAM_CHANGE_1, data+1);
}


///////////////////////////////////////////////////////////////////////////////////

void wimax_vsoc_tx_isr(uint32_t u32_vectorNum)
{
    INTR_NVIC_DisableIRQ(BB_TX_ENABLE_VECTOR_NUM);
    STRU_WIRELESS_INFO_DISPLAY *osdptr = (STRU_WIRELESS_INFO_DISPLAY *)(SRAM_BB_STATUS_SHARE_MEMORY_ST_ADDR);
    STRU_DEVICE_INFO *pst_devInfo = (STRU_DEVICE_INFO *)(DEVICE_INFO_SHM_ADDR);

    if( context.u8_flagdebugRequest & 0x80)
    {
        context.u8_debugMode = context.u8_flagdebugRequest & 0x01;

        if( context.u8_debugMode != FALSE )
        {
            grd_handle_RC_mode_cmd(MANUAL);
            grd_handle_MCS_mode_cmd(MANUAL);
            grd_handle_IT_mode_cmd(MANUAL);

            osdptr->head = 0x00;
            osdptr->tail = 0xff;    //end of the writing
        }

        if (context.u8_debugMode == TRUE)
        {
            BB_SPI_DisableEnable(0); //
        }
        else
        {
            BB_SPI_DisableEnable(1); //
        }

        osdptr->in_debug = context.u8_debugMode;        
        pst_devInfo->isDebug = context.u8_debugMode;
        
        context.u8_flagdebugRequest = 0;
        dlog_info("bugMode %d %d\n", osdptr->in_debug, context.u8_debugMode);
    }

    {
        TIM_StartTimer(grd_timer2_6);
        INTR_NVIC_EnableIRQ(TIMER_INTR26_VECTOR_NUM);
    }
    context.cycle_count ++;
}

void Grd_TIM2_6_IRQHandler(uint32_t u32_vectorNum)
{
    Reg_Read32(BASE_ADDR_TIMER2 + TMRNEOI_6);

    //Enable BB_TX intr
    INTR_NVIC_ClearPendingIRQ(BB_TX_ENABLE_VECTOR_NUM); //clear pending after TX Enable is LOW. MUST
    INTR_NVIC_EnableIRQ(BB_TX_ENABLE_VECTOR_NUM);

    //Disable TIM0 intr
    INTR_NVIC_DisableIRQ(TIMER_INTR26_VECTOR_NUM);
    TIM_StopTimer(grd_timer2_6);
    
    //Enable TIM1 intr
    TIM_StartTimer(grd_timer2_7);
    INTR_NVIC_EnableIRQ(TIMER_INTR27_VECTOR_NUM);

    if ( FALSE == context.u8_debugMode )
    {
        grd_handle_all_cmds();
        //grd_CheckSdramBuffer();
    }
	Timer1_Delay1_Cnt = 0;
}

uint8_t agc_level_max[50];
uint8_t agc_idx = 0;
void Grd_TIM2_7_IRQHandler(uint32_t u32_vectorNum)
{
    STRU_WIRELESS_INFO_DISPLAY *osdptr = (STRU_WIRELESS_INFO_DISPLAY *)(SRAM_BB_STATUS_SHARE_MEMORY_ST_ADDR);
    Reg_Read32(BASE_ADDR_TIMER2 + TMRNEOI_7); //disable the intr.

    if ( context.u8_debugMode )
    {
        INTR_NVIC_DisableIRQ(TIMER_INTR27_VECTOR_NUM);                
        TIM_StopTimer(grd_timer2_7);    
        return;
    }

    switch (Timer1_Delay1_Cnt)
    {
        case 0:
            BB_GetSweepResult( 0 );            
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
            if ( context.locked )
            {
                uint8_t  agc1, agc2;
                uint8_t  cnt = 0;
                uint16_t agcsum = 0;
                if ( agc_idx > sizeof(agc_level_max) )
                {
                    agc_idx = 0;
                }                

                agc1 = BB_ReadReg(PAGE2, AAGC_2_RD);
                agc2 = BB_ReadReg(PAGE2, AAGC_3_RD);
                agc_level_max[agc_idx] = (agc1 > agc2) ? agc1 : agc2;
                agc_idx ++;
                for(; cnt < sizeof(agc_level_max); cnt++ )
                {
                    agcsum += agc_level_max[cnt];
                }

                context.agclevel = agcsum / sizeof(agc_level_max);
            }

            if ( context.flag_updateRc > 0 )
            {
                if ( context.flag_updateRc ++ > 100 )
                {
                    context.flag_updateRc = 0;
                    if( context.u32_rcValue )
                    {
                        BB_write_RcRegs( context.u32_rcValue );
                        context.rc_skip_freq_mode = MANUAL;
                    }
                    else //if context.u32_rcValue ==0, means the auto mode
                    {
                        context.rc_skip_freq_mode = AUTO;
                    }
                }
            }

            Timer1_Delay1_Cnt++;
            break;

        case 3:
            Timer1_Delay1_Cnt++;
            BB_GetDevInfo();
            grd_judge_qam_mode();
            break;

        case 4:
            Timer1_Delay1_Cnt++;
            if ((context.dev_state == FEC_LOCK) && (context.locked))
            {
                s_st_calcDistData.u32_lockCnt += 1;
                if (s_st_calcDistData.u32_lockCnt >= 3)
                {
                    s_st_calcDistData.u32_lockCnt = 3;
                    grd_calc_dist();
                }
                
            }
            else
            {
                s_st_calcDistData.u32_lockCnt = 0;
            }
            break;

        case 5:
            Timer1_Delay1_Cnt++;
            BB_grd_GatherOSDInfo();
            break;

        case 6:
            Timer1_Delay1_Cnt++;
            if(context.it_skip_freq_mode == AUTO)
            {
                flag_snrPostCheck = grd_freq_skip_pre_judge( );
            }
            break;

        case 7:
            Timer1_Delay1_Cnt++;
            INTR_NVIC_DisableIRQ(TIMER_INTR27_VECTOR_NUM);
            TIM_StopTimer(grd_timer2_7);
            if( context.it_skip_freq_mode == AUTO && flag_snrPostCheck )
            {
                grd_freq_skip_post_judge( );
            }

            break;

        default:
            Timer1_Delay1_Cnt = 0;
            break;
    }
}

void Grd_Timer2_7_Init(void)
{
    grd_timer2_7.base_time_group = 2;
    grd_timer2_7.time_num = 7;
    grd_timer2_7.ctrl = 0;
    grd_timer2_7.ctrl |= TIME_ENABLE | USER_DEFINED;
    TIM_RegisterTimer(grd_timer2_7, 1250); //1.25ms
    reg_IrqHandle(TIMER_INTR27_VECTOR_NUM, Grd_TIM2_7_IRQHandler, NULL);
    INTR_NVIC_SetIRQPriority(TIMER_INTR27_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_TIMER01,0));
}

void Grd_Timer2_6_Init(void)
{
    grd_timer2_6.base_time_group = 2;
    grd_timer2_6.time_num = 6;
    grd_timer2_6.ctrl = 0;
    grd_timer2_6.ctrl |= TIME_ENABLE | USER_DEFINED;
    
    TIM_RegisterTimer(grd_timer2_6, 3500); //2.5s
    reg_IrqHandle(TIMER_INTR26_VECTOR_NUM, Grd_TIM2_6_IRQHandler, NULL);
    INTR_NVIC_SetIRQPriority(TIMER_INTR26_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_TIMER00,0));
}


//=====================================Grd RC funcions =====
uint8_t grd_rc_channel = 0;
void grd_rc_hopfreq(void)
{
	uint8_t max_ch_size = (context.freq_band == RF_2G) ? MAX_2G_RC_FRQ_SIZE : MAX_5G_RC_FRQ_SIZE;

    grd_rc_channel++;
    if(grd_rc_channel >= max_ch_size)
    {
        grd_rc_channel = 0;
    }
    
    BB_WriteReg(PAGE2, GRD_RC_CHANNEL, grd_rc_channel);
    
    if (s_u8_rcMaskEnable)
    {
        if ( !(s_u64_rcMask & (((uint64_t)1) << grd_rc_channel)) )
        {
            BB_set_Rcfrq(context.freq_band, grd_rc_channel);
        }
    }
    else
    {
        BB_set_Rcfrq(context.freq_band, grd_rc_channel);
    }
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
    dlog_info("mode= %d ", mode);
}


/*
  *   only set value to context, the request will be handle in 14ms interrupt.
 */
void grd_handle_IT_CH_cmd(uint8_t ch)
{
    context.it_manual_ch = ch;
    dlog_info("ch= %d", ch);    
}


/*
  * mode: set RC to auto or Manual
  * ch: the requested channel from  
  * 
 */
static void grd_handle_RC_mode_cmd(ENUM_RUN_MODE mode)
{
    if ( mode == AUTO )
    {
        context.flag_updateRc = 1;
        context.u32_rcValue = 0;

        BB_WriteReg(PAGE2, RC_CH_MODE_0, AUTO);
        dlog_info("Set RC auto");
    }
}


static void grd_handle_RC_CH_cmd(uint8_t ch)
{
	BB_set_Rcfrq(context.freq_band, ch);

    BB_WriteReg(PAGE2, RC_CH_CHANGE_0, 0x80+ch);
    BB_WriteReg(PAGE2, RC_CH_CHANGE_1, 0x80+ch + 1);

    dlog_info("ch =%d", ch);
}

/*
 * switch between 2.5G and 5G.
 */
static void grd_handle_RF_band_cmd(ENUM_RF_BAND rf_band)
{
    if(context.freq_band != rf_band)
    {
        BB_WriteReg(PAGE2, RF_BAND_CHANGE_0, 0xc0 + (uint8_t)rf_band);
        BB_WriteReg(PAGE2, RF_BAND_CHANGE_1, 0xc0 + (uint8_t)rf_band + 1);

        context.it_manual_rf_band = rf_band;
        dlog_info("To rf_band %d %d", context.freq_band, rf_band);
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

        context.CH_bandwidth = bw; 
    }

    dlog_info("CH_bandwidth =%d", context.CH_bandwidth);    
}

static void grd_handle_CH_qam_cmd(ENUM_BB_QAM qam)
{
    //set and soft-rest
    if(context.qam_mode != qam)
    {
        BB_WriteReg(PAGE2, RF_CH_QAM_CHANGE_0, 0xc0 | (uint8_t)qam);
        BB_WriteReg(PAGE2, RF_CH_QAM_CHANGE_1, 0xc0 | (uint8_t)qam + 1);       

        context.qam_mode = qam; 
        context.ldpc = grd_get_IT_LDPC();   
        grd_set_mcs_registers(context.qam_mode, context.ldpc, context.CH_bandwidth);
    }

    dlog_info("CH_QAM =%d", context.qam_mode);    
}

void grd_handle_CH_ldpc_cmd(ENUM_BB_LDPC e_ldpc)
{
    if(context.ldpc != e_ldpc)
    {
        BB_WriteReg(PAGE2, RF_CH_LDPC_CHANGE_0, e_ldpc);
        BB_WriteReg(PAGE2, RF_CH_LDPC_CHANGE_1, e_ldpc +1);
        context.ldpc = e_ldpc;
        context.qam_mode = grd_get_IT_QAM();

        grd_set_mcs_registers(context.qam_mode, context.ldpc, context.CH_bandwidth);
    }
    dlog_info("CH_LDPC =%d", context.ldpc);
}

        
static void grd_handle_MCS_mode_cmd(ENUM_RUN_MODE mode)
{
	context.qam_skip_mode = mode;
    dlog_info("qam_skip_mode = %d\n", context.qam_skip_mode);
    
    if ( mode == AUTO )
    {
	    ENUM_BB_QAM qam = grd_get_IT_QAM();
	    ENUM_BB_LDPC ldpc = grd_get_IT_LDPC();
    
	    if( qam == MOD_BPSK )
	    {
	        context.qam_ldpc = 1;
	    }
	    else if( qam == MOD_4QAM )
	    {
	        context.qam_ldpc = 2;
	    }
	    else if( qam == MOD_16QAM )
	    {
	        context.qam_ldpc = 3;
	    }
	    else if( qam == MOD_64QAM && ldpc == LDPC_1_2)
	    {
	        context.qam_ldpc = 4;
	    }
	    else if( qam == MOD_64QAM)
	    {
	        context.qam_ldpc = 5;
	    }
    
        grd_set_txmsg_mcs_change( context.qam_ldpc );        
    }
}


/*
  * handle set MCS mode: QAM, LDPC, encoder rate
*/
static void grd_handle_MCS_cmd(ENUM_BB_QAM qam, ENUM_BB_LDPC ldpc)
{
    grd_set_txmsg_qam_change(qam, context.CH_bandwidth, ldpc);
    dlog_info("qam, ldpc =%d %d", qam, ldpc);
}


/*
 * handle H264 encoder brc 
*/
static void grd_handle_brc_mode_cmd(ENUM_RUN_MODE mode)
{
    context.brc_mode = mode;

    BB_WriteReg(PAGE2, ENCODER_BRC_MODE_0, 0xe0+mode);
    BB_WriteReg(PAGE2, ENCODER_BRC_MODE_1, 0xe0+mode+1);

    dlog_info("brc mode =%d", mode);
}


/*
  * handle H264 encoder brc 
 */
static void grd_handle_brc_bitrate_cmd(uint8_t u8_ch, uint8_t brc_coderate)
{
    if (0 == u8_ch)
    {
        BB_WriteReg(PAGE2, ENCODER_BRC_CHAGE_0_CH1, (0xc0 | brc_coderate));
        BB_WriteReg(PAGE2, ENCODER_BRC_CHAGE_1_CH1, (0xc0 | brc_coderate)+1);
        context.brc_bps[0] = brc_coderate;

        dlog_info("brc_coderate_ch1 = %d ", brc_coderate);
    }
    else
    {
        BB_WriteReg(PAGE2, ENCODER_BRC_CHAGE_0_CH2, (0xc0 | brc_coderate));
        BB_WriteReg(PAGE2, ENCODER_BRC_CHAGE_1_CH2, (0xc0 | brc_coderate)+1);
        context.brc_bps[1] = brc_coderate;

        dlog_info("brc_coderate_ch2 = %d ", brc_coderate);
    }
}


void BB_grd_notify_rc_skip_freq(uint32_t u32_rcfrq)
{
    BB_WriteReg(PAGE2, RC_CH_MODE_0, MANUAL);

    BB_WriteReg(PAGE2, RC_FRQ_0, (u32_rcfrq >> 24 ) & 0xff);
    BB_WriteReg(PAGE2, RC_FRQ_1, (u32_rcfrq >> 16 ) & 0xff);
    BB_WriteReg(PAGE2, RC_FRQ_2, (u32_rcfrq >>  8 ) & 0xff);
    BB_WriteReg(PAGE2, RC_FRQ_3, (u32_rcfrq) & 0xff);
}


void grd_handle_one_cmd(STRU_WIRELESS_CONFIG_CHANGE* pcmd)
{
    uint8_t class  = pcmd->u8_configClass;
    uint8_t item   = pcmd->u8_configItem;
    uint32_t value = pcmd->u32_configValue;

    dlog_info("class item value %d %d 0x%0.8x ", class, item, value);
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
                grd_handle_RC_mode_cmd( (ENUM_RUN_MODE)MANUAL );
                BB_grd_notify_rc_skip_freq( value );    //set to manual and set the regigsters
                context.flag_updateRc = 1;              //to set the remote control
                context.u32_rcValue   = value;
                dlog_info("set RC FREQ %x", value);
                break;
            }

            case IT_CHANNEL_FREQ:
            {
                grd_handle_IT_mode_cmd( (ENUM_RUN_MODE)MANUAL);
                BB_write_ItRegs( value );
                BB_grd_notify_it_skip_freq_1();
                dlog_info("IT_CHANNEL_FREQ 0x%0.8x ", value);                
                break;
            }

            case FREQ_BAND_WIDTH_SELECT:
            {
                grd_handle_CH_bandwitdh_cmd((ENUM_CH_BW)value);
                dlog_info("FREQ_BAND_WIDTH_SELECT %x", value);                
                break;
            }

                
            default:
            {
                dlog_error("%s", "unknown WIRELESS_FREQ_CHANGE command");
                break;
            }
        }
    }

    if(class == WIRELESS_MCS_CHANGE)
    {
        switch(item)
        {
            case MCS_MODE_SELECT:
            {
                grd_handle_brc_mode_cmd( (ENUM_RUN_MODE)value);
                grd_handle_MCS_mode_cmd( (ENUM_RUN_MODE)value);
                
                //For osd information
                if ( context.brc_mode == MANUAL)
                {
                    context.brc_bps[0] = BB_get_bitrateByMcs(context.qam_ldpc);
                    context.brc_bps[1] = BB_get_bitrateByMcs(context.qam_ldpc);                
                }
            }
    
            break;

            case MCS_MODULATION_SELECT:
            {
                grd_set_txmsg_mcs_change(value);
            }
            break;

            case MCS_CODE_RATE_SELECT:
            {
                //grd_set_txmsg_ldpc(value);
                break;
            }
            case MCS_IT_QAM_SELECT:
            {
                grd_handle_CH_qam_cmd((ENUM_BB_QAM)value);
                dlog_info("MCS_IT_QAM_SELECT %x", value);                
                break;
            }
            
            case MCS_IT_CODE_RATE_SELECT:
            {
                grd_handle_CH_ldpc_cmd((ENUM_BB_LDPC)value);
                dlog_info("MCS_IT_CODE_RATE_SELECT %x", value);  
                break;              
            }
                
            case MCS_RC_QAM_SELECT:
            {
                context.rc_qam_mode = (ENUM_BB_QAM)(value & 0x01);
                BB_set_QAM(context.rc_qam_mode);
                dlog_info("MCS_RC_QAM_SELECT %x", value);                
                break;
            }
            
            case MCS_RC_CODE_RATE_SELECT:
            {
                context.rc_ldpc = (ENUM_BB_LDPC)(value & 0x01);
                BB_set_LDPC(context.rc_ldpc);
                dlog_info("MCS_RC_CODE_RATE_SELECT %x", value); 
                break;               
            }
            default:
                //dlog_error("%s", "unknown WIRELESS_MCS_CHANGE command");
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

            case ENCODER_DYNAMIC_BIT_RATE_SELECT_CH1:
                grd_handle_brc_bitrate_cmd(0, (uint8_t)value);
                dlog_info("ch1:%d",value);
                break;

            case ENCODER_DYNAMIC_BIT_RATE_SELECT_CH2:
                grd_handle_brc_bitrate_cmd(1, (uint8_t)value);
                dlog_info("ch2:%d",value);
                break;

            default:
                //dlog_error("%s", "unknown WIRELESS_ENCODER_CHANGE command");
                break;                
        }
    }
	
    if(class == WIRELESS_MISC)
    {
        BB_handle_misc_cmds(pcmd);
    }

    if(class == WIRELESS_OTHER)
    {
        switch(item)
        {
            case GET_DEV_INFO:
            {
                BB_GetDevInfo();
                break;
            }

            case SWITCH_ON_OFF_CH1:
            {
                BB_SwtichOnOffCh(0, (uint8_t)value);
                break;
            }

            case SWITCH_ON_OFF_CH2:
            {
                BB_SwtichOnOffCh(1, (uint8_t)value);
                break;
            }

            case BB_SOFT_RESET:
            {
                dlog_info("grd bb reset.");
                BB_softReset(BB_GRD_MODE);
                break;
            }

            case CALC_DIST_ZERO_CALI:
            {
                //dlog_info("calculation distance zero calibration.");
                grd_calc_dist_zero_calibration();
                break;
            }

            case SET_CALC_DIST_ZERO_POINT:
            {
                //dlog_info("set distance zero point.");
                grd_set_calc_dist_zero_point(value);
                break;
            }

            case SET_RC_FRQ_MASK:
            {
                grd_disable_enable_rc_frq_mask_func(value);
                break;
            }
            default:
                break;                
        }
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
    uint8_t u8_data;
    STRU_WIRELESS_INFO_DISPLAY *osdptr = (STRU_WIRELESS_INFO_DISPLAY *)(SRAM_BB_STATUS_SHARE_MEMORY_ST_ADDR);

    if (osdptr->osd_enable == 0)
    {
        return;
    }

    osdptr->messageId = 0x33;
    osdptr->head = 0xff; //starting writing
    osdptr->tail = 0x00;

    //osdptr->IT_channel = context.cur_IT_ch;

    osdptr->agc_value[0] = BB_ReadReg(PAGE2, AAGC_2_RD);
    osdptr->agc_value[1] = BB_ReadReg(PAGE2, AAGC_3_RD);
    
    osdptr->agc_value[2] = BB_ReadReg(PAGE2, 0xde);
    osdptr->agc_value[3] = BB_ReadReg(PAGE2, 0xdf);
    
    //osdptr->agc_value[2] = BB_ReadReg(PAGE2, RX3_GAIN_ALL_R);
    //osdptr->agc_value[3] = BB_ReadReg(PAGE2, RX4_GAIN_ALL_R);
    //osdptr->lock_status  = BB_ReadReg(PAGE2, FEC_5_RD);
    
    osdptr->snr_vlaue[0] = grd_get_it_snr();
    osdptr->snr_vlaue[1] = get_snr_average();
    //osdptr->snr_vlaue[2] = 0x12aa;

    osdptr->ldpc_error = (((uint16_t)BB_ReadReg(PAGE2, LDPC_ERR_HIGH_8)) << 8) | BB_ReadReg(PAGE2, LDPC_ERR_LOW_8);
    osdptr->harq_count = (BB_ReadReg(PAGE2, FEC_5_RD) & 0xF0) | ((BB_ReadReg(PAGE2, 0xd1)& 0xF0) >> 4);
    
    uint8_t tmp = BB_ReadReg(PAGE2, 0xdd);
    //if(osdptr->harq_count > 1 )
    //{
    //    dlog_info("err:0x%x harq:0x%x lost:0x%x SNR:0x%x 0x%x\n", osdptr->ldpc_error, osdptr->harq_count, tmp, grd_get_it_snr(),
    //                                                              (((uint16_t)BB_ReadReg(PAGE2, 0xc2)) << 8) | BB_ReadReg(PAGE2, 0xc3));
    //}
    
    #if 0
    osdptr->modulation_mode = grd_get_IT_QAM();
    #else
    osdptr->modulation_mode = context.qam_ldpc;
    #endif
    osdptr->code_rate       = grd_get_IT_LDPC();

    u8_data = BB_ReadReg(PAGE2, TX_2);
    osdptr->rc_modulation_mode = (u8_data >> 6) & 0x01;
    osdptr->rc_code_rate       = (u8_data >> 0) & 0x01;
    osdptr->ch_bandwidth    = context.CH_bandwidth;         
    osdptr->in_debug        = context.u8_debugMode;
    osdptr->lock_status     = ( osdptr->lock_status & 0xf0) | (BB_ReadReg(PAGE2, FEC_5_RD) & 0x0f);
    memset(osdptr->sweep_energy, 0, sizeof(osdptr->sweep_energy));
    BB_GetSweepNoise(osdptr->sweep_energy);

    if(context.brc_mode == AUTO)
    {
        osdptr->encoder_bitrate[0] = BB_get_bitrateByMcs(context.qam_ldpc);
        osdptr->encoder_bitrate[1] = BB_get_bitrateByMcs(context.qam_ldpc);
    }
    else
    {
        osdptr->encoder_bitrate[0] = context.brc_bps[0];
        osdptr->encoder_bitrate[1] = context.brc_bps[1];
    }

    osdptr->head = 0x00;
    osdptr->tail = 0xff;    //end of the writing
}

static void grd_calc_dist(void)
{
    uint8_t u8_data2[3];
    uint32_t u32_data2 = 0;
    uint32_t u32_i;
    static uint32_t u32_cnt = 1;
    uint32_t cmpData;
    static uint32_t cmpCnt = 0;

    u8_data2[2] = BB_ReadReg(PAGE3, 0xA7);
    u8_data2[1] = BB_ReadReg(PAGE3, 0xA6);
    u8_data2[0] = BB_ReadReg(PAGE3, 0xA5);

    cmpData = (u8_data2[0] << 16) | (u8_data2[1] << 8) | (u8_data2[2] << 0);

    switch(s_st_calcDistData.e_status)
    {
        case INVALID:
        {
            s_st_calcDistData.e_status = CALI_ZERO;
            s_st_calcDistData.u32_cnt = 0;
            break;
        }
        case CALI_ZERO:
        {
            memcpy(s_st_calcDistData.u8_rawData[s_st_calcDistData.u32_cnt % CALC_DIST_RAW_DATA_MAX_RECORD], u8_data2, 3);
            s_st_calcDistData.u32_cnt += 1;
            if (s_st_calcDistData.u32_cnt >= (CALC_DIST_RAW_DATA_MAX_RECORD))
            {
                s_st_calcDistData.u32_calcDistZero = grd_calc_dist_get_avg_value(&u32_data2);
                s_st_calcDistData.u32_cnt = 0;
                s_st_calcDistData.e_status = CALC_DIST_PREPARE;
                //dlog_info("CALI_ZERO");
                //dlog_info("Zero:%d", s_st_calcDistData.u32_calcDistZero);
            }
            break;
        }
        case CALC_DIST_PREPARE:
        {
            memcpy(s_st_calcDistData.u8_rawData[s_st_calcDistData.u32_cnt % CALC_DIST_RAW_DATA_MAX_RECORD], u8_data2, 3);
            s_st_calcDistData.u32_cnt += 1;
            if (s_st_calcDistData.u32_cnt >= CALC_DIST_RAW_DATA_MAX_RECORD)
            {
                grd_calc_dist_get_avg_value(&(s_st_calcDistData.u32_calcDistValue));

                s_st_calcDistData.u32_cnt = 0;
                s_st_calcDistData.e_status = CALC_DIST;
                //dlog_info("CALC_DIST_PREPARE");
                //dlog_info("value:%d", s_st_calcDistData.u32_calcDistValue);
            }
            break;
        }
        case CALC_DIST:
        {
            cmpData = abs(cmpData - s_st_calcDistData.u32_calcDistZero) * 3 / 2;
            if (abs(cmpData - s_st_calcDistData.u32_calcDistValue) < 50)
            {
                cmpCnt = 0;
                memcpy(s_st_calcDistData.u8_rawData[s_st_calcDistData.u32_cnt % CALC_DIST_RAW_DATA_MAX_RECORD], u8_data2, 3);
                s_st_calcDistData.u32_cnt += 1;
                
                grd_calc_dist_get_avg_value(&(s_st_calcDistData.u32_calcDistValue));

                u32_cnt += 1;
                if (0 == (u32_cnt % 300))
                {
                    dlog_info("Zero:%d value:%d", s_st_calcDistData.u32_calcDistZero, s_st_calcDistData.u32_calcDistValue);
                } 
            }
            else
            {
                cmpCnt++;
                if (cmpCnt > 5)
                {
                    cmpCnt = 0;
                    s_st_calcDistData.u32_cnt = 0;
                    s_st_calcDistData.e_status = CALC_DIST_PREPARE;
                }
            }
            break;
        }
        default:
        {
            s_st_calcDistData.e_status = CALI_ZERO;
            s_st_calcDistData.u32_cnt = 0;
            break;
        }     
    }
}

void grd_calc_dist_zero_calibration(void)
{
    s_st_calcDistData.e_status = INVALID;
    s_st_calcDistData.u32_calcDistValue = 0;
    s_st_calcDistData.u32_calcDistZero = 0;
    s_st_calcDistData.u32_cnt = 0;
    s_st_calcDistData.u32_lockCnt = 0;
}

void grd_set_calc_dist_zero_point(uint32_t value)
{
    s_st_calcDistData.u32_calcDistZero = value;
}


static uint32_t grd_calc_dist_get_avg_value(uint32_t *u32_dist)
{
    uint32_t u32_data2 = 0;
    uint32_t u32_i;
    
    for(u32_i = 0; u32_i < CALC_DIST_RAW_DATA_MAX_RECORD; u32_i++)
    {
        u32_data2 += ((s_st_calcDistData.u8_rawData[u32_i][0] << 16) | 
                     (s_st_calcDistData.u8_rawData[u32_i][1] << 8) | 
                     (s_st_calcDistData.u8_rawData[u32_i][2] << 0));
    }
    u32_data2 = u32_data2 / CALC_DIST_RAW_DATA_MAX_RECORD;

    if (u32_data2 > s_st_calcDistData.u32_calcDistZero)
    {
        *u32_dist = (u32_data2 - s_st_calcDistData.u32_calcDistZero) * 3 / 2;
    }
    else
    {
        *u32_dist = 0;
    }

    return u32_data2;
}

static void grd_init_rc_frq_mask_func(void)
{
    grd_write_mask_code_to_sky(s_u8_rcMaskEnable, &s_u64_rcMask);
}

void grd_disable_enable_rc_frq_mask_func(uint8_t flag)
{
    if (flag)
    {
        s_u8_rcMaskEnable = 1;
        dlog_info("enable rc frq mask");
    }
    else
    {
        s_u8_rcMaskEnable = 0;
        dlog_info("disable rc frq mask");
    }
}

static void grd_write_mask_code_to_sky(uint8_t enable, uint64_t *mask)
{
    uint8_t *pu8_tmpAddr;
    uint8_t i;
    uint64_t u64_tmpData = 0;

    if (0 == enable) // disable rc frq mask function
    {
        pu8_tmpAddr = (uint8_t *)(&u64_tmpData);
    }
    else
    {
        pu8_tmpAddr = (uint8_t *)(mask);
    }
    
    for (i=0; i<sizeof(uint64_t); i++)
    {
        BB_WriteReg(PAGE2, GRD_MASK_CODE + i, pu8_tmpAddr[i]);
    }
}


