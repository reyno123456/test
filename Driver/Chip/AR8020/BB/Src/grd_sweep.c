#include <stdint.h>
#include "grd_sweep.h"
#include "BB_ctrl.h"
#include "config_baseband_register.h"
#include "debuglog.h"

#define SWEEP_FREQ_BLOCK_ROWS    (4)

typedef struct
{
    uint32_t noise_energy[SWEEP_FREQ_BLOCK_ROWS][MAX_RC_FRQ_SIZE];
    uint32_t noise_energy_average[MAX_RC_FRQ_SIZE];
    uint32_t noise_energy_flucate[MAX_RC_FRQ_SIZE];
    uint8_t row_index;
    uint8_t isFull;
    uint8_t isInited;
}SWEEP_CH_NOISE_ENERGY;

SWEEP_CH_NOISE_ENERGY sweep_ch_noise_energy;

volatile uint8_t sweep_freq = 0;

uint32_t get_sweep_value(void)
{
    if(sweep_freq == 0)
    {
        return sweep_ch_noise_energy.noise_energy_average[MAX_RC_FRQ_SIZE - 1];
    }
    else
    {
        return sweep_ch_noise_energy.noise_energy_average[sweep_freq - 1];
    }
}

uint8_t get_sweep_channel(void)
{
    if(sweep_freq == 0)
    {
        return (MAX_RC_FRQ_SIZE-1);
    }
    else
    {
        return (sweep_freq - 1);
    }
}

void grd_set_next_sweep_freq(void)
{
    sweep_freq++;
    if(sweep_freq >= MAX_RC_FRQ_SIZE)
    {
        sweep_freq = 0;
    }

    BB_set_sweepfrq(sweep_freq);
}

void grd_sweep_freq_init(void)
{
    sweep_freq = 0;
    sweep_ch_noise_energy.isFull = 0;
    sweep_ch_noise_energy.isInited = 0;
    sweep_ch_noise_energy.row_index = 0;

    BB_set_sweepfrq(sweep_freq);
}

void clear_sweep_results(void)
{
    sweep_freq = 0;
    sweep_ch_noise_energy.row_index = 0;
}

uint32_t grd_get_it_sweep_noise_energy()
{
    uint32_t Energy_Low = BB_ReadReg(PAGE2, SWEEP_ENERGY_LOW);
    uint32_t Energy_Mid = BB_ReadReg(PAGE2, SWEEP_ENERGY_MID);
    uint32_t Energy_Hgh = BB_ReadReg(PAGE2, SWEEP_ENERGY_HIGH);
    
    return (Energy_Hgh<<16 | Energy_Mid<<8 | Energy_Low);
}

void grd_add_sweep_result(void)
{
    sweep_ch_noise_energy.noise_energy[sweep_ch_noise_energy.row_index][sweep_freq] = grd_get_it_sweep_noise_energy();

    if(sweep_freq >= MAX_RC_FRQ_SIZE - 1)
    {
        sweep_ch_noise_energy.row_index++;
        if(sweep_ch_noise_energy.row_index >= SWEEP_FREQ_BLOCK_ROWS)
        {
            sweep_ch_noise_energy.isFull = 1;
        }

        if(sweep_ch_noise_energy.row_index >= SWEEP_FREQ_BLOCK_ROWS)
        {
            sweep_ch_noise_energy.row_index = 0;
        }
    }
}

//sne short for "sweep_noise_energy"
void calu_sne_average_and_fluct(uint8_t ch)
{
    uint8_t i;
    uint32_t tmp,max_tmp;
    tmp = sweep_ch_noise_energy.noise_energy[0][ch];
    max_tmp = tmp;

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

    for(i=0;i<MAX_RC_FRQ_SIZE;i++)
    {
        calu_sne_average_and_fluct(i);
    }
    sweep_ch_noise_energy.isInited = 1;
}

uint8_t is_it_sweep_finish(void)
{
    return sweep_ch_noise_energy.isFull;
}

uint8_t is_init_sne_average_and_fluct(void)
{
    return sweep_ch_noise_energy.isInited;
}

uint8_t get_best_freq(void)
{
    uint32_t sne_average;
    uint8_t i;
    uint8_t ch;
    ch = 0;
    sne_average = sweep_ch_noise_energy.noise_energy_average[0];
    for(i=1;i<MAX_RC_FRQ_SIZE;i++)
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
    uint32_t sne_average;
    uint8_t i;
    uint8_t ch;
    ch = 0;
    sne_average = 0xffffffff;
    for(i=0;i<MAX_RC_FRQ_SIZE;i++)
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
    return sweep_freq;
}

uint8_t ch_to_index(uint8_t cur_ch)
{
    uint8_t i;
    for(i=0;i<MAX_RC_FRQ_SIZE;i++)
    {
        if(i == cur_ch)
        {
            return i;
        }
    }
    
    return 0xff;
}

uint8_t is_next_best_freq_pass(uint8_t cur_best_ch,uint8_t next_best_ch)
{
    uint8_t ret,i,cnt;
    uint8_t cur_ch_index,next_ch_index;

    ret = 0;
    cur_ch_index = ch_to_index(cur_best_ch);
    next_ch_index = ch_to_index(next_best_ch);

    if(next_ch_index == 0xff || cur_ch_index == 0xff)
    {
        return 0;
    }
    
    if(sweep_ch_noise_energy.noise_energy_average[cur_ch_index] >= \
      2*sweep_ch_noise_energy.noise_energy_average[next_ch_index])
    {
        cnt = 0;
        for(i=0;i<MAX_RC_FRQ_SIZE;i++)
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

        if(cnt == MAX_RC_FRQ_SIZE)
        {
            ret = 1;
        }
    }

    return ret;
}