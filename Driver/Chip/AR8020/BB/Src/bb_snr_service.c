#include <stdio.h>
#include <stdint.h>

#include "bb_snr_service.h"
#include "bb_ctrl_internal.h"
#include "bb_regs.h"
#include "debuglog.h"

#define WORK_FREQ_SNR_BLOCK_ROWS    (4)
#define WORK_FREQ_SNR_DAQ_CNT       (16)
#define WORK_FREQ_SNR_BLOCK_COLS    (WORK_FREQ_SNR_DAQ_CNT + 1) 
#define FAIL_TIMES_THLD             (3)

typedef struct
{
    uint16_t snr[WORK_FREQ_SNR_BLOCK_ROWS][WORK_FREQ_SNR_BLOCK_COLS];
    uint8_t row_index;
    uint8_t isFull;
}WORK_CH_SNR;

typedef struct
{
    uint16_t  u16_pieceSNR[WORK_FREQ_SNR_DAQ_CNT];
    uint8_t   u8_idx;
    uint8_t   u8_failCount0;
    uint8_t   u8_failCount1;
    //uint8_t   u8_result;
    //uint8_t   u8_Postflag;   // = 0: not need to do more checks in 14ms delay.
                             // = 1: need to do more checks in 14ms delay.
}STRU_pieceSNR;

STRU_pieceSNR stru_snr;

static WORK_CH_SNR work_ch_snr;

uint16_t get_snr_average(uint8_t row_index)
{
    return work_ch_snr.snr[row_index][WORK_FREQ_SNR_DAQ_CNT];
}

uint16_t calu_average(uint16_t *p_data, uint8_t len)
{
    uint8_t i;
    uint32_t sum = 0;
    for(i=0; i<len; i++)
    {
        sum += p_data[i];
    }

    return (uint16_t)(sum / len);
}

uint16_t grd_get_it_snr()
{
    uint8_t high = BB_ReadReg(PAGE2, SNR_REG_0);
    uint8_t low  = BB_ReadReg(PAGE2, SNR_REG_1);
    uint16_t snr = (((uint16_t)high) << 8) | low;
    
    static uint32_t cnt = 0;
    if(cnt++ > 5000)
    {
        cnt = 0;
        dlog_info("SNR: %0.4x", snr);
    }

    return snr;
}


/*
  * return type: 1:  snr check pass
  *              0:  snr check Fail
  *              2:  need to check in next cycle.
*/
int grd_check_piecewiseSnrPass(uint8_t u8_flag_start, uint16_t u16_thld)
{
    uint8_t ret = 0;

    if( u8_flag_start )
    {
        stru_snr.u8_idx        = 0;
        stru_snr.u8_failCount0 = 0;
        stru_snr.u8_failCount1 = 0;
    }

    uint8_t loc = BB_ReadReg(PAGE2, 0xc4) & 0x0F;
    uint8_t startaddr = 0xE0 + stru_snr.u8_idx * 2;
	uint8_t i = stru_snr.u8_idx;

    if(loc == WORK_FREQ_SNR_DAQ_CNT -2) //fix: the last one snr can't get at this time, get the snr in past 14ms
    {
        loc = (WORK_FREQ_SNR_DAQ_CNT - 1);
    }
    for(; i <= loc; i++)
    {
        stru_snr.u16_pieceSNR[stru_snr.u8_idx] = ((uint16_t)BB_ReadReg(PAGE2, startaddr) << 8) | (BB_ReadReg(PAGE2, startaddr + 1));
        startaddr += 2;
        uint8_t fail = (stru_snr.u16_pieceSNR[stru_snr.u8_idx] < u16_thld);
        if(stru_snr.u8_idx < WORK_FREQ_SNR_DAQ_CNT / 2)
        {
            if (stru_snr.u8_failCount0 < FAIL_TIMES_THLD)
            {
                if( fail )
                {
                    stru_snr.u8_failCount0 += 1;
                }
                else
                {
                    stru_snr.u8_failCount0 = 0;
                }
            }
        }
        else
        {
            if (stru_snr.u8_failCount1 < FAIL_TIMES_THLD)
            {
                if( fail )
                {
                    stru_snr.u8_failCount1 += 1;
                }
                else
                {
                    stru_snr.u8_failCount1 = 0;
                }                
            }
        }

        stru_snr.u8_idx++;
    }
    
    if( stru_snr.u8_failCount0 >= FAIL_TIMES_THLD && stru_snr.u8_failCount1 >= FAIL_TIMES_THLD )
    {
        ret = 0;
    }
    else if( stru_snr.u8_failCount0 < FAIL_TIMES_THLD || (stru_snr.u8_failCount1 + (WORK_FREQ_SNR_DAQ_CNT -1 - loc) < FAIL_TIMES_THLD))
    {
        ret = 1;
    }
    else
    {
        ret = 2;
    }

    if( stru_snr.u8_failCount0 > 0 || stru_snr.u8_failCount1 > 0 )
    {
        dlog_info("sliceSNR Loc:%d thld:0x%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x \n", loc, u16_thld,
                    stru_snr.u16_pieceSNR[0],  stru_snr.u16_pieceSNR[1],  stru_snr.u16_pieceSNR[2],  stru_snr.u16_pieceSNR[3],
                    stru_snr.u16_pieceSNR[4],  stru_snr.u16_pieceSNR[5],  stru_snr.u16_pieceSNR[6],  stru_snr.u16_pieceSNR[7],
                    stru_snr.u16_pieceSNR[8],  stru_snr.u16_pieceSNR[9],  stru_snr.u16_pieceSNR[10], stru_snr.u16_pieceSNR[11],
                    stru_snr.u16_pieceSNR[12], stru_snr.u16_pieceSNR[13], stru_snr.u16_pieceSNR[14], stru_snr.u16_pieceSNR[15]
                );

        dlog_info("Fail time %d %d %d ret=%d \n", u8_flag_start, stru_snr.u8_failCount0, stru_snr.u8_failCount1, ret);    
    }
    return ret;    
}


void grd_get_snr(void)
{
    uint8_t i;
    uint8_t startaddr = 0xE0;
    for(i = 0 ; i < WORK_FREQ_SNR_DAQ_CNT ;i ++)
    {
        work_ch_snr.snr[work_ch_snr.row_index][i] = ((uint16_t)BB_ReadReg(PAGE2, startaddr) << 8) | (BB_ReadReg(PAGE2, startaddr + 1));
        startaddr += 2;
    }
    work_ch_snr.snr[work_ch_snr.row_index][WORK_FREQ_SNR_DAQ_CNT] = calu_average(work_ch_snr.snr[work_ch_snr.row_index], WORK_FREQ_SNR_DAQ_CNT);

    //For test..
    {
        static int cnt = 0;
        if(cnt++ > 1000)
        {
            uint row = work_ch_snr.row_index;
            cnt = 0;
            uint16_t snr = (((uint16_t)BB_ReadReg(PAGE2, SNR_REG_0)) << 8) | BB_ReadReg(PAGE2, SNR_REG_1);
            uint8_t loc = BB_ReadReg(PAGE2, 0xc4) & 0x0F;
            dlog_info("SNR AVR: %0.4x ori:%0.4x LOC:%d--", work_ch_snr.snr[work_ch_snr.row_index][WORK_FREQ_SNR_DAQ_CNT], snr, loc);
            dlog_info("%x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x \n", work_ch_snr.snr[row][0],  work_ch_snr.snr[row][1],  work_ch_snr.snr[row][2],  work_ch_snr.snr[row][3],  
                                                                           work_ch_snr.snr[row][4],  work_ch_snr.snr[row][5],  work_ch_snr.snr[row][6],  work_ch_snr.snr[row][7],
                                                                           work_ch_snr.snr[row][8],  work_ch_snr.snr[row][9],  work_ch_snr.snr[row][10], work_ch_snr.snr[row][11],
                                                                           work_ch_snr.snr[row][12], work_ch_snr.snr[row][13], work_ch_snr.snr[row][14], work_ch_snr.snr[row][15]);
        }
    }

    work_ch_snr.row_index++;
    if(work_ch_snr.row_index >= WORK_FREQ_SNR_BLOCK_ROWS)
    {
        work_ch_snr.row_index = 0;
        work_ch_snr.isFull    = 1;
    }
}


/*
   * a coloum snr < threshold
   * if exist >= (two coloum snr < threshold, then snr is not ok
*/
uint8_t is_snr_ok(uint16_t iMCS)
{
    uint8_t i;
    uint8_t low_snr_cnt = 0;

    for(i=0; i< WORK_FREQ_SNR_DAQ_CNT; i++)
    {
        uint8_t j = 0;
        uint8_t cnt = 0;

        for( j=0; j < WORK_FREQ_SNR_BLOCK_ROWS; j++ )
        {
            if(work_ch_snr.snr[j][i] < iMCS)
            {
                cnt++;
            }
            else
            {
                break;
            }
        }

        if(cnt == WORK_FREQ_SNR_BLOCK_ROWS)
        {
            low_snr_cnt++;
        }
    }

    for(i=0; i< WORK_FREQ_SNR_BLOCK_ROWS; i++)
    {
        dlog_info("THR:%x| %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x %x", \
                       iMCS,
                       work_ch_snr.snr[i][0],  work_ch_snr.snr[i][1],  work_ch_snr.snr[i][2],  work_ch_snr.snr[i][3],  
                       work_ch_snr.snr[i][4],  work_ch_snr.snr[i][5],  work_ch_snr.snr[i][6],  work_ch_snr.snr[i][7],
                       work_ch_snr.snr[i][8],  work_ch_snr.snr[i][9],  work_ch_snr.snr[i][10], work_ch_snr.snr[i][11],
                       work_ch_snr.snr[i][12], work_ch_snr.snr[i][13], work_ch_snr.snr[i][14], work_ch_snr.snr[i][15]
            );
    }

    if (low_snr_cnt < 2)
    {
        dlog_info("SNR OK\n");
    }
    else
    {
        dlog_info("SNR Fail->Hop\n");
    }

    return (low_snr_cnt < 2);
}


/*
 1 : every row snr average is bigger then threshold
 2 : every row snr average is smaller then threshold
 0 : others
*/
uint8_t snr_static_for_qam_change(uint16_t threshod_left_section,uint16_t threshold_right_section)
{
    uint8_t i;
    uint8_t big = 0;
    uint8_t small = 0;

    if(!((work_ch_snr.isFull) && work_ch_snr.row_index == 0))
    {
        return 0xff;
    }
    for(i=0;i<WORK_FREQ_SNR_BLOCK_ROWS;i++)
    {
        if(work_ch_snr.snr[i][WORK_FREQ_SNR_DAQ_CNT] <= threshod_left_section)
        {
            small++;
        }
        else if(work_ch_snr.snr[i][WORK_FREQ_SNR_DAQ_CNT] >= threshold_right_section)
        {
            big++;
        }
    }

    if(big == WORK_FREQ_SNR_BLOCK_ROWS)
    {
        return 1;
    }

    if(small == WORK_FREQ_SNR_BLOCK_ROWS)
    {
        return 2;
    }

    return 0;
}
