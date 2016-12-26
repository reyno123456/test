#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debuglog.h"
#include "interrupt.h"
#include "bb_ctrl.h"
#include "bb_uart_com.h"
#include "bb_spi_com.h"

#define STATIC_TEST (0)

uint8_t g_BbSpiWriteFlag = 0;
uint8_t g_BbSpiReadFlag = 0;

static BB_SPI_Opr g_stTestBbSpiWrite = {
	
	.u8_arCnt = {0,~0}, 
	.u8_checkSum = 0,
	};

static BB_SPI_Opr g_stTestBbSpiRead = {
	
	.u8_arCnt = {0,~0}, 
	.u8_checkSum = 0,
	};


void test_BB_sky(void)
{
    ENUM_BB_MODE cur_mode = BB_SKY_MODE;
    char *log = "IN BB sky mode \n";
 
    BB_init(cur_mode );
    BB_UARTComInit();
    
    #if (STATIC_TEST==0) //normal mode.
    #else
        BB_debug_print_init_sky();
    #endif
    dlog_info("%s", log);
}

void test_BB_grd(void)
{
    ENUM_BB_MODE cur_mode = BB_GRD_MODE;
    char *log = "IN BB  Ground mode \r\n";

    BB_init(cur_mode);
	BB_UARTComInit();
    
    #if (STATIC_TEST==0) //normal mode.
    #else
        BB_debug_print_init_grd();
    #endif
    
    dlog_info("%s", log);
}





void command_test_BB_uart(char *index_str)
{
    static uint8_t data_buf_proc[128];

    unsigned char opt = strtoul(index_str, NULL, 0);

    if (opt == 0)
    {
        BB_UARTComRegisterSession(BB_UART_COM_SESSION_0);
    }
    else if (opt == 1)
    {
        uint8_t data_buf_tmp[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22};
        BB_UARTComSendMsg(BB_UART_COM_SESSION_0, data_buf_tmp, sizeof(data_buf_tmp));
    }
    else if (opt == 2)
    {
        uint32_t cnt = BB_UARTComReceiveMsg(BB_UART_COM_SESSION_0, data_buf_proc, sizeof(data_buf_proc));
        uint32_t i = 0;
        for(i = 0; i < cnt; i++)
        {
            dlog_info("%d,", data_buf_proc[i]);
        }
    }
}

void command_TestBbSpiWrite(void)
{
	static uint8_t u8_num = 1;

	uint32_t u32_len = sizeof(BB_SPI_Opr);
	uint8_t * p_addr = (uint8_t *)(&g_stTestBbSpiWrite);

	memset(p_addr+3, u8_num++,u32_len-3);

	BB_SPI_Write(&g_stTestBbSpiWrite);
	dlog_info("have writed");

}



void command_TestBbSpiRead(void)
{
	uint8_t * p_addr;
	
	if(0 == BB_SPI_Read(&g_stTestBbSpiRead))
	{
		p_addr = (uint8_t *)(&g_stTestBbSpiRead);
		
		dlog_info("cnt:%d checksum:%d data:%d",
						g_stTestBbSpiRead.u8_arCnt[0],
						g_stTestBbSpiRead.u8_checkSum,
						p_addr[4]);
	}
	else
	{
		dlog_info("no new data or check error");	
	}	
}



#if 0
#include "bb_spi.h"
#include "timer.h"
#include "bb_sys_param.h"
#include <math.h>
#include "bb_sky_ctrl.h"
#include "bb_grd_ctrl.h"



/*
 * this Function for demo only...
*/
static int open_video_path = 0;

void TIM0_BB_Grd_handler(uint32_t u32_vectorNum)
{
    uint8_t print_reg[][2] =  {
                            {PAGE2, 0xcc}, {PAGE2, 0xd0}, {PAGE2, 0xd7}, {PAGE2, 0xd8},
                            {PAGE2, 0xdd}, {PAGE2, 0xde}, {PAGE2, 0xdf}, 
                            {PAGE2, 0xa0}, {PAGE2, 0xa1},                   //AGC:
                            {PAGE2, 0xda}
                        };
    
    int idx = 0;
    Reg_Read32(BASE_ADDR_TIMER0 + TMRNEOI_0);
    
    for(idx = 0; idx < sizeof(print_reg) / sizeof(print_reg[0]); idx ++)
    {
        printf("%0.2x %0.2x\n", print_reg[idx][1], BB_SPI_ReadByte(print_reg[idx][0], print_reg[idx][1]));
    }
    
    if( open_video_path ==0 && BB_SPI_ReadByte(PAGE2, 0xda) & 0x01 )   //LDPC Lock
    {
        BB_SPI_WriteByteMask(PAGE1, 0x8d, 0xc0, 0xc0);
        open_video_path = 1;
        printf("VPATH %0.2x\n", BB_SPI_ReadByte(PAGE1, 0x8d));
    }
    
    if(BB_SPI_ReadByte(PAGE2, 0xcc) == 0x80) //Data Full
    {
        #define DMA_READY_0                0x40B00038
        #define DMA_READY_1                0x40B0003C

        Reg_Write32(DMA_READY_0, 1);
        Reg_Write32(DMA_READY_1, 1);
        
        printf("Reset SRAM DMA %x \n", BB_SPI_ReadByte(PAGE2, 0xcc));
    }
}


void TIM0_BB_Sky_handler(uint32_t u32_vectorNum)
{
    uint8_t RC_lock = 0;
    uint8_t print_reg[][2] =  {
                            {PAGE2, 0xd7}, {PAGE2, 0xd9},   //0xd9: RC lock
                            {PAGE2, 0xa0}, {PAGE2, 0xa1},   //AGC
                            {PAGE2, 0x00},
                        };
    
    int idx = 0;
    Reg_Read32(BASE_ADDR_TIMER0 + TMRNEOI_0);
    for(idx = 0; idx < sizeof(print_reg) / sizeof(print_reg[0]); idx ++)
    {
        printf("%0.2x %0.2x\n", print_reg[idx][1], BB_SPI_ReadByte(print_reg[idx][0], print_reg[idx][1]));
    }
    
    RC_lock = BB_SPI_ReadByte(PAGE2, 0xd9);
    if(RC_lock != 0x07)
    {
        BB_softReset(BB_SKY_MODE );
    }
}


void BB_debug_print_init_grd(void)
{
    init_timer_st timer0_0;
    timer0_0.base_time_group = 0;
    timer0_0.time_num = 0;
    timer0_0.ctrl = 0;
    timer0_0.ctrl |= TIME_ENABLE | USER_DEFINED;
    
    TIM_RegisterTimer(timer0_0, 2000*1000);    //2s
    INTR_NVIC_EnableIRQ(TIMER_INTR00_VECTOR_NUM);
    TIM_StartTimer(timer0_0);

    reg_IrqHandle(TIMER_INTR00_VECTOR_NUM, TIM0_BB_Grd_handler, NULL);
}


void BB_debug_print_init_sky(void)
{
    init_timer_st timer0_0;
    timer0_0.base_time_group = 0;
    timer0_0.time_num = 0;
    timer0_0.ctrl = 0;
    timer0_0.ctrl |= TIME_ENABLE | USER_DEFINED;
    
    TIM_RegisterTimer(timer0_0, 4000*1000);    //4s
    INTR_NVIC_EnableIRQ(TIMER_INTR00_VECTOR_NUM);
    TIM_StartTimer(timer0_0);

    reg_IrqHandle(TIMER_INTR00_VECTOR_NUM, TIM0_BB_Sky_handler, NULL);    
}

#endif

