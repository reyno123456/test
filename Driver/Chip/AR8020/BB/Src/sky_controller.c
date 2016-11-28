#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <stdint.h>

#include "reg_rw.h"
#include "timer.h"
#include "interrupt.h"
#include "systicks.h"
#include "config_functions_sel.h"
#include "sky_controller.h"
#include "config_baseband_register.h"
#include "sys_param.h"
#include "debuglog.h"
#include "sys_event.h"

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
static uint8_t cur_mod_regvalue = 0xff; 

static init_timer_st sky_timer0_0;
static init_timer_st sky_timer0_1;

static int sky_timer0_0_running = 0;
static int sky_timer0_1_running = 0;


void Sky_Parm_Initial(void)
{
    dev_state = SEARCH_ID;

    context.it_skip_freq_mode = AUTO;
    context.rc_skip_freq_mode = AUTO;
    context.qam_skip_mode == AUTO;
    context.cur_IT_ch = 0xff;

    sky_id_search_init();

    sky_Timer0_Init();
    sky_Timer1_Init();
    
    sky_search_id_timeout_irq_enable();

    reg_IrqHandle(BB_RX_ENABLE_VECTOR_NUM, wimax_vsoc_rx_isr);
    INTR_NVIC_EnableIRQ(BB_RX_ENABLE_VECTOR_NUM);
}


uint8_t sky_id_match(void)
{
    static int total_count = 0;
    static int lock_count = 0;
    uint8_t data = BB_ReadReg(PAGE2, FEC_4_RD) & 0x03;

    total_count ++;
    lock_count += ((data) ? 1 : 0);
        
    if(total_count > 500)
    {
        printf("-L:%d-\n", lock_count);    
        total_count = 0;
        lock_count = 0;
    }
    
    return (data) ? 1 : 0;
}


void sky_notify_encoder_brc(uint8_t br)
{
	STRU_SysEvent_BB_ModulationChange event;
	event.BB_MAX_support_br = br;
	SYS_EVENT_Notify(SYS_EVENT_ID_BB_SUPPORT_BR_CHANGE, (void*)&event);	
}

void sky_set_ITQAM_and_notify(uint8_t mod)
{
    uint8_t i = 0;
    
    dlog_info("QAM=>0x%.2x\r\n", mod);
    BB_WriteReg(PAGE2, TX_2, mod);

	if(context.brc_mode == AUTO)
	{
	    uint8_t br = BB_map_modulation_to_br(mod);
        dlog_info("br=%d\r\n", br);
        sky_notify_encoder_brc(br);
	}
}


void sky_agc_gain_toggle(void)
{
    if(FAR_AGC == en_agcmode)
    {
        BB_WriteReg(PAGE0, AGC_2, AAGC_GAIN_NEAR);
        en_agcmode = NEAR_AGC;
    }
    else
    {
        BB_WriteReg(PAGE0, AGC_2, AAGC_GAIN_FAR);
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
        en_agcmode = FAR_AGC;
        printf("=>F", rx1_gain, rx2_gain);
    }
     
    if( ((rx1_gain < POWER_GATE)&&(rx2_gain < POWER_GATE)) \
        && (rx1_gain > 0x00) && (rx2_gain >0x00) \
        && en_agcmode != NEAR_AGC)
    {
        BB_WriteReg(PAGE0, AGC_2, AAGC_GAIN_NEAR);
        en_agcmode = NEAR_AGC;
        printf("=>N", rx1_gain, rx2_gain);
    }
}


//*********************TX RX initial(14ms irq)**************
void wimax_vsoc_rx_isr()
{
    INTR_NVIC_DisableIRQ(BB_RX_ENABLE_VECTOR_NUM);   
    INTR_NVIC_EnableIRQ(TIMER_INTR00_VECTOR_NUM);
    TIM_StartTimer(sky_timer0_0);
}


void Sky_TIM0_IRQHandler(void)
{
    sky_timer0_0_running = 1;
    sky_search_id_timeout_irq_disable();

    Reg_Read32(BASE_ADDR_TIMER0 + TMRNEOI_0);
    INTR_NVIC_ClearPendingIRQ(BB_RX_ENABLE_VECTOR_NUM); //clear pending after TX Enable is LOW. MUST!
    INTR_NVIC_EnableIRQ(BB_RX_ENABLE_VECTOR_NUM);  
    
    INTR_NVIC_DisableIRQ(TIMER_INTR00_VECTOR_NUM);
    TIM_StopTimer(sky_timer0_0);

    if(sky_timer0_1_running == 0)
    {
        sky_physical_link_process();
    }

    sky_timer0_0_running = 0;
}


void sky_rc_hopfreq(void)
{
    sky_rc_channel++;
    if(sky_rc_channel >=  MAX_RC_FRQ_SIZE)
    {
        sky_rc_channel = 0;
    }

    BB_set_Rcfrq(sky_rc_channel);
}


void sky_set_it_freq(uint8_t ch)
{
    if(context.cur_IT_ch != ch)
    {
        BB_set_ITfrq(ch);
        printf("S=>%d\r\n", ch);
    }
}

void sky_get_id(uint8_t* idptr)
{
    idptr[0] = BB_ReadReg(PAGE2, FEC_1_RD);
    idptr[1] = BB_ReadReg(PAGE2, FEC_2_RD_1);
    idptr[2] = BB_ReadReg(PAGE2, FEC_2_RD_2);
    idptr[3] = BB_ReadReg(PAGE2, FEC_2_RD_3);
    idptr[4] = BB_ReadReg(PAGE2, FEC_2_RD_4);   
}

void sky_set_id(uint8_t *idptr)
{
    uint8_t i;
    uint8_t addr[] = {FEC_7, FEC_8, FEC_9, FEC_10, FEC_11};

    printf("RCid:%02x%02x%02x%02x%02x\r\n", idptr[0], idptr[1], idptr[2], idptr[3], idptr[4]);                
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
    if(dev_state == SEARCH_ID)
    {
        if(sky_id_search_run())
        {
            uint8_t *p_id = sky_id_search_get_best_id();
            //memcpy(context.id, p_id, 5);
            sky_set_id(p_id);

            dev_state = CHECK_ID_MATCH;
            sky_soft_reset();
        }
    }
    else if(dev_state == ID_MATCH_LOCK)
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
        }
        else
        {
            context.rc_unlock_cnt++;
            // sky_soft_reset();
        }

        if(MAX_RC_FRQ_SIZE <= context.rc_unlock_cnt)
        {
            dev_state = CHECK_ID_MATCH;
        }
    }
    else if(dev_state == CHECK_ID_MATCH)
    {
        context.locked = sky_id_match();
        if(context.locked)
        {
            if(context.rc_skip_freq_mode == AUTO)
            {
                sky_rc_hopfreq();
            }

            dev_state = ID_MATCH_LOCK;
        }
        else
        {
            sky_soft_reset();
        }
    }

    sky_auto_adjust_agc_gain();
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
    
    #define SKY_RC_ERR(status)  (0x80 == rc_status)
    #define SKY_CRC_OK(status)  ((status & 0x02) ? 1 : 0)
    
    if(SKY_RC_ERR(rc_status))
    {
        sky_soft_reset();
        return FALSE;
    }

    if(SKY_CRC_OK(rc_status))
    {
        sky_get_id(search_id_list.search_ids[search_id_list.count].id);

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
    if(dev_state == SEARCH_ID )
    {
        if(context.rc_skip_freq_mode == AUTO)
        {
            sky_rc_hopfreq();
        }

        sky_agc_gain_toggle();
        sky_soft_reset();
    }
}

void Sky_TIM1_IRQHandler(void)
{
    static int Timer1_Delay2_Cnt = 0;
    
    sky_timer0_1_running = 1; 
    
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
    reg_IrqHandle(TIMER_INTR01_VECTOR_NUM, Sky_TIM1_IRQHandler);
}

void sky_Timer0_Init(void)
{
    sky_timer0_0.base_time_group = 0;
    sky_timer0_0.time_num = 0;
    sky_timer0_0.ctrl = 0;
    sky_timer0_0.ctrl |= TIME_ENABLE | USER_DEFINED;

    TIM_RegisterTimer(sky_timer0_0, 6800);

    reg_IrqHandle(TIMER_INTR00_VECTOR_NUM, Sky_TIM0_IRQHandler);
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
        sky_set_it_freq(req_ch);
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
        context.rc_skip_freq_mode = (RUN_MODE)(data0 & 0x7F);
        if((RUN_MODE)data0 == MANUAL && data0!= 0xFE)
        {
            sky_rc_channel = (data0 & 0x7f);
            BB_set_Rcfrq(sky_rc_channel);
            dlog_info("rc_channel: %d \r\n", sky_rc_channel);
        }
    }
}


/*
 * handle 2G/5G switch 
*/
static void sky_handle_RF_band_cmd(void)
{
    uint8_t data0, data1;

    data0 = BB_ReadReg(PAGE2, RF_BAND_CHANGE_0);
    data1 = BB_ReadReg(PAGE2, RF_BAND_CHANGE_1);

    if( (data0+1==data1) && ((data0&0xc0)==0xc0) && context.RF_band != (data0&0x3F) ) //check right
    {
        ENUM_RF_BAND band = (ENUM_RF_BAND)(data0&0x3F);
        BB_set_RF_Band(BB_SKY_MODE, band);
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
    RUN_MODE mode = (RUN_MODE)(data0 & 0x1f);

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
        sky_notify_encoder_brc(bps*10);
        dlog_info("brc_bps = %d \r\n", bps);
    }
}

static void sky_handle_QAM_cmd(void)
{
    uint8_t data0 = BB_ReadReg(PAGE2, QAM_CHANGE_0);
    uint8_t data1 = BB_ReadReg(PAGE2, QAM_CHANGE_1);

    if(data0+1==data1 && cur_mod_regvalue != data0)
    {
        sky_set_ITQAM_and_notify(data0);
        cur_mod_regvalue = data0;
    }
}

void sky_handle_all_spi_cmds(void)
{
    sky_handle_RC_cmd();

    sky_handle_IT_CH_cmd();

    sky_handle_QAM_cmd();

    sky_handle_brc_mode_cmd();

    sky_handle_brc_bitrate_cmd();
}