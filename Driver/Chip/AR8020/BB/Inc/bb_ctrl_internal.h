#ifndef __BB_INTERNAL_CTRL_
#define __BB_INTERNAL_CTRL_

#include <stdint.h>

#include "bb_ctrl.h"
#include "bb_spi.h"

typedef enum
{
    IDLE,
    INIT_DATA,
    FEC_UNLOCK,
    FEC_LOCK,
    DELAY_14MS,
    CHECK_FEC_LOCK,
    ID_MATCH_LOCK,
    SEARCH_ID,
    CHECK_ID_MATCH
}DEVICE_STATE;


#define FALSE   (0)
#define TRUE    (1)


#define  BLUE_LED_GPIO      (67)
#define  RED_LED_GPIO       (71)


typedef struct _STRU_FRQ_CHANNEL           //  Remote Controller Freq Channnel
{
    uint8_t frq1;
    uint8_t frq2;
    uint8_t frq3;
    uint8_t frq4;
    uint8_t frq5;
}STRU_FRQ_CHANNEL;


typedef enum
{
    MISC_READ_RF_REG,
    MISC_WRITE_RF_REG,
    MISC_READ_BB_REG,
    MISC_WRITE_BB_REG,
} ENUM_WIRELESS_MISC_ITEM;


#define SFR_TRX_MODE_SEL            (*(volatile uint32_t *)0x40B00068)
#define SFR_TRX_MODE_GROUND         0x03
#define SFR_TRX_MODE_SKY            0x01



#define MAX_2G_RC_FRQ_SIZE (34)
#define MAX_2G_IT_FRQ_SIZE (6)

#define MAX_5G_RC_FRQ_SIZE (4)
#define MAX_5G_IT_FRQ_SIZE (4)

#define MAX(a,b) (((a) > (b)) ?  (a) :  (b) )


void BB_init(ENUM_BB_MODE en_mode);

void BB_uart10_spi_sel(uint32_t sel_dat);

int BB_softReset(ENUM_BB_MODE en_mode);

uint8_t BB_set_Rcfrq(ENUM_RF_BAND band, uint8_t ch);

uint8_t BB_set_ITfrq(ENUM_RF_BAND band, uint8_t ch);

uint8_t BB_set_sweepfrq(ENUM_RF_BAND band, uint8_t ch);

uint8_t BB_ReadReg(ENUM_REG_PAGES page, uint8_t addr);

uint8_t BB_WriteReg(ENUM_REG_PAGES page, uint8_t addr, uint8_t data);

uint8_t BB_map_modulation_to_br(uint8_t mod);

ENUM_BB_LDPC BB_get_LDPC(void);

ENUM_BB_QAM BB_get_QAM(void);

void BB_set_RF_Band(ENUM_BB_MODE sky_ground, ENUM_RF_BAND rf_band);

void BB_set_RF_bandwitdh(ENUM_BB_MODE sky_ground, ENUM_CH_BW rf_bw);

void BB_RF_2G_5G_switch(ENUM_RF_BAND rf_band);

int BB_GetCmd(STRU_WIRELESS_CONFIG_CHANGE *pconfig);

void BB_HandleEventsCallback(void *p);

void BB_handle_misc_cmds(STRU_WIRELESS_CONFIG_CHANGE* pcmd);

int BB_add_cmds(uint8_t type, uint32_t param0, uint32_t param1, uint32_t param2);

void BB_SetBoardMode(ENUM_BB_MODE en_mode);


void BB_set_QAM(ENUM_BB_QAM mod);

void BB_set_LDPC(ENUM_BB_LDPC ldpc);

uint8_t BB_write_ItRegs(uint32_t u32_it);

void sky_set_McsByIndex(uint8_t idx);

static int BB_before_RF_cali(void);

static int BB_after_RF_cali(void);

static int BB_RF_start_cali();

#endif
