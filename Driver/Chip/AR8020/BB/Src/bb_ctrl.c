#include <stdint.h>
#include <string.h>
#include "debuglog.h"
#include "bb_spi.h"
#include "bb_ctrl_internal.h"
#include "bb_grd_sweep.h"
#include "bb_grd_ctrl.h"
#include "bb_sky_ctrl.h"
#include "bb_uart_com.h"
#include "reg_rw.h"
#include "systicks.h"
#include "bb_regs.h"
#include "sys_event.h"
#include "rf_8003s.h"
#include "memory_config.h"
#include "boardParameters.h"

#define     BB_SPI_TEST         (0)
#define     RF_SPI_TEST         (0)

#define     VSOC_GLOBAL2_BASE   (0xA0030000)
#define     BB_SPI_UART_SEL     (0x9c)

volatile CONTEXT context;
static volatile ENUM_REG_PAGES en_curPage;


STRU_FRQ_CHANNEL *Rc_2G_frq    = NULL;
STRU_FRQ_CHANNEL *Sweep_2G_10m_frq = NULL;
STRU_FRQ_CHANNEL *Sweep_2G_20m_frq = NULL;
STRU_FRQ_CHANNEL *It_2G_frq    = NULL;

STRU_FRQ_CHANNEL *Rc_5G_frq = NULL;
STRU_FRQ_CHANNEL *Sweep_5G_10m_frq = NULL;
STRU_FRQ_CHANNEL *Sweep_5G_20m_frq = NULL;
STRU_FRQ_CHANNEL *It_5G_frq = NULL;


/*
  * cali_reg: Store the calibration registers value
 */
static uint8_t cali_reg[2][10];
static uint8_t *BB_sky_regs = NULL;
static uint8_t *BB_grd_regs = NULL;
static uint8_t *RF1_8003s_regs = NULL;
static uint8_t *RF2_8003s_regs = NULL;

static void BB_GetNv(void);

static int BB_before_RF_cali(void);

static void BB_after_RF_cali(ENUM_BB_MODE en_mode, STRU_BoardCfg *boardCfg);

static void BB_RF_start_cali( void );

static void BB_regs_init(ENUM_BB_MODE en_mode, STRU_BoardCfg *pstru_boardCfg)
{
    uint32_t page_cnt=0;    
    uint8_t *regs = (en_mode == BB_SKY_MODE) ? BB_sky_regs : BB_grd_regs;
    
    for(page_cnt = 0 ; page_cnt < 4; page_cnt ++)
    {
        uint32_t addr_cnt=0;
        ENUM_REG_PAGES page = (ENUM_REG_PAGES)(page_cnt << 6);
        /*
         * PAGE setting included in the regs array.
         */
        en_curPage = page;
        //update the board registers
        {
            uint8_t num;
            uint8_t cfgRegNum = ((en_mode == BB_SKY_MODE ) ? pstru_boardCfg->u8_bbSkyRegsCnt : pstru_boardCfg->u8_bbGrdRegsCnt);
            STRU_BB_REG *bbBoardReg  = ((en_mode == BB_SKY_MODE ) ? (STRU_BB_REG *)pstru_boardCfg->pstru_bbSkyRegs : 
                                                                    (STRU_BB_REG *)pstru_boardCfg->pstru_bbGrdRegs);

            for(num = 0; num < cfgRegNum; num++ )
            {
                regs[((uint16_t)bbBoardReg[num].page << 8) + bbBoardReg[num].addr] = bbBoardReg[num].value;
            }
        }

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
        BB_SPI_curPageWriteByte(0x00,0xB1);
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


void BB_use_param_setting(PARAM *user_setting)
{
    memcpy( (uint8_t *)((void *)(context.qam_threshold_range)),
            (uint8_t *)((void *)(user_setting->qam_change_threshold)),
            sizeof(context.qam_threshold_range));

    context.it_skip_freq_mode = user_setting->it_skip_freq_mode;
    context.rc_skip_freq_mode = user_setting->rc_skip_freq_mode;
    context.qam_skip_mode = user_setting->qam_skip_mode;


    context.CH_bandwidth      = BW_10M;

    context.e_rfbandMode      = AUTO;
    context.trx_ctrl          = IT_RC_MODE;
}


void BB_init(ENUM_BB_MODE en_mode, STRU_BoardCfg *boardCfg)
{
    PARAM *user_setting = BB_get_sys_param();
    BB_use_param_setting(user_setting);
    context.en_bbmode = en_mode;

    STRU_SettingConfigure* cfg_addr = NULL;
    GET_CONFIGURE_FROM_FLASH(cfg_addr);

    BB_sky_regs   = &(cfg_addr->bb_sky_configure[0][0]);
    BB_grd_regs   = &(cfg_addr->bb_grd_configure[0][0]);
    RF1_8003s_regs = &(cfg_addr->rf1_configure[0]);
    RF2_8003s_regs = &(cfg_addr->rf2_configure[0]);
    
    Rc_2G_frq    = (STRU_FRQ_CHANNEL *)(cfg_addr->RC_2_4G_frq);
    Sweep_2G_10m_frq = (STRU_FRQ_CHANNEL *)(cfg_addr->IT_2_4G_frq);
    Sweep_2G_20m_frq = (STRU_FRQ_CHANNEL *)(cfg_addr->IT_2_4G_20M_sweep_frq);
    It_2G_frq    = (STRU_FRQ_CHANNEL *)(cfg_addr->IT_2_4G_frq);

    Rc_5G_frq = (STRU_FRQ_CHANNEL *)(cfg_addr->RC_5G_frq);
    Sweep_5G_10m_frq = (STRU_FRQ_CHANNEL *)(cfg_addr->IT_5G_frq);
    Sweep_5G_20m_frq = (STRU_FRQ_CHANNEL *)(cfg_addr->IT_5G_20M_sweep_frq);
    It_5G_frq = (STRU_FRQ_CHANNEL *)(cfg_addr->IT_5G_frq);
    context.e_bandsupport = boardCfg->e_bandsupport;

    BB_GetNv();

    BB_uart10_spi_sel(0x00000003);
    BB_SPI_init();

    context.u8_bbStartMcs = ((context.CH_bandwidth == BW_20M) ? (boardCfg->u8_bbStartMcs20M) : (boardCfg->u8_bbStartMcs10M));

    BB_regs_init(en_mode, boardCfg);
    RF8003s_init(RF1_8003s_regs, RF2_8003s_regs, boardCfg, en_mode);

    BB_softReset(en_mode);

    BB_before_RF_cali();
    BB_RF_start_cali();

    BB_WriteReg(PAGE0, TX_CALI_ENABLE, 0x00);   //disable calibration

    BB_after_RF_cali(en_mode, boardCfg);
    RF8003s_afterCali(en_mode, boardCfg);

    //choose the start band
    if ( context.e_bandsupport == RF_2G_5G || context.e_bandsupport == RF_2G )
    {
        context.e_curBand = RF_2G;
    }
    else
    {
        context.e_curBand = RF_5G;
    }
    BB_set_RF_Band(en_mode, context.e_curBand);
    BB_set_RF_bandwitdh(en_mode, context.CH_bandwidth);
    BB_softReset(en_mode);

    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_USER_CFG_CHANGE, BB_HandleEventsCallback);
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


uint8_t BB_set_sweepfrq(ENUM_RF_BAND band, ENUM_CH_BW e_bw, uint8_t ch)
{
    STRU_FRQ_CHANNEL *ch_ptr;

    if (BW_10M == e_bw)
    {
        ch_ptr = ((band == RF_2G)?Sweep_2G_10m_frq:Sweep_5G_10m_frq);
    }
    else
    {
        ch_ptr = ((band == RF_2G)?Sweep_2G_20m_frq:Sweep_5G_20m_frq);
    }

    //set sweep frequency
    if ( band == RF_2G )
    {
        BB_WriteRegMask(PAGE2, 0x20, 0x00, 0x04);
    }
    else
    {
        // P2 0x20 [2]=1,sweep frequency,5G
        BB_WriteRegMask(PAGE2, 0x20, 0x04, 0x04);   
    }
    
    BB_WriteReg(PAGE2, SWEEP_FREQ_0, ch_ptr[ch].frq1);
    BB_WriteReg(PAGE2, SWEEP_FREQ_1, ch_ptr[ch].frq2);
    BB_WriteReg(PAGE2, SWEEP_FREQ_2, ch_ptr[ch].frq3);
    BB_WriteReg(PAGE2, SWEEP_FREQ_3, ch_ptr[ch].frq4);
}


void BB_grd_notify_it_skip_freq(ENUM_RF_BAND band, uint8_t u8_ch)
{
    STRU_FRQ_CHANNEL *pstru_frq = ((band == RF_2G)?It_2G_frq:It_5G_frq);

    BB_WriteReg(PAGE2, IT_FRQ_0, pstru_frq[u8_ch].frq1);
    BB_WriteReg(PAGE2, IT_FRQ_1, pstru_frq[u8_ch].frq2);
    BB_WriteReg(PAGE2, IT_FRQ_2, pstru_frq[u8_ch].frq3);
    BB_WriteReg(PAGE2, IT_FRQ_3, pstru_frq[u8_ch].frq4);
}


const uint8_t mcs_idx_bitrate_map_10m[] = 
{
    1,      //0.6Mbps BPSK 1/2
    2,      //1.2     BPSK 1/2
    3,      //2.4     QPSK 1/2
    8,      //5.0     16QAM 1/2
    11,     //7.5     64QAM 1/2
    13,     //10      64QAM 2/3
};

const uint8_t mcs_idx_bitrate_map_20m[] = 
{
    2,      //1.2Mbps BPSK 1/2
    3,      //2.4     BPSK 1/2
    8,      //5.0     QPSK 1/2
    11,      //7.5     16QAM 1/2
    13,     //10     16QAM 2/3
};

uint8_t BB_get_bitrateByMcs(ENUM_CH_BW bw, uint8_t u8_mcs)
{
    if (BW_10M == bw)
    {
        return mcs_idx_bitrate_map_10m[u8_mcs];
    }
    else
    {
        return mcs_idx_bitrate_map_20m[u8_mcs];
    }
}

void BB_grd_notify_it_skip_freq_1(void)
{
    BB_WriteReg(PAGE2, IT_FRQ_0, context.stru_itRegs.frq1);
    BB_WriteReg(PAGE2, IT_FRQ_1, context.stru_itRegs.frq2);
    BB_WriteReg(PAGE2, IT_FRQ_2, context.stru_itRegs.frq3);
    BB_WriteReg(PAGE2, IT_FRQ_3, context.stru_itRegs.frq4);
}


uint8_t BB_write_ItRegs(uint32_t u32_it)
{
    context.stru_itRegs.frq1 = (uint8_t)(u32_it >> 24) & 0xff;
    context.stru_itRegs.frq2 = (uint8_t)(u32_it >> 16) & 0xff;
    context.stru_itRegs.frq3 = (uint8_t)(u32_it >>  8) & 0xff;
    context.stru_itRegs.frq4 = (uint8_t)(u32_it) & 0xff;

    BB_WriteReg(PAGE2, AGC3_0, context.stru_itRegs.frq1);
    BB_WriteReg(PAGE2, AGC3_1, context.stru_itRegs.frq2);
    BB_WriteReg(PAGE2, AGC3_2, context.stru_itRegs.frq3);
    BB_WriteReg(PAGE2, AGC3_3, context.stru_itRegs.frq4);
}



uint8_t BB_set_ITfrq(ENUM_RF_BAND band, uint8_t ch)
{
	STRU_FRQ_CHANNEL *it_ch_ptr = ((band == RF_2G)?It_2G_frq:It_5G_frq);

    context.stru_itRegs.frq1 = it_ch_ptr[ch].frq1;
    context.stru_itRegs.frq2 = it_ch_ptr[ch].frq2;
    context.stru_itRegs.frq3 = it_ch_ptr[ch].frq3;
    context.stru_itRegs.frq4 = it_ch_ptr[ch].frq4;

	BB_WriteReg(PAGE2, AGC3_0, it_ch_ptr[ch].frq1);
	BB_WriteReg(PAGE2, AGC3_1, it_ch_ptr[ch].frq2);
	BB_WriteReg(PAGE2, AGC3_2, it_ch_ptr[ch].frq3);
	BB_WriteReg(PAGE2, AGC3_3, it_ch_ptr[ch].frq4);    
}

uint8_t BB_write_RcRegs(uint32_t u32_rc)
{
    context.stru_rcRegs.frq1 = (uint8_t)(u32_rc >> 24) & 0xff;
    context.stru_rcRegs.frq2 = (uint8_t)(u32_rc >> 16) & 0xff;
    context.stru_rcRegs.frq3 = (uint8_t)(u32_rc >>  8) & 0xff;
    context.stru_rcRegs.frq4 = (uint8_t)(u32_rc) & 0xff;

    BB_WriteReg(PAGE2, AGC3_a, context.stru_rcRegs.frq1);
    BB_WriteReg(PAGE2, AGC3_b, context.stru_rcRegs.frq2);
    BB_WriteReg(PAGE2, AGC3_c, context.stru_rcRegs.frq3);
    BB_WriteReg(PAGE2, AGC3_d, context.stru_rcRegs.frq4);
}


uint8_t BB_set_Rcfrq(ENUM_RF_BAND band, uint8_t ch)
{
	STRU_FRQ_CHANNEL *pu8_rcRegs = ((band == RF_2G)?Rc_2G_frq:Rc_5G_frq);

    context.stru_rcRegs.frq1 = pu8_rcRegs[ch].frq1;
    context.stru_rcRegs.frq2 = pu8_rcRegs[ch].frq2;
    context.stru_rcRegs.frq3 = pu8_rcRegs[ch].frq3;
    context.stru_rcRegs.frq4 = pu8_rcRegs[ch].frq4;

    BB_WriteReg(PAGE2, AGC3_a, pu8_rcRegs[ch].frq1);
    BB_WriteReg(PAGE2, AGC3_b, pu8_rcRegs[ch].frq2);
    BB_WriteReg(PAGE2, AGC3_c, pu8_rcRegs[ch].frq3);
    BB_WriteReg(PAGE2, AGC3_d, pu8_rcRegs[ch].frq4); 
}


void BB_set_QAM(ENUM_BB_QAM mod)
{
    uint8_t data = BB_ReadReg(PAGE2, TX_2);
    BB_WriteReg(PAGE2, TX_2, (data & 0x3f) | ((uint8_t)mod << 6));
}

void BB_set_LDPC(ENUM_BB_LDPC ldpc)
{
    uint8_t data = BB_ReadReg(PAGE2, TX_2);
    BB_WriteReg(PAGE2, TX_2, (data & 0xF8) | (uint8_t)ldpc);
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
        BB_WriteRegMask(PAGE0, 0x20, 0x08, 0x0c);
        BB_WriteRegMask(PAGE2, 0x21, 0x00, 0x90);
        BB_WriteRegMask(PAGE2, 0x20, 0x00, 0x04);
    }
    else
    {
        // P0 0x20 [3]=0, [2]=1,2G PA off,5G PA on
        BB_WriteRegMask(PAGE0, 0x20, 0x04, 0x0C); 
        // P2 0x21 [7]=1, [4]=1,rf_freq_sel_tx,rf_freq_sel_rx,5G
        BB_WriteRegMask(PAGE2, 0x21, 0x90, 0x90); 
        // P2 0x20 [2]=1,sweep frequency,5G
        BB_WriteRegMask(PAGE2, 0x20, 0x04, 0x04);

        //softreset
        //BB_softReset(sky_ground);

    }

    //calibration and reset
    BB_RF_2G_5G_switch(rf_band);
}


/*
 * set RF bandwidth = 10M or 20M
*/
void BB_set_RF_bandwitdh(ENUM_BB_MODE sky_ground, ENUM_CH_BW rf_bw)
{
    if (sky_ground == BB_SKY_MODE)
    {
        BB_WriteRegMask(PAGE2, TX_2, (rf_bw << 3), 0x38); /*bit[5:3]*/
        if (BW_20M == (context.CH_bandwidth))
        {
            BB_WriteRegMask(PAGE2, 0x05, 0x80, 0xC0);
        }
    }
    else
    {
        BB_WriteRegMask(PAGE2, RX_MODULATION, (rf_bw << 0), 0x07); /*bit[2:0]*/
    }
   
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
}


static int BB_before_RF_cali(void)
{
    BB_WriteRegMask(PAGE0, 0x20, 0x00, 0x0c);
}


static void BB_after_RF_cali(ENUM_BB_MODE en_mode, STRU_BoardCfg *boardCfg)
{
    //BB_WriteRegMask(PAGE0, 0x20, 0x80, 0x80);
    // enalbe RXTX
    //BB_WriteRegMask(PAGE1, 0x94, 0x10, 0xFF);    //remove to fix usb problem

    STRU_BB_REG * bb_regs;
    uint8_t bb_regcnt;
    uint8_t cnt;
    
    if( NULL == boardCfg)
    {
        return;
    }

    if (en_mode == BB_SKY_MODE)
    {
        bb_regcnt = boardCfg->u8_bbSkyRegsCntAfterCali;
        bb_regs   = (STRU_BB_REG * )boardCfg->pstru_bbSkyRegsAfterCali;
    }
    else
    {
        bb_regcnt = boardCfg->u8_bbGrdRegsCntAfterCali;
        bb_regs   = (STRU_BB_REG * )boardCfg->pstru_bbGrdRegsAfterCali;
    }

    if ( bb_regcnt > 0 && NULL != bb_regs )
    {
        for(cnt = 0; cnt < bb_regcnt; cnt ++)
        {
            ENUM_REG_PAGES page = (ENUM_REG_PAGES )(bb_regs[cnt].page << 6);
            BB_WriteReg(page, bb_regs[cnt].addr, bb_regs[cnt].value);
        }
    }
}

static void BB_RF_start_cali( void )
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
        {0x41,  0xFF}, {0x60,  0xFE}, {0x70,  0xFD}, {0x80,  0xFC}, 
        {0x8F,  0xFB}, {0x9F,  0xFA}, {0xAF,  0xF8}, {0xBE,  0xF7}, 
        {0xCE,  0xF6}, {0xDD,  0xF4}, {0xED,  0xF2}, {0xFD,  0xF0}, 
        {0x10C, 0xEE}, {0x11C, 0xEC}, {0x12B, 0xEA}, {0x13B, 0xE7}, 
        {0x14A, 0xE5}, {0x15A, 0xE2}, {0x169, 0xE0}, {0x179, 0xDD}, 
        {0x188, 0xDA}, {0x198, 0xD7}, {0x1A7, 0xD4}, {0x1B6, 0xD0},
        {0x1C6, 0xCD}, {0x1D5, 0xC9},
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
    BB_WriteRegMask(PAGE0, 0x60, 0x02, 0x02);   //fix calibration result.
#if 0
    data = BB_ReadReg(PAGE0, 0x00);
    data |= 0x01;
    BB_WriteReg(PAGE0, 0x00, data);
    
    data &= 0xfe;
    BB_WriteReg(PAGE0, 0x00, data);  
#endif

    BB_WriteReg(PAGE2, RF_BAND_CHANGE_0, rf_band);
    BB_WriteReg(PAGE2, RF_BAND_CHANGE_1, rf_band);
    
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

    dlog_info("Insert Message: %d %d %d\r\n", pcmd->u8_configClass, pcmd->u8_configItem, pcmd->u32_configValue);
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
            cmd.u8_configClass  = WIRELESS_FREQ_CHANGE;
            cmd.u8_configItem   = FREQ_BAND_WIDTH_SELECT;
            cmd.u32_configValue  = param0;
            break;
        }

        case 1:
        {
            cmd.u8_configClass  = WIRELESS_FREQ_CHANGE;
            cmd.u8_configItem   = FREQ_BAND_MODE;
            cmd.u32_configValue  = param0;
            break;            
        }
        
        case 2:
        {
            cmd.u8_configClass  = WIRELESS_FREQ_CHANGE;
            cmd.u8_configItem   = FREQ_BAND_SELECT;
            cmd.u32_configValue  = param0;
            break;
        }

        case 3:
        {
            cmd.u8_configClass  = WIRELESS_FREQ_CHANGE;
            cmd.u8_configItem   = FREQ_CHANNEL_MODE;
            cmd.u32_configValue  = param0;
            break;
        }
    
        case 4:
        {
            cmd.u8_configClass  = WIRELESS_FREQ_CHANGE;
            cmd.u8_configItem   = FREQ_CHANNEL_SELECT;
            cmd.u32_configValue  = param0;
            break;
        }

        case 5:        
        {
            cmd.u8_configClass  = WIRELESS_MCS_CHANGE;
            cmd.u8_configItem   = MCS_MODE_SELECT;
            cmd.u32_configValue  = param0;
            break;
        }

        case 6:
        {
            cmd.u8_configClass  = WIRELESS_MCS_CHANGE;
            cmd.u8_configItem   = MCS_MODULATION_SELECT;
            cmd.u32_configValue  = param0;
            break;            
        }

        case 7:
        {
            cmd.u8_configClass  = WIRELESS_MCS_CHANGE;
            cmd.u8_configItem   = MCS_CODE_RATE_SELECT;
            cmd.u32_configValue  = param0;
            break;
        }

        case 8:
        {
            cmd.u8_configClass  = WIRELESS_ENCODER_CHANGE;
            cmd.u8_configItem   = ENCODER_DYNAMIC_BIT_RATE_MODE;
            cmd.u32_configValue  = param0;
            break;
        }

        case 9:
        {
            cmd.u8_configClass  = WIRELESS_ENCODER_CHANGE;
            cmd.u8_configItem   = ENCODER_DYNAMIC_BIT_RATE_SELECT_CH1;
            cmd.u32_configValue  = param0;
            break;
        }

        case 10:
        {
            cmd.u8_configClass  = WIRELESS_ENCODER_CHANGE;
            cmd.u8_configItem   = ENCODER_DYNAMIC_BIT_RATE_SELECT_CH2;
            cmd.u32_configValue = param0;
            break;
        }

        case 11:
        {
            cmd.u8_configClass  = WIRELESS_MISC;
            cmd.u8_configItem   = MISC_READ_RF_REG;
            cmd.u32_configValue = (param0) | (param1 << 8);

            dlog_info("1:%d 2:%d 3:%d 4:%d", type, param0, param1, param2);
            break;
        }

        case 12:
        {
            cmd.u8_configClass  = WIRELESS_MISC;
            cmd.u8_configItem   = MISC_WRITE_RF_REG;
                               //8003s num: addr: value
            cmd.u32_configValue  = (param0) | (param1 << 8) | (param2 << 16);
            break;
        }

        case 13:
        {
            cmd.u8_configClass  = WIRELESS_MISC;
            cmd.u8_configItem   = MISC_READ_BB_REG;
                               //page, addr
            cmd.u32_configValue  = param0 | (param1 << 8);
            break;
        }
        
        case 14:
        {
            cmd.u8_configClass  = WIRELESS_MISC;
            cmd.u8_configItem   = MISC_WRITE_BB_REG;
                               //page, addr, value
            cmd.u32_configValue  = (param0) | (param1<<8) | (param2<<16);
            break;
        }

        case 15:
        {
            cmd.u8_configClass  = WIRELESS_DEBUG_CHANGE;
            cmd.u8_configItem   = 1;
            break;
        }

        case 16:
        {
            cmd.u8_configClass  = WIRELESS_AUTO_SEARCH_ID;
            cmd.u8_configItem   = 0;
            break;
        }
        
        case 17:
        {
            cmd.u8_configClass  = WIRELESS_MCS_CHANGE;
            cmd.u8_configItem   = MCS_IT_QAM_SELECT;
            cmd.u32_configValue  = (param0);
            break;
        }
        
        case 18:
        {
            cmd.u8_configClass  = WIRELESS_MCS_CHANGE;
            cmd.u8_configItem   = MCS_IT_CODE_RATE_SELECT;
            cmd.u32_configValue  = (param0);
            break;
        }

        case 19:
        {
            cmd.u8_configClass  = WIRELESS_MCS_CHANGE;
            cmd.u8_configItem   = MCS_RC_QAM_SELECT;
            cmd.u32_configValue  = (param0);
            break;
        }
        
        case 20:
        {
            cmd.u8_configClass  = WIRELESS_MCS_CHANGE;
            cmd.u8_configItem   = MCS_RC_CODE_RATE_SELECT;
            cmd.u32_configValue  = (param0);
            break;
        }
        
        case 21:
        {
            cmd.u8_configClass  = WIRELESS_OTHER;
            cmd.u8_configItem   = CALC_DIST_ZERO_CALI;
            cmd.u32_configValue  = (param0);
            break;
        }

        case 22:
        {
            cmd.u8_configClass  = WIRELESS_OTHER;
            cmd.u8_configItem   = SET_CALC_DIST_ZERO_POINT;
            cmd.u32_configValue  = (param0);
            break;
        }

        case 23:
        {
            cmd.u8_configClass  = WIRELESS_OTHER;
            cmd.u8_configItem   = SET_RC_FRQ_MASK;
            cmd.u32_configValue  = (param0);
            break;
        }
        
        case 24:
        {
            cmd.u8_configClass  = WIRELESS_FREQ_CHANGE;
            cmd.u8_configItem   = IT_CHANNEL_FREQ;
            cmd.u32_configValue  = (param0);
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
    STRU_WIRELESS_CONFIG_CHANGE* pcmd = (STRU_WIRELESS_CONFIG_CHANGE* )p;
    uint8_t  class  = pcmd->u8_configClass;
    uint8_t  item   = pcmd->u8_configItem;
    uint32_t value  = pcmd->u32_configValue;

    if( class == WIRELESS_DEBUG_CHANGE && item == 0 && (value == 0 || value == 1))
    {    
        uint8_t u8_debugMode = ((value == 0) ? TRUE:FALSE);

        if( context.u8_debugMode != u8_debugMode )
        {
            context.u8_flagdebugRequest = u8_debugMode | 0x80;
            BB_SPI_curPageWriteByte(0x01, 0x02);
            en_curPage = (BB_SPI_curPageReadByte(0x0) & 0xc0);
        }
        dlog_info("Event Debug: %d \n", u8_debugMode);         
    }
    else
    {
        int ret = BB_InsertCmd( (STRU_WIRELESS_CONFIG_CHANGE * )p);
    }
}


void BB_handle_misc_cmds(STRU_WIRELESS_CONFIG_CHANGE* pcmd)
{
    uint8_t class = pcmd->u8_configClass;
    uint8_t item  = pcmd->u8_configItem;

    uint8_t value  = (uint8_t)(pcmd->u32_configValue);
    uint8_t value1 = (uint8_t)(pcmd->u32_configValue >> 8);
    uint8_t value2 = (uint8_t)(pcmd->u32_configValue >> 16);
    uint8_t value3 = (uint8_t)(pcmd->u32_configValue >> 24);

    if(class == WIRELESS_MISC)
    {
        switch(item)
        {
            case MISC_READ_RF_REG:
            {
                uint8_t v;
                BB_SPI_curPageWriteByte(0x01, (value == 0)? 0x01 : 0x03);               //value2==0: write RF8003-0
                                                                                        //value2==1: write RF8003-1
                RF8003s_SPI_ReadReg(value1, &v);
                dlog_info("RF read 8003-%d addr=0x%0.2x value=0x%0.2x", value, value1, v);
                BB_SPI_curPageWriteByte(0x01,0x02);
                break;
            }

            case MISC_WRITE_RF_REG:
            {
                BB_SPI_curPageWriteByte(0x01, (value == 0)? 0x01 : 0x03);              //value2==0: write RF8003-0
                                                                                       //value2==1: write RF8003-1
                RF8003s_SPI_WriteReg(value1, value2);
                BB_SPI_curPageWriteByte(0x01,0x02);

                dlog_info("RF write 8003-%d addr=0x%0.2x value=0x%0.2x", value, value1, value2);
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

            case MICS_IT_ONLY_MODE:
            {
                BB_WriteReg(PAGE2, 0x02, 0x06);
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
    
    cmd.u8_configClass  = WIRELESS_FREQ_CHANGE;
    cmd.u8_configItem   = FREQ_BAND_WIDTH_SELECT;
    cmd.u32_configValue  = (uint32_t)en_bw;

    return BB_InsertCmd(&cmd);
}



/** 
 * @brief       API for set frequency band (2G/5G) selection mode (ATUO / Manual), the function can only be called by cpu2
 * @param[in]   mode: selection mode (ATUO / Manual)
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetFreqBandSelectionMode(ENUM_RUN_MODE en_mode)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;
    
    cmd.u8_configClass  = WIRELESS_FREQ_CHANGE;
    cmd.u8_configItem   = FREQ_BAND_MODE;
    cmd.u32_configValue  = (uint32_t)en_mode;

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
    
    cmd.u8_configClass  = WIRELESS_FREQ_CHANGE;
    cmd.u8_configItem   = FREQ_BAND_SELECT;
    cmd.u32_configValue  = (uint32_t)band;

    return BB_InsertCmd(&cmd);
}



/** 
 * @brief       API for set IT(image transmit) channel selection RUN mode(AUTO/Manual). the function can only be called by cpu2
 * @param[in]   qam: the modulation QAM mode for image transmit.
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetITChannelSelectionMode(ENUM_RUN_MODE en_mode)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;
    
    cmd.u8_configClass  = WIRELESS_FREQ_CHANGE;
    cmd.u8_configItem   = FREQ_CHANNEL_MODE;
    cmd.u32_configValue  = (uint32_t)en_mode;

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
    
    cmd.u8_configClass  = WIRELESS_FREQ_CHANGE;
    cmd.u8_configItem   = FREQ_CHANNEL_SELECT;
    cmd.u32_configValue  = channelNum;

    return BB_InsertCmd(&cmd);
}



////////////////// handlers for WIRELESS_MCS_CHANGE //////////////////

/** 
 * @brief       API for set MCS(modulation, coderate scheme) mode, the function can only be called by cpu2
 * @param[in]   mode: auto or manual selection.
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetMCSmode(ENUM_RUN_MODE en_mode)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;
    
    cmd.u8_configClass  = WIRELESS_MCS_CHANGE;
    cmd.u8_configItem   = MCS_MODE_SELECT;
    cmd.u32_configValue  = (uint32_t)en_mode;

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
    
    cmd.u8_configClass  = WIRELESS_MCS_CHANGE;
    cmd.u8_configItem   = MCS_MODULATION_SELECT;
    cmd.u32_configValue  = (uint32_t)qam;

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

    cmd.u8_configClass  = WIRELESS_MCS_CHANGE;
    cmd.u8_configItem   = MCS_CODE_RATE_SELECT;
    cmd.u32_configValue  = (uint32_t)ldpc;

    return BB_InsertCmd(&cmd);
}


////////////////// handlers for WIRELESS_ENCODER_CHANGE //////////////////

/** 
 * @brief       API for set the encoder bitrate control mode, the function can only be called by cpu2
 * @param[in]   mode: auto or manual selection.
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetEncoderBrcMode(ENUM_RUN_MODE en_mode)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;

    cmd.u8_configClass  = WIRELESS_ENCODER_CHANGE;
    cmd.u8_configItem   = ENCODER_DYNAMIC_BIT_RATE_MODE;
    cmd.u32_configValue  = (uint32_t)en_mode;

    return BB_InsertCmd(&cmd);
}


/** 
 * @brief       API for set the encoder bitrate Unit:Mbps, the function can only be called by cpu2
 * @param[in]   bitrate_Mbps: select the bitrate unit: Mbps
 * @retval      TURE:  success to add command
 * @retval      FALSE, Fail to add command
 */
int BB_SetEncoderBitrateCh1(uint8_t bitrate_Mbps)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;

    cmd.u8_configClass  = WIRELESS_ENCODER_CHANGE;
    cmd.u8_configItem   = ENCODER_DYNAMIC_BIT_RATE_SELECT_CH1;
    cmd.u32_configValue  = (uint32_t)bitrate_Mbps;

    return BB_InsertCmd(&cmd);
}

int BB_SetEncoderBitrateCh2(uint8_t bitrate_Mbps)
{
    STRU_WIRELESS_CONFIG_CHANGE cmd;

    cmd.u8_configClass  = WIRELESS_ENCODER_CHANGE;
    cmd.u8_configItem   = ENCODER_DYNAMIC_BIT_RATE_SELECT_CH2;
    cmd.u32_configValue  = (uint32_t)bitrate_Mbps;

    return BB_InsertCmd(&cmd);
}

static void BB_GetNv(void)
{
    volatile uint32_t tmpCnt = 0;
    volatile STRU_NV *pst_nv = (STRU_NV *)SRAM_NV_MEMORY_ST_ADDR;

    while(0x23178546 != (pst_nv->st_nvMng.u32_nvInitFlag))
    {
        tmpCnt = 0;
        while((tmpCnt++) < 200);
    }
    
    memcpy((uint8_t*)(context.u8_flashId), (void *)(pst_nv->st_nvDataUpd.u8_nvBbRcId), 5);
    context.u8_flashId[5] = pst_nv->st_nvDataUpd.u8_nvChk;

    if(TRUE != (pst_nv->st_nvMng.u8_nvVld)) // 
    {
        context.u8_idSrcSel = RC_ID_AUTO_SEARCH;
    }
    else // 
    {
        context.u8_idSrcSel = RC_ID_USE_FLASH_SAVE;
    }

    dlog_info("nv:0x%x 0x%x 0x%x 0x%x 0x%x 0x%x",
               pst_nv->st_nvDataUpd.u8_nvBbRcId[0],
               pst_nv->st_nvDataUpd.u8_nvBbRcId[1],
               pst_nv->st_nvDataUpd.u8_nvBbRcId[2],
               pst_nv->st_nvDataUpd.u8_nvBbRcId[3],
               pst_nv->st_nvDataUpd.u8_nvBbRcId[4], 
               pst_nv->st_nvDataUpd.u8_nvChk);
}

/** 
 * @brief       
 * @param   
 * @retval      
 * @note      
 */
int BB_GetDevInfo(void)
{ 
    uint8_t u8_data;
    STRU_DEVICE_INFO *pst_devInfo = (STRU_DEVICE_INFO *)(DEVICE_INFO_SHM_ADDR);

    pst_devInfo->messageId = 0x19;
    pst_devInfo->paramLen = sizeof(STRU_DEVICE_INFO);

    pst_devInfo->skyGround = context.en_bbmode;
    pst_devInfo->band = context.e_curBand;
    
    //pst_devInfo->bandWidth = context.CH_bandwidth;
    pst_devInfo->itHopping = context.it_skip_freq_mode;
    pst_devInfo->rcHopping = context.rc_skip_freq_mode;
    pst_devInfo->adapterBitrate = context.qam_skip_mode;
    u8_data = BB_ReadReg(PAGE1, 0x8D);
    pst_devInfo->channel1_on = (u8_data >> 6) & 0x01;
    pst_devInfo->channel2_on = (u8_data >> 7) & 0x01;
    pst_devInfo->isDebug = context.u8_debugMode;
    if (BB_GRD_MODE == context.en_bbmode )
    {
        u8_data = BB_ReadReg(PAGE2, GRD_FEC_QAM_CR_TLV);
        pst_devInfo->itQAM = u8_data & 0x03;
        pst_devInfo->itCodeRate  = ((u8_data >>2) & 0x07);
        u8_data = BB_ReadReg(PAGE2, RX_MODULATION);
        pst_devInfo->bandWidth = (u8_data >> 0) & 0x07;
       
        u8_data = BB_ReadReg(PAGE2, TX_2);
        pst_devInfo->rcQAM = (u8_data >> 6) & 0x01;
        pst_devInfo->rcCodeRate = (u8_data >> 0) & 0x01;
    }
    else
    {
        u8_data = BB_ReadReg(PAGE2, TX_2);
        pst_devInfo->itQAM = (u8_data >> 6) & 0x03;
        pst_devInfo->bandWidth = (u8_data >> 3) & 0x07;
        pst_devInfo->itCodeRate  = ((u8_data >> 0) & 0x07);
        
        u8_data = BB_ReadReg(PAGE2, 0x09);
        pst_devInfo->rcQAM = (u8_data >> 0) & 0x01;
        pst_devInfo->rcCodeRate = (u8_data >> 2) & 0x01;
    }

    if(context.brc_mode == AUTO)
    {
        pst_devInfo->ch1Bitrates = context.qam_ldpc;
        pst_devInfo->ch2Bitrates = context.qam_ldpc;
    }
    else
    {
        pst_devInfo->ch1Bitrates = context.brc_bps[0];
        pst_devInfo->ch2Bitrates = context.brc_bps[1];
    }

    pst_devInfo->u8_itRegs[0] = context.stru_itRegs.frq1;
    pst_devInfo->u8_itRegs[1] = context.stru_itRegs.frq2;
    pst_devInfo->u8_itRegs[2] = context.stru_itRegs.frq3;
    pst_devInfo->u8_itRegs[3] = context.stru_itRegs.frq4;

    pst_devInfo->u8_rcRegs[0] = context.stru_rcRegs.frq1;
    pst_devInfo->u8_rcRegs[1] = context.stru_rcRegs.frq2;
    pst_devInfo->u8_rcRegs[2] = context.stru_rcRegs.frq3;
    pst_devInfo->u8_rcRegs[3] = context.stru_rcRegs.frq4;   

    //pst_devInfo->u8_startWrite = 0;
    //pst_devInfo->u8_endWrite = 1;    
}

/** 
 * @brief       
 * @param   
 * @retval      
 * @note      
 */
int BB_SwtichOnOffCh(uint8_t u8_ch, uint8_t u8_data)
{
    uint8_t u8_regVal;

    u8_regVal = BB_ReadReg(PAGE1, 0x8D);
    if ((0 == u8_ch) && (0 == u8_data))
    {
        u8_regVal &= ~(0x40); // channel1 
        BB_WriteReg(PAGE1, 0x8D, u8_regVal); 
    }
    else if ((0 == u8_ch) && (1 == u8_data))
    {
        u8_regVal |= (0x40); // channel1 
        BB_WriteReg(PAGE1, 0x8D, u8_regVal); 
    }
    else if ((1 == u8_ch) && (0 == u8_data))
    {
        u8_regVal &= ~(0x80); // channel2 
        BB_WriteReg(PAGE1, 0x8D, u8_regVal); 
    }
    else if ((1 == u8_ch) && (1 == u8_data))
    {
        u8_regVal |= (0x80); // channel2 
        BB_WriteReg(PAGE1, 0x8D, u8_regVal); 
    }
    else
    {
    }
}

/** 
 * @brief       
 * @param   
 * @retval      
 * @note      
 */
int BB_WrSpiChkFlag(void)
{
    BB_WriteReg(PAGE2, SPI_CHK1, 0x55);
    BB_WriteReg(PAGE2, SPI_CHK2, 0xAA);

    return 0;
}

/** 
 * @brief       
 * @param   
 * @retval      
 * @note      
 */
int BB_ChkSpiFlag(void)
{
    if ((0x55 == BB_ReadReg(PAGE2, SPI_CHK1)) &&
        (0xAA == BB_ReadReg(PAGE2, SPI_CHK2)))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}


