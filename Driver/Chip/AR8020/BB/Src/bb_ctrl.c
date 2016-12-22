#include <stdint.h>
#include <string.h>

#include "debuglog.h"
#include "bb_spi.h"
#include "wireless_interface.h"
#include "bb_ctrl_internal.h"
#include "bb_grd_sweep.h"
#include "bb_grd_ctrl.h"
#include "bb_sky_ctrl.h"
#include "bb_uart_com.h"
#include "reg_rw.h"
#include "systicks.h"
#include "bb_sys_param.h"
#include "bb_regs.h"
#include "sys_event.h"
#include "rf8003s.h"


#define     BB_SPI_TEST         (0)
#define     RF_SPI_TEST         (0)

#define     VSOC_GLOBAL2_BASE   (0xA0030000)
#define     BB_SPI_UART_SEL     (0x9c)

CONTEXT context;
static ENUM_REG_PAGES en_curPage;

const STRU_FRQ_CHANNEL Rc_frq[MAX_2G_RC_FRQ_SIZE] =     // 2.4G
{
    {0x55, 0x55, 0x55, 0x50},
    {0x66, 0x66, 0x66, 0x50},
    {0x77, 0x77, 0x77, 0x50},
    {0x88, 0x88, 0x88, 0x50},
    {0x99, 0x99, 0x99, 0x50},
    {0xAA, 0xAA, 0xAA, 0x50},
    {0xBB, 0xBB, 0xBB, 0x50},
    {0xCC, 0xCC, 0xCC, 0x50},
    {0xDD, 0xDD, 0xDD, 0x50},
    {0xEE, 0xEE, 0xEE, 0x50},
    {0x00, 0x00, 0x00, 0x51},
    {0x11, 0x11, 0x11, 0x51},
    {0x22, 0x22, 0x22, 0x51},
    {0x33, 0x33, 0x33, 0x51},
    {0x44, 0x44, 0x44, 0x51},
    {0x55, 0x55, 0x55, 0x51},
    {0x66, 0x66, 0x66, 0x51},
    {0x77, 0x77, 0x77, 0x51},
    {0x88, 0x88, 0x88, 0x51},
    {0x99, 0x99, 0x99, 0x51},
    {0xAA, 0xAA, 0xAA, 0x51},
    {0xBB, 0xBB, 0xBB, 0x51},
    {0xCC, 0xCC, 0xCC, 0x51},
    {0xDD, 0xDD, 0xDD, 0x51},
    {0xEE, 0xEE, 0xEE, 0x51},
    {0x00, 0x00, 0x00, 0x52},
    {0x11, 0x11, 0x11, 0x52},
    {0x22, 0x22, 0x22, 0x52},
    {0x33, 0x33, 0x33, 0x52},
    {0x44, 0x44, 0x44, 0x52},
    {0x55, 0x55, 0x55, 0x52},
    {0x66, 0x66, 0x66, 0x52},
    {0x77, 0x77, 0x77, 0x52},
    {0x88, 0x88, 0x88, 0x52}
};



const STRU_FRQ_CHANNEL It_frq[MAX_2G_IT_FRQ_SIZE] = {     //2.4G
    {0x00,0x00,0x00,0x4b }, {0x00,0x00,0x00,0x4c },
    {0x00,0x00,0x00,0x4d }, {0x00,0x00,0x00,0x4e },
    {0x00,0x00,0x00,0x4f }, {0x00,0x00,0x00,0x50 },
};


const STRU_FRQ_CHANNEL Rc_5G_frq[MAX_5G_RC_FRQ_SIZE] = {     // 5G
    {0x00,0x00,0x00,0x5F }, {0x00,0x00,0x00,0x60 },
    {0x00,0x00,0x00,0x61 }, {0x00,0x00,0x00,0x62 },
};


const STRU_FRQ_CHANNEL It_5G_frq[MAX_5G_IT_FRQ_SIZE] = {     //5G
    {0x00,0x00,0x00,0x5F }, {0x00,0x00,0x00,0x60 },
    {0x00,0x00,0x00,0x61 }, {0x00,0x00,0x00,0x62 },
};



static int BB_RF_start_cali();
/*
  * cali_reg: Store the calibration registers value
 */
static uint8_t cali_reg[2][10] = {{0}, {0}};

static void BB_regs_init(ENUM_BB_MODE en_mode)
{
    extern const uint8_t BB_sky_regs[][256];
    extern const uint8_t BB_grd_regs[][256];
    
    uint32_t page_cnt=0;
    const uint8_t *regs = (en_mode == BB_SKY_MODE) ? (const uint8_t *)BB_sky_regs : (const uint8_t *)BB_grd_regs;
    
    for(page_cnt = 0 ; page_cnt < 4; page_cnt ++)
    {
        uint32_t addr_cnt=0;
        
        ENUM_REG_PAGES page = (page_cnt==0)? PAGE0: \
                              ((page_cnt==1)?PAGE1: \
                              ((page_cnt==2)?PAGE2:PAGE3));
        /*
         * PAGE setting included in the regs array.
         */
        en_curPage = page;

        for(addr_cnt = 0; addr_cnt < 256; addr_cnt++)
        {
            //PAGE1 reg[0xa1] reg[0xa2] reg[0xa4] reg[0xa5] are PLL setting for cpu0, cpu1, cpu2, set in the sysctrl.c when system init
            if(page==PAGE1 && (addr_cnt==0xa1||addr_cnt==0xa2||addr_cnt==0xa4||addr_cnt==0xa5))
            {}
            else
            {
                BB_SPI_curPageWriteByte((uint8_t)addr_cnt, *regs);
            }
            regs++;
        }
    }
}


int BB_softReset(ENUM_BB_MODE en_mode)
{
    uint8_t reg_after_reset;
    if(en_mode == BB_GRD_MODE)
    {
        BB_SPI_curPageWriteByte(0x00,0xB2);
        BB_SPI_curPageWriteByte(0x00,0xB0);
        reg_after_reset = 0xB0;
    }
    else
    {        
        BB_SPI_curPageWriteByte(0x00, 0x81);
        BB_SPI_curPageWriteByte(0x00, 0x80);
        reg_after_reset = 0x80;
    }

    //bug fix: write reset register may fail. 
    int count = 0;
    while(count++ < 5)
    {
        uint8_t rst = BB_SPI_curPageReadByte(0x00);
        if(rst != reg_after_reset)
        {
            BB_SPI_curPageWriteByte(0x00, reg_after_reset);
            count ++;
        }
        else
        {
            break;
        }
    }

    en_curPage = PAGE2;
    return 0;
}



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


void BB_use_param_setting(PARAM *user_setting)
{
    memcpy(context.id, user_setting->rc_id.id1, 5);
    memcpy(context.rc_mask, user_setting->user_param.rc_mask, sizeof(context.rc_mask));
    memcpy(context.it_mask, user_setting->user_param.it_mask, sizeof(context.it_mask));
    
    memcpy( (uint8_t *)((void *)(context.qam_change_threshold)),
            (uint8_t *)((void *)(user_setting->user_param.qam_change_threshold)),
            sizeof(context.qam_change_threshold));

    #if 0
    memcpy(context.sp20dbm,  user_setting->user_param.sp20dbm,  4);
    memcpy(context.sp20dbmb, user_setting->user_param.sp20dbmb, 4);
    memcpy(context.gp20dbm,  user_setting->user_param.gp20dbm,  4);
    memcpy(context.gp20dbmb, user_setting->user_param.gp20dbmb, 4);
    context.cur_ch   = 0xff;
    #endif
    
    context.ldpc     = user_setting->user_param.ldpc;
    context.qam_mode = user_setting->user_param.qam_mode;
    
    context.qam_ldpc = merge_qam_ldpc_to_index(context.qam_mode,context.ldpc);

    context.it_skip_freq_mode = user_setting->user_param.it_skip_freq_mode;
    context.rc_skip_freq_mode = user_setting->user_param.rc_skip_freq_mode;
    context.pwr = user_setting->user_param.power;
    context.enable_freq_offset = user_setting->user_param.enable_freq_offset;
    context.freq_band = user_setting->user_param.freq_band;
    context.search_id_enable = user_setting->user_param.search_id_enable;
    context.rf_power_mode = user_setting->user_param.rf_power_mode;

    context.it_manual_ch  = 0xff;
    context.qam_skip_mode = AUTO;
    context.CH_bandwidth      = BW_10M;
    context.it_manual_rf_band = 0xff;
    context.trx_ctrl          = IT_RC_MODE;
    
    gen_qam_threshold_range();    
}


void BB_init(ENUM_BB_MODE en_mode)
{    
    BB_SPI_init();
    BB_uart10_spi_sel(0x00000003);
    
    PARAM *user_setting = BB_get_sys_param();
    BB_use_param_setting(user_setting);
    BB_regs_init(en_mode);

    {
        extern const uint8_t RF_8003s_regs[128]; 
        RF8003s_init( (uint8_t *)RF_8003s_regs);
    }

    BB_softReset(en_mode);

    //RF calibration in both sky& Ground.
    BB_RF_start_cali();
    BB_set_RF_Band(en_mode, context.freq_band);
    BB_softReset(en_mode);

    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_USER_CFG_CHANGE_LOCAL, BB_HandleEventsCallback);

    if(en_mode == BB_SKY_MODE)
    {
        BB_SKY_start();
    }
    else
    {
        BB_GRD_start();
    }
    
    dlog_info("BB mode Band %d %d %s \r\n", en_mode, context.freq_band, "BB_init Done");
}


void BB_uart10_spi_sel(uint32_t sel_dat)
{
    write_reg32( (uint32_t *)(VSOC_GLOBAL2_BASE + BB_SPI_UART_SEL),	sel_dat);
}


uint8_t BB_WriteReg(ENUM_REG_PAGES page, uint8_t addr, uint8_t data)
{
    if(en_curPage != page)
    {
        BB_SPI_WriteByte(page, addr, data);
        en_curPage = page;
    }
    else
    {
        BB_SPI_curPageWriteByte(addr, data);
    }
}

uint8_t BB_ReadReg(ENUM_REG_PAGES page, uint8_t addr)
{
    uint8_t reg;
    if(en_curPage != page)
    {
        reg = BB_SPI_ReadByte(page, addr);
        en_curPage = page;
    }
    else
    {
        reg = BB_SPI_curPageReadByte(addr);
    }
    return reg;
}

int BB_WriteRegMask(ENUM_REG_PAGES page, uint8_t addr, uint8_t data, uint8_t mask)
{
    uint8_t ori = BB_ReadReg(page, addr);
    data = (ori & (~mask)) | data;
    return BB_WriteReg(page, addr, data);
}


int BB_ReadRegMask(ENUM_REG_PAGES page, uint8_t addr, uint8_t mask)
{
    return BB_ReadReg(page, addr) & mask;
}


uint8_t BB_set_sweepfrq(ENUM_RF_BAND band, uint8_t ch)
{
	STRU_FRQ_CHANNEL *ch_ptr = (STRU_FRQ_CHANNEL *)((band == RF_2G)?It_frq:It_5G_frq);

	BB_WriteReg(PAGE2, SWEEP_FREQ_0, ch_ptr[ch].frq1);
	BB_WriteReg(PAGE2, SWEEP_FREQ_1, ch_ptr[ch].frq2);
	BB_WriteReg(PAGE2, SWEEP_FREQ_2, ch_ptr[ch].frq3);
	BB_WriteReg(PAGE2, SWEEP_FREQ_3, ch_ptr[ch].frq4);
   
}

uint8_t BB_set_ITfrq(ENUM_RF_BAND band, uint8_t ch)
{
	STRU_FRQ_CHANNEL *it_ch_ptr = (STRU_FRQ_CHANNEL *)((band == RF_2G)?It_frq:It_5G_frq);

	BB_WriteReg(PAGE2, AGC3_0, it_ch_ptr[ch].frq1);
	BB_WriteReg(PAGE2, AGC3_1, it_ch_ptr[ch].frq2);
	BB_WriteReg(PAGE2, AGC3_2, it_ch_ptr[ch].frq3);
	BB_WriteReg(PAGE2, AGC3_3, it_ch_ptr[ch].frq4);

}

uint8_t BB_set_Rcfrq(ENUM_RF_BAND band, uint8_t ch)
{
	STRU_FRQ_CHANNEL *rc_ch_ptr = (STRU_FRQ_CHANNEL *)((band == RF_2G)?Rc_frq:Rc_5G_frq);

    BB_WriteReg(PAGE2, AGC3_a, rc_ch_ptr[ch].frq1);
    BB_WriteReg(PAGE2, AGC3_b, rc_ch_ptr[ch].frq2);
    BB_WriteReg(PAGE2, AGC3_c, rc_ch_ptr[ch].frq3);
    BB_WriteReg(PAGE2, AGC3_d, rc_ch_ptr[ch].frq4); 
}


void BB_set_QAM(ENUM_BB_QAM mod)
{
    uint8_t data = BB_ReadReg(PAGE2, TX_2);
    BB_WriteReg(PAGE2, TX_2, (data & 0x3f) | ((uint8_t)mod << 6));
}

void BB_set_LDPC(ENUM_BB_LDPC ldpc)
{
    uint8_t data = BB_ReadReg(PAGE2, TX_2);
    BB_WriteReg(PAGE2, TX_2, (data & 0x07) | (uint8_t)ldpc);
}


static uint8_t mod_br_map[][2] = 
{
    ((MOD_BPSK<<6)  | (BW_10M <<3)  | LDPC_1_2),  0, //encoder br:500k
    ((MOD_4QAM<<6)  | (BW_10M <<3)  | LDPC_1_2),  2, //encoder br:2M
    ((MOD_4QAM<<6)  | (BW_10M <<3)  | LDPC_2_3),  4, //encoder br:4M
    ((MOD_16QAM<<6) | (BW_10M <<3)  | LDPC_1_2),  6, //encoder br:7M 
    ((MOD_64QAM<<6) | (BW_10M <<3)  | LDPC_1_2),  9, //encoder br:10M
    ((MOD_64QAM<<6) | (BW_10M <<3)  | LDPC_2_3),  12, //encoder br:15M
};

uint8_t BB_map_modulation_to_br(uint8_t mod)
{
    uint8_t br = mod_br_map[0][1];
    ENUM_CH_BW bw = (ENUM_CH_BW)((mod >> 3)& 0x07);
    uint8_t i  = 0;

    mod |= (uint8_t)(BW_10M <<3);
    for(i = 0; i < sizeof(mod_br_map) / sizeof(mod_br_map[0]); i++)
    {
        if(mod_br_map[i][0] >= mod)
        {
            br = mod_br_map[i][1];
            break;
        }
    }

    if(bw == BW_20M)
    {
        br = br*3/2;
    }

    return br;
}


/************************************************************
PAGE2	0x20[2]	rf_freq_sel_rx_sweep    RW    sweep frequency selection for the 2G frequency band o or 5G frequency band,
		0'b0: 2G frequency band
		1'b1: 5G frequency band

PAGE2	0x21[7]	rf_freq_sel_tx	          RW     The frequency band selection for the transmitter
		1'b0: 2G frequency band
		1'b1 for 5G frequency band

PAGE2	0x21[4]	rf_freq_sel_rx_work	   RW    The frequency band selection for the receiver
		1'b0: 2G frequency band
		1'b1 for 5G frequency band
*****************************************************************/
/*
 * set RF baseband to 2.4G or 5G
 */
void BB_set_RF_Band(ENUM_BB_MODE sky_ground, ENUM_RF_BAND rf_band)
{
    if(rf_band == RF_2G)
    {
        #if 0
            BB_WriteRegMask(PAGE2, 0x20, 0, 0x04);      //bit[2] to 0
            BB_WriteRegMask(PAGE2, 0x21, 0, 0x90);      //set to bit[7] bit[4] 0
        #else
            //For 5G test only.
            BB_WriteReg(PAGE2, 0x21, 0x60);
            BB_WriteReg(PAGE0, 0x20, 0xFE);
        #endif
    }
    else
    {
        #if 0
        BB_WriteRegMask(PAGE2, 0x20, 0x04, 0x04);   //set to 1
        BB_WriteRegMask(PAGE2, 0x21, 0x90, 0x90);   //set to bit[7] bit[4] 0     
        #else
        if(sky_ground == BB_GRD_MODE)
        {
            BB_WriteReg(PAGE2, 0x21, 0x70);
            BB_WriteReg(PAGE0, 0x20, 0xF9);
        }
        else
        {
            //For 5G test only.
            BB_WriteReg(PAGE2, 0x21, 0xF0);
            BB_WriteReg(PAGE0, 0x20, 0xFE);
            BB_set_QAM(MOD_16QAM);
        }
        BB_WriteReg(PAGE2, 0x02, 0x06);
        BB_set_ITfrq(RF_5G, 0);   
        //softreset
        BB_softReset(sky_ground);
        #endif
    }

    //calibration and reset
    BB_RF_2G_5G_switch(rf_band);
    grd_sweep_freq_init(); //re-start sweep.
    dlog_info("Set Band %d %d\r\n", sky_ground, rf_band);
}


/*
 * set RF bandwidth = 10M or 20M
*/
void BB_set_RF_bandwitdh(ENUM_BB_MODE sky_ground, ENUM_CH_BW rf_bw)
{
    uint8_t regaddr = (sky_ground == BB_SKY_MODE)? RX_MODULATION: TX_2;
    BB_WriteRegMask(PAGE2, RX_MODULATION, (rf_bw << 3), 0x38); /*bit[5:3]*/
   
    //softreset
    BB_softReset(sky_ground);
}

void read_cali_register(uint8_t *buf)
{
    buf[0] = BB_ReadReg(PAGE0, 0xd0);
    buf[1] = BB_ReadReg(PAGE0, 0xd1);
    buf[2] = BB_ReadReg(PAGE0, 0xd2);
    buf[3] = BB_ReadReg(PAGE0, 0xd3);
    buf[4] = BB_ReadReg(PAGE0, 0xd4);
    buf[5] = BB_ReadReg(PAGE0, 0xd5);
    buf[6] = BB_ReadReg(PAGE0, 0xd6);
    buf[7] = BB_ReadReg(PAGE0, 0xd7);
    buf[8] = BB_ReadReg(PAGE0, 0xd8);
    buf[9] = BB_ReadReg(PAGE0, 0xd9);

    //dlog_info("cali Registers1: %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x\r\n", 
    //           buf[0], buf[1], buf[2], buf[3], buf[4],
    //           buf[5], buf[6], buf[7], buf[8], buf[9]);

    char test[15];
    test[0] = BB_ReadReg(PAGE0, 0x64);
    test[1] = BB_ReadReg(PAGE0, 0x67);
    test[2] = BB_ReadReg(PAGE0, 0x68);
    test[3] = BB_ReadReg(PAGE0, 0x69);
    test[4] = BB_ReadReg(PAGE0, 0x6a);
    test[5] = BB_ReadReg(PAGE0, 0x6b);
    test[6] = BB_ReadReg(PAGE0, 0x6c);
    test[7] = BB_ReadReg(PAGE0, 0x6d);
    test[8] = BB_ReadReg(PAGE0, 0x6e);
    test[9] = BB_ReadReg(PAGE0, 0x6f);
    
    //dlog_info("Before cali: %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x\r\n", 
    //           test[0], test[1], test[2], test[3], test[4],
    //           test[5], test[6], test[7], test[8], test[9]);    
}


static int BB_RF_start_cali()
{
    uint8_t data;

    //step 1
    //1.1 Enable RF calibration 0x61= 0x0F
    BB_WriteReg(PAGE0, TX_CALI_ENABLE, 0x0F);

    //1.2: soft reset0
    //page0 0x64[4] = 1'b1
    data = BB_ReadReg(PAGE0, 0x64);
    data = data | 0x10;
    BB_WriteReg(PAGE0, 0x64, data);
    //page0 0x64[4] = 1'b0
    data = data & 0xEF;
    BB_WriteReg(PAGE0, 0x64, data);

    data = BB_ReadReg(PAGE0, 0x00);
    data |= 0x01;
    BB_WriteReg(PAGE0, 0x00, data);
    //page0 0x64[4] = 1'b0
    data &= 0xFE;
    BB_WriteReg(PAGE0, 0x00, data);

    //1.3: wait 1s
    SysTicks_DelayMS(1000);

    //select the 2G,  Read RF calibration register values
    data = BB_ReadReg(PAGE0, 0x64);
    BB_WriteReg(PAGE0, 0x64, (data&0x7F));
    read_cali_register(cali_reg[0]);

    //select the 5G,  Read RF calibration register values
    BB_WriteReg(PAGE0, 0x64, (data | 0x80));
    read_cali_register(cali_reg[1]);
}


void BB_RF_2G_5G_switch(ENUM_RF_BAND rf_band)
{
    uint8_t data, data1;
    char *regbuf = ((rf_band==RF_2G) ? cali_reg[0]:cali_reg[1]); 

    //write 0xd0[7] -> 0x67[7]
    data = (BB_ReadReg(PAGE0, 0x67) & 0x7f) | (regbuf[0] & 0x80);
    BB_WriteReg(PAGE0, 0x67, data);

    //write 0xd0[5:2] -> 0x68[7:4]
    data1 = ((regbuf[0] & 0x3c) << 2);
    data = (BB_ReadReg(PAGE0, 0x68) & 0x0f) | data1;
    BB_WriteReg(PAGE0, 0x68, data);

    //write 0xd1[7] -> 0x67[6]
    data = (BB_ReadReg(PAGE0, 0x67) & 0xbf) |  ((regbuf[1] & 0x80) >> 1);
    BB_WriteReg(PAGE0, 0x67, data);

    //write 0xd1[5:2] -> 0x68[3:0]
    data1 = ((regbuf[1] & 0x3c) >> 2);
    data = (BB_ReadReg(PAGE0, 0x68) & 0xf0) | data1;
    BB_WriteReg(PAGE0, 0x68, data);
    
    //write 0xd4[7] -> 0x67[5]
    data = (BB_ReadReg(PAGE0, 0x67) &  0xdf) | ((regbuf[4] & 0x80) >> 2);
    BB_WriteReg(PAGE0, 0x67, data);

    //write 0xd4[6:3] -> 0x6a[7:4]
    data1 = ((regbuf[4] & 0x78) << 1);
    data = (BB_ReadReg(PAGE0, 0x6A) & 0x0f) | data1;
    BB_WriteReg(PAGE0, 0x6A, data);
    
    //0xd3[3:0]->0x6a[3:0]
    data = (BB_ReadReg(PAGE0, 0x6A) & 0xf0) | (regbuf[3] & 0x0f);;
    BB_WriteReg(PAGE0, 0x6A, data);
    
    //0xd2[7:0] -> 0x6b[7:0]
    BB_WriteReg(PAGE0, 0x6b, regbuf[2]);
    
    uint16_t tmp = (((uint16_t)regbuf[3] & 0x0f) << 8 ) | regbuf[2]; //abs(tmp[11:0])
    if(tmp & 0x800) //tmp[11]
    {
        tmp = (~tmp) + 1;
        tmp &= 0x0fff;
    }

    typedef struct _STRU_thresh_regvalue
    {
        uint16_t thresh;
        uint8_t value;
    }STRU_thresh_regvalue;

    STRU_thresh_regvalue thresh_regvalue[] = 
    {
        {0x41, 0XFF}, {0x60, 0XFE}, {0x70, 0XFD}, {0x80, 0XFC}, 
        {0x8F, 0XFB}, {0x9F, 0XFA}, {0xAF, 0XF8}, {0xBE, 0XF7}, 
        {0xCE, 0XF6}, {0xDD, 0XF4}, {0xED, 0XF2}, {0xFD, 0XF0}, 
        {0x10C, 0XEE}, {0x11C, 0XEC}, {0x12B, 0XEA}, {0x13B, 0XE7}, 
        {0x14A, 0XE5}, {0x15A, 0XE2}, {0x169, 0XE0}, {0x179, 0XDD}, 
        {0x188, 0XDA}, {0x198, 0XD7}, {0x1A7, 0XD4}, {0x1B6, 0XD0},
        {0x1C6, 0XCD}, {0x1D5, 0XC9},
    };

    uint8_t regvalue = 0xc6;
    uint8_t idx = 0;
    for(idx = 0; idx < sizeof(thresh_regvalue)/sizeof(thresh_regvalue[0]); idx++)
    {
        if(tmp <= thresh_regvalue[idx].thresh)
        {
            regvalue = thresh_regvalue[idx].value;
            break;
        }
    }

    BB_WriteReg(PAGE0, 0x69, regvalue);

    //write 0xd5[7] -> 0x67[3]
    data1 = (regbuf[5] & 0x80) >> 4;
    data = BB_ReadReg(PAGE0, 0x67) & 0xf7 | data1;
    BB_WriteReg(PAGE0, 0x67, data);
    
    //0xd5[5:2]->0x67[7:4]
    data1 = (regbuf[5] & 0x3c) << 2;
    data = (BB_ReadReg(PAGE0, 0x6c) & 0x0f) | data1;
    BB_WriteReg(PAGE0, 0x6c, data);

    //write 0xd6[7] -> 0x67[2]
    data1 = (regbuf[6] & 0x80) >> 5;
    data = (BB_ReadReg(PAGE0, 0x67) & 0xfb) | data1;
    BB_WriteReg(PAGE0, 0x67, data);

    //0xd6[5:2]->0x6c[3:0]
    data1 = (regbuf[6] >> 2) & 0x0f ;
    data = (BB_ReadReg(PAGE0, 0x6c) & 0xf0) | data1;
    BB_WriteReg(PAGE0, 0x6c, data);

    //write 0xd9[7] -> 0x67[1]
    data1 = (regbuf[9] & 0x80) >> 6;
    data = (BB_ReadReg(PAGE0, 0x67) & 0xfd) | data1;
    BB_WriteReg(PAGE0, 0x67, data);
    
    //write 0xd9[6:3] -> 0x6E[7:4]
    data1 = (regbuf[9]<<1) & 0xf0;
    data = (BB_ReadReg(PAGE0, 0x6e) & 0x0f) | data1;
    BB_WriteReg(PAGE0, 0x6e, data); 

    //0xd8[3:0] -> 0x6E[3:0]
    data1 = regbuf[8] & 0x0f;
    data = (BB_ReadReg(PAGE0, 0x6e) & 0xf0) | data1;
    BB_WriteReg(PAGE0, 0x6e, data);

    //0xd7[7:0] -> 0x6F[7:0]
    BB_WriteReg(PAGE0, 0x6f, regbuf[7]);

    tmp = (((uint16_t)regbuf[8] & 0x0f)<<8) | regbuf[7];
    if(tmp & 0x800) //tmp[11]
    {
        tmp = (~tmp) + 1;
        tmp &= 0x0fff;
    }

    regvalue = 0xc6;
    for(idx = 0; idx < sizeof(thresh_regvalue)/sizeof(thresh_regvalue[0]); idx++)
    {
        if(tmp <= thresh_regvalue[idx].thresh)
        {
            regvalue = thresh_regvalue[idx].value;
            break;
        }
    }

    BB_WriteReg(PAGE0, 0x6d, regvalue);

    BB_WriteReg(PAGE0, TX_CALI_ENABLE, 0X00);
    BB_WriteReg(PAGE0, TX_CALI_ENABLE, 0X02);

    data = BB_ReadReg(PAGE0, 0x00);
    data |= 0x01;
    BB_WriteReg(PAGE0, 0x00, data);
    
    data &= 0xfe;
    BB_WriteReg(PAGE0, 0x00, data);

    uint8_t test[10];
    test[0] = BB_ReadReg(PAGE0, 0x64);
    test[1] = BB_ReadReg(PAGE0, 0x67);
    test[2] = BB_ReadReg(PAGE0, 0x68);
    test[3] = BB_ReadReg(PAGE0, 0x69);
    test[4] = BB_ReadReg(PAGE0, 0x6a);
    test[5] = BB_ReadReg(PAGE0, 0x6b);
    test[6] = BB_ReadReg(PAGE0, 0x6c);
    test[7] = BB_ReadReg(PAGE0, 0x6d);
    test[8] = BB_ReadReg(PAGE0, 0x6e);
    test[9] = BB_ReadReg(PAGE0, 0x6f);
    
    dlog_info("Before cali: %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x\r\n", 
               test[0], test[1], test[2], test[3], test[4],
               test[5], test[6], test[7], test[8], test[9]);        
}


typedef struct _STRU_grd_cmds
{
    uint8_t avail;                      /*command is using*/
    STRU_WIRELESS_CONFIG_CHANGE config;
}STRU_grd_cmds;


static STRU_grd_cmds grd_cmds_poll[16];

/*
 * BB_Getcmd: get command from command buffer pool and free the buffer
*/
int BB_GetCmd(STRU_WIRELESS_CONFIG_CHANGE *pconfig)
{
    uint8_t found = 0;
    uint8_t i;
    for(i = 0; i < sizeof(grd_cmds_poll)/sizeof(grd_cmds_poll[0]); i++)
    {
        if(grd_cmds_poll[i].avail == 1)
        {
            memcpy(pconfig, &(grd_cmds_poll[i].config), sizeof(*pconfig));
            grd_cmds_poll[i].avail = 0;
            found = 1;
            break;
        }
    }

    return (found) ? TRUE:FALSE;
}

int BB_InsertCmd(STRU_WIRELESS_CONFIG_CHANGE *p)
{
    uint8_t i;
    uint8_t found = 0;
    STRU_WIRELESS_CONFIG_CHANGE *pcmd = (STRU_WIRELESS_CONFIG_CHANGE *)p;

    dlog_info("Insert Message: %d %d %d\r\n", pcmd->configClass, pcmd->configItem, pcmd->configValue);
    for(i = 0; i < sizeof(grd_cmds_poll)/sizeof(grd_cmds_poll[0]); i++)
    {
        if(grd_cmds_poll[i].avail == 0)
        {
            memcpy((void *)(&grd_cmds_poll[i].config), p, sizeof(grd_cmds_poll[0]));
            grd_cmds_poll[i].avail = 1;
            found = 1;
            break;
        }
    }

    if(!found)
    {
        dlog_error("ERROR:Insert Event");
    }

    return (found? TRUE:FALSE);
}


int BB_add_cmds(uint8_t type, uint32_t param0, uint32_t param1, uint32_t param2)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;
    int ret = 1;
    switch(type)
    {
        case 0:
        {        
            cmd.configClass  = WIRELESS_FREQ_CHANGE;
            cmd.configItem   = FREQ_BAND_WIDTH_SELECT;
            cmd.configValue  = param0;
            break;
        }

        case 1:
        {
            cmd.configClass  = WIRELESS_FREQ_CHANGE;
            cmd.configItem   = FREQ_BAND_MODE;
            cmd.configValue  = param0;
            break;            
        }
        
        case 2:
        {
            cmd.configClass  = WIRELESS_FREQ_CHANGE;
            cmd.configItem   = FREQ_BAND_SELECT;
            cmd.configValue  = param0;
            break;
        }

        case 3:
        {
            cmd.configClass  = WIRELESS_FREQ_CHANGE;
            cmd.configItem   = FREQ_CHANNEL_MODE;
            cmd.configValue  = param0;
            break;
        }
    
        case 4:
        {
            cmd.configClass  = WIRELESS_FREQ_CHANGE;
            cmd.configItem   = FREQ_CHANNEL_SELECT;
            cmd.configValue  = param0;
            break;
        }

        case 5:        
        {
            cmd.configClass  = WIRELESS_MCS_CHANGE;
            cmd.configItem   = MCS_MODE_SELECT;
            cmd.configValue  = param0;
            break;
        }

        case 6:
        {
            cmd.configClass  = WIRELESS_MCS_CHANGE;
            cmd.configItem   = MCS_MODULATION_SELECT;
            cmd.configValue  = param0;
            break;            
        }

        case 7:
        {
            cmd.configClass  = WIRELESS_MCS_CHANGE;
            cmd.configItem   = MCS_CODE_RATE_SELECT;
            cmd.configValue  = param0;
            break;
        }

        case 8:
        {
            cmd.configClass  = WIRELESS_ENCODER_CHANGE;
            cmd.configItem   = ENCODER_DYNAMIC_BIT_RATE_MODE;
            cmd.configValue  = param0;
            break;
        }

        case 9:
        {
            cmd.configClass  = WIRELESS_ENCODER_CHANGE;
            cmd.configItem   = ENCODER_DYNAMIC_BIT_RATE_SELECT;
            cmd.configValue  = param0;
            break;
        }

        case 10:
        {
            cmd.configClass  = WIRELESS_ENCODER_CHANGE;
            cmd.configItem   = ENCODER_DYNAMIC_BIT_RATE_SELECT;
            cmd.configValue  = param0;
            break;
        }

        case 11:
        {
            cmd.configClass  = WIRELESS_MISC;
            cmd.configItem   = MISC_READ_RF_REG;
            cmd.configValue  = param0;
            
            dlog_info("1:%d 2:%d 3:%d 4:%d", type, param0, param1, param2);
            break;
        }

        case 12:
        {
            cmd.configClass  = WIRELESS_MISC;
            cmd.configItem   = MISC_WRITE_RF_REG;
                               //addr, value
            cmd.configValue  = (param0) | (param1 << 8);
            break;
        }

        case 13:
        {
            cmd.configClass  = WIRELESS_MISC;
            cmd.configItem   = MISC_READ_BB_REG;
                               //page, addr
            cmd.configValue  = param0 | (param1 << 8);
            break;
        }
        
        case 14:
        {
            cmd.configClass  = WIRELESS_MISC;
            cmd.configItem   = MISC_WRITE_BB_REG;
                               //page, addr, value
            cmd.configValue  = (param0) | (param1<<8) | (param2<<16);
            break;
        }

        default:
        {
            ret = 0;
            break;
        }
    }

    if(ret)
    {
       ret = BB_InsertCmd( &cmd);
    }

    return ret;
}


void BB_HandleEventsCallback(void *p)
{
    int ret = BB_InsertCmd( (STRU_WIRELESS_CONFIG_CHANGE * )p);
}


void BB_handle_misc_cmds(STRU_WIRELESS_CONFIG_CHANGE* pcmd)
{
    uint8_t class = pcmd->configClass;
    uint8_t item  = pcmd->configItem;
    
    uint8_t value  = (uint8_t)(pcmd->configValue);
    uint8_t value1 = (uint8_t)(pcmd->configValue >> 8);
    uint8_t value2 = (uint8_t)(pcmd->configValue >> 16);
    uint8_t value3 = (uint8_t)(pcmd->configValue >> 24);

    if(class == WIRELESS_MISC)
    {
        switch(item)
        {
            case MISC_READ_RF_REG:
            {
                uint8_t v;
                RF8003s_SPI_ReadReg(value * 2, &v);
                dlog_info("RF read addr=0x%0.2x value=0x%0.2x", value, v);
                break;
            }

            case MISC_WRITE_RF_REG:
            {
                RF8003s_SPI_WriteReg(value* 2, value1);
                dlog_info("RF write addr=0x%0.2x value=0x%0.2x", value, value1);
                break;
            }

            case MISC_READ_BB_REG:
            {
                uint8_t v = BB_ReadReg( (ENUM_REG_PAGES)value, (uint8_t)value1);
                dlog_info("BB read PAGE=0x%0.2x addr=0x%0.2x value=0x%0.2x", value, value1, v);
                break;
            }

            case MISC_WRITE_BB_REG:
            {
                BB_WriteReg((ENUM_REG_PAGES)value, (uint8_t)value1, (uint8_t)value2);
                dlog_info("BB write PAGE=0x%0.2x addr=0x%0.2x value=0x%0.2x", value, value1, value2);
                break;
            }
        }
    }
}

////////////////// handlers for WIRELESS_FREQ_CHANGE //////////////////

/** 
 * @brief       API for set channel Bandwidth 10M/20M, the function can only be called by cpu2
 * @param[in]   en_bw: channel bandwidth setting 10M/20M
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetFreqBandwidthSelection(ENUM_CH_BW en_bw)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;
    
    cmd.configClass  = WIRELESS_FREQ_CHANGE;
    cmd.configItem   = FREQ_BAND_WIDTH_SELECT;
    cmd.configValue  = (uint32_t)en_bw;

    return BB_InsertCmd(&cmd);
}



/** 
 * @brief       API for set frequency band (2G/5G) selection mode (ATUO / Manual), the function can only be called by cpu2
 * @param[in]   mode: selection mode (ATUO / Manual)
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetFreqBandSelectionMode(RUN_MODE en_mode)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;
    
    cmd.configClass  = WIRELESS_FREQ_CHANGE;
    cmd.configItem   = FREQ_BAND_MODE;
    cmd.configValue  = (uint32_t)en_mode;

    return BB_InsertCmd(&cmd);
}



/** 
 * @brief       API for set frequency band (2G/5G), the function can only be called by cpu2
 * @param[in]   mode: selection mode (ATUO / Manual)
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetFreqBand(ENUM_RF_BAND band)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;
    
    cmd.configClass  = WIRELESS_FREQ_CHANGE;
    cmd.configItem   = FREQ_BAND_SELECT;
    cmd.configValue  = (uint32_t)band;

    return BB_InsertCmd(&cmd);
}



/** 
 * @brief       API for set IT(image transmit) channel selection RUN mode(AUTO/Manual). the function can only be called by cpu2
 * @param[in]   qam: the modulation QAM mode for image transmit.
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetITChannelSelectionMode(RUN_MODE en_mode)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;
    
    cmd.configClass  = WIRELESS_FREQ_CHANGE;
    cmd.configItem   = FREQ_CHANNEL_MODE;
    cmd.configValue  = (uint32_t)en_mode;

    return BB_InsertCmd(&cmd);
}


/** 
 * @brief       API for set IT(image transmit) channel Number. the function can only be called by cpu2
 * @param[in]   channelNum: the current channel number int current Frequency band(2G/5G)
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetITChannel(uint8_t channelNum)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;
    
    cmd.configClass  = WIRELESS_FREQ_CHANGE;
    cmd.configItem   = FREQ_CHANNEL_SELECT;
    cmd.configValue  = channelNum;

    return BB_InsertCmd(&cmd);
}



////////////////// handlers for WIRELESS_MCS_CHANGE //////////////////

/** 
 * @brief       API for set MCS(modulation, coderate scheme) mode, the function can only be called by cpu2
 * @param[in]   mode: auto or manual selection.
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetMCSmode(RUN_MODE en_mode)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;
    
    cmd.configClass  = WIRELESS_MCS_CHANGE;
    cmd.configItem   = MCS_MODE_SELECT;
    cmd.configValue  = (uint32_t)en_mode;

    return BB_InsertCmd(&cmd);
}


/** 
 * @brief       API for set the image transmit QAM mode, the function can only be called by cpu2
 * @param[in]   qam: modulation qam mode
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetITQAM(ENUM_BB_QAM qam)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;
    
    cmd.configClass  = WIRELESS_MCS_CHANGE;
    cmd.configItem   = MCS_MODULATION_SELECT;
    cmd.configValue  = (uint32_t)qam;

    return BB_InsertCmd(&cmd);
}


/** 
 * @brief       API for set the image transmit LDPC coderate, the function can only be called by cpu2
 * @param[in]   ldpc:  ldpc coderate 
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetITLDPC(ENUM_BB_LDPC ldpc)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;

    cmd.configClass  = WIRELESS_MCS_CHANGE;
    cmd.configItem   = MCS_CODE_RATE_SELECT;
    cmd.configValue  = (uint32_t)ldpc;

    return BB_InsertCmd(&cmd);
}


////////////////// handlers for WIRELESS_ENCODER_CHANGE //////////////////

/** 
 * @brief       API for set the encoder bitrate control mode, the function can only be called by cpu2
 * @param[in]   mode: auto or manual selection.
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetEncoderBrcMode(RUN_MODE en_mode)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;

    cmd.configClass  = WIRELESS_ENCODER_CHANGE;
    cmd.configItem   = ENCODER_DYNAMIC_BIT_RATE_MODE;
    cmd.configValue  = (uint32_t)en_mode;

    return BB_InsertCmd(&cmd);
}


/** 
 * @brief       API for set the encoder bitrate Unit:Mbps, the function can only be called by cpu2
 * @param[in]   bitrate_Mbps: select the bitrate unit: Mbps
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetEncoderBitrate(uint8_t bitrate_Mbps)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;

    cmd.configClass  = WIRELESS_ENCODER_CHANGE;
    cmd.configItem   = ENCODER_DYNAMIC_BIT_RATE_SELECT;
    cmd.configValue  = (uint32_t)bitrate_Mbps;

    return BB_InsertCmd(&cmd);
}
