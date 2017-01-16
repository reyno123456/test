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

static DebugMode g_stSkyDebugMode = 
{
    .u8_enterDebugModeCnt 	= 0,
    .bl_enterDebugModeFlag 	= 0,
    .bl_isDebugMode 		= 0,
};

static SEARCH_IDS_LIST search_id_list;
static uint32_t start_time_cnt = 0;
static uint8_t  sky_rc_channel = 0;

static enum EN_AGC_MODE en_agcmode = UNKOWN_AGC;

static init_timer_st sky_timer0_0;
static init_timer_st sky_timer0_1;

static int sky_timer0_0_running = 0;
static int sky_timer0_1_running = 0;
static int switch_5G_count = 0;

static void sky_handle_debug_mode_cmd_spi(void);
static void sky_handle_debug_mode_cmd_event(uint8_t value);
static void sky_handle_one_cmd(STRU_WIRELESS_CONFIG_CHANGE* pcmd);
static void sky_handle_all_cmds(void);

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

    sky_Timer0_Init();
    sky_Timer1_Init();    
    sky_search_id_timeout_irq_enable(); //enabole TIM1 timeout

    reg_IrqHandle(BB_RX_ENABLE_VECTOR_NUM, wimax_vsoc_rx_isr, NULL);
    INTR_NVIC_EnableIRQ(BB_RX_ENABLE_VECTOR_NUM);

    context.qam_ldpc = 0;
    sky_set_McsByIndex(context.qam_ldpc);
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


void sky_notify_encoder_brc(uint8_t br)
{
	STRU_SysEvent_BB_ModulationChange event;
	event.BB_MAX_support_br = br;
	SYS_EVENT_Notify(SYS_EVENT_ID_BB_SUPPORT_BR_CHANGE, (void*)&event);	
    
    dlog_info("brc =%d\r\n", br);        
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
        sky_notify_encoder_brc( mcs_idx_bitrate_map[idx] );
	}
}


void sky_agc_gain_toggle(void)
{
    dlog_info("AGCToggle\r\n");
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
	
    INTR_NVIC_EnableIRQ(TIMER_INTR00_VECTOR_NUM);
    TIM_StartTimer(sky_timer0_0);

    STRU_WIRELESS_INFO_DISPLAY *osdptr = (STRU_WIRELESS_INFO_DISPLAY *)(OSD_STATUS_SHM_ADDR);
    osdptr->in_debug    = (uint8_t)(g_stSkyDebugMode.bl_isDebugMode);
	sky_handle_all_cmds();
}


void Sky_TIM0_IRQHandler(uint32_t u32_vectorNum)
{
    sky_timer0_0_running = 1;
    sky_search_id_timeout_irq_disable();

    Reg_Read32(BASE_ADDR_TIMER0 + TMRNEOI_0);
    INTR_NVIC_ClearPendingIRQ(BB_RX_ENABLE_VECTOR_NUM); //clear pending after TX Enable is LOW. MUST!
    INTR_NVIC_EnableIRQ(BB_RX_ENABLE_VECTOR_NUM);  
    
    INTR_NVIC_DisableIRQ(TIMER_INTR00_VECTOR_NUM);
    TIM_StopTimer(sky_timer0_0);

    if(0 == sky_timer0_1_running && 0 == g_stSkyDebugMode.bl_isDebugMode)
    {
        sky_physical_link_process();
        
        //patch for 5G switch: For demo test only.
        if(context.freq_band == RF_5G && switch_5G_count < 5)
        {
            switch_5G_count++;
            sky_soft_reset();
        }

        BB_sky_GatherOSDInfo();
    }

    sky_timer0_0_running = 0;
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
    if(context.dev_state == SEARCH_ID)
    {
        if(sky_id_search_run())
        {
            uint8_t *p_id = sky_id_search_get_best_id();
            sky_set_RC_id(p_id);

            context.dev_state = CHECK_ID_MATCH;
            sky_soft_reset();
        }
        else
        {
            context.rc_unlock_cnt ++;
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
    TIM_StartTimer(sky_timer0_1);
}

int sky_search_id_timeout_irq_disable()
{
    TIM_StopTimer(sky_timer0_1);
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

void Sky_TIM1_IRQHandler(uint32_t u32_vectorNum)
{
    static int Timer1_Delay2_Cnt = 0;    

	if(1 == (g_stSkyDebugMode.bl_isDebugMode)) 
	{
		return;
	}  
 
	sky_timer0_1_running = 1; 

    dlog_info("sky_search_id_timeout_irq_enable \r\n");
    INTR_NVIC_ClearPendingIRQ(TIMER_INTR01_VECTOR_NUM);
    if(Timer1_Delay2_Cnt < 560)
    {
        Timer1_Delay2_Cnt ++;
    }
    else
    {
        if(sky_timer0_0_running == 0) //To avoid the spi access conflict
        {
            sky_search_id_timeout();
        }
        Timer1_Delay2_Cnt = 0;
    }

    sky_timer0_1_running = 0;
}


void sky_Timer1_Init(void)
{
    sky_timer0_1.base_time_group = 0;
    sky_timer0_1.time_num = 1;
    sky_timer0_1.ctrl = 0;
    sky_timer0_1.ctrl |= TIME_ENABLE | USER_DEFINED;
    TIM_RegisterTimer(sky_timer0_1, 1000);
    reg_IrqHandle(TIMER_INTR01_VECTOR_NUM, Sky_TIM1_IRQHandler, NULL);
}

void sky_Timer0_Init(void)
{
    sky_timer0_0.base_time_group = 0;
    sky_timer0_0.time_num = 0;
    sky_timer0_0.ctrl = 0;
    sky_timer0_0.ctrl |= TIME_ENABLE | USER_DEFINED;

    TIM_RegisterTimer(sky_timer0_0, 6800);

    reg_IrqHandle(TIMER_INTR00_VECTOR_NUM, Sky_TIM0_IRQHandler, NULL);
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
static void sky_handle_brc_bitrate_cmd(void)
{
	uint8_t data0 = BB_ReadReg(PAGE2, ENCODER_BRC_CHAGE_0);
    uint8_t data1 = BB_ReadReg(PAGE2, ENCODER_BRC_CHAGE_1);
    uint8_t bps = data0&0x3F;

    if( (data0+1==data1) && ( (data0&0xc0)==0xc0) && (context.brc_bps != bps))
    {
        context.brc_bps = bps;
        sky_notify_encoder_brc(bps);
        dlog_info("brc_bps = %d \r\n", bps);
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


static void sky_handle_debug_mode_cmd_spi(void)
{
    STRU_WIRELESS_INFO_DISPLAY *osdptr = (STRU_WIRELESS_INFO_DISPLAY *)(OSD_STATUS_SHM_ADDR);
    uint8_t data0 = BB_ReadReg(PAGE2, NTF_TEST_MODE_0);
    uint8_t data1 = BB_ReadReg(PAGE2, NTF_TEST_MODE_1);

    if((data0+1 == data1) && (0 == data0))//enter test mode
    {
		if(1 != (g_stSkyDebugMode.bl_isDebugMode))
		{
			g_stSkyDebugMode.bl_isDebugMode = 1;
            osdptr->in_debug    = (uint8_t)(g_stSkyDebugMode.bl_isDebugMode);
			dlog_info("g_stSkyDebugMode.bl_isDebugMode = 1");
		}
    }
    else if((data0+1 == data1) && (0 != data0))//out test mode
    {
		if(0 != (g_stSkyDebugMode.bl_isDebugMode))
		{
			g_stSkyDebugMode.bl_isDebugMode = 0;
			dlog_info("g_stSkyDebugMode.bl_isDebugMode = 0");
		}
    }
	else
	{
		;
	}
}

static void sky_handle_debug_mode_cmd_event(uint8_t value)
{
	if(0 == value)//enter test mode
    {
		if(1 != (g_stSkyDebugMode.bl_isDebugMode))
		{
			g_stSkyDebugMode.bl_isDebugMode = 1;
			dlog_info("g_stSkyDebugMode.bl_isDebugMode = 1");

            context.rc_skip_freq_mode = MANUAL;
            context.it_skip_freq_mode = MANUAL;
            context.qam_skip_mode     = MANUAL;
		}
    }
    else//out test mode
    {
		if(0 != (g_stSkyDebugMode.bl_isDebugMode))
		{
			g_stSkyDebugMode.bl_isDebugMode = 0;
			dlog_info("g_stSkyDebugMode.bl_isDebugMode = 0");
		}
    }
}

void sky_handle_all_spi_cmds(void)
{
    sky_handle_RC_cmd();

    sky_handle_IT_CH_cmd();

    //sky_handle_QAM_cmd();
    
    sky_handle_MCS_cmd();

    sky_handle_brc_mode_cmd();

    sky_handle_brc_bitrate_cmd();

    sky_handle_RF_band_cmd();

	//sky_handle_debug_mode_cmd_spi();
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

            case ENCODER_DYNAMIC_BIT_RATE_SELECT:
                sky_notify_encoder_brc((uint8_t)value);
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
                sky_handle_debug_mode_cmd_event( (uint8_t)value);
                break;

            case 1:
                BB_WriteReg(PAGE2, 0x02, 0x06);
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
    STRU_WIRELESS_INFO_DISPLAY *osdptr = (STRU_WIRELESS_INFO_DISPLAY *)(OSD_STATUS_SHM_ADDR);

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

    osdptr->lock_status    = get_rc_status();
	osdptr->in_debug     = (uint8_t)(g_stSkyDebugMode.bl_isDebugMode);

    osdptr->head = 0x00;
    osdptr->tail = 0xff;    //end of the writing
}
