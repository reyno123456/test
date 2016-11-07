#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include "debuglog.h"

//#include "config_functions_sel.h"
#include "timer.h"
#include "interrupt.h"
#include "BB_ctrl.h"
#include "BB_spi.h"

static int test_bbctrl_sky(void);
static int test_bbctrl_grd(void);

void test_BB_sky(void)
{
    ENUM_BB_MODE cur_mode = BB_SKY_MODE;
    char *log = "IN BB sky mode \n";

    STRU_BB_initType initType = {
        .en_mode = cur_mode,
    };
 
    BB_uart10_spi_sel(0x00000003);
    BB_init(&initType);
    
    #if 1
    test_bbctrl_sky();
    #else
    BB_debug_print_init_sky();
    #endif
    printf("%s", log);
}

void test_BB_grd(void)
{
    ENUM_BB_MODE cur_mode = BB_GRD_MODE;
    char *log = "IN BB  Ground mode \r\n";

    STRU_BB_initType initType = {
        .en_mode = cur_mode,
    };
 
    BB_uart10_spi_sel(0x00000003);
    BB_init(&initType);
    
    #if 1
    test_bbctrl_grd();
    #else
    BB_debug_print_init_grd();
    #endif
}

static int test_bbctrl_sky(void)
{
    Sky_Parm_Initial();
    //Sky_Id_Initial();
    //Sys_Parm_Init();
}

static int test_bbctrl_grd(void)
{
    Grd_Parm_Initial();
    BB_Grd_Id_Initial();
    //Sys_Parm_Init();
}


/*
 * this Function for demo only...
*/
static int open_video_path = 0;

void TIM0_BB_Grd_handler(void)
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


void TIM0_BB_Sky_handler(void)
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

    reg_IrqHandle(TIMER_INTR00_VECTOR_NUM, TIM0_BB_Grd_handler);
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

    reg_IrqHandle(TIMER_INTR00_VECTOR_NUM, TIM0_BB_Sky_handler);    
}
