#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "systicks.h"
#include "debuglog.h"
#include "bb_ctrl_internal.h"
#include "bb_grd_sweep.h"
#include "bb_regs.h"


#define SWEEP_FREQ_BLOCK_ROWS           (6)

#define BW_10M_START_TH                 (12)
#define BW_10M_END_TH                   (19)
#define BW_10M_MAX_CH_CNT               (BW_10M_END_TH - BW_10M_START_TH + 1)
#define BW_10M_VALID_CH_CNT             (BW_10M_MAX_CH_CNT - 2)
#define BW_10M_VALID_1_5M_CH_START      (BW_10M_START_TH + 1)


#define BW_20M_START_TH                 (8)
#define BW_20M_END_TH                   (23)
#define BW_20M_MAX_CH_CNT               (BW_20M_END_TH - BW_20M_START_TH + 1)
#define BW_20M_VALID_CH_CNT             (BW_20M_MAX_CH_CNT - 4)
#define BW_20M_VALID_1_5M_CH_START      (BW_20M_START_TH + 2)


#define NEXT_NUM(cur, max)              ( ((cur + 1) >= max) ? 0: (cur + 1) )

#define AGC_LEVEL_0                     (0x5f)  //-90dmb
#define AGC_LEVEL_1                     (0x64)  //-96dmb

#define MAX(a,b) (((a) > (b)) ?  (a) :  (b) )
#define MIN(a,b) (((a) < (b)) ?  (a) :  (b) )

#define BAND_SWEEP_CH(band) ( (band == RF_2G)?  MAX_2G_IT_FRQ_SIZE : MAX_5G_IT_FRQ_SIZE )
#define GET_IT_TOTAL_CH(band) ( (band == RF_2G)?  MAX_2G_IT_FRQ_SIZE : MAX_5G_IT_FRQ_SIZE )

#define CLEAR_MAIN_CNT  (1000)
static int flaglog = 0;

typedef struct
{
    int16_t     s16_2G_slicePower[SWEEP_FREQ_BLOCK_ROWS][MAX_2G_IT_FRQ_SIZE*8];            //1.5M bandwidth noise in channels for 2G
    int16_t     s16_5G_slicePower[SWEEP_FREQ_BLOCK_ROWS][MAX_5G_IT_FRQ_SIZE *8];        //1.5M bandwidth noise in channels for 5G

    int16_t     s16_2G_power_average[SWEEP_FREQ_BLOCK_ROWS][MAX_2G_IT_FRQ_SIZE];           //channel power: 10M bandwidth
    int16_t     s16_5G_power_average[SWEEP_FREQ_BLOCK_ROWS][MAX_5G_IT_FRQ_SIZE];        //channel power: 10M bandwidth

    //////////////////For 2G time-domain power
    int32_t         i32_2G_noisePower[SWEEP_FREQ_BLOCK_ROWS][MAX_2G_IT_FRQ_SIZE];
    int32_t         i32_2G_noisepower_average[MAX_2G_IT_FRQ_SIZE];
    int32_t         i32_2G_variance[MAX_2G_IT_FRQ_SIZE];

    //////////////////For 5G time-domain power
    int32_t         i32_5G_noisePower[SWEEP_FREQ_BLOCK_ROWS][MAX_5G_IT_FRQ_SIZE];
    int32_t         i32_5G_noisepower_average[MAX_5G_IT_FRQ_SIZE];
    int32_t         i32_5G_variance[MAX_5G_IT_FRQ_SIZE];

    //////////////////
    int32_t         i32_2G_power[SWEEP_FREQ_BLOCK_ROWS][MAX_2G_IT_FRQ_SIZE];
    int32_t         i32_2G_power_average[MAX_2G_IT_FRQ_SIZE];

    //////////////////
    int32_t         i32_5G_power[SWEEP_FREQ_BLOCK_ROWS][MAX_5G_IT_FRQ_SIZE];
    int32_t         i32_5G_power_average[MAX_5G_IT_FRQ_SIZE];

    ENUM_RF_BAND e_curBand;
    ENUM_CH_BW   e_bw;
    uint8_t      u8_mainCh;          //current VT channel
    uint8_t      u8_mainSweeepRow;
    uint8_t      u8_mainSweepCh;

    uint8_t      u8_optCh;           //optional VT channel
    uint8_t      u8_optSweepRow;
    uint8_t      u8_optSweepCh;
    uint8_t      u8_best2GChCnt[MAX_2G_IT_FRQ_SIZE];
    uint8_t      u8_best5GChCnt[MAX_5G_IT_FRQ_SIZE];

    uint8_t      u8_otherSweepCh;    //channel number
    uint8_t      u8_otherSweepRow;

    uint8_t      u8_prevSweepCh;     //previous sweep channel, main channel and optional channel may change

    uint8_t      u8_cycleCnt;
    uint8_t      u8_preMainCh;
    uint16_t     u16_preMainCount;   //if cycle >= u16_preMainCount, clear the u8_preMainCh

    uint8_t      u8_isFull;
} STRU_SWEEP_NOISE_POWER;


STRU_SWEEP_NOISE_POWER stru_sweepPower;


static int calc_power_db(ENUM_RF_BAND e_rfBand, uint8_t bw, uint32_t power_td,
                         int16_t *power_fd, int16_t *power_db, int32_t *power_sum, 
                         uint8_t cnt, uint8_t sweep_ch, int flaglog);

static void calc_average_and_fluct(ENUM_RF_BAND e_rfBand, uint8_t u8_ItCh);

static uint8_t BB_GetSweepTotalCh(ENUM_RF_BAND e_rfBand, ENUM_CH_BW e_bw);

static uint8_t BB_UpdateOptCh(ENUM_RF_BAND e_rfBand, ENUM_CH_BW e_bw, uint8_t sweep_ch);

static uint8_t BB_JudgeAdjacentFrequency(uint8_t judge_ch);

static void BB_GetItMinMaxCh(ENUM_RF_BAND e_rfBand, ENUM_CH_BW e_bw, uint8_t *min_ch, uint8_t *max_ch);

static void BB_GetItAdjacentFrequency(uint8_t ch, uint8_t *pre, uint8_t *next);

/*
 * to start sweep
 */
void BB_SweepStart(ENUM_RF_BAND e_rfBand, ENUM_CH_BW e_bw)
{
    stru_sweepPower.u8_otherSweepCh =  0;
    stru_sweepPower.u8_otherSweepRow     =  0;
    stru_sweepPower.u8_isFull     =  0;
    stru_sweepPower.u8_cycleCnt   =  0;
    stru_sweepPower.u8_mainSweeepRow =  0;
    stru_sweepPower.u8_optSweepRow  =  0;
    stru_sweepPower.u8_preMainCh  =  0xff;
    stru_sweepPower.u8_mainCh     =  0xff;
    stru_sweepPower.e_bw          =  e_bw;
    stru_sweepPower.e_curBand     =  e_rfBand;
    stru_sweepPower.u8_prevSweepCh  = 0x0;
    stru_sweepPower.u16_preMainCount= 0x0;

    BB_set_sweepfrq( e_rfBand, e_bw, 0 );   
}


static int16_t* BB_GetOneBlockNoiseAver(ENUM_RF_BAND e_rfBand, int16_t *ps16_dstNoise)
{
    uint8_t u8_colNum;
    uint8_t col;

    u8_colNum = BAND_SWEEP_CH(e_rfBand) * 8;
    for(col = 0; col < u8_colNum; col++)
    {
        int16_t  sum  = 0;
        int16_t  aver = 0;
        uint16_t row;

        for(row = 0; row < SWEEP_FREQ_BLOCK_ROWS; row++)
        {
            sum += (e_rfBand == RF_2G) ? stru_sweepPower.s16_2G_slicePower[row][col] : 
                                         stru_sweepPower.s16_5G_slicePower[row][col];
        }
        aver  = (sum / SWEEP_FREQ_BLOCK_ROWS);

        sum = 0;
        for(row = 0; row < SWEEP_FREQ_BLOCK_ROWS; row++)
        {   
            int16_t power = (e_rfBand == RF_2G)? stru_sweepPower.s16_2G_slicePower[row][col] : 
                                                 stru_sweepPower.s16_5G_slicePower[row][col];
            if ( power - aver >= 3 )
            {
                sum += (aver + 3);
            }
            else if( power - aver <= -3)
            {
                sum += (aver - 3);
            }
            else
            {
                sum += power;
            }
        }

        *ps16_dstNoise = (sum / SWEEP_FREQ_BLOCK_ROWS);
        ps16_dstNoise++;
    }

    return ps16_dstNoise;
}

void BB_GetSweepNoise(int16_t *ptr_noise_power)
{
    uint8_t col;
    uint8_t i;
    int16_t value;

    
    for(col = 0; col < MAX_2G_IT_FRQ_SIZE; col++)
    {
        value = (int16_t)(stru_sweepPower.i32_2G_power_average[col]);
        for(i = 0; i < 8; i++)
        {
            ptr_noise_power[(col * 8) + i] = value;
        }
    }

    for(col = 0; col < MAX_5G_IT_FRQ_SIZE; col++)
    {
        value = (int16_t)(stru_sweepPower.i32_5G_power_average[col]);
        for(i = 0; i < 8; i++)
        {
            ptr_noise_power[(col + MAX_2G_IT_FRQ_SIZE) * 8 + i] = value;
        }
    }
}


/*
 * adapt to AR8020 new sweep function
 * return 0: sweep fail. Do not switch to next sweep channel
 * return 1: sweep OK.
*/
static uint8_t BB_GetSweepPower(ENUM_RF_BAND e_rfBand, uint8_t bw, uint8_t row, uint8_t sweep_ch, uint8_t flag)
{
    uint8_t  num = 0;
    uint8_t  i   = 0;
    uint8_t  ret = 0;
    uint8_t  offset = sweep_ch;

    uint16_t ch_fd_power[16];
    uint32_t start = SysTicks_GetTickCount();
    uint8_t  power_fd_addr = 0x60;
    uint8_t u8_maxCh = BB_GetSweepTotalCh( e_rfBand, bw);
                                       
    if(sweep_ch >= u8_maxCh)
    {
        dlog_error("sweepCh maxchannel: %d %d", sweep_ch, u8_maxCh);
    }

    //get the time domain noise power
    uint32_t power_td =  (((uint32_t)(BB_ReadReg(PAGE2, SWEEP_ENERGY_HIGH)) << 16) |
                                     (BB_ReadReg(PAGE2, SWEEP_ENERGY_MID) << 8)  |
                                     (BB_ReadReg(PAGE2, SWEEP_ENERGY_LOW)));
    
    //pu32_power_td[offset] = power_td;
    if(power_td == 0)
    {
        return 0;
    }



    num = ((bw == BW_10M) ? BW_10M_MAX_CH_CNT : BW_20M_MAX_CH_CNT);
    power_fd_addr += ( ((bw == BW_10M) ? BW_10M_END_TH : BW_20M_END_TH ) * 2 /*2 byts each snr */ );

    for(i = 0; i < num; i++)
    {
        ch_fd_power[i] = (((uint16_t)BB_ReadReg(PAGE3, power_fd_addr) << 8) | BB_ReadReg(PAGE3, power_fd_addr+1));
        power_fd_addr -= 2;
        if(ch_fd_power[i] == 0)
        {
            return 0;
        }
    }

    flaglog = 0;

    {
        static int loop = 0;
        static int prech = 0;
    }

    int16_t *ps16_slicePower = (e_rfBand == RF_2G) ? stru_sweepPower.s16_2G_slicePower[row] : 
                                                     stru_sweepPower.s16_5G_slicePower[row];
    int32_t *ps32_power = (e_rfBand == RF_2G) ? stru_sweepPower.i32_2G_power[row] : 
                                                     stru_sweepPower.i32_5G_power[row];
                                                     
    ret = calc_power_db(e_rfBand, bw, power_td, 
                        ch_fd_power, ps16_slicePower, ps32_power,
                        num, sweep_ch, flag);

    num = GET_IT_TOTAL_CH(e_rfBand);
    for(i = 0; i < num; i++)
    {
        calc_average_and_fluct(e_rfBand, i);
    }

    uint32_t spend = SysTicks_GetTickCount() - start;
    if( spend >= 1 )
    {
        dlog_info("!!spend %d", spend);
    }

    return ret;
}


int BB_Sweep_updateCh(uint8_t mainch)
{
    uint8_t opt;
    stru_sweepPower.u8_preMainCh     = stru_sweepPower.u8_mainCh;
    stru_sweepPower.u16_preMainCount = 0; //clear after 1000 cycles

    stru_sweepPower.u8_mainSweepCh = (BW_10M == (stru_sweepPower.e_bw)) ? (mainch) : (mainch / 2);
    stru_sweepPower.u8_mainCh      = mainch;

    //re-select the option channel
    BB_selectBestCh(SELECT_OPT, NULL, &opt, NULL, 0);

    stru_sweepPower.u8_optCh = opt;
    stru_sweepPower.u8_optSweepCh = (BW_10M == (stru_sweepPower.e_bw)) ? (opt) : (opt / 2);

    memset(stru_sweepPower.u8_best2GChCnt, 0, sizeof(stru_sweepPower.u8_best2GChCnt));
    memset(stru_sweepPower.u8_best5GChCnt, 0, sizeof(stru_sweepPower.u8_best5GChCnt));
    //dlog_info("update main opt: %d %d", mainch, opt);    
}


static int BB_SweepBeforeFull( void )
{
    uint8_t u8_maxCh = BB_GetSweepTotalCh( stru_sweepPower.e_curBand, stru_sweepPower.e_bw);
    uint8_t result   = BB_GetSweepPower( stru_sweepPower.e_curBand,
                                         stru_sweepPower.e_bw, 
                                         stru_sweepPower.u8_otherSweepRow, 
                                         stru_sweepPower.u8_prevSweepCh, 
                                         0);
    //dlog_info("u8_prevSweepCh = %d row=%d %d", stru_sweepPower.u8_prevSweepCh, stru_sweepPower.u8_curRow, SWEEP_FREQ_BLOCK_ROWS);
    if( result )
    {
        uint8_t nextch = NEXT_NUM(stru_sweepPower.u8_prevSweepCh, u8_maxCh);
        if ( nextch < stru_sweepPower.u8_prevSweepCh )
        {
            stru_sweepPower.u8_otherSweepRow = NEXT_NUM(stru_sweepPower.u8_otherSweepRow, SWEEP_FREQ_BLOCK_ROWS);
            if( stru_sweepPower.u8_otherSweepRow == 0 )
            {
                uint8_t mainch = 0, opt = 0;
                stru_sweepPower.u8_isFull = 1;
                BB_selectBestCh(SELECT_MAIN_OPT, &mainch, &opt, NULL, 0);

                stru_sweepPower.u8_mainSweepCh = (BW_10M == (stru_sweepPower.e_bw)) ? (mainch) : (mainch / 2);
                stru_sweepPower.u8_mainCh      = mainch;
                
                stru_sweepPower.u8_prevSweepCh = stru_sweepPower.u8_mainSweepCh;

                stru_sweepPower.u8_optCh = opt;
                stru_sweepPower.u8_optSweepCh = (BW_10M == (stru_sweepPower.e_bw)) ? (opt) : (opt / 2);
                stru_sweepPower.u8_cycleCnt = MAIN_CH_CYCLE;
                return result;
            }
            else
            {
                stru_sweepPower.u8_prevSweepCh = nextch;
            }
        }
        else
        {
            stru_sweepPower.u8_prevSweepCh = nextch;
        }

        BB_set_sweepfrq( stru_sweepPower.e_curBand, stru_sweepPower.e_bw, stru_sweepPower.u8_prevSweepCh );
    }

    return result;
}


static int BB_set_sweepChannel( void )
{
    uint8_t cycle = (stru_sweepPower.u8_cycleCnt % 0x03);
    ENUM_RF_BAND e_rfBand = stru_sweepPower.e_curBand;
    ENUM_CH_BW   e_bw = stru_sweepPower.e_bw;
    uint8_t u8_maxCh = BB_GetSweepTotalCh( e_rfBand, e_bw );
    if( cycle == OTHER_CH_CYCLE )
    {
        uint8_t u8_nextch = 0;
        do
        {
            u8_nextch = NEXT_NUM(stru_sweepPower.u8_otherSweepCh, u8_maxCh);
            if( u8_nextch < stru_sweepPower.u8_otherSweepCh )
            {
                stru_sweepPower.u8_otherSweepRow = NEXT_NUM(stru_sweepPower.u8_otherSweepRow, SWEEP_FREQ_BLOCK_ROWS);
            }
            stru_sweepPower.u8_otherSweepCh = u8_nextch;
        } while( u8_nextch == stru_sweepPower.u8_mainSweepCh || u8_nextch == stru_sweepPower.u8_optSweepCh );

        stru_sweepPower.u8_prevSweepCh = stru_sweepPower.u8_otherSweepCh;
    }
    else if( cycle == MAIN_CH_CYCLE )
    {
        stru_sweepPower.u8_prevSweepCh = stru_sweepPower.u8_mainSweepCh;
    }
    else if( cycle == OPT_CH_CYCLE )
    {
        stru_sweepPower.u8_prevSweepCh = stru_sweepPower.u8_optSweepCh;
    }

    BB_set_sweepfrq( e_rfBand, e_bw, stru_sweepPower.u8_prevSweepCh );

    STRU_WIRELESS_INFO_DISPLAY *osdptr = (STRU_WIRELESS_INFO_DISPLAY *)(SRAM_BB_STATUS_SHARE_MEMORY_ST_ADDR);
    if ( RF_2G == context.freq_band )
    {
        osdptr->IT_channel  = stru_sweepPower.u8_mainCh;
        osdptr->u8_optCh    = stru_sweepPower.u8_optCh;        
    }
    else
    {
        uint8_t shift = BAND_SWEEP_CH(RF_2G);
        osdptr->IT_channel  = stru_sweepPower.u8_mainCh + shift;
        osdptr->u8_optCh    = stru_sweepPower.u8_optCh  + shift;
    }

    return 0;
}


static int BB_SweepAfterFull( uint8_t flag )
{
    uint8_t result = 0;
    uint8_t u8_maxCh = GET_IT_TOTAL_CH( stru_sweepPower.e_curBand );
    uint8_t cycle  = (stru_sweepPower.u8_cycleCnt % 0x03);

    ENUM_RF_BAND e_rfBand = stru_sweepPower.e_curBand;
    ENUM_CH_BW   e_bw = stru_sweepPower.e_bw;

    uint8_t u8_mainSweepCh = stru_sweepPower.u8_mainSweepCh;
    uint8_t u8_optSweepCh  = stru_sweepPower.u8_optSweepCh;
    uint8_t u8_prevSweepCh = stru_sweepPower.u8_prevSweepCh;

    if( u8_prevSweepCh == u8_mainSweepCh )
    { 
        result = BB_GetSweepPower( e_rfBand,
                                   e_bw, 
                                   stru_sweepPower.u8_mainSweeepRow,
                                   u8_prevSweepCh,
                                   flag);
        stru_sweepPower.u8_mainSweeepRow  = NEXT_NUM(stru_sweepPower.u8_mainSweeepRow, SWEEP_FREQ_BLOCK_ROWS);
    }
    else if ( u8_prevSweepCh == u8_optSweepCh)
    {    
        result = BB_GetSweepPower( e_rfBand,
                                   e_bw, 
                                   stru_sweepPower.u8_optSweepRow, 
                                   u8_prevSweepCh,
                                   flag); //
        stru_sweepPower.u8_optSweepRow  = NEXT_NUM(stru_sweepPower.u8_optSweepRow, SWEEP_FREQ_BLOCK_ROWS);
    }
    else
    {  
        result = BB_GetSweepPower( e_rfBand,
                                   e_bw, 
                                   stru_sweepPower.u8_otherSweepRow, 
                                   u8_prevSweepCh,
                                   flag);

        BB_UpdateOptCh(e_rfBand, e_bw, u8_prevSweepCh);
    }

    stru_sweepPower.u8_cycleCnt ++;

    //set next channel to sweep
    if( result )
    {
        BB_set_sweepChannel();
    }

    return result;
}


uint8_t BB_GetSweepResult( uint8_t flag )
{
    uint8_t  ret = 0;
    uint32_t start = SysTicks_GetTickCount();
    if ( !stru_sweepPower.u8_isFull )
    {
        ret = BB_SweepBeforeFull( );
    }
    else
    {
        ret = BB_SweepAfterFull( flag );
    }
    
    if ( stru_sweepPower.u8_preMainCh != 0xFF && stru_sweepPower.u16_preMainCount++ >= CLEAR_MAIN_CNT )
    {
        //clear the premain;
        //stru_sweepPower.u16_preMainCount = 0;
        //stru_sweepPower.u8_preMainCh     = 0xFF;
    }
    
    uint32_t spend = SysTicks_GetTickCount() - start;
    if( spend >= 1)
    {
        dlog_info("spend=%d\n", spend);
    }

    return ret;
}

#if 0
/*
 *  opt: 0:         Force main channel 
 *       1:         Force opt channel 
 *       other:     do not care
*/
uint8_t BB_forceSweep( uint8_t opt )
{
    if( opt == MAIN_CH_CYCLE || opt == OPT_CH_CYCLE)
    {
        stru_sweepPower.u8_cycleCnt    = opt;
        stru_sweepPower.u8_prevSweepCh = (opt == MAIN_CH_CYCLE) ? stru_sweepPower.u8_mainSweepCh : 
                                                                  stru_sweepPower.u8_optSweepCh;
        return BB_set_sweepfrq( stru_sweepPower.e_curBand, stru_sweepPower.u8_prevSweepCh );
    }

    return 0;
}
#endif


int32_t BB_CompareCh1Ch2ByPowerAver(ENUM_RF_BAND e_rfBand, uint8_t u8_itCh1, uint8_t u8_itCh2, int32_t level)
{
    int32_t value = 0;

    int32_t * pi32_power_average = (e_rfBand == RF_2G) ? stru_sweepPower.i32_2G_power_average :
                                                     stru_sweepPower.i32_5G_power_average;
                                                
    if (pi32_power_average[u8_itCh1] < (pi32_power_average[u8_itCh2] - level))
    {
        value = 1;
    }
    
    return value;
}

int32_t BB_CompareCh1Ch2ByPower(ENUM_RF_BAND e_rfBand, uint8_t u8_itCh1, uint8_t u8_itCh2, uint8_t u8_cnt)
{
    int32_t value = 0;
    uint8_t row;
    uint8_t tmpCnt = 0;
    int32_t *pu32_power;

    if (u8_cnt > SWEEP_FREQ_BLOCK_ROWS)
    {
        u8_cnt = SWEEP_FREQ_BLOCK_ROWS;
    }

    for( row = 0; row < SWEEP_FREQ_BLOCK_ROWS; row++)
    {
        pu32_power = (e_rfBand == RF_2G) ? stru_sweepPower.i32_2G_power[row] : 
                                              stru_sweepPower.i32_5G_power[row];
        
        tmpCnt += ((pu32_power[u8_itCh1] < pu32_power[u8_itCh2]) ? (1) : (0));
    }
    
    value = ((tmpCnt >= u8_cnt) ? (1) : (0));

    return value;
}


static uint8_t is_inlist(uint8_t item, uint8_t *pu8_exlude, uint8_t u8_cnt)
{
    uint8_t flag = 0;
    uint8_t i;
    for ( i = 0; i < u8_cnt && flag == 0; i++) //channel in the excluding list
    {
        flag = ( item == pu8_exlude[i] );
    }

    return flag;
}

static uint8_t find_best(uint8_t *pu8_exlude, uint8_t u8_exclude_cnt, ENUM_RF_BAND e_rfBand, uint8_t log)
{
    uint8_t u8_start = 0;
    uint8_t ch = 0;
    int16_t aver, fluct;
    uint8_t num = GET_IT_TOTAL_CH(e_rfBand);

    for ( u8_start; 
          u8_start < num && is_inlist( u8_start, pu8_exlude, u8_exclude_cnt );  //channel in the excluding list
          u8_start++) 
    {}

    
    for( ch; ch < num; ch++)
    {
        if ( !is_inlist( ch, pu8_exlude, u8_exclude_cnt ) )
        {
            if(BB_CompareCh1Ch2ByPowerAver(e_rfBand, ch, u8_start, CMP_POWER_AVR_LEVEL))
            {
                u8_start = ch;
            }
        }
    }

    return u8_start;
}

static void BB_GetItMinMaxCh(ENUM_RF_BAND e_rfBand, ENUM_CH_BW e_bw, uint8_t *min_ch, uint8_t *max_ch)
{
    if (BW_10M == e_bw)
    {
        *max_ch = GET_IT_TOTAL_CH(e_rfBand) - 1;
        *min_ch = 0;
    }
    else // 20M
    {
        *max_ch = GET_IT_TOTAL_CH(e_rfBand) - 2;
        *min_ch = 1;
    }
}

static void BB_GetItAdjacentFrequency(uint8_t ch, uint8_t *pre, uint8_t *next)
{
    *pre = ch - 1;
    *next = ch + 1;
}


/*
 * u8_opt: 
 *         SELECT_OPT: do not change the main channel. only select one optional channel
 *         SELECT_MAIN_OPT: select the main and opt channel
 *         CHANGE_MAIN: change the main channel, and select another one
 * return: 
 *         0: no suggest main and opt channel
 *         1: get main and opt channel
*/
uint8_t BB_selectBestCh(ENUM_CH_SEL_OPT e_opt, uint8_t *u8_mainCh, uint8_t *u8_optCh, uint8_t *u8_other, uint8_t log)
{
    uint8_t  ch = 0;
    int16_t  aver, fluct;
    uint32_t start = SysTicks_GetTickCount();
    uint8_t  exclude[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
    uint8_t  excludecnt = 0;
    ENUM_RF_BAND e_band = stru_sweepPower.e_curBand;
    ENUM_CH_BW e_bw = stru_sweepPower.e_bw;
    uint8_t bestCh  = 0xff;         //select one from all the channels
    uint8_t betterCh= 0xff;
    uint8_t other   = 0xff;
    //uint8_t index = 3;
    uint8_t min_ch;
    uint8_t max_ch;
    uint8_t pre_ch;
    uint8_t next_ch;
    uint8_t tmpCh;
    
    if( !stru_sweepPower.u8_isFull )
    {
        return 0;
    }
    
    BB_GetItMinMaxCh(e_band, e_bw, &min_ch, &max_ch);
    
    //select main channel
    if( e_opt == SELECT_MAIN_OPT )
    {
        if (BW_20M == (stru_sweepPower.e_bw))
		{
			exclude[0] = 0;
            exclude[1] = max_ch + 1;
            excludecnt = 2;
		}
        else
		{
		    excludecnt = 0;
		}
        bestCh = find_best(exclude, excludecnt, e_band, log);
        if ( u8_mainCh )
        {
            *u8_mainCh = bestCh;
        }
    }

    //select optional channel
    if ( e_opt == SELECT_MAIN_OPT || e_opt == SELECT_OPT )
    {
        exclude[0] = stru_sweepPower.u8_preMainCh;
        exclude[1] = stru_sweepPower.u8_mainCh;
        exclude[2] = bestCh;
        
        tmpCh = (e_opt == SELECT_OPT) ? (stru_sweepPower.u8_mainCh) : (bestCh);
        BB_GetItAdjacentFrequency(tmpCh, &pre_ch, &next_ch);
        exclude[3] = pre_ch;
        exclude[4] = next_ch;
        excludecnt = 5;

        if ( log )
        {
            dlog_info("exclude: %d %d %d", exclude[0], exclude[1], exclude[2]);
        }

        betterCh = find_best(exclude, excludecnt, e_band, log);

        if ((pre_ch >= min_ch) && (pre_ch <= max_ch))
        {
            if (BB_CompareCh1Ch2ByPower(e_band, pre_ch, betterCh, SWEEP_FREQ_BLOCK_ROWS))
            {
                betterCh = pre_ch;
            }
        }
        if ((next_ch >= min_ch) && (next_ch <= max_ch))
        {
            if (BB_CompareCh1Ch2ByPower(e_band, next_ch, betterCh, SWEEP_FREQ_BLOCK_ROWS))
            {
                betterCh = next_ch;
            }
        }
        
        if ( u8_optCh )
        {
            *u8_optCh= betterCh;
        }
    }

    if ( e_opt == SELECT_OTHER )
    {
        exclude[0] = stru_sweepPower.u8_preMainCh;
        exclude[1] = stru_sweepPower.u8_mainCh;
        exclude[2] = stru_sweepPower.u8_optCh;   
        excludecnt = 3;

        other = find_best(exclude, excludecnt, e_band, log);

        if (u8_other)
        {
            *u8_other = other;
        }
    }

    if ( log )
    {
        dlog_info("--ch--: %d %d %d", bestCh, betterCh, e_opt);
    }    

    uint32_t spend = SysTicks_GetTickCount() - start;
    if( spend >= 1 )
    {
        dlog_info("!!spend %d", spend);
    }

    return 1;
}



uint8_t get_opt_channel( void )
{
    uint8_t other;
    int32_t level;
    uint8_t ret = stru_sweepPower.u8_optCh;
    ENUM_RF_BAND e_band = stru_sweepPower.e_curBand;

    BB_selectBestCh(SELECT_OTHER, NULL, NULL, &other, 0);
    
    if ( context.agclevel >= AGC_LEVEL_1 )
    {
        level = 3;
    }
    else if( context.agclevel >= AGC_LEVEL_0 )
    {
        level = 6;
    }
    else
    {
        level = 9;
    }

    level = ((1 == BB_JudgeAdjacentFrequency(other)) ? (2 * level) : (level));
    if (BB_CompareCh1Ch2ByPower(e_band, other, stru_sweepPower.u8_optCh, SWEEP_FREQ_BLOCK_ROWS))
    {
        ret = other; 
    }

    return ret;
}


static int8_t log10_lookup_table[]=
{
    0,
    3,
    4,
    6,
    6,
    7,
    8,
    9,
    9,
    10,
    10,
    10,
    11,
    11,
    11,
    12,
    12,
    12,
    12,
    13,
    13,
    13,
    13,
    13,
    13,
    14,
    14,
    14,
    14,
    14,
    14,
    15,
    15,
    15,
    15,
    15,
    15,
    15,
    15,
    16,
    16,
    16,
    16,
    16,
    16,
    16,
    16,
    16,
    16,
    16,
    17,
    17,
    17,
    17,
    17,
    17,
    17,
    17,
    17,
    17,
    17,
    17,
    17,
    18,
    18,
    18,
    18,
    18,
    18,
    18,
    18,
    18,
    18,
    18,
    18,
    18,
    18,
    18,
    18,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
    19,
};


static int get_10log10(uint32_t data)
{
    uint8_t result = 0;
    while(data >= 100)
    {
        result += 10;
        data = data / 10;
    }
    return (result + log10_lookup_table[data]);
}


/*
 * get the dbm noise power
 * power_td: total power in time domain
 * power_fd: each 1.56M bandwidth noise
 * power_db: the power in dbm in each 1.56M
*/
static int calc_power_db(ENUM_RF_BAND e_rfBand, uint8_t bw, uint32_t power_td,
                         int16_t *power_fd, int16_t *power_db, int32_t *power_sum, 
                         uint8_t cnt, uint8_t sweep_ch, int flaglog)
{
    uint8_t  i = 0;
    uint8_t  offset = 0;

#if 1
    // calcation the power average by frequency-domain
    uint64_t sum_fd = 0;
    for(i = 0 ; i < cnt; i++)
    {
        sum_fd += ((uint64_t)power_fd[i] * power_fd[i]);
    }

    uint64_t sum_fd1 = 0;
    uint64_t sum_fd2 = 0;
    if (BW_10M == bw)
    {
        offset = sweep_ch;
        for(i = 1 ; i < 7; i++)
        {
            sum_fd1 += ((uint64_t)power_fd[i] * power_fd[i]);
        }
        sum_fd1 = sum_fd1 * power_td / sum_fd;
        power_sum[offset] = get_10log10(sum_fd1);
        power_sum[offset] += ((power_sum[offset] > 30) ? (-123) : (-125));
    }
    else // 20M
    {
        offset = sweep_ch * 2;
        for(i = 2 ; i < 8; i++)
        {
            sum_fd1 += ((uint64_t)power_fd[i] * power_fd[i]);
        }
        sum_fd1 = sum_fd1 * power_td / sum_fd;
        
        
        for(i = 8 ; i < 14; i++)
        {
            sum_fd2 += ((uint64_t)power_fd[i] * power_fd[i]);
        }
        sum_fd2 = sum_fd2 * power_td / sum_fd;

        if (((MAX_5G_IT_FRQ_SIZE/2) == sweep_ch) && (RF_5G == e_rfBand)) // 5G last sweep ch
        {
            power_sum[offset] = get_10log10(sum_fd2);
            power_sum[offset] += ((power_sum[offset] > 30) ? (-123) : (-125));
        }
        else
        {
            power_sum[offset] = get_10log10(sum_fd1);
            power_sum[offset] += ((power_sum[offset] > 30) ? (-123) : (-125));
            power_sum[offset + 1] = get_10log10(sum_fd2);
            power_sum[offset + 1] += ((power_sum[offset + 1] > 30) ? (-123) : (-125));
        }
    }
#else
    // calcation the power average by time-domain

    int tmp = power_td;
    tmp = get_10log10(tmp);
    tmp += ((tmp > 30) ? (-123) : (-125));

    for(i = 0 ; i < cnt; i++)
    {
        power_db[i] = tmp;
    }
#endif

    if( flaglog )
    {
        dlog_info("%d %d %d %d %d %d",  power_db[0], power_db[1], power_db[2],
                                        power_db[3], power_db[4], power_db[5]);
    }

    return 1;
}

static void calc_average_and_fluct(ENUM_RF_BAND e_rfBand, uint8_t u8_ItCh)
{
    uint16_t row = 0;
    int32_t *pu32_power;
    int32_t *pu32_power_average;

    pu32_power_average = (e_rfBand == RF_2G) ? stru_sweepPower.i32_2G_power_average : 
                                                    stru_sweepPower.i32_5G_power_average;  
    pu32_power_average[u8_ItCh] = 0;
    for( row = 0; row < SWEEP_FREQ_BLOCK_ROWS; row++)
    {
        pu32_power = (e_rfBand == RF_2G) ? stru_sweepPower.i32_2G_power[row] : 
                                              stru_sweepPower.i32_5G_power[row];
        {
            pu32_power_average[u8_ItCh] += pu32_power[u8_ItCh];
        }
    }
    pu32_power_average[u8_ItCh] /= SWEEP_FREQ_BLOCK_ROWS;
}

static uint8_t BB_GetSweepTotalCh(ENUM_RF_BAND e_rfBand, ENUM_CH_BW e_bw)
{
    uint8_t value;
    //(band == RF_2G)?  MAX_2G_IT_FRQ_SIZE : MAX_5G_IT_FRQ_SIZE 
    if (RF_2G == e_rfBand)
    {
        value = MAX_2G_IT_FRQ_SIZE;
    }
    else // 5G
    {
        value = MAX_5G_IT_FRQ_SIZE;
    }

    if (BW_20M == e_bw)
    {
        value = (value + 1) / 2;
    }

    return value;
}

static uint8_t BB_JudgeAdjacentFrequency(uint8_t judge_ch)
{
    if ((judge_ch == (stru_sweepPower.u8_mainCh + 1)) || (judge_ch == (stru_sweepPower.u8_mainCh - 1)))
    {
        return 1;
    }
    else
    {
        return 0;
    }
}

static uint8_t BB_UpdateOptCh(ENUM_RF_BAND e_rfBand, ENUM_CH_BW e_bw, uint8_t sweep_ch)
{
    uint8_t tmpCh = stru_sweepPower.u8_optCh;
    uint8_t u8_maxCh = GET_IT_TOTAL_CH( e_rfBand );
    int32_t level;
    uint8_t *pu8_bestChCnt = ((RF_2G == (stru_sweepPower.e_curBand)) ? (stru_sweepPower.u8_best2GChCnt) : 
                                                                       (stru_sweepPower.u8_best5GChCnt));

    if (BW_10M == e_bw)
    {
        level = ((1 == BB_JudgeAdjacentFrequency(sweep_ch)) ? (2 * CMP_POWER_AVR_LEVEL) : (CMP_POWER_AVR_LEVEL));
        if (BB_CompareCh1Ch2ByPower(e_rfBand, sweep_ch, tmpCh, SWEEP_FREQ_BLOCK_ROWS))
        {
            tmpCh = sweep_ch;
        }
    }
    else // 20M
    {
        if (0 == sweep_ch)
        {
            level = ((1 == BB_JudgeAdjacentFrequency(sweep_ch * 2 + 1)) ? (2 * CMP_POWER_AVR_LEVEL) : (CMP_POWER_AVR_LEVEL));
            if (BB_CompareCh1Ch2ByPower(e_rfBand, sweep_ch * 2 + 1, tmpCh, SWEEP_FREQ_BLOCK_ROWS))
            {
                tmpCh = sweep_ch * 2 + 1;
            }
        }
        else if ( (((u8_maxCh + 1) / 2) - 1) == sweep_ch )
        {
            if (RF_2G == e_rfBand)
            {
                level = ((1 == BB_JudgeAdjacentFrequency(sweep_ch * 2)) ? (2 * CMP_POWER_AVR_LEVEL) : (CMP_POWER_AVR_LEVEL));
                if (BB_CompareCh1Ch2ByPower(e_rfBand, sweep_ch * 2, tmpCh, SWEEP_FREQ_BLOCK_ROWS))
                {
                    tmpCh = sweep_ch * 2;
                }
            }
            else // 5G
            {
                // do nothing
            }
        }
        else
        {
            level = ((1 == BB_JudgeAdjacentFrequency(sweep_ch * 2)) ? (2 * CMP_POWER_AVR_LEVEL) : (CMP_POWER_AVR_LEVEL));
            if (BB_CompareCh1Ch2ByPower(e_rfBand, sweep_ch * 2, tmpCh, SWEEP_FREQ_BLOCK_ROWS))
            {
                tmpCh = sweep_ch * 2;
            }

            level = ((1 == BB_JudgeAdjacentFrequency(sweep_ch * 2 + 1)) ? (2 * CMP_POWER_AVR_LEVEL) : (CMP_POWER_AVR_LEVEL));
            if (BB_CompareCh1Ch2ByPower(e_rfBand, sweep_ch * 2 + 1, tmpCh, SWEEP_FREQ_BLOCK_ROWS)) 
            {
                tmpCh = sweep_ch * 2 + 1;
            }
        }
    }

    if (tmpCh != (stru_sweepPower.u8_optCh))
    {
        pu8_bestChCnt[tmpCh] += 1;
        if (pu8_bestChCnt[tmpCh] > 10)
        {
            stru_sweepPower.u8_optCh = tmpCh;
            stru_sweepPower.u8_optSweepCh = ((BW_10M == e_bw) ? (tmpCh) : (tmpCh / 2));
            memset(pu8_bestChCnt, 0, u8_maxCh);
        }
    }
    else
    {
        pu8_bestChCnt[tmpCh] = 0;
    }
}
