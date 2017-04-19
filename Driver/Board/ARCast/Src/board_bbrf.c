#include <string.h>
#include "boardParameters.h"


const STRU_BB_REG arcast_bb_regs_beforeCali[] = 
{
    {0, 0xBE, 0x05},  //RF power for PA
    {0, 0xCE, 0x05},  //RF power for PA
};

const STRU_RF_REG arcast_rf1_regs_beforeCali[] =
{
    //improve RX NF
    {0x01, 0x04},
    {0x04, 0xc0},
    {0x05, 0x0c},
    {0x06, 0x80},

    {0x33, 0x4c},       //internal PA: PA mode
};


const STRU_BB_REG arcast_bb_skyregs_afterCali[] = 
{
    {1, 0x90, 0xF7}, //turn off DAC_B
    {1, 0x91, 0x78}, //2RX
    {2, 0x03, 0x00}  //1T2R
};

const STRU_BB_REG arcast_bb_grdregs_afterCali[] = 
{
    {1, 0x90, 0xF7},  //turn off DAC_B
    {1, 0x91, 0x78},  //2RX    
    {2, 0x08, 0x52}   //1T2R
};


const STRU_RF_REG arcast_rf1_grdregs_afterCali[] = 
{
    {0x35, 0x70},
    {0x45, 0x87},
    {0x00, 0x74}, //1TX only
    {0x2D, 0xF6},
    {0x37, 0xE0}
};

const STRU_RF_REG arcast_rf1_skyregs_afterCali[] = 
{
    {0x35, 0x70},
    {0x45, 0x87},
    {0x00, 0x74}, //1TX only
    {0x2D, 0xF6},
    {0x37, 0xE0}
};


STRU_BoardCfg stru_boardCfg = 
{
    //before calibration
    .u8_bbSkyRegsCnt 	= sizeof(arcast_bb_regs_beforeCali) / sizeof(STRU_BB_REG),
    .pstru_bbSkyRegs    = arcast_bb_regs_beforeCali,
    
    .u8_bbGrdRegsCnt    = sizeof(arcast_bb_regs_beforeCali) / sizeof(STRU_BB_REG),
    .pstru_bbGrdRegs    = arcast_bb_regs_beforeCali,

    .u8_rf8003Cnt 	    = 1,

    .u8_rf1SkyRegsCnt   = sizeof(arcast_rf1_regs_beforeCali) / sizeof(STRU_RF_REG),
    .pstru_rf1SkyRegs   = arcast_rf1_regs_beforeCali,
    
    .u8_rf1GrdRegsCnt   = sizeof(arcast_rf1_regs_beforeCali) / sizeof(STRU_RF_REG),
    .pstru_rf1GrdRegs   = arcast_rf1_regs_beforeCali,
    
    .u8_rf2GrdRegsCnt   = 0,
    .pstru_rf2GrdRegs   = NULL,

    //after calibration
    .u8_bbSkyRegsCntAfterCali 	 = sizeof(arcast_bb_skyregs_afterCali) / sizeof(STRU_BB_REG),
    .pstru_bbSkyRegsAfterCali    = arcast_bb_skyregs_afterCali,
    
    .u8_bbGrdRegsCntAfterCali    = 3, //sizeof(arcast_bb_grdregs_afterCali) / sizeof(STRU_BB_REG),
    .pstru_bbGrdRegsAfterCali    = arcast_bb_grdregs_afterCali,

    .u8_rf1SkyRegsCntAfterCali   = sizeof(arcast_rf1_skyregs_afterCali)/sizeof(STRU_RF_REG),
    .pstru_rf1SkyRegsAfterCali   = arcast_rf1_skyregs_afterCali,
    
    .u8_rf1GrdRegsCntAfterCali   = sizeof(arcast_rf1_grdregs_afterCali)/sizeof(STRU_RF_REG),
    .pstru_rf1GrdRegsAfterCali   = arcast_rf1_grdregs_afterCali,
    
    .u8_rf2GrdRegsCntAfterCali   = 0,
    .pstru_rf2GrdRegsAfterCali   = NULL,
};

