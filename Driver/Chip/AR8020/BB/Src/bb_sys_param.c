#include "bb_sys_param.h"
#include <string.h>

#define SYS_PARAM_START_FLASH_ADDR  ((uint32_t)(0x0800FC00))

PARAM *sys_param;

#define SUPPORT_USER_SETTING (0)

uint8_t flash_setting[128];

/* 
 *default_sys_param
*/

const SYS_PARAM default_sys_param =
{
     .usb_sel   = 0x00,
     .usb_cofig = 0x00,
     .freq_band_sel = AUTO,
     .it_mode       = 0x03,
     .qam_mode      = MOD_4QAM,
     .ldpc          = LDPC_1_2,
     .id_num        = 0x02,
     .test_enable       = 0xff,
     .it_skip_freq_mode = AUTO,
     .rc_skip_freq_mode = AUTO,
     .search_id_enable  = 0xFF,
     .freq_band = RF_2G,
     .power     = 23,
     .gp20dbm   = {0x08,0x08,0x07,0x06},
     .sp20dbm   = {0x08,0x08,0x07,0x06},
     .gp20dbmb  = {0x08,0x08,0x07,0x06},
     .sp20dbmb  = {0x08,0x08,0x7,0x06},		 
     .qam_skip_mode = AUTO,
     .qam_change_threshold = { {0, 0x6A}, {0x54, 0x129}, {0xec, 0x388}, {0x2ce, 0xa3e}, {0x823, 0x1d94}, {0x12aa, 0xffff}},
     .enable_freq_offset   = DISABLE_FLAG,
     .rf_power_mode = MANUAL,
};


uint8_t BB_load_default_setting(PARAM *p_param)
{
    uint8_t i;
    uint8_t *pw,*pc;

    pw = (uint8_t *)((void *)((uint8_t *)p_param + offsetof(PARAM,user_param)));
    pc = (uint8_t *)((void *)(&default_sys_param));

    memcpy(pw, pc, sizeof(SYS_PARAM));

    return 0x00;
}



PARAM * BB_get_sys_param(void)
{
    sys_param = (PARAM *)(flash_setting);
    BB_load_default_setting(sys_param);
    
    return sys_param;
}
