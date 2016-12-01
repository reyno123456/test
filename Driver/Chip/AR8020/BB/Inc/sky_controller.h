#ifndef __SKY_CONTROLLER_H
#define __SKY_CONTROLLER_H

#include <stdint.h>
#include "BB_ctrl.h"

enum EN_AGC_MODE
{
    FAR_AGC     = 0,
    NEAR_AGC    = 1,
    UNKOWN_AGC  = 0xff,
};

void Sky_Parm_Initial(void);

void sky_Timer1_Init(void);

void sky_Timer0_Init(void);

void sky_id_search_init(void);

void sky_get_id(uint8_t* idptr);

void sky_set_id(uint8_t *idptr);

void sky_physical_link_process(void);

void Sky_Adjust_AGCGain(void);

void wimax_vsoc_rx_isr(void);

uint8_t sky_id_search_run(void);

void sky_agc_gain_toggle(void);

uint8_t* sky_id_search_get_best_id(void);

void sky_set_ITQAM_and_notify(uint8_t mod);

int sky_search_id_timeout_irq_disable();

int sky_search_id_timeout_irq_enable();

void sky_handle_all_spi_cmds(void);

#endif
