#include <string.h>
#include "boardParameters.h"

#define AR8020TEST_BB_SKY_REGS_CNT     	(0)

#define AR8020TEST_BB_GRD_REGS_CNT      (0)

#define AR8020TEST_RF1_SKY_REGS_CNT     (0)

#define AR8020TEST_RF2_SKY_REGS_CNT     (0)


#define AR8020TEST_RF1_GRD_REGS_CNT     (0)

#define AR8020TEST_RF2_GRD_REGS_CNT     (0)


const STRU_BB_REG AR8020TEST_bb_sky_regs[] = 
{
};

const STRU_RF_REG AR8020TEST_rf1_sky_regs[] =
{
};


const STRU_RF_REG AR8020TEST_rf2_sky_regs[] =
{
};


STRU_BoardCfg stru_boardCfg = 
{
    .u8_bbSkyRegsCnt 	= AR8020TEST_BB_SKY_REGS_CNT,
    .pstru_bbSkyRegs    = NULL,
    
    .u8_bbGrdRegsCnt    = AR8020TEST_BB_GRD_REGS_CNT,
    .pstru_bbGrdRegs    = NULL,

    .u8_rf8003Cnt 	    = 1,

    .u8_rf1SkyRegsCnt   = AR8020TEST_RF1_SKY_REGS_CNT,
    .pstru_rf1SkyRegs   = NULL,
    
    .u8_rf2SkyRegsCnt   = AR8020TEST_RF2_SKY_REGS_CNT,
    .pstru_rf2SkyRegs   = NULL,

    .u8_rf1GrdRegsCnt   = AR8020TEST_RF1_GRD_REGS_CNT,
    .pstru_rf1GrdRegs   = NULL,
    
    .u8_rf2GrdRegsCnt   = AR8020TEST_RF2_GRD_REGS_CNT,
    .pstru_rf2GrdRegs   = NULL
};

