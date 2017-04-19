#include <string.h>
#include "boardParameters.h"

#ifdef BBRF_2T4R
const STRU_BB_REG ek2T4R_bb_grd_regs[] =  //ground bb before 
{
    {2, 0x08, 0x72}  //2T4R
};


const STRU_RF_REG ek2T4R_rf1_grd_regs[] =  //ground rf1 before 
{
    {0x71, 0x04},   //40M clock for RF8003S_2
};


const STRU_BB_REG ek2T4R_bb_grd_regsAfterCali[] =  //ground bb after
{
    {1, 0x90, 0xF7}, //turn off DAC_B for 1TX
    {1, 0x91, 0xFF}, //4RX
};


const STRU_BB_REG ek2T4R_bb_skyregs_afterCali[] =     //sky bb after
{
    {1, 0x91, 0x78}, //2RX
    {2, 0x03, 0x80}  //2T
};


const STRU_RF_REG ek2T4R_rf1_grdregs_afterCali[] =   //ground rf1 after
{
    {0x35, 0x70},
    {0x45, 0x87},
    {0x00, 0x74}, //1Tx only
    {0x2D, 0xF6},
    {0x37, 0xE0}
};

const STRU_RF_REG ek2T4R_rf1_skyregs_afterCali[] =    //sky rf after
{
    {0x35, 0x70},
    {0x45, 0x87},
};

const STRU_RF_REG ek2T4R_rf2_grdregs_afterCali[] =   //ground rf2 after
{
    {0x35, 0x70},
    {0x45, 0x87},
    {0x00, 0x60}, //RX only for 2G
    {0x40, 0x60}, //RX only for 5G
    {0x2D, 0xF6},
    {0x37, 0xE0},
};


STRU_BoardCfg stru_boardCfg = 
{
    .u8_bbSkyRegsCnt 	= 0,
    .pstru_bbSkyRegs    = NULL,
    
    .u8_bbGrdRegsCnt    = sizeof(ek2T4R_bb_grd_regs) / sizeof(STRU_BB_REG),
    .pstru_bbGrdRegs    = ek2T4R_bb_grd_regs,

    .u8_rf8003Cnt 	    = 2,

    .u8_rf1SkyRegsCnt   = 0,
    .pstru_rf1SkyRegs   = NULL,

    .u8_rf1GrdRegsCnt   = sizeof(ek2T4R_rf1_grd_regs) / sizeof(STRU_RF_REG),
    .pstru_rf1GrdRegs   = ek2T4R_rf1_grd_regs,
    
    .u8_rf2GrdRegsCnt   = 0,
    .pstru_rf2GrdRegs   = NULL,

     //after calibration
    .u8_bbSkyRegsCntAfterCali 	 = sizeof (ek2T4R_bb_skyregs_afterCali) / sizeof (STRU_BB_REG),
    .pstru_bbSkyRegsAfterCali    = ek2T4R_bb_skyregs_afterCali,

    .u8_bbGrdRegsCntAfterCali    = sizeof(ek2T4R_bb_grd_regsAfterCali) / sizeof(STRU_BB_REG),
    .pstru_bbGrdRegsAfterCali    = ek2T4R_bb_grd_regsAfterCali,

    .u8_rf1SkyRegsCntAfterCali   = sizeof(ek2T4R_rf1_skyregs_afterCali) / sizeof(STRU_RF_REG),
    .pstru_rf1SkyRegsAfterCali   = ek2T4R_rf1_skyregs_afterCali,

    .u8_rf1GrdRegsCntAfterCali   = sizeof(ek2T4R_rf1_grdregs_afterCali) / sizeof(STRU_RF_REG),
    .pstru_rf1GrdRegsAfterCali   = ek2T4R_rf1_skyregs_afterCali,
    
    .u8_rf2GrdRegsCntAfterCali   = sizeof(ek2T4R_rf2_grdregs_afterCali) / sizeof(STRU_RF_REG),
    .pstru_rf2GrdRegsAfterCali   = ek2T4R_rf2_grdregs_afterCali,
};

#endif

#ifdef BBRF_2T2R

const STRU_BB_REG AR8020TEST_bb_regsAfterCali[] = 
{
    {1, 0x90, 0xF7}, //turn off DAC_B for 1TX
    {1, 0x91, 0x78}, //2RX
};


const STRU_RF_REG AR8020TEST_rf1_grdregs_afterCali[] = 
{
    {0x35, 0x70},
    {0x45, 0x87},
    {0x00, 0x74}, //1Tx only
    {0x2D, 0xF6},
    {0x37, 0xE0}
};

const STRU_RF_REG AR8020TEST_rf1_skyregs_afterCali[] = 
{
    {0x35, 0x70},
    {0x45, 0x87},
};


STRU_BoardCfg stru_boardCfg = 
{
    .u8_bbSkyRegsCnt 	= 0,
    .pstru_bbSkyRegs    = NULL,
    
    .u8_bbGrdRegsCnt    = 0,
    .pstru_bbGrdRegs    = NULL,

    .u8_rf8003Cnt 	    = 1,

    .u8_rf1SkyRegsCnt   = 0,
    .pstru_rf1SkyRegs   = NULL,
    
    .u8_rf1GrdRegsCnt   = 0,
    .pstru_rf1GrdRegs   = NULL,
    
    .u8_rf2GrdRegsCnt   = 0,
    .pstru_rf2GrdRegs   = NULL,

     //after calibration
    .u8_bbSkyRegsCntAfterCali 	 = sizeof(AR8020TEST_bb_regsAfterCali) / sizeof(STRU_BB_REG),
    .pstru_bbSkyRegsAfterCali    = AR8020TEST_bb_regsAfterCali,

    .u8_bbGrdRegsCntAfterCali    = sizeof(AR8020TEST_bb_regsAfterCali) / sizeof(STRU_BB_REG),
    .pstru_bbGrdRegsAfterCali    = AR8020TEST_bb_regsAfterCali,

    .u8_rf1SkyRegsCntAfterCali   = sizeof(AR8020TEST_rf1_skyregs_afterCali) / sizeof(STRU_RF_REG),
    .pstru_rf1SkyRegsAfterCali   = AR8020TEST_rf1_skyregs_afterCali,

    .u8_rf1GrdRegsCntAfterCali   = sizeof(AR8020TEST_rf1_grdregs_afterCali) / sizeof(STRU_RF_REG),
    .pstru_rf1GrdRegsAfterCali   = AR8020TEST_rf1_skyregs_afterCali,
    
    .u8_rf2GrdRegsCntAfterCali   = 0,
    .pstru_rf2GrdRegsAfterCali   = NULL,
};
#endif
