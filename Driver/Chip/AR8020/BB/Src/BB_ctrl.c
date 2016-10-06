#include <stdint.h>
#include "debuglog.h"
#include "spi.h"
#include "BB_ctrl.h"
#include "BB_init_regs.h"
#include "reg_rw.h"

#define     BB_SPI_BASE_IDX     (SPI_7)
#define     CLKRATE_MHZ	        (9)             //9MHz SPI clkrate

#define     BB_TEST_UART        (9)
#define     BB_SPI_TEST         (0)
#define     RF_SPI_TEST         (0)

#define VSOC_GLOBAL2_BASE           0xA0030000
#define BB_SPI_UART_SEL             0x9c

#define _FPGA_BB_

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

static int BB_SPI_init(void)
{
    STRU_SPI_InitTypes init = {
        .ctrl0   = 0x47,        // [15:12]: Control Frame Size ;[11]: Shift Register Loop 0: Normal Mode, 1: test mode;[10] Slave Output Enable;
                                // [9:8]: Transfer_mode,2'b00:Trans and Receive;[7:6] SCPOL,SCPH (SPI Oper_Mode),2'b0: mode0 ; 2'b3: mode3 ;
                                // [5:4] FRF=2'b00: SPI Protocal; [3:0]=0x7:data frame size = 8bit; 
        .clk_Mhz = CLKRATE_MHZ, //
        .Tx_Fthr = 0x03,        // transmit FIFO Threshold Level <=3
        .Rx_Ftlr = 0x6,         // receive FIFO Threshold Level  >=6
        .SER     = 0x01         // slave enbale
    };

    SPI_master_init(BB_SPI_BASE_IDX, &init);
}

static int BB_SPI_curPageWriteByte(uint8_t addr, uint8_t data)
{
    uint8_t wdata[] = {0x0e, addr, data};
    return SPI_write_read(BB_SPI_BASE_IDX, wdata, sizeof(wdata), 0, 0); 
}

static uint8_t BB_SPI_curPageReadByte(uint8_t addr)
{
    uint8_t rbuf[3];
    uint8_t wdata[] = {0x0f, addr, addr};
    SPI_write_read(BB_SPI_BASE_IDX, wdata, sizeof(wdata), rbuf, 3);

    return rbuf[2];
}

int BB_SPI_WriteByte(ENUM_REG_PAGES page, uint8_t addr, uint8_t data)
{
    //if(BB_ctx.en_curPage != page)
    {
        uint8_t dat = BB_SPI_curPageReadByte(0x00);
        BB_SPI_curPageWriteByte(0x00, (dat & 0x3F) | page);

        BB_ctx.en_curPage = page;
    }

    return BB_SPI_curPageWriteByte(addr, data);        
}

uint8_t BB_SPI_ReadByte(ENUM_REG_PAGES page, uint8_t addr)
{
    //if(BB_ctx.en_curPage != page)
    {
        uint8_t dat = BB_SPI_curPageReadByte(0x00);
        BB_SPI_curPageWriteByte(0x00, (dat & 0x3F) | page);
    }
    
    return BB_SPI_curPageReadByte(addr);
}

int BB_SPI_WriteByteMask(ENUM_REG_PAGES page, uint8_t addr, uint8_t data, uint8_t mask)
{
    uint8_t ori = BB_SPI_ReadByte(page, addr);
    data = (ori & (~mask)) | data;
    return BB_SPI_curPageWriteByte(addr, data);   
}

int BB_SPI_ReadByteMask(ENUM_REG_PAGES page, uint8_t addr, uint8_t mask)
{
    return BB_SPI_ReadByte(page, addr) & mask;
}

static void BB_regs_init(ENUM_BB_MODE en_mode)
{
    uint32_t page_cnt=0;
    BB_ctx.en_curMode = en_mode;
    const uint8_t *regs = (en_mode == BB_SKY_MODE) ? (const uint8_t *)BB_sky_regs : (const uint8_t *)BB_grd_regs;
    dlog_info("%d %d \n", en_mode, regs);
    for(page_cnt = 0 ; page_cnt < 4; page_cnt ++)
    {
        uint32_t addr_cnt=0;
        
        ENUM_REG_PAGES page = (page_cnt==0)? PAGE0: \
                              ((page_cnt==1)?PAGE1: \
                              ((page_cnt==2)?PAGE2:PAGE3));

        for(addr_cnt = 0; addr_cnt < 256; addr_cnt++)
        {
            BB_SPI_WriteByte(page, (uint8_t)addr_cnt, *regs);
            regs++;
        }
	}
    
    printf("addr 0x0 = %d \n", BB_SPI_ReadByte(PAGE2, 0));
}


int BB_softReset(ENUM_RST_MODE en_mode)
{
    /*
     * todo: support different reset mode.
    */
    if(en_mode == BB_GRD_MODE)
    {
        #if 0
        #ifdef _FPGA_BB_
            BB_SPI_WriteByte(PAGE1,0x90,0x80);
            BB_SPI_WriteByte(PAGE1,0x90,0x00);
            
            BB_SPI_WriteByte(PAGE1,0x98,0x81);
            BB_SPI_WriteByte(PAGE1,0x98,0x01);
        #endif
        #endif
        
        BB_SPI_WriteByte(PAGE2,0x00,0x81);
        BB_SPI_WriteByte(PAGE2,0x00,0xB0);
    }
    else
    {        
        BB_SPI_WriteByte(PAGE2, 0x00, 0x81);
        BB_SPI_WriteByte(PAGE2, 0x00, 0x80);
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
    /*
     * Todo: add reg setting to support IT_RC_MODE.
    */
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
        
    #if 0
    if(ptr_initType->en_mode == BB_GRD_MODE)
    {
        BB_selectVideoPath(ptr_initType->en_vidPath);
    }
    #endif

    //BB_softReset(BB_RESET_TXRX);
    if (BB_SKY_MODE == ptr_initType->en_mode)
    {
        // reset 8020 wimax
        BB_SPI_WriteByte(PAGE2, 0x00, 0x81);
        BB_SPI_WriteByte(PAGE2, 0x00, 0x80);
    }
    else
    {
        //  reset 8020 wimax
        BB_SPI_WriteByte(PAGE1, 0x00, 0x40);
    
    #ifdef _FPGA_BB_
        BB_SPI_WriteByte(PAGE1, 0x90, 0x80);
        BB_SPI_WriteByte(PAGE1, 0x90, 0x00);
        BB_SPI_WriteByte(PAGE1, 0x98, 0x81);
        BB_SPI_WriteByte(PAGE1, 0x98, 0x01);
    #endif
        
        BB_SPI_WriteByte(PAGE1,0x00,0x81);
        BB_SPI_WriteByte(PAGE2,0x00,0xB0);

        {
            uint loop = 0;
            while(loop++ < 20000);
        }
        BB_SPI_WriteByte(PAGE2,0x20,0xd4);
    }

    #if 0
    if(ptr_initType->en_rcvEnable)
    {
        BB_GrdReceive(ptr_initType->en_rcvEnable);
    }
    #endif
    
    printf("reg %d %d\n", BB_SPI_ReadByte(PAGE2, 0x00), BB_SPI_ReadByte(PAGE2, 0x01));
}


void BB_uart10_spi_sel(uint32_t sel_dat)
{
    write_reg32( (uint32_t *)(VSOC_GLOBAL2_BASE + BB_SPI_UART_SEL),	sel_dat);
}
