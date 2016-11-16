#include <stdint.h>
#include "debuglog.h"
#include "BB_spi.h"
#include "BB_ctrl.h"
#include "BB_init_regs.h"
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
    ENUM_BB_MODE en_curMode;
    ENUM_REG_PAGES en_curPage;
    ENUM_TRX_CTRL en_TRXctrl;
    ENUM_VID_PATH en_VIDPath;
}STRU_BB_ctrl_ctx;


static STRU_BB_ctrl_ctx BB_ctx = {
    .en_curPage = PAGE_UNKNOW,
    .en_curMode = BB_MODE_UNKNOWN,
    .en_TRXctrl = BB_RESET_UNKNOWN,
};


const struct RC_FRQ_CHANNEL Rc_frq[MAX_RC_FRQ_SIZE] = {     // 2.4G
    { 0,0x00,0x00,0x00,0x4b }, { 1,0x00,0x00,0x00,0x4c },
    { 2,0x00,0x00,0x00,0x4d }, { 3,0x00,0x00,0x00,0x4e },
    { 4,0x00,0x00,0x00,0x4f }, { 5,0x00,0x00,0x00,0x50 },
    { 6,0x00,0x00,0x00,0x51 }, { 7,0x00,0x00,0x00,0x52 },
    { 8,0x00,0x00,0x00,0x53 }, { 9,0x00,0x00,0x00,0x54 },
    { 10,0x00,0x00,0x00,0x55}, { 11,0x00,0x00,0x00,0x56}
};


const struct IT_FRQ_CHANNEL It_frq[MAX_RC_FRQ_SIZE] = {     //2.4G
    { 0,0x00,0x00,0x00,0x4b }, { 1,0x00,0x00,0x00,0x4c },
    { 2,0x00,0x00,0x00,0x4d }, { 3,0x00,0x00,0x00,0x4e },
    { 4,0x00,0x00,0x00,0x4f }, { 5,0x00,0x00,0x00,0x50 },
    { 6,0x00,0x00,0x00,0x51 }, { 7,0x00,0x00,0x00,0x52 },
    { 8,0x00,0x00,0x00,0x53 }, { 9,0x00,0x00,0x00,0x54 },
    { 10,0x00,0x00,0x00,0x55}, { 11,0x00,0x00,0x00,0x56}
};


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
    if(en_mode == BB_GRD_MODE)
    {
        BB_SPI_curPageWriteByte(0x00,0xB2);
        BB_SPI_curPageWriteByte(0x00,0xB0);
		BB_ctx.en_curPage = PAGE2;
    }
    else
    {        
        BB_SPI_curPageWriteByte(0x00, 0x81);
        BB_SPI_curPageWriteByte(0x00, 0x80);
        
        //bug fix: write reset register may fail.
        int count = 0;
        while(1)
        {
            uint8_t rst = BB_SPI_curPageReadByte(0x00);
            if(rst != 0x80)
            {
                dlog_error("RST:%0.2x %d\r\n", rst, count);
                BB_SPI_curPageWriteByte(0x00, 0x80);
                count ++;
            }
            else
            {
                break;
            }
        }

        BB_ctx.en_curPage = PAGE2;
    }
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
    
    {
        BB_SPI_curPageWriteByte(0x01,0x01);     //SPI change into 8003
        RF_8003x_spi_init(ptr_initType->en_mode);
        BB_SPI_curPageWriteByte(0x01,0x02);     //SPI change into 8020
    }

    // reset 8020 wimax
    if (BB_SKY_MODE == ptr_initType->en_mode)
    {
        
        BB_SPI_WriteByte(PAGE2, 0x00, 0x81);
        BB_SPI_WriteByte(PAGE2, 0x00, 0x80);
    }
    else
    {
        BB_SPI_WriteByte(PAGE1,0x00,0xB1);
        BB_SPI_WriteByte(PAGE2,0x00,0xB0);
    }
    
    dlog_info("%s", "BB_init Done \n");
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

uint8_t BB_set_sweepfrq(uint8_t ch)
{
    if(ch < sizeof(It_frq) / sizeof(It_frq[0]))
    {
        BB_WriteReg(PAGE2, SWEEP_FREQ_0, It_frq[ch].frq1);
        BB_WriteReg(PAGE2, SWEEP_FREQ_1, It_frq[ch].frq2);
        BB_WriteReg(PAGE2, SWEEP_FREQ_2, It_frq[ch].frq3);
        BB_WriteReg(PAGE2, SWEEP_FREQ_3, It_frq[ch].frq4);
        return 0;
    }
    
    return 1;    
}

uint8_t BB_set_ITfrq(uint8_t ch)
{
    if(ch < sizeof(It_frq) / sizeof(It_frq[0]))
    {
        BB_WriteReg(PAGE2, AGC3_0, It_frq[ch].frq1);
        BB_WriteReg(PAGE2, AGC3_1, It_frq[ch].frq2);
        BB_WriteReg(PAGE2, AGC3_2, It_frq[ch].frq3);
        BB_WriteReg(PAGE2, AGC3_3, It_frq[ch].frq4);
        return 0;
    }
    return 1;
}

uint8_t BB_set_Rcfrq(uint8_t ch)
{
    if(ch < sizeof(Rc_frq) / sizeof(Rc_frq[0]))
    {
        BB_WriteReg(PAGE2, AGC3_a, Rc_frq[ch].frq1);
        BB_WriteReg(PAGE2, AGC3_b, Rc_frq[ch].frq2);
        BB_WriteReg(PAGE2, AGC3_c, Rc_frq[ch].frq3);
        BB_WriteReg(PAGE2, AGC3_d, Rc_frq[ch].frq4); 
        return 0;
    }
    return 1;    
}


void BB_set_QAM(EN_BB_QAM mod)
{
    uint8_t data = BB_ReadReg(PAGE2, TX_2);
    BB_WriteReg(PAGE2, TX_2, (data & 0x3f) | ((uint8_t)mod << 6));
}

EN_BB_QAM BB_get_QAM(void)
{
    return (EN_BB_QAM)(BB_ReadReg(PAGE2, TX_2) >> 6);
}

void BB_set_LDPC(EN_BB_LDPC ldpc)
{
    uint8_t data = BB_ReadReg(PAGE2, TX_2);
    BB_WriteReg(PAGE2, TX_2, (data & 0x07) | (uint8_t)ldpc);
}

EN_BB_LDPC BB_get_LDPC(void)
{
    return (EN_BB_LDPC)(BB_ReadReg(PAGE2, TX_2) & 0x07);
}


