#include <stdint.h>
#include "debuglog.h"
#include "BB_spi.h"
#include "BB_ctrl.h"
#include "BB_init_regs.h"
#include "BB_uart_com.h"
#include "reg_rw.h"
#include "sys_param.h"
#include "config_baseband_register.h"

#define     BB_SPI_TEST         (0)
#define     RF_SPI_TEST         (0)

#define     VSOC_GLOBAL2_BASE   (0xA0030000)
#define     BB_SPI_UART_SEL     (0x9c)


CONTEXT context;

typedef struct
{
    ENUM_BB_MODE   en_curMode;
    ENUM_REG_PAGES en_curPage;
    ENUM_TRX_CTRL  en_TRXctrl;
    ENUM_VID_PATH  en_VIDPath;
}STRU_BB_ctrl_ctx;


static STRU_BB_ctrl_ctx BB_ctx = {
    .en_curPage = PAGE_UNKNOW,
    .en_curMode = BB_MODE_UNKNOWN,
    .en_TRXctrl = BB_RESET_UNKNOWN,
};


const STRU_RC_FRQ_CHANNEL Rc_frq[MAX_2G_RC_FRQ_SIZE] = {     // 2.4G
    { 0,0x00,0x00,0x00,0x4b }, { 1,0x00,0x00,0x00,0x4c },
    { 2,0x00,0x00,0x00,0x4d }, { 3,0x00,0x00,0x00,0x4e },
    { 4,0x00,0x00,0x00,0x4f }, { 5,0x00,0x00,0x00,0x50 },
};


const STRU_IT_FRQ_CHANNEL It_frq[MAX_2G_RC_FRQ_SIZE] = {     //2.4G
    { 0,0x00,0x00,0x00,0x4b }, { 1,0x00,0x00,0x00,0x4c },
    { 2,0x00,0x00,0x00,0x4d }, { 3,0x00,0x00,0x00,0x4e },
    { 4,0x00,0x00,0x00,0x4f }, { 5,0x00,0x00,0x00,0x50 },
};


const STRU_RC_FRQ_CHANNEL Rc_5G_frq[MAX_5G_RC_FRQ_SIZE] = {     // 5G
    { 0,0x00,0x00,0x00,0x5F }, { 1,0x00,0x00,0x00,0x60 },
    { 2,0x00,0x00,0x00,0x61 }, { 3,0x00,0x00,0x00,0x62 },
};


const STRU_IT_FRQ_CHANNEL It_5G_frq[MAX_5G_RC_FRQ_SIZE] = {     //5G
    { 0,0x00,0x00,0x00,0x5F }, { 1,0x00,0x00,0x00,0x60 },
    { 2,0x00,0x00,0x00,0x61 }, { 3,0x00,0x00,0x00,0x62 },
};

static int BB_RF_start_cali();
/*
  * cali_reg: Store the calibration registers value
 */
static uint8_t cali_reg[2][10] = {{0}, {0}};

static void BB_regs_init(ENUM_BB_MODE en_mode)
{
    uint32_t page_cnt=0;
    BB_ctx.en_curMode = en_mode;
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
        BB_ctx.en_curPage = page;

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


int BB_softReset(ENUM_RST_MODE en_mode)
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

    BB_ctx.en_curPage = PAGE2;
    return 0;
}



int BB_selectVideoPath(ENUM_VID_PATH path)
{
    if (BB_ctx.en_VIDPath != path)
    {
        uint8_t dat = BB_SPI_ReadByte(PAGE1, 0x8d) | 0xC0;
        BB_SPI_WriteByte(PAGE1, 0x8d, dat); 

        BB_ctx.en_VIDPath = path;
    }
    return 0;
}

int BB_TRX_ctrl(ENUM_TRX_CTRL en_trx)
{
    if(BB_ctx.en_TRXctrl != en_trx)
    {
        if( BB_ctx.en_TRXctrl == IT_ONLY_MODE)
        {
            BB_SPI_WriteByte(PAGE2, 0x02, 0x06);
            if(BB_ctx.en_curMode == BB_GRD_MODE)
            {
                uint8_t dat = BB_SPI_ReadByte(PAGE2, 0x20) & 0xF7;
                BB_SPI_WriteByte(PAGE2, 0x20, dat);       
            }
        }
    }    

    return 0;
}

/**
  * @brief  start / stop Ground recevie.
  * @param  en_rcv ref@ENUM_RCV_enable
  * @retval None
  */
int BB_GrdReceive(ENUM_RCV_enable en_rcv)
{
    uint8_t dat = BB_SPI_ReadByte(PAGE2, 0x20);

    dat = (en_rcv == GRD_RCV_ENABLE) ? (dat&0xF7) : (dat|0x08);
    BB_SPI_WriteByte(PAGE2, 0x20, dat);
    
    return 0;
}

/**
  * @brief : Write 8003 RF register by SPI 
  * @param : addr: 8003 SPI address
  * @param : data: data for 8003
  * @retval None
  */
static int SPI_Write8003(uint8_t addr, uint8_t data)
{
    uint8_t wdata[] = {0x80, addr, data};   //RF_8003S_SPI: wr: 0x80 ; 
    return SPI_write_read(BB_SPI_BASE_IDX, wdata, sizeof(wdata), 0, 0);  
}

static int SPI_Read8003(uint8_t addr)
{
    uint8_t wdata[3] = {0x0, addr, addr}; //RF_8003S_SPI:  rd: 0x00
    uint8_t rdata[3] = {0};
    SPI_write_read(BB_SPI_BASE_IDX, wdata, sizeof(wdata), rdata, 3); 
    return rdata[2];
}


void RF_8003x_spi_init(ENUM_BB_MODE mode)
{
    uint8_t *rf_setting = (mode==BB_SKY_MODE) ? RF_8003s_SKY_regs : RF_8003s_GRD_regs;
    uint8_t size = (mode==BB_SKY_MODE) ? sizeof(RF_8003s_SKY_regs) : sizeof(RF_8003s_GRD_regs);
    
    uint8_t idx;
    for(idx=0; idx < size; idx++)
    {
        SPI_Write8003(idx *2, rf_setting[idx]);

        #if (RF_SPI_TEST ==1)
        uint8_t data = SPI_Read8003(idx*2);
        dlog_info("%d %d \n", idx, data);
        #endif
    }
    
    {
        //add patch, reset 8003
        SPI_Write8003(0x15 *2, 0x51);
        SPI_Write8003(0x15 *2, 0x50);
    }
}

void BB_init(STRU_BB_initType *ptr_initType)
{
    BB_SPI_init();
    
    #if (BB_SPI_TEST==1)
    uint8_t i = 0, j =0;
    uint8_t test_addr[] = {0x20, 0x40, 0x80};
    uint8_t err_flag = 0;
    
    for(j = 0; j < sizeof(test_addr); j++)
    {
        for(i = 0 ; i < 254; i++)
        {
            BB_SPI_WriteByte(PAGE0, test_addr[j], i);
            char data = BB_SPI_ReadByte(PAGE0, test_addr[j]);
            dlog_info("%d %d \n", i, data);
        }
    }
    #endif
    
    BB_regs_init(ptr_initType->en_mode);

    // RF 8003 init
    {
        BB_SPI_curPageWriteByte(0x01,0x01);     //SPI change into 8003
        RF_8003x_spi_init(ptr_initType->en_mode);
        BB_SPI_curPageWriteByte(0x01,0x02);     //SPI change into 8020
    }
    BB_softReset(ptr_initType->en_mode);

    //RF calibration in both sky& Ground.
    //BB_RF_cali(RF_2G);
    //BB_RF_2G_5G_switch(RF_2G);
    
    BB_set_RF_Band(ptr_initType->en_mode, ptr_initType->en_rf_band);

    BB_set_sweepfrq(ptr_initType->en_rf_band, 0);
    BB_set_ITfrq(ptr_initType->en_rf_band, 0);
    BB_set_Rcfrq(ptr_initType->en_rf_band, 0);
    BB_softReset(ptr_initType->en_mode);
    context.RF_band = ptr_initType->en_rf_band;

    dlog_info("BB mode Band %d %d %s \r\n", ptr_initType->en_mode, ptr_initType->en_rf_band, "BB_init Done");
}


void BB_uart10_spi_sel(uint32_t sel_dat)
{
    write_reg32( (uint32_t *)(VSOC_GLOBAL2_BASE + BB_SPI_UART_SEL),	sel_dat);
}


uint8_t BB_WriteReg(ENUM_REG_PAGES page, uint8_t addr, uint8_t data)
{
    if(BB_ctx.en_curPage != page)
    {
        BB_SPI_WriteByte(page, addr, data);
        BB_ctx.en_curPage = page;
    }
    else
    {
        BB_SPI_curPageWriteByte(addr, data);
    }
}

uint8_t BB_ReadReg(ENUM_REG_PAGES page, uint8_t addr)
{
    uint8_t reg;
    if(BB_ctx.en_curPage != page)
    {
        reg = BB_SPI_ReadByte(page, addr);
        BB_ctx.en_curPage = page;
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
	STRU_RC_FRQ_CHANNEL *ch_ptr = (STRU_RC_FRQ_CHANNEL *)((band == RF_2G)?It_frq:It_5G_frq);

	BB_WriteReg(PAGE2, SWEEP_FREQ_0, ch_ptr[ch].frq1);
	BB_WriteReg(PAGE2, SWEEP_FREQ_1, ch_ptr[ch].frq2);
	BB_WriteReg(PAGE2, SWEEP_FREQ_2, ch_ptr[ch].frq3);
	BB_WriteReg(PAGE2, SWEEP_FREQ_3, ch_ptr[ch].frq4);
   
}

uint8_t BB_set_ITfrq(ENUM_RF_BAND band, uint8_t ch)
{
	STRU_RC_FRQ_CHANNEL *it_ch_ptr = (STRU_RC_FRQ_CHANNEL *)((band == RF_2G)?It_frq:It_5G_frq);

	BB_WriteReg(PAGE2, AGC3_0, it_ch_ptr[ch].frq1);
	BB_WriteReg(PAGE2, AGC3_1, it_ch_ptr[ch].frq2);
	BB_WriteReg(PAGE2, AGC3_2, it_ch_ptr[ch].frq3);
	BB_WriteReg(PAGE2, AGC3_3, it_ch_ptr[ch].frq4);

}

uint8_t BB_set_Rcfrq(ENUM_RF_BAND band, uint8_t ch)
{
	STRU_RC_FRQ_CHANNEL *rc_ch_ptr = (STRU_RC_FRQ_CHANNEL *)((band == RF_2G)?Rc_frq:Rc_5G_frq);

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
    ((MOD_BPSK<<6)  | (BW_10M <<3)  | LDPC_1_2),  10, //encoder br:1M
    ((MOD_4QAM<<6)  | (BW_10M <<3)  | LDPC_1_2),  30, //encoder br:3M
    ((MOD_4QAM<<6)  | (BW_10M <<3)  | LDPC_2_3),  40, //encoder br:4M
    ((MOD_16QAM<<6) | (BW_10M <<3)  | LDPC_1_2),  50, //encoder br:5M 
    ((MOD_64QAM<<6) | (BW_10M <<3)  | LDPC_1_2),  60, //encoder br:6M
    ((MOD_64QAM<<6) | (BW_10M <<3)  | LDPC_2_3),  70, //encoder br:7M
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
    //BB_RF_start_cali();
    BB_RF_2G_5G_switch(rf_band);
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

    dlog_info("cali Registers1: %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x\r\n", 
               buf[0], buf[1], buf[2], buf[3], buf[4],
               buf[5], buf[6], buf[7], buf[8], buf[9]);

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
    
    dlog_info("Before cali: %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x\r\n", 
               test[0], test[1], test[2], test[3], test[4],
               test[5], test[6], test[7], test[8], test[9]);    
}


static int BB_RF_start_cali()
{
#if 0
    uint8_t data;
    uint8_t *regbuf;

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
#endif
}


void BB_RF_2G_5G_switch(ENUM_RF_BAND rf_band)
{
#if 0
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
    if(tmp <= 0x41)
    {
        BB_WriteReg(PAGE0, 0x69, 0XFF);
    }
    else if(tmp <= 0x60)
    {
        BB_WriteReg(PAGE0, 0x69, 0XFE);
    }
    else if(tmp <= 0x70)
    {
        BB_WriteReg(PAGE0, 0x69, 0XFD);
    }
    else if(tmp <= 0x80)
    {
        BB_WriteReg(PAGE0, 0x69, 0XFC);
    }
    else if(tmp <= 0x8F)
    {
        BB_WriteReg(PAGE0, 0x69, 0XFB);
    }
    else if(tmp <= 0x9F)
    {
        BB_WriteReg(PAGE0, 0x69, 0XFA);
    }
    else if(tmp <= 0xAF)
    {
        BB_WriteReg(PAGE0, 0x69, 0XF8);
    }
    else if(tmp <= 0xBE)
    {
        BB_WriteReg(PAGE0, 0x69, 0XF7);
    }
    else if(tmp <= 0xCE)
    {
        BB_WriteReg(PAGE0, 0x69, 0XF6);
    }
    else if(tmp <= 0xDD)
    {
        BB_WriteReg(PAGE0, 0x69, 0XF4);
    }
    else if(tmp <= 0xED)
    {
        BB_WriteReg(PAGE0, 0x69, 0XF2);
    }
    else if(tmp <= 0xFD)
    {
        BB_WriteReg(PAGE0, 0x69, 0XF0);
    }
    else if(tmp <= 0x10C)
    {
        BB_WriteReg(PAGE0, 0x69, 0XEE);
    }
    else if(tmp <= 0x11C)
    {
        BB_WriteReg(PAGE0, 0x69, 0XEC);
    }
    else if(tmp <= 0x12B)
    {
        BB_WriteReg(PAGE0, 0x69, 0XEA);
    }
    else if(tmp <= 0x13B)
    {
        BB_WriteReg(PAGE0, 0x69, 0XE7);
    } 
    else if(tmp <= 0x14A)
    {
        BB_WriteReg(PAGE0, 0x69, 0XE5);
    }
    else if(tmp <= 0x15A)
    {
        BB_WriteReg(PAGE0, 0x69, 0XE2);
    } 
    else if(tmp <= 0x169)
    {
        BB_WriteReg(PAGE0, 0x69, 0XE0);
    }
    else if(tmp <= 0x179)
    {
        BB_WriteReg(PAGE0, 0x69, 0XDD);
    }
    else if(tmp <= 0x188)
    {
        BB_WriteReg(PAGE0, 0x69, 0XDA);
    }
    else if(tmp <= 0x198)
    {
        BB_WriteReg(PAGE0, 0x69, 0XD7);
    }
    else if(tmp <= 0x1A7)
    {
        BB_WriteReg(PAGE0, 0x69, 0XD4);
    }
    else if(tmp <= 0x1B6)
    {
        BB_WriteReg(PAGE0, 0x69, 0XD0);
    }
    else if(tmp <= 0x1C6)
    {
        BB_WriteReg(PAGE0, 0x69, 0XCD);
    }
    else if(tmp <= 0x1D5)
    {
        BB_WriteReg(PAGE0, 0x69, 0XC9);
    }
    else
    {
        BB_WriteReg(PAGE0, 0x69, 0XC6);
    }

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
    if(tmp <= 0x41)
    {
        BB_WriteReg(PAGE0, 0x6D, 0XFF);
    }
    else if(tmp <= 0x60)
    {
        BB_WriteReg(PAGE0, 0x6D, 0XFE);
    }
    else if(tmp <= 0x70)
    {
        BB_WriteReg(PAGE0, 0x6D, 0XFD);
    }
    else if(tmp <= 0x80)
    {
        BB_WriteReg(PAGE0, 0x6D, 0XFC);
    }
    else if(tmp <= 0x8F)
    {
        BB_WriteReg(PAGE0, 0x6D, 0XFB);
    }
    else if(tmp <= 0x9F)
    {
        BB_WriteReg(PAGE0, 0x6D, 0XFA);
    }
    else if(tmp <= 0xAF)
    {
        BB_WriteReg(PAGE0, 0x6D, 0XF8);
    }
    else if(tmp <= 0xBE)
    {
        BB_WriteReg(PAGE0, 0x6D, 0XF7);
    }
    else if(tmp <= 0xCE)
    {
        BB_WriteReg(PAGE0, 0x6D, 0XF6);
    }
    else if(tmp <= 0xDD)
    {
        BB_WriteReg(PAGE0, 0x6D, 0XF4);
    }
    else if(tmp <= 0xED)
    {
        BB_WriteReg(PAGE0, 0x6D, 0XF2);
    }
    else if(tmp <= 0xFD)
    {
        BB_WriteReg(PAGE0, 0x6D, 0XF0);
    }
    else if(tmp <= 0x10C)
    {
        BB_WriteReg(PAGE0, 0x6D, 0XEE);
    }
    else if(tmp <= 0x11C)
    {
        BB_WriteReg(PAGE0, 0x6D, 0XEC);
    }
    else if(tmp <= 0x12B)
    {
        BB_WriteReg(PAGE0, 0x6D, 0XEA);
    }
    else if(tmp <= 0x13B)
    {
        BB_WriteReg(PAGE0, 0x6D, 0XE7);
    } 
    else if(tmp <= 0x14A)
    {
        BB_WriteReg(PAGE0, 0x6D, 0XE5);
    }
    else if(tmp <= 0x15A)
    {
        BB_WriteReg(PAGE0, 0x6D, 0XE2);
    } 
    else if(tmp <= 0x169)
    {
        BB_WriteReg(PAGE0, 0x6D, 0XE0);
    }
    else if(tmp <= 0x179)
    {
        BB_WriteReg(PAGE0, 0x6D, 0XDD);
    }
    else if(tmp <= 0x188)
    {
        BB_WriteReg(PAGE0, 0x6D, 0XDA);
    }
    else if(tmp <= 0x198)
    {
        BB_WriteReg(PAGE0, 0x6D, 0XD7);
    }
    else if(tmp <= 0x1A7)
    {
        BB_WriteReg(PAGE0, 0x6D, 0XD4);
    }
    else if(tmp <= 0x1B6)
    {
        BB_WriteReg(PAGE0, 0x6D, 0XD0);
    }
    else if(tmp <= 0x1C6)
    {
        BB_WriteReg(PAGE0, 0x6D, 0XCD);
    }
    else if(tmp <= 0x1D5)
    {
        BB_WriteReg(PAGE0, 0x6D, 0XC9);
    }
    else
    {
        BB_WriteReg(PAGE0, 0x6D, 0XC6);
    }

    BB_WriteReg(PAGE0, TX_CALI_ENABLE, 0X00);
    BB_WriteReg(PAGE0, TX_CALI_ENABLE, 0X02);

    data = BB_ReadReg(PAGE0, 0x00);
    data |= 0x01;
    BB_WriteReg(PAGE0, 0x00, data);
    
    data &= 0xfe;
    BB_WriteReg(PAGE0, 0x00, data);  

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
    
    dlog_info("after cali: %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x %.2x\r\n", 
               test[0], test[1], test[2], test[3], test[4],
               test[5], test[6], test[7], test[8], test[9]);        
#endif            
}
