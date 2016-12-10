#include "bb_sys_param.h"


#define SYS_PARAM_START_FLASH_ADDR ((uint32_t)(0x0800FC00))
#define offsetof(TYPE, MEMBER) ((uint32_t) &((TYPE *)0)->MEMBER)
PARAM *sys_param;

#define SUPPORT_USER_SETTING (0)

uint8_t flash_setting[1024];

/* 
 *default_sys_param
*/
const uint8_t mask_freq[] = {19,30,50,71,91,122,153,190};

const SYS_PARAM default_sys_param =
{
     .usb_sel   = 0x00,
     .usb_cofig = 0x00,
     .freq_band_sel = AUTO,
     .it_mode       = 0x03,
     .qam_mode      = MOD_4QAM,
     .ldpc          = LDPC_1_2,
     .id_num        = 0x02,
     //.mimo_mode = _2T2R,
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
     //default no need mask, 
     .rc_mask = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, \
                 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, \
                 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, \
                 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},

     .it_mask = {0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff},

     //bpsk_ldpc12 ,qam4_ldpc12,qam4_ldpc23,qam16_ldpc12,qam64_ldpc12,qam64_ldpc23
     //3.8db,6.5db,7.7db,11.2db,16.2db,18db
     .qam_change_threshold = {0x009B,0x011E,0x0179,0x03F6,0x0AAA,0x0FC6},
     .enable_freq_offset   = DISABLE_FLAG,
     .rf_power_mode = MANUAL,
};


uint8_t BB_load_sys_param(uint32_t *p_data, uint32_t size)
{
    //return flash_read(SYS_PARAM_START_FLASH_ADDR,p_data,size);
}

uint8_t BB_fsave_sys_param(uint32_t *p_data,uint32_t size)
{
    //return flash_write(SYS_PARAM_START_FLASH_ADDR,p_data,size);
}

uint8_t BB_load_default_setting(PARAM *p_param)
{
    uint8_t i;
    uint8_t *pw,*pc;

    pw = (uint8_t *)((void *)((uint8_t *)p_param + offsetof(PARAM,user_param)));
    pc = (uint8_t *)((void *)(&default_sys_param));
    for(i=0;i<sizeof(SYS_PARAM);i++)
    {
        *pw = *pc;
        pw++;
        pc++;
    }
    
    return 0x00;
}

void BB_reback_to_fac_setting(void)
{
    uint8_t i;
    uint8_t *pw,*pc;
    SYS_PARAM *p_sys_param;

    pw = (uint8_t *)((void *)((uint8_t *)sys_param + offsetof(PARAM,user_param)));
    pc = (uint8_t *)((void *)(&default_sys_param));
    
    for(i=0;i<sizeof(SYS_PARAM);i++)
    {
        *pw = *pc;
        pw++;
        pc++;
    }

    p_sys_param = (SYS_PARAM *)(pw - sizeof(SYS_PARAM));

    #if 0
    memcpy(context.sp20dbm ,  p_sys_param->sp20dbm,  4);
    memcpy(context.sp20dbmb,  p_sys_param->sp20dbmb, 4);
    memcpy(context.gp20dbm ,  p_sys_param->gp20dbm,  4);
    memcpy(context.gp20dbmb,  p_sys_param->gp20dbmb, 4);
    #endif
}


void BB_disable_rc_freq(uint8_t *pMask)
{
    uint8_t i;
    for(i=0; i<sizeof(mask_freq); i++)
    {
        pMask[mask_freq[i]/8] &= (~(1 << (7 - mask_freq[i] % 8)));
    }
}


PARAM * BB_get_sys_param(void)
{
    BB_load_sys_param((uint32_t *)((void *)(flash_setting)), 1024);
    sys_param = (PARAM *)(flash_setting);

    #if(SUPPORT_USER_SETTING == 1)
        if(sys_param->is_init == 0xff)
    #endif
        {
            BB_load_default_setting(sys_param);
            sys_param->is_init = 0x00;
            BB_fsave_sys_param((uint32_t *)((void *)(sys_param)), 1024);
        }
        
        BB_disable_rc_freq(sys_param->user_param.rc_mask);
    
    return sys_param;
}
