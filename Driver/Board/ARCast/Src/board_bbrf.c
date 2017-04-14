#include "boardParameters.h"

#define ARCAST_BB_SKY_REGS_CNT     	(1)

#define ARCAST_BB_GRD_REGS_CNT      (3)

#define ARCAST_RF1_SKY_REGS_CNT     (0)

#define ARCAST_RF2_SKY_REGS_CNT     (0)


#define ARCAST_RF1_GRD_REGS_CNT     (5)

#define ARCAST_RF2_GRD_REGS_CNT     (5)


const STRU_BB_REG arcast_bb_sky_regs[ARCAST_BB_SKY_REGS_CNT] = 
{
    {2, 0x03, 0x00}  //1T2R
};

const STRU_BB_REG arcast_bb_grd_regs[ARCAST_BB_GRD_REGS_CNT] = 
{
    {0, 0xBE, 0x05},  //RF power
    {0, 0xCE, 0x05},  //RF power
    {2, 0x08, 0x52}   //1T2R
};

const STRU_RF_REG arcast_rf1_sky_regs[] =
{
};


const STRU_RF_REG arcast_rf2_sky_regs[] =
{
};

    

const STRU_RF_REG arcast_rf1_grd_regs[ARCAST_RF1_GRD_REGS_CNT] =
{
    {0x01, 0x04},
    {0x04, 0xc0},
    {0x05, 0x0c},
    {0x06, 0x80},
    {0x33, 0x4c},    
};


const STRU_RF_REG arcast_rf2_grd_regs[ARCAST_RF2_GRD_REGS_CNT] =
{
    {0x01, 0x04},
    {0x04, 0xc0},
    {0x05, 0x0c},
    {0x06, 0x80},
    {0x33, 0x4c}, 
};



STRU_BoardCfg stru_boardCfg = 
{
    .u8_bbSkyRegsCnt 	= ARCAST_BB_SKY_REGS_CNT,
    .pstru_bbSkyRegs    = arcast_bb_sky_regs,
    
    .u8_bbGrdRegsCnt    = ARCAST_BB_GRD_REGS_CNT,
    .pstru_bbGrdRegs    = arcast_bb_grd_regs,

    .u8_rf8003Cnt 	    = 1,

    .u8_rf1SkyRegsCnt   = ARCAST_RF1_SKY_REGS_CNT,
    .pstru_rf1SkyRegs   = arcast_rf1_sky_regs,
    
    .u8_rf2SkyRegsCnt   = ARCAST_RF2_SKY_REGS_CNT,
    .pstru_rf2SkyRegs   = arcast_rf2_sky_regs,

    .u8_rf1GrdRegsCnt   = ARCAST_RF1_GRD_REGS_CNT,
    .pstru_rf1GrdRegs   = arcast_rf1_grd_regs,
    
    .u8_rf2GrdRegsCnt   = ARCAST_RF2_GRD_REGS_CNT,
    .pstru_rf2GrdRegs   = arcast_rf2_grd_regs
};

