#include <stdint.h>

#include "bb_snr_service.h"
#include "bb_ctrl_internal.h"
#include "bb_regs.h"
#include "debuglog.h"

#define WORK_FREQ_SNR_BLOCK_ROWS    (4) 
#define WORK_FREQ_SNR_DAQ_CNT       (8)
#define WORK_FREQ_SNR_BLOCK_COLS    (WORK_FREQ_SNR_DAQ_CNT + 1) 

typedef struct
{
    uint16_t snr[WORK_FREQ_SNR_BLOCK_ROWS][WORK_FREQ_SNR_BLOCK_COLS];
    uint8_t row_index;
    uint8_t isFull;
}WORK_CH_SNR;


WORK_CH_SNR work_ch_snr;

uint8_t snr_cnt = 0;

uint16_t get_snr_average(uint8_t row_index)
{
    return work_ch_snr.snr[row_index][WORK_FREQ_SNR_DAQ_CNT];
}

uint16_t calu_average(uint16_t *p_data,uint8_t len)
{
    uint32_t tmp;
    uint8_t i;
    if(len == 0)
    {
        return 0;
    }
    else if(len == 1)
    {
        return p_data[0];
    }
    else
    {
        tmp = p_data[0];
        for(i=1;i<len;i++)
        {
            tmp += p_data[i];
        }

        return (uint16_t)(tmp / len);
    }
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
    
void grd_add_snr_daq(void)
{
   work_ch_snr.snr[work_ch_snr.row_index][snr_cnt] = grd_get_it_snr();
   snr_cnt++;
   if(snr_cnt >= WORK_FREQ_SNR_DAQ_CNT)
   {
        work_ch_snr.snr[work_ch_snr.row_index][WORK_FREQ_SNR_DAQ_CNT] = \
            calu_average(work_ch_snr.snr[work_ch_snr.row_index],WORK_FREQ_SNR_DAQ_CNT);

        work_ch_snr.row_index++;
        snr_cnt = 0;
        
        if(!work_ch_snr.isFull)
        {
             if(work_ch_snr.row_index == WORK_FREQ_SNR_BLOCK_ROWS)
             {
                 work_ch_snr.isFull = 1;
             }
        }

        if(work_ch_snr.row_index == WORK_FREQ_SNR_BLOCK_ROWS)
        {
            work_ch_snr.row_index = 0;
        }
   }
}
/*
   * a coloum snr < threshold
   * if exist >= (two coloum snr < threshold, then snr is not ok
*/
uint8_t is_snr_ok(uint16_t iMCS)
{
    uint8_t i,j,cnt,low_snr_cnt;
    low_snr_cnt = 0;

    for(i=0;i<WORK_FREQ_SNR_DAQ_CNT;i++)
    {
        cnt = 0;
        for(j=0;j<WORK_FREQ_SNR_BLOCK_ROWS;j++)
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

    return low_snr_cnt < 2;
}

/*
 1 : every row snr average is bigger then threshold
 2 : every row snr average is smaller then threshold
 0 : others
*/
uint8_t snr_static_for_qam_change(uint16_t threshod_left_section,uint16_t threshold_right_section)
{
    uint8_t i;
    uint8_t big,small;
    big = 0;
    small = 0;

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

uint16_t get_snr_qam_threshold(void)
{
    uint8_t i;
    uint16_t tmp;

    tmp = work_ch_snr.snr[0][WORK_FREQ_SNR_DAQ_CNT];
    for(i=1;i<WORK_FREQ_SNR_BLOCK_ROWS;i++)
    {
        tmp += work_ch_snr.snr[i][WORK_FREQ_SNR_DAQ_CNT];
    }

    return (tmp / WORK_FREQ_SNR_BLOCK_ROWS);
}
