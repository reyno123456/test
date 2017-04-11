#ifndef _SWEEP_FREQ_SERVICE_H__
#define _SWEEP_FREQ_SERVICE_H__

#include <stdint.h>
#include "bb_ctrl_internal.h"

typedef enum _ENUM_CH_SEL_OPT
{
    SELECT_OPT      = 0,
    SELECT_MAIN_OPT = 1,
    SELECT_OTHER    = 2
}ENUM_CH_SEL_OPT;


#define MAIN_CH_CYCLE   (0)
#define OPT_CH_CYCLE    (1)
#define OTHER_CH_CYCLE  (2)


#define BAND_MAX_SWEEP_CH(band) ( (band == RF_2G)?  MAX_2G_IT_FRQ_SIZE : MAX_5G_IT_FRQ_SIZE )


void BB_SweepStart( ENUM_RF_BAND e_rfBand, ENUM_CH_BW e_bw);

uint8_t BB_GetSweepResult( uint8_t flag );

void BB_GetSweepNoise(uint8_t row, int16_t *ptr_noise_power);

int16_t compare_chNoisePower(uint8_t u8_itCh1, uint8_t u8_itCh2, 
                             int16_t *ps16_averdiff, int16_t *ps16_fluctdiff, uint8_t log);

uint8_t BB_selectBestCh(ENUM_CH_SEL_OPT e_opt, 
                        uint8_t *u8_mainCh, 
                        uint8_t *u8_optCh, 
                        uint8_t *u8_other, 
                        uint8_t log);


uint8_t BB_forceSweep( uint8_t opt );

int BB_Sweep_updateCh(uint8_t mainch);

int BB_set_sweepChannel( void );


uint8_t BB_Sweep_GetoptCh( void );

uint8_t get_opt_channel( void );
#endif

