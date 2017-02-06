#include <stdio.h>
#include <stdint.h>
#include <string.h>

#include "debuglog.h"
#include "bb_sys_param.h"
#include "bb_ctrl_internal.h"
#include "bb_grd_sweep.h"
#include "bb_regs.h"

#define SWEEP_FREQ_BLOCK_ROWS           (4)

#define MAX_1_5M_CH                     (19)
#define BW_10M_VALID_1_5M_CH_START      (12)
#define BW_20M_VALID_1_5M_CH_START      (8)


#define BW_10M_VALID_CH_CNT     (MAX_1_5M_CH - BW_10M_VALID_1_5M_CH_START + 1)
#define BW_20M_VALID_CH_CNT     (MAX_1_5M_CH - BW_20M_VALID_1_5M_CH_START + 1)


#define TOTAL_SWEEP_CH (MAX_2G_IT_FRQ_SIZE + MAX_5G_IT_FRQ_SIZE)
#define BAND_MAX_SWEEP_CH(band) ((band == RF_2G)? MAX_2G_IT_FRQ_SIZE:MAX_5G_IT_FRQ_SIZE)

typedef struct
{
    int16_t noise_energy_1_5M[SWEEP_FREQ_BLOCK_ROWS][TOTAL_SWEEP_CH*8];      //1.5M bandwidth noise in channels.
    int16_t noise_energy[SWEEP_FREQ_BLOCK_ROWS][TOTAL_SWEEP_CH];

    int16_t noise_energy_average[TOTAL_SWEEP_CH];
    int16_t noise_energy_flucate[TOTAL_SWEEP_CH];

    uint8_t row_index;
    uint8_t isFull;
    uint8_t isInited;
    uint8_t sweep_ch;
}SWEEP_CH_NOISE_ENERGY;


SWEEP_CH_NOISE_ENERGY sweep_ch_noise_energy = {0};

void grd_sweep_freq_init(void)
{
    sweep_ch_noise_energy.sweep_ch  = 0;
    sweep_ch_noise_energy.isFull    = 0;
    sweep_ch_noise_energy.row_index = 0;
    sweep_ch_noise_energy.isInited  = 0;

    BB_set_sweepfrq(context.freq_band, sweep_ch_noise_energy.sweep_ch);
}


void grd_get_sweep_noise(uint8_t row, int16_t *ptr_noise_power)
{
    memcpy( (void *)ptr_noise_power, (void *)sweep_ch_noise_energy.noise_energy_1_5M[row], sizeof(sweep_ch_noise_energy.noise_energy_1_5M[row]) );
}

void grd_set_next_sweep_freq(void)
{
    uint8_t max_ch = BAND_MAX_SWEEP_CH(context.freq_band);

    sweep_ch_noise_energy.sweep_ch++;
    if(sweep_ch_noise_energy.sweep_ch >= max_ch)
    {
        sweep_ch_noise_energy.sweep_ch = 0;
    }

    BB_set_sweepfrq(context.freq_band, sweep_ch_noise_energy.sweep_ch);
}

void clear_sweep_results(void)
{
    sweep_ch_noise_energy.sweep_ch  = 0;
    sweep_ch_noise_energy.row_index = 0;
}


/*
  *  get the time domain noise energy
 */
static uint32_t grd_get_it_sweep_td_noise_energy(void)
{
    uint32_t Energy_Low = BB_ReadReg(PAGE2, SWEEP_ENERGY_LOW);
    uint32_t Energy_Mid = BB_ReadReg(PAGE2, SWEEP_ENERGY_MID);
    uint32_t Energy_Hgh = BB_ReadReg(PAGE2, SWEEP_ENERGY_HIGH);

    return ( (Energy_Hgh<<16) | (Energy_Mid<<8) | Energy_Low);
}


/*
 * adapt to AR8020 new sweep function
 * return 0: sweep fail. Do not switch to next sweep channel
 * return 1: sweep OK.
*/
int8_t grd_get_it_sweep_noise_energy(uint8_t bw, uint8_t row, uint8_t ch)
{
    uint8_t  num = 0;
    uint8_t  i   = 0;
    uint32_t total_noise = 0;
    uint16_t ch_fd_power[16];

    uint32_t power_td = grd_get_it_sweep_td_noise_energy();
    uint8_t  power_fd_addr = 0x60;

    if(power_td == 0)
    {
        return 0;
    }

    if(bw == BW_10M)
    {
        num = BW_10M_VALID_CH_CNT;
        power_fd_addr += (BW_10M_VALID_1_5M_CH_START * 2);
    }
    else
    {
        num = BW_20M_VALID_CH_CNT;
        power_fd_addr += (BW_20M_VALID_1_5M_CH_START * 2);
    }

    //Get the frequency domain power
    for(i = 0; i < num; i++)
    {
        ch_fd_power[i] = (((uint16_t)BB_ReadReg(PAGE3, power_fd_addr) << 8) | BB_ReadReg(PAGE3, power_fd_addr+1));
        power_fd_addr += 2;

        if(ch_fd_power[i] == 0)
        {
            return 0;
        }
    }

    //dlog_info("%.4x %.4x %.4x %.4x %.4x %.4x %.4x %.4x %.4x\r\n", ch_fd_power[0], ch_fd_power[1], ch_fd_power[2],
    //                                                          ch_fd_power[3], ch_fd_power[4], ch_fd_power[5],
    //                                                          ch_fd_power[6], ch_fd_power[7], power_td);
    //get the dbm noise power
    uint16_t start_idx = (uint16_t)ch*8;
    return calc_power_db(bw, power_td, ch_fd_power, 
                         &(sweep_ch_noise_energy.noise_energy_1_5M[row][start_idx]),
                         &(sweep_ch_noise_energy.noise_energy[row][ch])
                         );
}


/*
 * retval :  1:  sweep OK
 *             0:  sweep fail
 */
int8_t grd_add_sweep_result(int8_t bw)
{
    uint8_t row = sweep_ch_noise_energy.row_index;
    uint8_t ch  = sweep_ch_noise_energy.sweep_ch;
    uint8_t result = grd_get_it_sweep_noise_energy(bw, row, ch);
    uint8_t max_ch = BAND_MAX_SWEEP_CH(context.freq_band);

    if(result == 1 && ch >= max_ch - 1)
    {
        sweep_ch_noise_energy.row_index++;
        if(sweep_ch_noise_energy.row_index >= SWEEP_FREQ_BLOCK_ROWS)
        {
            sweep_ch_noise_energy.row_index = 0;
            sweep_ch_noise_energy.isFull = 1;
        }
    }

    return result;
}


//sne short for "sweep_noise_energy"
void calu_sne_average_and_fluct(uint8_t ch)
{
    uint8_t i;
    int16_t tmp = sweep_ch_noise_energy.noise_energy[0][ch];
    int16_t max_tmp = tmp;

    for(i=1;i<SWEEP_FREQ_BLOCK_ROWS;i++)
    {
        tmp += sweep_ch_noise_energy.noise_energy[i][ch];
        max_tmp = MAX(max_tmp,sweep_ch_noise_energy.noise_energy[i][ch]);
    }

    sweep_ch_noise_energy.noise_energy_average[ch] = tmp / SWEEP_FREQ_BLOCK_ROWS;
    sweep_ch_noise_energy.noise_energy_flucate[ch] = max_tmp - sweep_ch_noise_energy.noise_energy_average[ch];
}

void init_sne_average_and_fluct(void)
{
    uint8_t i;
    for(i=0; i< TOTAL_SWEEP_CH; i++)
    {
        calu_sne_average_and_fluct(i);
    }
    sweep_ch_noise_energy.isInited = 1;
}

uint8_t is_init_sne_average_and_fluct(void)
{
    return sweep_ch_noise_energy.isInited;
}

uint8_t is_it_sweep_finish(void)
{
    return sweep_ch_noise_energy.isFull;
}

uint8_t get_best_freq(void)
{
    int16_t sne_average;
    uint8_t i  = 0;
    uint8_t ch = 0;
    uint8_t max_ch = BAND_MAX_SWEEP_CH(context.freq_band);

    sne_average = sweep_ch_noise_energy.noise_energy_average[0];
    for(i=1; i< max_ch; i++)
    {
        if(sne_average > sweep_ch_noise_energy.noise_energy_average[i])
        {
            sne_average = sweep_ch_noise_energy.noise_energy_average[i];
            ch = i;
        }
    }
    return ch;
}


uint8_t get_next_best_freq(uint8_t cur_best_ch)
{
    int16_t sne_average;
    uint8_t i;
    uint8_t ch = 0;

    sne_average = 0xff;

    for(i=0; i< BAND_MAX_SWEEP_CH(context.freq_band); i++)
    {
        if(i == cur_best_ch)
        {
            continue;
        }
        if(sne_average > sweep_ch_noise_energy.noise_energy_average[i])
        {
            sne_average = sweep_ch_noise_energy.noise_energy_average[i];
            ch = i;
        }
    }
    return ch;
}

uint8_t get_sweep_freq(void)
{
    return sweep_ch_noise_energy.sweep_ch;
}

uint8_t ch_to_index(uint8_t cur_IT_ch)
{
    return cur_IT_ch;
}

uint8_t is_next_best_freq_pass(uint8_t cur_best_ch, uint8_t next_best_ch)
{
    uint8_t ret,i,cnt;
    uint8_t cur_ch_index, next_ch_index;

    ret = 0;
    cur_ch_index  = ch_to_index(cur_best_ch);
    next_ch_index = ch_to_index(next_best_ch);

    if(next_ch_index == 0xff || cur_ch_index == 0xff)
    {
        return 0;
    }

    if(sweep_ch_noise_energy.noise_energy_average[cur_ch_index] >= \
		sweep_ch_noise_energy.noise_energy_average[next_ch_index] + 3)
    {
        cnt = 0;
        for(i=0; i< BAND_MAX_SWEEP_CH(context.freq_band); i++)
        {
            if(sweep_ch_noise_energy.noise_energy_flucate[next_ch_index] <= sweep_ch_noise_energy.noise_energy_flucate[i])
            {
                cnt++;
            }
            else
            {
                break;
            }
        }

        if(cnt == BAND_MAX_SWEEP_CH(context.freq_band))
        {
            ret = 1;
        }
    }

    return ret;
}

static int8_t log10_lookup_table[]= {
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

int16_t get_10log10(uint16_t data)
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
 * power_td: total power in time domain
 * power_fd: each 1.56M bandwidth noise 
 * power_db: the power in dbm in each 1.56M
*/
int16_t calc_power_db(int8_t bw, uint32_t power_td, 
                      int16_t *power_fd, int16_t *power_db, 
                      int16_t *total_power)
{
    uint8_t  i = 0;
    uint64_t sum_fd = 0;
    uint8_t  cnt = ((bw == BW_10M) ? BW_10M_VALID_CH_CNT : BW_20M_VALID_CH_CNT);
    int16_t  sum_power = 0;
    int16_t  power[8];
    
    for(i = 0 ; i < cnt; i++)
    {
        sum_fd += ((uint64_t)power_fd[i] * power_fd[i]);
    }
    if(sum_fd == 0)
    {
        return 0;
    }
    for(i = 0 ; i < cnt; i++)
    {
        int16_t tmp = (uint64_t)power_fd[i] * power_fd[i] * power_td / sum_fd;
		tmp = get_10log10(tmp);
        if( tmp > 30)
        {
            tmp -= 123;
        }
        else
        {
            tmp -= 125;
        }   
        power_db[i] = tmp;
        sum_power += tmp;
    }

    
    *total_power = sum_power / cnt;
    return 1;
}
