#include "boardParameters.h"

#define ARCAST_BB_SKY_REGS_CNT     	(3)

#define ARCAST_BB_GRD_REGS_CNT      (3)

#define ARCAST_RF1_REGS_CNT     (1)

#define ARCAST_RF2_REGS_CNT     (1)


const STRU_BB_REG arcast_bb_sky_regs[ARCAST_BB_SKY_REGS_CNT] = 
{
    {0, 0xBE, 0x10}, //RF power
    {0, 0xCE, 0x10}, //RF power
    {2, 0x03, 0x00}  //1T2R
};

const STRU_BB_REG arcast_bb_grd_regs[ARCAST_BB_GRD_REGS_CNT] = 
{
    {0, 0xBE, 0x05},  //RF power
    {0, 0xCE, 0x05},  //RF power
    {2, 0x08, 0x52}   //1T2R
};


const STRU_RF_REG arcast_rf1_regs[ARCAST_RF1_REGS_CNT] =
{
    {0x33, 0x4c}
};


const STRU_RF_REG arcast_rf2_regs[ARCAST_RF2_REGS_CNT] =
{
    {0x33, 0x4c}
};



STRU_BoardCfg stru_boardCfg = 
{
    .u8_bbSkyRegsCnt 	= ARCAST_BB_SKY_REGS_CNT,
    .pstru_bbSkyRegs    = arcast_bb_sky_regs,
    
    .u8_bbGrdRegsCnt    = ARCAST_BB_GRD_REGS_CNT,
    .pstru_bbGrdRegs    = arcast_bb_grd_regs,

    .u8_rf8003Cnt 	    = 1,

    .u8_rf1RegsCnt 	    = ARCAST_RF1_REGS_CNT,
    .pstru_rf1Regs 	    = arcast_rf1_regs,
    
    .u8_rf2RegsCnt      = ARCAST_RF2_REGS_CNT,
    .pstru_rf2Regs      = arcast_rf2_regs
};

