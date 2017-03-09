#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "bb_snr_service.h"
#include "bb_ctrl_internal.h"
#include "bb_regs.h"
#include "systicks.h"
#include "debuglog.h"
#include "bb_sys_param.h"

#define WORK_FREQ_SNR_BLOCK_ROWS    (4)
#define WORK_FREQ_SNR_DAQ_CNT       (16)
#define WORK_FREQ_SNR_BLOCK_COLS    (WORK_FREQ_SNR_DAQ_CNT + 1) 
#define FAIL_TIMES_THLD             (3)


typedef struct
{
    uint16_t  u16_pieceSNR[WORK_FREQ_SNR_DAQ_CNT];
    uint8_t   u8_idx;
    uint8_t   u8_failCount0;
    uint8_t   u8_failCount1;
}STRU_pieceSNR;

STRU_pieceSNR stru_snr;

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

        dlog_info("SNR Fail=%d %d %d ret=%d \n", u8_flag_start, stru_snr.u8_failCount0, stru_snr.u8_failCount1, ret);    
    }
    return ret;    
}


//////////////////////////////////////////////////////
//// SNR use for the mcs.

#define MCS_SNR_BLOCK_ROWS              (8)
#define IT_SKIP_SNR_WINDOW_SIZE         (4)
#define SNR_CMP_CNT                     (200)

typedef struct
{
    uint16_t snr[MCS_SNR_BLOCK_ROWS];   
    uint8_t  snr_cmpResult[SNR_CMP_CNT];       //
    uint8_t  row_index;
    uint16_t u16_cmpCnt;
    uint8_t  isFull;
    uint16_t snr_avg;
    uint8_t  cur_index;
    uint16_t u16_upCount;
    uint16_t u16_downCount;
}STRU_MCS_WORK_CH_SNR;


uint8_t snr_cnt = 0;
uint8_t newest_4pack[IT_SKIP_SNR_WINDOW_SIZE];
uint8_t newest_4pack_index;


static STRU_MCS_WORK_CH_SNR work_ch_snr;

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


uint16_t grd_get_it_snr( void )
{
    uint16_t snr;
    snr = (((uint16_t)BB_ReadReg(PAGE2, SNR_REG_0)) << 8) | BB_ReadReg(PAGE2, SNR_REG_1);

    static uint32_t cnt = 0;
    if( cnt++ > 500 )
    {
        cnt = 0;
        dlog_info("SNR1:%0.4x\n", snr);
    }

    return snr;
}


uint16_t get_snr_average(void)
{
    return calu_average( work_ch_snr.snr, MCS_SNR_BLOCK_ROWS );
}


#define MCS1_DOWN_CONT_SNR          (10)
#define MCS1_DOWN_SNR               (50)
#define OTHER_MCS_DOWN_CONT_SNR     (10)

uint8_t count_num_inbuf(uint8_t *buf, uint16_t len, uint8_t data)
{
    uint8_t cnt = 0;
    uint8_t i = 0;
    for ( i = 0 ;i < len; i++ )
    {
        cnt += (( buf[i] == data) ? 1 : 0);
    }
    
    return cnt;
}

QAMUPDONW snr_static_for_qam_change(uint16_t threshod_left_section, uint16_t threshold_right_section)
{
    uint16_t aver;
    uint8_t ret;

    if ( !work_ch_snr.isFull )
    {
        return QAMKEEP;
    }

    if( work_ch_snr.u16_cmpCnt >= SNR_CMP_CNT )
    {
        work_ch_snr.u16_cmpCnt = 0;
    }

    aver = calu_average( work_ch_snr.snr, MCS_SNR_BLOCK_ROWS );
    if( aver > threshold_right_section )
    {
        work_ch_snr.u16_upCount ++;
        work_ch_snr.u16_downCount = 0;

        //dlog_info("Up: %d %d %x %x ", context.qam_ldpc, work_ch_snr.u16_upCount, aver, threshold_right_section);

        work_ch_snr.snr_cmpResult[ work_ch_snr.u16_cmpCnt ] = QAMUP;
        if( ( context.qam_ldpc == 0 && work_ch_snr.u16_upCount > 500 ) || 
            ( context.qam_ldpc != 0 && work_ch_snr.u16_upCount > 200 ) )
        {
            work_ch_snr.u16_upCount = 0;
            ret = QAMUP;        //up
        }
        else
        {
            ret = QAMKEEP;     //keep
        }
    }
    else if( aver < threshod_left_section )
    {
        uint8_t cnt = 0;
        work_ch_snr.u16_downCount ++;

        work_ch_snr.snr_cmpResult[ work_ch_snr.u16_cmpCnt ] = QAMDOWN;
        cnt = count_num_inbuf(work_ch_snr.snr_cmpResult, SNR_CMP_CNT, QAMDOWN);

        if (  ( context.qam_ldpc == 1 && work_ch_snr.u16_downCount >= MCS1_DOWN_CONT_SNR ) || 
              ( context.qam_ldpc == 1 && cnt >= MCS1_DOWN_SNR ) ||
              ( context.qam_ldpc != 1 && work_ch_snr.u16_downCount >= OTHER_MCS_DOWN_CONT_SNR ) )
        {
            memset( work_ch_snr.snr_cmpResult, QAMKEEP, SNR_CMP_CNT );
            work_ch_snr.u16_downCount = 0;
            ret = QAMDOWN;
            //dlog_info("Down: %d %d %d %x %x %d", context.qam_ldpc, work_ch_snr.u16_downCount, cnt, aver, threshod_left_section, ret);            
        }
        else
        {
            ret = QAMKEEP;
        }
    }
    else
    {
        work_ch_snr.u16_downCount = 0;
        work_ch_snr.u16_upCount = 0;
        work_ch_snr.snr_cmpResult[ work_ch_snr.u16_cmpCnt ] = QAMKEEP;
        ret = QAMKEEP;
    }
    
    work_ch_snr.u16_cmpCnt ++;
    return ret;
}


void arlink_snr_daq(void)
{
    if( work_ch_snr.row_index >= MCS_SNR_BLOCK_ROWS )
    {
        work_ch_snr.row_index = 0;
    }
    work_ch_snr.snr[work_ch_snr.row_index] = grd_get_it_snr();
    work_ch_snr.row_index ++;

    if( work_ch_snr.row_index == MCS_SNR_BLOCK_ROWS )
    {
        work_ch_snr.isFull = 1;
        work_ch_snr.row_index = 0;
    }
}


#define MOVE_AVERAGE_LEN (1000)
typedef struct
{
    uint8_t buffer[MOVE_AVERAGE_LEN];
    uint16_t index;
    uint32_t down_qam_retrans_cnt;
    uint32_t up_qam64_retrans_cnt;
    uint32_t up_otherqam_retrans_cnt;
    uint8_t down_qam_flag;
    uint8_t up_qam64_flag;
    uint8_t up_otherqam_flag;
    uint8_t last_value;
}MOVE_AVERAGE_BUF;


#define DOWN_QAM_WINDOW_SIZE        (16)
#define UP_OTHER_QAM_WINDOW_SIZE    (70)

void reset_move_avg_buf(MOVE_AVERAGE_BUF *p_move_avg_buf)
{
    p_move_avg_buf->index = 0;
    p_move_avg_buf->down_qam_retrans_cnt = 0;
    p_move_avg_buf->up_otherqam_retrans_cnt = 0;
    p_move_avg_buf->up_qam64_retrans_cnt = 0;
    p_move_avg_buf->down_qam_flag = 0;
    p_move_avg_buf->up_otherqam_flag = 0;
    p_move_avg_buf->up_qam64_flag = 0;
}


void update_move_avg_buf(MOVE_AVERAGE_BUF *p_move_avg_buf,uint8_t new_value)
{
    uint16_t tmp;

    if(p_move_avg_buf->down_qam_flag)
    {
        tmp = p_move_avg_buf->index >= DOWN_QAM_WINDOW_SIZE ? (p_move_avg_buf->index - DOWN_QAM_WINDOW_SIZE) : (MOVE_AVERAGE_LEN + p_move_avg_buf->index - DOWN_QAM_WINDOW_SIZE);
        p_move_avg_buf->down_qam_retrans_cnt -= p_move_avg_buf->buffer[tmp];
    }

    if(p_move_avg_buf->up_otherqam_flag)
    {
        tmp = p_move_avg_buf->index >= UP_OTHER_QAM_WINDOW_SIZE ? (p_move_avg_buf->index - UP_OTHER_QAM_WINDOW_SIZE) : (MOVE_AVERAGE_LEN + p_move_avg_buf->index - UP_OTHER_QAM_WINDOW_SIZE);
        p_move_avg_buf->up_otherqam_retrans_cnt -= p_move_avg_buf->buffer[tmp];
    }

    if(p_move_avg_buf->up_qam64_flag)
    {
        p_move_avg_buf->up_qam64_retrans_cnt -= p_move_avg_buf->buffer[p_move_avg_buf->index];
    }

    if(new_value > p_move_avg_buf->last_value)
    {
        p_move_avg_buf->buffer[p_move_avg_buf->index] = 1;//new_value - p_move_avg_buf->last_value;
    }
    else
    {
       p_move_avg_buf->buffer[p_move_avg_buf->index] = 0;
       new_value = 0;
    }

    p_move_avg_buf->down_qam_retrans_cnt += p_move_avg_buf->buffer[p_move_avg_buf->index];
    p_move_avg_buf->up_otherqam_retrans_cnt += p_move_avg_buf->buffer[p_move_avg_buf->index];
    p_move_avg_buf->up_qam64_retrans_cnt += p_move_avg_buf->buffer[p_move_avg_buf->index];

    p_move_avg_buf->last_value = new_value;
    p_move_avg_buf->index++;

    if(!p_move_avg_buf->down_qam_flag && p_move_avg_buf->index >= DOWN_QAM_WINDOW_SIZE )
    {
        p_move_avg_buf->down_qam_flag = TRUE;
    }

    if(!p_move_avg_buf->up_otherqam_flag && p_move_avg_buf->index >= UP_OTHER_QAM_WINDOW_SIZE )
    {
        p_move_avg_buf->up_otherqam_flag = TRUE;
    }

    if(!p_move_avg_buf->up_qam64_flag && p_move_avg_buf->index >= MOVE_AVERAGE_LEN )
    {
        p_move_avg_buf->up_qam64_flag = TRUE;
    }

    p_move_avg_buf->index %= MOVE_AVERAGE_LEN;

}
MOVE_AVERAGE_BUF retrans_buf;

void grd_set_txmsg_mcs_change(uint8_t index )
{
    BB_WriteReg(PAGE2, MCS_INDEX_MODE_0, index);
    BB_WriteReg(PAGE2, MCS_INDEX_MODE_1, index +1);

    dlog_info("MCS2=> 0x%x\n", index);
}


void up_down_qamldpc(QAMUPDONW up_down)
{
    if (context.qam_skip_mode == MANUAL)
    {
        dlog_info("-BRC MANUAL-");
        return;
    }

    if(QAMUP == up_down)
    {
        if(context.qam_ldpc == QAM_CHANGE_THRESHOLD_COUNT - 1)  //highest QAM ENUM_BB_LDPC mode
        {
            return;
        }
        context.qam_ldpc++;
    }
    else
    {
        if(context.qam_ldpc > 0)
        {
            context.qam_ldpc--;
        }
    }

    grd_set_txmsg_mcs_change( context.qam_ldpc );
}


uint8_t is_timeout(uint32_t start_value, uint32_t threshold)
{
    uint32_t cur = SysTicks_GetTickCount();
    if( cur >= start_value)
    {
        if( cur - start_value > threshold)
        {
            return TRUE;
        }
    }
    else
    {
        if(0xffffffff - (start_value - cur) > threshold)
        {
            return TRUE;
        }
    }

    return FALSE;
}

void grd_judge_qam_mode(void)
{
    QAMUPDONW qamupdown;
    uint16_t ldpc_err_num = (((uint16_t)BB_ReadReg(PAGE2, LDPC_ERR_HIGH_8)) << 8) | BB_ReadReg(PAGE2, LDPC_ERR_LOW_8);

    arlink_snr_daq();
    if( ldpc_err_num > 0 )
    {
        context.ldpc_error_move_cnt = 0;
    }
    else
    {
        if( context.ldpc_error_move_cnt < 100 )
        {
            context.ldpc_error_move_cnt++;
        }
    }
    uint8_t retrans_num = ((BB_ReadReg(PAGE2, FEC_5_RD)& 0xF0) >> 4);

    if(context.dev_state != FEC_LOCK)
    {
        reset_move_avg_buf(&retrans_buf);
        return;
    }

    update_move_avg_buf(&retrans_buf, retrans_num);
    qamupdown = snr_static_for_qam_change(context.qam_threshold_range[context.qam_ldpc][0], context.qam_threshold_range[context.qam_ldpc][1]);
    
    if(qamupdown == QAMKEEP)
    {
        return;
    }

    if(qamupdown == QAMUP)
    {
        if(context.qam_ldpc == 0)
        {
            if( retrans_buf.up_otherqam_retrans_cnt >= 35 )
            {
                dlog_info("MCS %d %d", context.qam_ldpc, retrans_buf.up_otherqam_retrans_cnt);
                return;
            }
        }
        else
        {
            if(context.ldpc_error_move_cnt < 70)
            {
                return;
            }

            #if 0
            //remove the harq reference
            if(retrans_buf.up_otherqam_retrans_cnt > 6 || (!retrans_buf.up_otherqam_flag))
            {
                return;
            }
            #endif

            if(!is_timeout(context.qam_switch_time, 2000))
            {
                return;
            }
        } 
    }

    context.qam_switch_time = SysTicks_GetTickCount();
    up_down_qamldpc(qamupdown);
}