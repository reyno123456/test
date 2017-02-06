/*****************************************************************************
 * Copyright: 2016-2020, Artosyn. Co., Ltd.
 * File name: can.c
 * Description: mipi drive function implementation 
 * Author: Artosyn FW
 * Version: V0.01 
 * Date: 2017.01.22
 * History: 
 * 2017.01.22 the first edition.
 * *****************************************************************************/

#include "mipi.h"
#include "reg_rw.h"
#include "debuglog.h"

static STRU_REG_VALUE MIPI_init_setting[] = 
{
    {0x00, 0x20}, // RX_control 
    {0x04, 0x10}, // Start size v0
    {0x08, 0x00}, // Control v0
    {0x0C, 0x04}, // Vsync counter end v0
    {0x10, 0x04}, // Vsync counter start v0
    {0x14, 0x10}, // Start size v1
    {0x18, 0x00}, // Control v1
    {0x1C, 0x04}, // Vsync counter end v1
    {0x20, 0x04}, // Vsync counter start v1
    {0x24, 0x44}, // virtual channel control
    {0x40, 0x0F}, // Frame Vsync control
    {0x80, 0x00}, // prbs test control
    {0x84, 0x00}, // clear error status [7:0]
    {0x88, 0x00}, // clear error status [15:8]
    {0x8C, 0x00}, // PRBS test counter [7:0]
    {0x90, 0x00}, // PRBS test counter [15:8]
    {0x94, 0x40}, // PRBS test counter [23:16]
    {0x98, 0x10}, // PRBS error counter
    {0x9C, 0xFF}, // Error interrupt enable [7:0]
    {0xA0, 0xFF}, // Error interrupt enable[15:8]
    {0xA4, 0x00}, // clear error status [17:16]
    {0xA8, 0x03}, // Error interrupt enable[17:16]
    {0xC0, 0x02}, // Error status [7:0]
    {0xC4, 0x40}, // Error status [15:8]
    {0xC8, 0x00}, // Error status [17:16]
    {0xCC, 0x00}, // PRBS test status
    {0xE0, 0x00}, // Pixel number [7:0]
    {0xE4, 0x0F}, // Pixel number [15:8]
    {0xE8, 0x38}, // line number [7:0]
    {0xEC, 0x04}, // line number [15:8]
    {0xF0, 0x00}, // Pixel number [7:0]
    {0xF4, 0x00}, // Pixel number [15:8]
    {0xF8, 0x00}, // line number [7:0]
    {0xFC, 0x00}, // line number [15:8]
    {0x100, 0xC3}, // mipi_phy_ctl1
    {0x104, 0x00}, // mipi_phy_ctl2
    {0x108, 0x00}, // pd_ivref_test_mode
    {0x10C, 0x00} // phy_bist_flag
};

static STRU_REG_VALUE ENCODER_init_setting[] = 
{
    {0x00, 0x01540736}, 
    {0x04, 0x07800438}, 
    {0x08, 0x011E4D28}, 
    {0x0C, 0x00000000}, 
    {0x10, 0x00006000}, 
    {0x14, 0x00030050}, 
    {0x18, 0x10300806}, 
    {0x1C, 0x00989680}, 
    {0x20, 0x04040000}, 
    {0x24, 0x00003907}, 
    {0x28, 0x2B02589E}, 
    {0x2C, 0x12469422}, 
    {0x30, 0x00FFFF00}, 
    {0x34, 0x00002E00}, 
    {0x38, 0x18004600}, 
    {0x3C, 0x20004E00}, 
    {0x40, 0x22005000}, 
    {0x44, 0x28005600}, 
    {0x48, 0x23230401}, 
    {0x4C, 0x000007BD}, 
    {0x50, 0x00000078}, 
    {0x54, 0x0000044D}, 
    {0x58, 0x0000E540}, 
    {0x5C, 0x000203D4}, 
    {0x60, 0x00000000}, 
    {0x64, 0x00170736}, 
    {0x68, 0x050002D0}, 
    {0x6C, 0x1E1E4D28}, 
    {0x70, 0x00000000}, 
    {0x74, 0xF0004000}, 
    {0x78, 0x00030050}, 
    {0x7C, 0x20300806}, 
    {0x80, 0x00989680}, 
    {0x84, 0x04040000}, 
    {0x88, 0x00003000}, 
    {0x8C, 0x2B0258FE}, 
    {0x90, 0x12469422}, 
    {0x94, 0x01FFFF00}, 
    {0x98, 0x5C008A00}, 
    {0x9C, 0x7400A200}, 
    {0xA0, 0x7C00AA00}, 
    {0xA4, 0x7E00AC00}, 
    {0xA8, 0x8400B200}, 
    {0xAC, 0x23230401}, 
    {0xB0, 0x00000000}, 
    {0xB4, 0x00000000}, 
    {0xB8, 0x00000000}, 
    {0xBC, 0x00000000}, 
    {0xC0, 0x00000000}, 
    {0xC4, 0x00000000}, 
    {0xC8, 0x18212931}, 
    {0xCC, 0x39414A08}, 
    {0xD0, 0x00000000}, 
    {0xD4, 0x00000000}, 
    {0xD8, 0x00000000}, 
    {0xDC, 0x00000000}, 
    {0xE0, 0x00066AA8}, 
    {0xE4, 0xA86A4102}, 
    {0xE8, 0x94260044}, 
    {0xEC, 0x18000000}, 
    {0xF0, 0x00000000}, 
    {0xF4, 0x00000000}, 
    {0xF8, 0x00000000}, 
    {0xFC, 0x00000000}
};

/**
* @brief    
* @param     
* @retval  
* @note    
*/
int32_t MIPI_Init(void)
{
    uint32_t u32_i;
    uint32_t *pu32_addr;
    uint32_t u32_data;
    uint32_t u32_setLen = sizeof(MIPI_init_setting) / sizeof(MIPI_init_setting[0]);
    STRU_REG_VALUE *pst_regValue = &(MIPI_init_setting[0]);

    for(u32_i = 0; u32_i < u32_setLen; u32_i++)
    {
        Reg_Write32(MIPI_BASE_ADDR + (pst_regValue[u32_i].u32_regAddr), 
                    pst_regValue[u32_i].u32_val);
    }
    dlog_info("mipi register init finished.");


    u32_setLen = sizeof(ENCODER_init_setting) / sizeof(ENCODER_init_setting[0]);
    pst_regValue = &(ENCODER_init_setting[0]);

    for(u32_i = 0; u32_i < u32_setLen; u32_i++)
    {
        Reg_Write32(ENCODER_BASE_ADDR + (pst_regValue[u32_i].u32_regAddr), 
                    pst_regValue[u32_i].u32_val);
    }
    dlog_info("encoder register init finished.");
}
