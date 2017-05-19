#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "systicks.h"
#include "debuglog.h"
#include "bb_ctrl_internal.h"
#include "bb_grd_sweep.h"
#include "bb_regs.h"

#define MAX_1_5M_CH                     (18)
#define BW_10M_VALID_1_5M_CH_START      (13)

#define BW_20M_VALID_1_5M_CH_START      (8)
#define SWEEP_FREQ_BLOCK_ROWS           (6)

#define BW_10M_VALID_CH_CNT             (MAX_1_5M_CH - BW_10M_VALID_1_5M_CH_START + 1)
#define BW_20M_VALID_CH_CNT             (MAX_1_5M_CH - BW_20M_VALID_1_5M_CH_START + 1)

#define NEXT_NUM(cur, max)              ( ((cur + 1) >= max) ? 0: (cur + 1) )

#define AGC_LEVEL_0                     (0x5f)  //-90dmb
#define AGC_LEVEL_1                     (0x64)  //-96dmb

#define MAX(a,b) (((a) > (b)) ?  (a) :  (b) )
#define MIN(a,b) (((a) < (b)) ?  (a) :  (b) )

#define BAND_SWEEP_CH(band) ( (band == RF_2G)?  MAX_2G_IT_FRQ_SIZE : MAX_5G_IT_FRQ_SIZE )

#define CLEAR_MAIN_CNT  (1000)
static int flaglog = 0;

typedef struct
{
    int16_t     s16_slicePower[SWEEP_FREQ_BLOCK_ROWS][MAX_2G_IT_FRQ_SIZE*8];            //1.5M bandwidth noise in channels for 2G
    int16_t     s16_5G_slicePower[SWEEP_FREQ_BLOCK_ROWS][MAX_5G_IT_FRQ_SIZE *8];        //1.5M bandwidth noise in channels for 5G

    int16_t     s16_power_average[SWEEP_FREQ_BLOCK_ROWS][MAX_2G_IT_FRQ_SIZE];           //channel power: 10M bandwidth
    int16_t     s16_5G_power_average[SWEEP_FREQ_BLOCK_ROWS][MAX_5G_IT_FRQ_SIZE];        //channel power: 10M bandwidth

    //////////////////For 2G time-domain power
    int         i32_noisePower[SWEEP_FREQ_BLOCK_ROWS][MAX_2G_IT_FRQ_SIZE];
    int         i32_noisepower_average[MAX_2G_IT_FRQ_SIZE];
    int         i32_variance[MAX_2G_IT_FRQ_SIZE];

    //////////////////For 5G time-domain power
    int         i32_5G_noisePower[SWEEP_FREQ_BLOCK_ROWS][MAX_5G_IT_FRQ_SIZE];
    int         i32_5G_noisepower_average[MAX_5G_IT_FRQ_SIZE];
    int         i32_5G_variance[MAX_5G_IT_FRQ_SIZE];

    ENUM_RF_BAND e_curBand;
    ENUM_CH_BW   e_bw;
    uint8_t      u8_mainCh;          //current VT channel
    uint8_t      u8_curMainRow;
    uint8_t      u8_mainSweepCh;

    uint8_t      u8_optCh;           //optional VT channel
    uint8_t      u8_curOptRow;
    uint8_t      u8_optSweepCh;
    uint8_t      u8_bestChCnt[MAX_2G_IT_FRQ_SIZE];
    uint8_t      u8_best5GChCnt[MAX_5G_IT_FRQ_SIZE];

    uint8_t      u8_otherSweepCh;    //channel number
    uint8_t      u8_curRow;

    uint8_t      u8_prevSweepCh;     //previous sweep channel, main channel and optional channel may change

    uint8_t      u8_cycleCnt;
    uint8_t      u8_preMainCh;
    uint16_t     u16_preMainCount;   //if cycle >= u16_preMainCount, clear the u8_preMainCh

    uint8_t      u8_isFull;
} STRU_SWEEP_NOISE_POWER;


STRU_SWEEP_NOISE_POWER stru_sweepPower;


static int calc_power_db(uint8_t bw, uint32_t power_td,
                         int16_t *power_fd, int16_t *power_db, 
                         uint8_t cnt, int flaglog);

static void calc_average_and_fluct(ENUM_RF_BAND e_rfBand, uint8_t u8_bw , uint8_t u8_ItCh);

/*
 * to start sweep
 */
void BB_SweepStart(ENUM_RF_BAND e_rfBand, ENUM_CH_BW e_bw)
{
    stru_sweepPower.u8_otherSweepCh =  0;
    stru_sweepPower.u8_curRow     =  0;
    stru_sweepPower.u8_isFull     =  0;
    stru_sweepPower.u8_cycleCnt   =  0;
    stru_sweepPower.u8_curMainRow =  0;
    stru_sweepPower.u8_curOptRow  =  0;
	stru_sweepPower.u8_preMainCh  =  0xff;
    stru_sweepPower.u8_mainCh     =  0xff;
    stru_sweepPower.e_bw          =  e_bw;
    stru_sweepPower.e_curBand     =  e_rfBand;
    stru_sweepPower.u8_prevSweepCh  = 0x0;
    stru_sweepPower.u16_preMainCount= 0x0;

    BB_set_sweepfrq( e_rfBand, 0 );   
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
            sum += (e_rfBand == RF_2G) ? stru_sweepPower.s16_slicePower[row][col] : 
                                         stru_sweepPower.s16_5G_slicePower[row][col];
        }
        aver  = (sum / SWEEP_FREQ_BLOCK_ROWS);

        sum = 0;
        for(row = 0; row < SWEEP_FREQ_BLOCK_ROWS; row++)
        {   
            int16_t power = (e_rfBand == RF_2G)? stru_sweepPower.s16_slicePower[row][col] : 
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
    int16_t *ps16_slicePower;

    ps16_slicePower = BB_GetOneBlockNoiseAver(RF_2G, ptr_noise_power);
    BB_GetOneBlockNoiseAver(RF_5G, ps16_slicePower);
}


/*
 * adapt to AR8020 new sweep function
 * return 0: sweep fail. Do not switch to next sweep channel
 * return 1: sweep OK.
*/
static uint8_t BB_GetSweepPower(ENUM_RF_BAND e_rfBand, uint8_t bw, uint8_t row, uint8_t ch, uint8_t flag)
{
    uint8_t  num = 0;
    uint8_t  i   = 0;
    uint8_t  ret = 0;

    uint16_t ch_fd_power[16];
    uint32_t start = SysTicks_GetTickCount();
    uint8_t  power_fd_addr = 0x60;

    int *pu32_power_td = (e_rfBand == RF_2G) ? stru_sweepPower.i32_noisePower[row] : 
                                               stru_sweepPower.i32_5G_noisePower[row];
    if(ch >= BAND_SWEEP_CH( e_rfBand ))
    {
        dlog_error("sweepCh maxchannel: %d %d", ch, BAND_SWEEP_CH( stru_sweepPower.e_curBand ));
    }

    //get the time domain noise power
    uint32_t power_td =  (((uint32_t)(BB_ReadReg(PAGE2, SWEEP_ENERGY_HIGH)) << 16) |
                                     (BB_ReadReg(PAGE2, SWEEP_ENERGY_MID) << 8)  |
                                     (BB_ReadReg(PAGE2, SWEEP_ENERGY_LOW)));

    pu32_power_td[ch] = power_td;
    if(power_td == 0)
    {
        return 0;
    }

    num = ((bw == BW_10M) ? BW_10M_VALID_CH_CNT : BW_20M_VALID_CH_CNT);
    power_fd_addr += ( ((bw == BW_10M) ? BW_10M_VALID_1_5M_CH_START : BW_20M_VALID_1_5M_CH_START ) * 2 /*2 byts each snr */ );

    for(i = 0; i < num; i++)
    {
        ch_fd_power[i] = (((uint16_t)BB_ReadReg(PAGE3, power_fd_addr) << 8) | BB_ReadReg(PAGE3, power_fd_addr+1));
        power_fd_addr += 2;
        if(ch_fd_power[i] == 0)
        {
            return 0;
        }
    }

    flaglog = 0;

    {
        static int loop = 0;
        static int prech = 0;
        if ( 0 )
        {
            dlog_info("ch:%d %d %.4x %.4x %.4x %.4x %.4x %.4x %.4x %.4x %.6x", ch, row, ch_fd_power[0], ch_fd_power[1], ch_fd_power[2],
                      ch_fd_power[3], ch_fd_power[4], ch_fd_power[5],
                      ch_fd_power[6], ch_fd_power[7], power_td);
            loop = 0;
            prech = ch;
            flaglog = 1;
        }
    }

    int16_t *ps16_slicePower = (e_rfBand == RF_2G) ? stru_sweepPower.s16_slicePower[row] : 
                                                     stru_sweepPower.s16_5G_slicePower[row];
    ret = calc_power_db(bw, power_td, ch_fd_power,
                        ps16_slicePower + ((uint16_t)ch * 8 + 1),
                        num,
                        flag
                        );

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

    stru_sweepPower.u8_mainSweepCh = mainch;
    stru_sweepPower.u8_mainCh      = mainch;

    //re-select the option channel
    BB_selectBestCh(SELECT_OPT, NULL, &opt, NULL, 0);

    stru_sweepPower.u8_optCh = opt;
    stru_sweepPower.u8_optSweepCh = opt;

    memset(stru_sweepPower.u8_bestChCnt, 0, sizeof(stru_sweepPower.u8_bestChCnt));
    //dlog_info("update main opt: %d %d", mainch, opt);    
}


static int BB_SweepBeforeFull( void )
{
    uint8_t u8_maxCh = BAND_SWEEP_CH( stru_sweepPower.e_curBand );
    uint8_t result   = BB_GetSweepPower( stru_sweepPower.e_curBand,
                                         stru_sweepPower.e_bw, 
                                         stru_sweepPower.u8_curRow, 
                                         stru_sweepPower.u8_prevSweepCh, 
                                         0);
    //dlog_info("u8_prevSweepCh = %d row=%d %d", stru_sweepPower.u8_prevSweepCh, stru_sweepPower.u8_curRow, SWEEP_FREQ_BLOCK_ROWS);
    if( result )
    {
        uint8_t nextch = NEXT_NUM(stru_sweepPower.u8_prevSweepCh, u8_maxCh);
        if ( nextch < stru_sweepPower.u8_prevSweepCh )
        {
            stru_sweepPower.u8_curRow = NEXT_NUM(stru_sweepPower.u8_curRow, SWEEP_FREQ_BLOCK_ROWS);
            if( stru_sweepPower.u8_curRow == 0 )
            {
                uint8_t mainch = 0, opt = 0;
                stru_sweepPower.u8_isFull = 1;
                BB_selectBestCh(SELECT_MAIN_OPT, &mainch, &opt, NULL, 0);

                stru_sweepPower.u8_mainSweepCh = mainch;
                stru_sweepPower.u8_mainCh      = mainch;
                stru_sweepPower.u8_prevSweepCh = mainch;

                stru_sweepPower.u8_optCh = opt;
                stru_sweepPower.u8_optSweepCh = opt;
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

        BB_set_sweepfrq( stru_sweepPower.e_curBand, stru_sweepPower.u8_prevSweepCh );
    }

    return result;
}


static int BB_set_sweepChannel( void )
{
    uint8_t cycle = (stru_sweepPower.u8_cycleCnt % 0x03);
    ENUM_RF_BAND e_rfBand = stru_sweepPower.e_curBand;
    uint8_t u8_maxCh = BAND_SWEEP_CH( e_rfBand );
    if( cycle == OTHER_CH_CYCLE )
    {
        uint8_t u8_nextch = 0;
        do
        {
            u8_nextch = NEXT_NUM(stru_sweepPower.u8_otherSweepCh, u8_maxCh);
            if( u8_nextch < stru_sweepPower.u8_otherSweepCh )
            {
                stru_sweepPower.u8_curRow = NEXT_NUM(stru_sweepPower.u8_curRow, SWEEP_FREQ_BLOCK_ROWS);
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

    BB_set_sweepfrq( e_rfBand, stru_sweepPower.u8_prevSweepCh );

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

    uint8_t u8_maxCh = BAND_SWEEP_CH( stru_sweepPower.e_curBand );
    uint8_t cycle  = (stru_sweepPower.u8_cycleCnt % 0x03);

    ENUM_RF_BAND e_rfBand = stru_sweepPower.e_curBand;
    ENUM_CH_BW   e_bw = stru_sweepPower.e_bw;

    uint8_t u8_mainCh = stru_sweepPower.u8_mainCh;
    uint8_t u8_optCh  = stru_sweepPower.u8_optCh;
    uint8_t u8_prevSweepCh = stru_sweepPower.u8_prevSweepCh;

    if( u8_prevSweepCh == u8_mainCh )
    {
        stru_sweepPower.u8_curMainRow  = NEXT_NUM(stru_sweepPower.u8_curMainRow, SWEEP_FREQ_BLOCK_ROWS);    
        result = BB_GetSweepPower( e_rfBand,
                                   e_bw, 
                                   stru_sweepPower.u8_curMainRow,
                                   u8_prevSweepCh,
                                   flag);
    }
    else if ( u8_prevSweepCh == u8_optCh)
    {    
        stru_sweepPower.u8_curOptRow  = NEXT_NUM(stru_sweepPower.u8_curOptRow, SWEEP_FREQ_BLOCK_ROWS);
        result = BB_GetSweepPower( e_rfBand,
                                   e_bw, 
                                   stru_sweepPower.u8_curOptRow, 
                                   u8_prevSweepCh,
                                   flag); //
    }
    else
    {  
        stru_sweepPower.u8_curRow  = NEXT_NUM(stru_sweepPower.u8_curRow, SWEEP_FREQ_BLOCK_ROWS);
        result = BB_GetSweepPower( e_rfBand,
                                   e_bw, 
                                   stru_sweepPower.u8_curRow, 
                                   u8_prevSweepCh,
                                   flag);
        {
            //compare the other channel and current opt channel
            int16_t diff, fluct;
            //stru_sweepPower.u8_optCh * 10 / stru_sweepPower.u8_prevSweepCh
            //fluct > 0
            compare_chNoisePower(e_rfBand,
                                 u8_prevSweepCh, u8_optCh, 
                                 &diff, &fluct,
                                 0);
            if ( (diff > 20) || (diff > 10 && fluct > 0))
            {
                uint8_t *pu8_bestChCnt = ( stru_sweepPower.e_curBand ) ? stru_sweepPower.u8_bestChCnt : 
                                                                         stru_sweepPower.u8_best5GChCnt;
                
                //dlog_info("ch: %d %d %d %d", stru_sweepPower.u8_prevSweepCh, stru_sweepPower.u8_optCh, diff, fluct);
                pu8_bestChCnt[u8_prevSweepCh] += 1;
    
                //if the channel is the 8 times better than the current opt channel
                if ( pu8_bestChCnt[u8_prevSweepCh] > 10 )
                {
                    stru_sweepPower.u8_optCh = u8_prevSweepCh;
                    memset(pu8_bestChCnt, 0, u8_maxCh);
                }
            }
        }
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

/*
 *          ps16_averdiff   the average dbm * 10
 *          ps16_fluctdiff: the fluct dbm * 10
 *  return: the average dbm * 10
*/
int16_t compare_chNoisePower( ENUM_RF_BAND e_rfBand,
                              uint8_t u8_itCh1, uint8_t u8_itCh2,
                              int16_t *ps16_averdiff, int16_t *ps16_fluctdiff, 
                              uint8_t log)
{
    calc_average_and_fluct(e_rfBand, stru_sweepPower.e_bw, u8_itCh1 );
    calc_average_and_fluct(e_rfBand, stru_sweepPower.e_bw, u8_itCh2 );

    int * pi32_noisepower_average = (e_rfBand == RF_2G) ? stru_sweepPower.i32_noisepower_average :
                                                          stru_sweepPower.i32_5G_noisepower_average;
    int * pi32_variance = (e_rfBand == RF_2G) ? stru_sweepPower.i32_variance : 
                                                stru_sweepPower.i32_5G_variance;
    if ( pi32_noisepower_average[u8_itCh1] == 0 )
    {
        dlog_info("Error: stru_sweepPower.i32_noisepower_average[u8_itCh1]");
    }
    else
    {
        *ps16_averdiff = pi32_noisepower_average[u8_itCh2] * 10 / pi32_noisepower_average[u8_itCh1];
    }

    *ps16_fluctdiff = pi32_variance[u8_itCh2] - pi32_variance[u8_itCh1];
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
    uint8_t u8_start;
    int16_t aver, fluct;
    uint8_t num = BAND_SWEEP_CH(e_rfBand);

    for ( u8_start = 0; 
          u8_start < num && is_inlist( u8_start, pu8_exlude, u8_exclude_cnt );  //channel in the excluding list
          u8_start++) 
    {}

    uint8_t ch;
    for( ch = 0; ch < num; ch++)
    {
        if ( !is_inlist( ch, pu8_exlude, u8_exclude_cnt ) )
        {
            compare_chNoisePower(e_rfBand, ch, u8_start, &aver, &fluct, 0);
            //fluct[u8_start] - fluct[ch]
            if( (aver > 20) || (aver > 10 && fluct > 0))
            {
                u8_start = ch;
            }
        }
    }

    return u8_start;
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
    uint8_t  num ;
    int16_t  aver, fluct;
    uint32_t start = SysTicks_GetTickCount();
    uint8_t  exclude[6];
    ENUM_RF_BAND e_band = stru_sweepPower.e_curBand;

    if( !stru_sweepPower.u8_isFull )
    {
        return 0;
    }

    num = BAND_SWEEP_CH(e_band);
    for(ch = 0; ch < num; ch++)
    {
        calc_average_and_fluct(e_band, stru_sweepPower.e_bw, ch);
        if( log )
        {
            dlog_info("ch: %d %d %d %d %d %d %d", ch, stru_sweepPower.s16_power_average[0][ch], stru_sweepPower.s16_power_average[1][ch], stru_sweepPower.s16_power_average[2][ch],
                                                   stru_sweepPower.s16_power_average[3][ch], stru_sweepPower.s16_power_average[4][ch], stru_sweepPower.s16_power_average[5][ch]);
        }
    }

    uint8_t bestCh  = 0xff;         //select one from all the channels
    uint8_t betterCh= 0xff;
    uint8_t other   = 0xff;

    //select main channel
    if( e_opt == SELECT_MAIN_OPT )
    {
        bestCh = find_best(exclude, 0, e_band, log);
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

        if ( log )
        {
            dlog_info("exclude: %d %d %d", exclude[0], exclude[1], exclude[2]);
        }

        betterCh = find_best(exclude, 3, e_band, log);
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

        other = find_best(exclude, 3, e_band, log);

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
    uint8_t other, i;
    int16_t s16_cur_mainPower;
    int16_t s16_cur_power, s16_aver_power, s16_sum;
    uint8_t level;
    int16_t *ps16_slicePower;
    int16_t *ps16_power_average;
    int16_t s16_ch_per_row = BAND_SWEEP_CH(stru_sweepPower.e_curBand);

    BB_selectBestCh(SELECT_OTHER, NULL, NULL, &other, 0);

    {
        s16_sum = 0;
        //1st and last snr is useless.
        for(i = 1 ; (i - 1 ) < BW_10M_VALID_CH_CNT; i++)
        {
            ps16_slicePower = (stru_sweepPower.e_curBand == RF_2G) ? stru_sweepPower.s16_slicePower[stru_sweepPower.u8_curMainRow] : 
                                                                     stru_sweepPower.s16_5G_slicePower[stru_sweepPower.u8_curMainRow];
            s16_sum += ps16_slicePower[stru_sweepPower.u8_mainCh * 8 + i];
        }
        s16_cur_mainPower = s16_sum / BW_10M_VALID_CH_CNT;
    }

    {
        s16_sum = 0;
        for(i = 1 ; (i - 1 ) < BW_10M_VALID_CH_CNT; i++)
        {
            ps16_slicePower = (stru_sweepPower.e_curBand == RF_2G) ? stru_sweepPower.s16_slicePower[stru_sweepPower.u8_curOptRow] : 
                                                                     stru_sweepPower.s16_5G_slicePower[stru_sweepPower.u8_curOptRow];
            s16_sum += ps16_slicePower[stru_sweepPower.u8_optCh * 8 + i];
        }
        s16_cur_power = s16_sum / BW_10M_VALID_CH_CNT;
    }

    {
        s16_sum = 0;
        for(i = 0 ; i < SWEEP_FREQ_BLOCK_ROWS; i++)
        {
            ps16_power_average = (stru_sweepPower.e_curBand == RF_2G) ? stru_sweepPower.s16_power_average[i] : 
                                                                        stru_sweepPower.s16_5G_power_average[i];
            s16_sum += ps16_power_average[other];
        }
        s16_aver_power = s16_sum / SWEEP_FREQ_BLOCK_ROWS;
    }

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

    //dlog_info("diff:%d %d %x %d %d %d \n", stru_sweepPower.u8_curMainRow, stru_sweepPower.u8_curOptRow, context.agclevel,
    //                                 (s16_cur_power - s16_aver_power), (s16_cur_mainPower - s16_aver_power), level );

    if ( s16_cur_power - s16_aver_power >= level && s16_cur_mainPower - s16_aver_power >= level)
    {
        return other;
    }
    else
    {
        return stru_sweepPower.u8_optCh;
    }
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
static int calc_power_db(uint8_t bw, uint32_t power_td,
                         int16_t *power_fd, int16_t *power_db, uint8_t cnt, int flaglog)
{
    uint8_t  i = 0;

#if 1
    // calcation the power average by frequency-domain
    uint64_t sum_fd = 0;
    for(i = 0 ; i < cnt; i++)
    {
        sum_fd += ((uint64_t)power_fd[i] * power_fd[i]);
    }

    for(i = 0 ; i < cnt; i++)
    {
        int tmp = (uint64_t)power_fd[i] * power_fd[i] * power_td / sum_fd;
        tmp = get_10log10(tmp);
        
        tmp += ((tmp > 30) ? (-123) : (-125));
        power_db[i] = tmp;
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


/*
 *
*/
static void calc_average_and_fluct(ENUM_RF_BAND e_rfBand, uint8_t u8_bw , uint8_t u8_ItCh)
{
    uint8_t  cnt = ((u8_bw == BW_10M) ? BW_10M_VALID_CH_CNT : BW_20M_VALID_CH_CNT);
    uint16_t row = 0;

    //base on slice power.
    for( row = 0; row < SWEEP_FREQ_BLOCK_ROWS; row++)
    {
        uint8_t i = 0;
        int16_t sum_power = 0;

        int16_t *ps16_slicePower    = (e_rfBand == RF_2G) ? stru_sweepPower.s16_slicePower[row] : 
                                                            stru_sweepPower.s16_5G_slicePower[row];
        int16_t *ps16_power_average = (e_rfBand == RF_2G) ? stru_sweepPower.s16_power_average[row] : 
                                                            stru_sweepPower.s16_5G_power_average[row];
        for(i = 1; (i-1) < cnt; i++)
        {
            sum_power += ps16_slicePower[(u8_ItCh * 8) + i];
        }
        
        ps16_power_average[u8_ItCh] = (sum_power / cnt);   
    }

    //base on time-domain power    
    int *pu32_power_td;
    int *pu32_noisepower_average;
    int *pi32_variance;

    int u32_avr = 0;
    //add each row, get the average
    for( row = 0; row < SWEEP_FREQ_BLOCK_ROWS; row++)
    {
        pu32_power_td = (e_rfBand == RF_2G) ? stru_sweepPower.i32_noisePower[row] : 
                                              stru_sweepPower.i32_5G_noisePower[row];
        u32_avr += pu32_power_td[u8_ItCh];
    }
    u32_avr /= SWEEP_FREQ_BLOCK_ROWS;

    //3db limitation average
    pu32_noisepower_average = (e_rfBand == RF_2G) ? stru_sweepPower.i32_noisepower_average : 
                                                    stru_sweepPower.i32_5G_noisepower_average;    
    for( row = 0; row < SWEEP_FREQ_BLOCK_ROWS; row++)
    {
        pu32_power_td = (e_rfBand == RF_2G) ? stru_sweepPower.i32_noisePower[row] : 
                                              stru_sweepPower.i32_5G_noisePower[row];
        if (pu32_power_td[u8_ItCh] >= u32_avr * 2)
        {
            pu32_noisepower_average[u8_ItCh] += (u32_avr * 2);
        }
        else if (pu32_power_td[u8_ItCh] >= u32_avr / 2)
        {
            pu32_noisepower_average[u8_ItCh] += (u32_avr / 2);
        }
        else
        {
            pu32_noisepower_average[u8_ItCh] += pu32_power_td[u8_ItCh];
        }
    }
    pu32_noisepower_average[u8_ItCh] /= SWEEP_FREQ_BLOCK_ROWS;

    //calc virance
    pi32_variance = (e_rfBand == RF_2G) ? stru_sweepPower.i32_variance :
                                          stru_sweepPower.i32_5G_variance;
    pi32_variance[u8_ItCh] = 0;
    for( row = 0; row < SWEEP_FREQ_BLOCK_ROWS; row++)
    {
        int diff;
        if ( pu32_noisepower_average[u8_ItCh] > u32_avr)
        {
            diff = pu32_noisepower_average[u8_ItCh] - u32_avr;
        }
        else
        {
            diff = u32_avr - pu32_noisepower_average[u8_ItCh];
        }
        pi32_variance[u8_ItCh] += (diff * diff);
    }
}


