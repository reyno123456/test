#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>
#include "reg_rw.h"
#include "timer.h"
#include "interrupt.h"
#include "systicks.h"
#include "bb_sky_ctrl.h"
#include "bb_regs.h"
#include "bb_sys_param.h"
#include "debuglog.h"
#include "sys_event.h"
#include "gpio.h"


#define MAX_SEARCH_ID_NUM  (5)
#define SEARCH_ID_TIMEOUT  (2000) //ms

typedef struct
{
    uint8_t id[5];
    uint8_t agc1;
    uint8_t agc2;
}SEARCH_IDS;

typedef struct
{
    SEARCH_IDS search_ids[MAX_SEARCH_ID_NUM];
    uint8_t count;
}SEARCH_IDS_LIST;


static SEARCH_IDS_LIST search_id_list;
static uint32_t start_time_cnt = 0;
static uint8_t  sky_rc_channel = 0;

static enum EN_AGC_MODE en_agcmode = UNKOWN_AGC;

static init_timer_st sky_timer2_6;
static init_timer_st sky_timer2_7;

static int sky_timer2_6_running = 0;
static int sky_timer2_7_running = 0;
static int switch_5G_count = 0;


static void sky_handle_one_cmd(STRU_WIRELESS_CONFIG_CHANGE* pcmd);
static void sky_handle_all_cmds(void);
static int32_t sky_chk_flash_id_validity(void);
static int32_t sky_write_id(uint8_t *u8_idArray);
static int32_t cal_chk_sum(uint8_t *pu8_data, uint32_t u32_len, uint8_t *u8_check);

void BB_SKY_start(void)
{   
    context.rc_skip_freq_mode = AUTO;

    context.cur_IT_ch         = 0xff;
    context.it_manual_rf_band = 0xff;
    context.rc_unlock_cnt     = 0;
    context.dev_state         = SEARCH_ID;
    //context.dev_state = (context.search_id_enable == 0xff)? SEARCH_ID : CHECK_ID_MATCH;

    sky_id_search_init();
    
    GPIO_SetMode(RED_LED_GPIO, GPIO_MODE_1);
    GPIO_SetPinDirect(RED_LED_GPIO, GPIO_DATA_DIRECT_OUTPUT);

    GPIO_SetMode(BLUE_LED_GPIO, GPIO_MODE_1);
    GPIO_SetPinDirect(BLUE_LED_GPIO, GPIO_DATA_DIRECT_OUTPUT);
    
    GPIO_SetPin(RED_LED_GPIO, 0);   //RED LED ON
    GPIO_SetPin(BLUE_LED_GPIO, 1);  //BLUE LED OFF

    sky_Timer2_6_Init();
    sky_Timer2_7_Init();    
    sky_search_id_timeout_irq_enable(); //enabole TIM1 timeout

    reg_IrqHandle(BB_RX_ENABLE_VECTOR_NUM, wimax_vsoc_rx_isr, NULL);
    INTR_NVIC_SetIRQPriority(BB_RX_ENABLE_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_BB_RX,0));
    INTR_NVIC_EnableIRQ(BB_RX_ENABLE_VECTOR_NUM);

    context.qam_ldpc = 0;
    sky_set_McsByIndex(context.qam_ldpc);

    BB_GetDevInfo();
}


uint8_t sky_id_match(void)
{
    static int total_count = 0;
    static int lock_count = 0;
    uint8_t data = BB_ReadReg(PAGE2, FEC_4_RD) & 0x03;

    total_count ++;
    lock_count += ((data == 0x03) ? 1 : 0);

    if(total_count > 500)
    {
        dlog_info("-L:%d-\n", lock_count);    
        total_count = 0;
        lock_count = 0;
    }
    
    return (data==0x03) ? 1 : 0;
}


void sky_notify_encoder_brc_ch1(uint8_t br)
{
    STRU_SysEvent_BB_ModulationChange event;
    event.BB_MAX_support_br = br;
    event.u8_bbCh = 0;
    SYS_EVENT_Notify(SYS_EVENT_ID_BB_SUPPORT_BR_CHANGE, (void*)&event);	
    
    dlog_info("ch1 brc =%d\r\n", br);        
}

void sky_notify_encoder_brc_ch2(uint8_t br)
{
    STRU_SysEvent_BB_ModulationChange event;
    event.BB_MAX_support_br = br;
    event.u8_bbCh = 1;
    SYS_EVENT_Notify(SYS_EVENT_ID_BB_SUPPORT_BR_CHANGE, (void*)&event);	
    
    dlog_info("ch2 brc =%d\r\n", br);        
}
void sky_set_McsByIndex(uint8_t idx)
{
    uint8_t mcs_idx_bitrate_map[] = 
    {
        1,      //0.6Mbps
        2,      //1.2
        3,      //2.4
        6,      //5.0
        9,      //7.5
        11,     //10
    };

    uint8_t mcs_idx_reg0x0f_map[] = 
    {
        0x47,
        0x87,
        0x57,
        0x37,
        0x37,
        0x27
    };

    uint8_t map_idx_to_mode[] = 
    {
        ((MOD_BPSK<<6)  | (BW_10M <<3)  | LDPC_1_2), //
        ((MOD_BPSK<<6)  | (BW_10M <<3)  | LDPC_1_2),
        ((MOD_4QAM<<6)  | (BW_10M <<3)  | LDPC_1_2),
        //((MOD_4QAM<<6)  | (BW_10M <<3)  | LDPC_2_3),
        ((MOD_16QAM<<6) | (BW_10M <<3)  | LDPC_1_2),
        //((MOD_16QAM<<6) | (BW_10M <<3)  | LDPC_2_3),
        ((MOD_64QAM<<6) | (BW_10M <<3)  | LDPC_1_2),
        ((MOD_64QAM<<6) | (BW_10M <<3)  | LDPC_2_3),
    };

    dlog_info("MCS=> %d\n", map_idx_to_mode[idx]);

    BB_WriteReg( PAGE2, 0x0f, mcs_idx_reg0x0f_map[idx] );
    BB_WriteReg( PAGE2, TX_2, map_idx_to_mode[idx]);
    
	if ( context.brc_mode == AUTO )
	{
        sky_notify_encoder_brc_ch1( mcs_idx_bitrate_map[idx] );
        sky_notify_encoder_brc_ch2( mcs_idx_bitrate_map[idx] );
	}
}


void sky_agc_gain_toggle(void)
{
    static int loop = 0;
    if(loop++ > 50)
    {
        dlog_info("AGCToggle\r\n");  
        loop = 0; 
    }

    if(FAR_AGC == en_agcmode)
    {
        BB_WriteReg(PAGE0, AGC_2, AAGC_GAIN_NEAR);
        BB_WriteReg(PAGE0, AGC_3, AAGC_GAIN_NEAR);
        en_agcmode = NEAR_AGC;
    }
    else
    {
        BB_WriteReg(PAGE0, AGC_2, AAGC_GAIN_FAR);
        BB_WriteReg(PAGE0, AGC_3, AAGC_GAIN_FAR);
        en_agcmode = FAR_AGC;
    }
}


void sky_auto_adjust_agc_gain(void)
{
    uint8_t rx1_gain = BB_ReadReg(PAGE2, AAGC_2_RD);
    uint8_t rx2_gain = BB_ReadReg(PAGE2, AAGC_3_RD);

    if((rx1_gain >= POWER_GATE)&&(rx2_gain >= POWER_GATE) && en_agcmode != FAR_AGC)
    {
        BB_WriteReg(PAGE0, AGC_2, AAGC_GAIN_FAR);
        BB_WriteReg(PAGE0, AGC_3, AAGC_GAIN_FAR);
        en_agcmode = FAR_AGC;
        dlog_info("=>F", rx1_gain, rx2_gain);
    }
     
    if( ((rx1_gain < POWER_GATE)&&(rx2_gain < POWER_GATE)) \
        && (rx1_gain > 0x00) && (rx2_gain >0x00) \
        && en_agcmode != NEAR_AGC)
    {
        BB_WriteReg(PAGE0, AGC_2, AAGC_GAIN_NEAR);
        BB_WriteReg(PAGE0, AGC_3, AAGC_GAIN_NEAR);
        en_agcmode = NEAR_AGC;
        dlog_info("=>N", rx1_gain, rx2_gain);
    }
}


//*********************TX RX initial(14ms irq)**************
void wimax_vsoc_rx_isr(uint32_t u32_vectorNum)
{
    INTR_NVIC_DisableIRQ(BB_RX_ENABLE_VECTOR_NUM);   
    STRU_WIRELESS_INFO_DISPLAY *osdptr = (STRU_WIRELESS_INFO_DISPLAY *)(SRAM_BB_STATUS_SHARE_MEMORY_ST_ADDR);

    if( context.u8_flagdebugRequest & 0x80)
    {
        context.u8_debugMode = (context.u8_flagdebugRequest & 0x01);
        osdptr->in_debug = context.u8_debugMode;
        if( context.u8_debugMode )
        {
            osdptr->head = 0x00;
            osdptr->tail = 0xff;    //end of the writing
        }

        context.u8_flagdebugRequest = 0;
    }

    INTR_NVIC_EnableIRQ(TIMER_INTR26_VECTOR_NUM);
    TIM_StartTimer(sky_timer2_6);
}


void Sky_TIM2_6_IRQHandler(uint32_t u32_vectorNum)
{
    static uint32_t u32_cnt = 0;
    sky_timer2_6_running = 1;
    sky_search_id_timeout_irq_disable();

    Reg_Read32(BASE_ADDR_TIMER2 + TMRNEOI_6);
    INTR_NVIC_ClearPendingIRQ(BB_RX_ENABLE_VECTOR_NUM); //clear pending after TX Enable is LOW. MUST!
    INTR_NVIC_EnableIRQ(BB_RX_ENABLE_VECTOR_NUM);  
    
    INTR_NVIC_DisableIRQ(TIMER_INTR26_VECTOR_NUM);
    TIM_StopTimer(sky_timer2_6);
    
    if( context.u8_debugMode )
    {
        return;
    }
    
    sky_handle_all_cmds();
    if(0 == sky_timer2_7_running )
    {
        sky_physical_link_process();
        
        //patch for 5G switch: For demo test only.
        if(context.freq_band == RF_5G && switch_5G_count < 5)
        {
            switch_5G_count++;
            sky_soft_reset();
        }
        if (0 == (u32_cnt++ % 2))
        {
            BB_sky_GatherOSDInfo();
        }
        else
        {
            BB_GetDevInfo();
        }
    }

    sky_timer2_6_running = 0;
}


void sky_rc_hopfreq(void)
{
	uint8_t max_ch_size = (context.freq_band == RF_2G) ? (MAX_2G_RC_FRQ_SIZE):(MAX_5G_RC_FRQ_SIZE);
	
    sky_rc_channel++;
    if(sky_rc_channel >=  max_ch_size)
    {
        sky_rc_channel = 0;
    }
	BB_set_Rcfrq(context.freq_band, sky_rc_channel);
}


void sky_set_it_freq(ENUM_RF_BAND band, uint8_t ch)
{
    BB_set_ITfrq(band, ch);
	context.cur_IT_ch = ch;

    dlog_info("S=>%d\r\n", ch);
}

void sky_get_RC_id(uint8_t* idptr)
{
    idptr[0] = BB_ReadReg(PAGE2, FEC_1_RD);
    idptr[1] = BB_ReadReg(PAGE2, FEC_2_RD_1);
    idptr[2] = BB_ReadReg(PAGE2, FEC_2_RD_2);
    idptr[3] = BB_ReadReg(PAGE2, FEC_2_RD_3);
    idptr[4] = BB_ReadReg(PAGE2, FEC_2_RD_4);   
}

void sky_set_RC_id(uint8_t *idptr)
{
    uint8_t i;
    uint8_t addr[] = {FEC_7, FEC_8, FEC_9, FEC_10, FEC_11};

    dlog_info("RCid:%02x%02x%02x%02x%02x\r\n", idptr[0], idptr[1], idptr[2], idptr[3], idptr[4]);                
    for(i=0; i < sizeof(addr); i++)
    {
        BB_WriteReg(PAGE2, addr[i], idptr[i]);
    }
}


void sky_soft_reset(void)
{
    BB_softReset(BB_SKY_MODE);
}

void sky_physical_link_process(void)
{
    uint8_t *p_id;

    if(context.dev_state == SEARCH_ID)
    {
        if(RC_ID_AUTO_SEARCH == (context.u8_idSrcSel)) // auto serch
        {
            if(sky_id_search_run())
            {
                p_id = sky_id_search_get_best_id();
                sky_set_RC_id(p_id);
                sky_write_id(p_id);

                context.dev_state = CHECK_ID_MATCH;
                sky_soft_reset();
                dlog_info("use auto search id");
            }
            else
            {
                context.rc_unlock_cnt ++;
            }
        }
        else if(RC_ID_USE_FLASH_SAVE == (context.u8_idSrcSel)) // read flash 
        {
            if(0 == sky_chk_flash_id_validity()) // flash id ok
            {
                sky_set_RC_id(context.u8_flashId);
                context.dev_state = CHECK_ID_MATCH;
                sky_soft_reset();
                dlog_info("use fixed id");
            }
            else // flash id error,set to auto search
            {
                context.u8_idSrcSel = RC_ID_AUTO_SEARCH;
                search_id_list.count = 0; // completely re-search.
            }       
        }
        else
        {
            context.u8_idSrcSel = RC_ID_AUTO_SEARCH;
            search_id_list.count = 0; // completely re-search.
        }
    }
    else if(context.dev_state == ID_MATCH_LOCK)
    {
        if(context.rc_skip_freq_mode == AUTO)
        {
            sky_rc_hopfreq();
        }

        context.locked = sky_id_match();
        if(context.locked)
        {
            uint8_t data0, data1;
            context.rc_unlock_cnt = 0;
            sky_handle_all_spi_cmds();            
            GPIO_SetPin(BLUE_LED_GPIO, 0);  //BLUE LED ON
            GPIO_SetPin(RED_LED_GPIO, 1);   //RED LED OFF
        }
        else
        {
            context.rc_unlock_cnt++;
            // sky_soft_reset();
        }

        if(40 <= context.rc_unlock_cnt)
        {            
            GPIO_SetPin(BLUE_LED_GPIO, 1);  //BLUE LED OFF
            GPIO_SetPin(RED_LED_GPIO, 0);   //RED LED ON
            
            context.qam_ldpc = 0;
            sky_set_McsByIndex(0);
            
            context.dev_state = CHECK_ID_MATCH;
            context.rc_unlock_cnt = 0;
        }

        sky_auto_adjust_agc_gain(); //
    }
    else if(context.dev_state == CHECK_ID_MATCH)
    {
        if( context.freq_band == RF_5G)
        {
            //For test, do nothing when 5G
        }
        else
        {
            context.locked = sky_id_match();
            if(context.locked)
            {
                if(context.rc_skip_freq_mode == AUTO)
                {
                    sky_rc_hopfreq();
                }

                context.rc_unlock_cnt = 0;
                context.dev_state = ID_MATCH_LOCK;
            }
            else
            {
                context.rc_unlock_cnt++;
                sky_soft_reset();
            }
        }
    }
    
    if(context.rc_unlock_cnt > 40 //560ms unlock
       && (context.dev_state == SEARCH_ID || context.dev_state == CHECK_ID_MATCH))  
    {
        context.rc_unlock_cnt = 0;
        sky_search_id_timeout();
    }    
}

void sky_id_search_init(void)
{
    search_id_list.count = 0;
}

uint8_t get_rc_status(void)
{
    return BB_ReadReg(PAGE2, FEC_4_RD);
}


uint8_t sky_id_search_run(void)
{    
    uint8_t rc_status = get_rc_status();
    
    #define SKY_RC_ERR(status)  (0x80 == status)
    #define SKY_CRC_OK(status)  ((status & 0x02) ? 1 : 0)

    context.rc_status = rc_status;
    if( SKY_RC_ERR(rc_status))
    {
        sky_soft_reset();
        return FALSE;
    }

    if(SKY_CRC_OK(rc_status))
    {
        sky_get_RC_id(search_id_list.search_ids[search_id_list.count].id);

        //record AGC.
        search_id_list.search_ids[search_id_list.count].agc1 = BB_ReadReg(PAGE2, AAGC_2_RD);
        search_id_list.search_ids[search_id_list.count].agc2 = BB_ReadReg(PAGE2, AAGC_3_RD);
        search_id_list.count += 1;

        //record first get id time
        if(search_id_list.count == 1)
        {
            start_time_cnt = SysTicks_GetTickCount();
        }

        if(search_id_list.count >= MAX_SEARCH_ID_NUM)
        {
            return TRUE;
        }
    }

    if(search_id_list.count > 0)
    {
        return (SysTicks_GetTickCount() - start_time_cnt > SEARCH_ID_TIMEOUT);
    }

    return FALSE;
}


uint8_t* sky_id_search_get_best_id(void)
{
    uint8_t i,best_id_cnt;
    uint16_t agc1_agc2;
    if(search_id_list.count == 1)
    {
        return search_id_list.search_ids[0].id;
    }

    best_id_cnt = 0;
    agc1_agc2 = search_id_list.search_ids[best_id_cnt].agc1 + search_id_list.search_ids[best_id_cnt].agc2;

    for(i=1;i<search_id_list.count;i++)
    {
        if(search_id_list.search_ids[i].agc1 + search_id_list.search_ids[i].agc2 < agc1_agc2)
        {
            best_id_cnt = i;
            agc1_agc2 = search_id_list.search_ids[i].agc1 + search_id_list.search_ids[i].agc2;
        }
    }

    return search_id_list.search_ids[best_id_cnt].id;
}


int sky_search_id_timeout_irq_enable()
{
    TIM_StartTimer(sky_timer2_7);
}

int sky_search_id_timeout_irq_disable()
{
    TIM_StopTimer(sky_timer2_7);
}


void sky_search_id_timeout(void)
{
    if(context.rc_skip_freq_mode == AUTO)
    {
        sky_rc_hopfreq();
    }

    sky_agc_gain_toggle();
    sky_soft_reset();
}

void Sky_TIM2_7_IRQHandler(uint32_t u32_vectorNum)
{
    static int Timer1_Delay2_Cnt = 0;    

	sky_timer2_7_running = 1; 

    dlog_info("sky_search_id_timeout_irq_enable \r\n");
    INTR_NVIC_ClearPendingIRQ(TIMER_INTR27_VECTOR_NUM);
	
	if(TRUE == context.u8_debugMode) 
	{
		return;
	}  	
	
    if(Timer1_Delay2_Cnt < 560)
    {
        Timer1_Delay2_Cnt ++;
    }
    else
    {
        if(sky_timer2_6_running == 0) //To avoid the spi access conflict
        {
            sky_search_id_timeout();
        }
        Timer1_Delay2_Cnt = 0;
    }

    sky_timer2_7_running = 0;
}


void sky_Timer2_7_Init(void)
{
    sky_timer2_7.base_time_group = 2;
    sky_timer2_7.time_num = 7;
    sky_timer2_7.ctrl = 0;
    sky_timer2_7.ctrl |= TIME_ENABLE | USER_DEFINED;
    TIM_RegisterTimer(sky_timer2_7, 1000);
    reg_IrqHandle(TIMER_INTR27_VECTOR_NUM, Sky_TIM2_7_IRQHandler, NULL);
	INTR_NVIC_SetIRQPriority(TIMER_INTR27_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_TIMER01,0));
}

void sky_Timer2_6_Init(void)
{
    sky_timer2_6.base_time_group = 2;
    sky_timer2_6.time_num = 6;
    sky_timer2_6.ctrl = 0;
    sky_timer2_6.ctrl |= TIME_ENABLE | USER_DEFINED;

    TIM_RegisterTimer(sky_timer2_6, 6800);

    reg_IrqHandle(TIMER_INTR26_VECTOR_NUM, Sky_TIM2_6_IRQHandler, NULL);
	INTR_NVIC_SetIRQPriority(TIMER_INTR26_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_TIMER00,0));
}



//////////////////////////////////////////////////////////////////////////////////////////

/*
 * read spi and handle it. interanal call in the intr.
*/
static void sky_handle_IT_CH_cmd(void)
{
    uint8_t data0, data1;

    data0 = BB_ReadReg(PAGE2, IT_FREQ_TX_0);
    data1 = BB_ReadReg(PAGE2, IT_FREQ_TX_1);
    uint8_t req_ch = data0 & 0x1f;

    if((data0+1 == data1) && ((data0&0xE0) == 0xE0) && (context.cur_IT_ch != req_ch))
    {
        sky_set_it_freq(context.freq_band, req_ch);
        context.cur_IT_ch = req_ch;
    }
}


static void sky_handle_RC_cmd(void)
{
    uint8_t data0, data1, data2, data3;

    data0 = BB_ReadReg(PAGE2, RC_CH_MODE_0);      //(AUTO, MANUAL)
    data1 = BB_ReadReg(PAGE2, RC_CH_MODE_1);      //(AUTO, MANUAL)

    data2 = BB_ReadReg(PAGE2, RC_CH_CHANGE_0);
    data3 = BB_ReadReg(PAGE2, RC_CH_CHANGE_1);

    if(data0+1 == data1 && data2+1==data3)
    {
        context.rc_skip_freq_mode = (ENUM_RUN_MODE)(data0 & 0x7F);
        if((ENUM_RUN_MODE)data0 == MANUAL && data0!= 0xFE)
        {
            sky_rc_channel = (data0 & 0x7f);
            BB_set_Rcfrq(context.freq_band, sky_rc_channel);
            dlog_info("rc_channel: %d \r\n", sky_rc_channel);
        }
    }
}


/*
 * handle 2G/5G switch 
*/
static void sky_handle_RF_band_cmd(void)
{
    uint8_t data0 = BB_ReadReg(PAGE2, RF_BAND_CHANGE_0);
    uint8_t data1 = BB_ReadReg(PAGE2, RF_BAND_CHANGE_1);
    
    if( (data0 == 0xc0 && data1 == 0xc1) || (data0 == 0xc1 && data1 == 0xc2))
    {
        ENUM_RF_BAND band = (ENUM_RF_BAND)(data0 & 0x01);
        BB_set_RF_Band(BB_SKY_MODE, band);
        context.freq_band = band;
        dlog_info("sky set band %d \r\n", band);
    }
}


/*
 *  handle command for 10M, 20M
*/
static void sky_handle_CH_bandwitdh_cmd(void)
{   
    uint8_t data0, data1;
    
    data0 = BB_ReadReg(PAGE2, RF_CH_BW_CHANGE_0);
    data1 = BB_ReadReg(PAGE2, RF_CH_BW_CHANGE_1);

    if( data1==data0+1 && (data0&0xc0)==0xc0)
    {
        ENUM_CH_BW bw = (ENUM_CH_BW)(data0&0x3F);

        if(context.CH_bandwidth != bw)
        {
            //set and soft-rest
            BB_set_RF_bandwitdh(BB_SKY_MODE, bw);
            context.CH_bandwidth = bw;
            dlog_info("CH_bandwidth =%d\r\n", context.CH_bandwidth);
        }
    }
}


static void sky_handle_brc_mode_cmd(void)
{
    uint8_t data0 = BB_ReadReg(PAGE2, ENCODER_BRC_MODE_0);
	uint8_t data1 = BB_ReadReg(PAGE2, ENCODER_BRC_MODE_1);
    ENUM_RUN_MODE mode = (ENUM_RUN_MODE)(data0 & 0x1f);

    if( (data1==data0+1) && ((data0&0xe0)==0xe0) && (context.brc_mode != mode))
    {
        dlog_info("brc_mode = %d \r\n", mode);
    	context.brc_mode = mode;
    }
}


/*
  * handle H264 encoder brc 
 */
static void sky_handle_brc_bitrate_cmd_ch1(void)
{
    uint8_t data0 = BB_ReadReg(PAGE2, ENCODER_BRC_CHAGE_0_CH1);
    uint8_t data1 = BB_ReadReg(PAGE2, ENCODER_BRC_CHAGE_1_CH1);
    uint8_t bps = data0&0x3F;

    if( (data0+1==data1) && ( (data0&0xc0)==0xc0) && (context.brc_bps[0] != bps))
    {
        context.brc_bps[0] = bps;
        sky_notify_encoder_brc_ch1(bps);
        dlog_info("ch1 brc_bps = %d \r\n", bps);
    }
}

static void sky_handle_brc_bitrate_cmd_ch2(void)
{
    uint8_t data0 = BB_ReadReg(PAGE2, ENCODER_BRC_CHAGE_0_CH2);
    uint8_t data1 = BB_ReadReg(PAGE2, ENCODER_BRC_CHAGE_1_CH2);
    uint8_t bps = data0&0x3F;

    if( (data0+1==data1) && ( (data0&0xc0)==0xc0) && (context.brc_bps[1] != bps))
    {
        context.brc_bps[1] = bps;
        sky_notify_encoder_brc_ch2(bps);
        dlog_info("ch2 brc_bps = %d \r\n", bps);
    }
}

static void sky_handle_QAM_cmd(void)
{
    uint8_t data0 = BB_ReadReg(PAGE2, QAM_CHANGE_0);
    uint8_t data1 = BB_ReadReg(PAGE2, QAM_CHANGE_1);

    if(data0+1==data1 && context.qam_ldpc != data0)
    {
        BB_WriteReg(PAGE2, TX_2, data0);
    }
}

static void sky_handle_MCS_cmd(void)
{
    uint8_t data0 = BB_ReadReg(PAGE2, MCS_INDEX_MODE_0);
    uint8_t data1 = BB_ReadReg(PAGE2, MCS_INDEX_MODE_1);

    if(data0+1==data1 && context.qam_ldpc != data0)
    {
        sky_set_McsByIndex(data0);
        context.qam_ldpc = data0;
    }
}

static void sky_handle_ldpc_cmd(void)
{
    uint8_t data0 = BB_ReadReg(PAGE2, LDPC_INDEX_0);
    uint8_t data1 = BB_ReadReg(PAGE2, LDPC_INDEX_1);

    if(data0+1==data1 && context.ldpc != data0)
    {
        context.ldpc = data0;
        BB_set_LDPC(context.ldpc);
        dlog_info("ldpc=>%d\n", context.ldpc);
    }
}

void sky_handle_all_spi_cmds(void)
{
    sky_handle_RC_cmd();

    sky_handle_IT_CH_cmd();

    //sky_handle_QAM_cmd();
    
    sky_handle_MCS_cmd();

    sky_handle_ldpc_cmd();

    sky_handle_brc_mode_cmd();

    sky_handle_CH_bandwitdh_cmd();

    sky_handle_brc_bitrate_cmd_ch1();
    sky_handle_brc_bitrate_cmd_ch2();

    sky_handle_RF_band_cmd();
}

static void sky_handle_one_cmd(STRU_WIRELESS_CONFIG_CHANGE* pcmd)
{
    uint8_t class  = pcmd->u8_configClass;
    uint8_t item   = pcmd->u8_configItem;
    uint32_t value = pcmd->u32_configValue;

    dlog_info("class item value %d %d 0x%0.8d \r\n", class, item, value);
    if(class == WIRELESS_FREQ_CHANGE)
    {
        switch(item)
        {
            case FREQ_BAND_SELECT:
            {
                //grd_handle_RF_band_cmd((ENUM_RF_BAND)value);
                context.freq_band = (ENUM_RF_BAND)(value);
                BB_set_RF_Band(BB_SKY_MODE, context.freq_band);
                dlog_info("context.freq_band %d \r\n", context.freq_band);
                break;
            }
            case FREQ_CHANNEL_MODE: //auto manual
            {
                //grd_handle_IT_mode_cmd((ENUM_RUN_MODE)value);
                context.it_skip_freq_mode = (ENUM_RUN_MODE)value;
                break;
            }
            case FREQ_CHANNEL_SELECT:
            {
                sky_set_it_freq(context.freq_band, (uint8_t)value);
                break;
            }

            case RC_CHANNEL_MODE:
            {
                context.rc_skip_freq_mode = (ENUM_RUN_MODE)value;
                break;
            }

            case RC_CHANNEL_SELECT:
            {
                sky_rc_channel = (uint8_t)value;
                BB_set_Rcfrq(context.freq_band, sky_rc_channel);
                break;
            }

            case RC_CHANNEL_FREQ:
            {
                context.rc_skip_freq_mode = (ENUM_RUN_MODE)MANUAL;                
                BB_write_RcRegs(value);
                
                dlog_info("RC_CHANNEL_FREQ %x\r\n", value);
                break;
            }
            
            case IT_CHANNEL_FREQ:
            {
                context.it_skip_freq_mode = MANUAL;
                BB_write_ItRegs(value);
                dlog_info("IT_CHANNEL_FREQ %x\r\n", value);                
                break;
            }

            case FREQ_BAND_WIDTH_SELECT:
            {
                context.CH_bandwidth = (ENUM_CH_BW)value;
                BB_set_RF_bandwitdh(BB_SKY_MODE, context.CH_bandwidth);
                dlog_info("FREQ_BAND_WIDTH_SELECT %x\r\n", value);                
                break;
            }
            default:
            {
                dlog_error("%s\r\n", "unknown WIRELESS_FREQ_CHANGE command");
                break;
            }
        }
    }
    
    if(class == WIRELESS_ENCODER_CHANGE)
    {
        switch(item)
        {
            case ENCODER_DYNAMIC_BIT_RATE_MODE:
                context.brc_mode = (ENUM_RUN_MODE)value;
                break;

            case ENCODER_DYNAMIC_BIT_RATE_SELECT_CH1:
                sky_notify_encoder_brc_ch1((uint8_t)value);
                break;

            case ENCODER_DYNAMIC_BIT_RATE_SELECT_CH2:
                sky_notify_encoder_brc_ch2((uint8_t)value);
                break;
            default:
                dlog_error("%s\r\n", "unknown WIRELESS_ENCODER_CHANGE command");
                break;                
        }
    }
    
    if(class == WIRELESS_MISC)
    {
        BB_handle_misc_cmds(pcmd);
    }

    if(class == WIRELESS_AUTO_SEARCH_ID)
    {
        sky_set_auto_search_rc_id();
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
                //BB_SwtichOnOffCh(0, (uint8_t)value);
                dlog_info("sky invalid cmd.");
                break;
            }

            case SWITCH_ON_OFF_CH2:
            {
                //BB_SwtichOnOffCh(1, (uint8_t)value);
                dlog_info("sky invalid cmd.");
                break;
            }

            default:
                break;                
        }
    }
}


static void sky_handle_all_cmds(void)
{   
    int ret = 0;
    int cnt = 0;
    STRU_WIRELESS_CONFIG_CHANGE cfg;
    while( (cnt++ < 5) && (1 == BB_GetCmd(&cfg)))
    {
        sky_handle_one_cmd( &cfg );
    }
}


/*
 *
*/
static void BB_sky_GatherOSDInfo(void)
{
    STRU_WIRELESS_INFO_DISPLAY *osdptr = (STRU_WIRELESS_INFO_DISPLAY *)(SRAM_BB_STATUS_SHARE_MEMORY_ST_ADDR);

    if (osdptr->osd_enable == 0)
    {
        return;
    }

    osdptr->messageId = 0x33;
    osdptr->head = 0xff; //starting writing
    osdptr->tail = 0x00;

    osdptr->IT_channel = context.cur_IT_ch;

    osdptr->agc_value[0] = BB_ReadReg(PAGE2, AAGC_2_RD);
    osdptr->agc_value[1] = BB_ReadReg(PAGE2, AAGC_3_RD);
    
    osdptr->agc_value[2] = get_rc_status();
    osdptr->agc_value[3] = BB_ReadReg(PAGE2, 0xd7);
    
    //osdptr->agc_value[2] = BB_ReadReg(PAGE2, RX3_GAIN_ALL_R);
    //osdptr->agc_value[3] = BB_ReadReg(PAGE2, RX4_GAIN_ALL_R);

    osdptr->lock_status  = get_rc_status();
    osdptr->in_debug     = context.u8_debugMode;

    osdptr->head = 0x00;
    osdptr->tail = 0xff;    //end of the writing
}

void sky_set_auto_search_rc_id(void)
{
    context.u8_idSrcSel = RC_ID_AUTO_SEARCH;
    context.dev_state = SEARCH_ID;
    search_id_list.count = 0; // completely re-search.
}

static int32_t sky_chk_flash_id_validity(void)
{
    uint8_t u8_chk = 0;
   
    cal_chk_sum(&(context.u8_flashId[0]), 5, &u8_chk);
    
    if (u8_chk == (context.u8_flashId[5])) // data is valid,use flash save id
    {
        return 0;
    }
    
    return -1;
}

static int32_t sky_write_id(uint8_t *u8_idArray)
{
    STRU_SysEvent_NvMsg st_nvMsg;

    // src:cpu0 dst:cpu2
    st_nvMsg.u8_nvSrc = INTER_CORE_CPU2_ID;
    st_nvMsg.u8_nvDst = INTER_CORE_CPU0_ID;

    // parament number
    st_nvMsg.e_nvNum = NV_NUM_RCID;

    // parament set
    st_nvMsg.u8_nvPar[0] = context.u8_idSrcSel;
    memcpy(&(st_nvMsg.u8_nvPar[1]), u8_idArray, 5);
    cal_chk_sum(&(st_nvMsg.u8_nvPar[1]), 5, &(st_nvMsg.u8_nvPar[6]));

    // send msg
    SYS_EVENT_Notify(SYS_EVENT_ID_NV_MSG, (void *)(&(st_nvMsg)));

    return 0;
}

static int32_t cal_chk_sum(uint8_t *pu8_data, uint32_t u32_len, uint8_t *u8_check)
{
    uint8_t u8_i;
    uint8_t u8_chk = 0;

    if (NULL == pu8_data)
    {
        return -1;
    }

    for (u8_i = 0; u8_i < u32_len; u8_i++)
    {
        u8_chk += pu8_data[u8_i];
    }
    
    *u8_check = u8_chk;

    return 0;
}
