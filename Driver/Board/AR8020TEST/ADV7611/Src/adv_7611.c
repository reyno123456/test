#include <stddef.h>
#include <stdint.h>
#include "data_type.h"
#include "adv_define.h"
#include "i2c.h"
#include "adv_7611.h"
#include "debuglog.h"
#include "sys_event.h"

static unsigned char hdmi_edid_table[][3] =
{
    {0x64, 0x77, 0x00},    //i2c repeater address 0x64
    {0x6c, 0x00, 0x00},    //i2c edid address 0x6c
    {0x6c, 0x01, 0xFF},
    {0x6c, 0x02, 0xFF},
    {0x6c, 0x03, 0xFF},
    {0x6c, 0x04, 0xFF},
    {0x6c, 0x05, 0xFF},
    {0x6c, 0x06, 0xFF},
    {0x6c, 0x07, 0x00},
    {0x6c, 0x08, 0x06},
    {0x6c, 0x09, 0x8F},
    {0x6c, 0x0A, 0x07},
    {0x6c, 0x0B, 0x11},
    {0x6c, 0x0C, 0x01},
    {0x6c, 0x0D, 0x00},
    {0x6c, 0x0E, 0x00},
    {0x6c, 0x0F, 0x00},
    {0x6c, 0x10, 0x17},
    {0x6c, 0x11, 0x11},
    {0x6c, 0x12, 0x01},
    {0x6c, 0x13, 0x03},
    {0x6c, 0x14, 0x80},
    {0x6c, 0x15, 0x0C},
    {0x6c, 0x16, 0x09},
    {0x6c, 0x17, 0x78},
    {0x6c, 0x18, 0x0A},
    {0x6c, 0x19, 0x1E},
    {0x6c, 0x1A, 0xAC},
    {0x6c, 0x1B, 0x98},
    {0x6c, 0x1C, 0x59},
    {0x6c, 0x1D, 0x56},
    {0x6c, 0x1E, 0x85},
    {0x6c, 0x1F, 0x28},
    {0x6c, 0x20, 0x29},
    {0x6c, 0x21, 0x52},
    {0x6c, 0x22, 0x57},
    {0x6c, 0x23, 0x00},
    {0x6c, 0x24, 0x00},
    {0x6c, 0x25, 0x00},
    {0x6c, 0x26, 0x01},
    {0x6c, 0x27, 0x01},
    {0x6c, 0x28, 0x01},
    {0x6c, 0x29, 0x01},
    {0x6c, 0x2A, 0x01},
    {0x6c, 0x2B, 0x01},
    {0x6c, 0x2C, 0x01},
    {0x6c, 0x2D, 0x01},
    {0x6c, 0x2E, 0x01},
    {0x6c, 0x2F, 0x01},
    {0x6c, 0x30, 0x01},
    {0x6c, 0x31, 0x01},
    {0x6c, 0x32, 0x01},
    {0x6c, 0x33, 0x01},
    {0x6c, 0x34, 0x01},
    {0x6c, 0x35, 0x01},
    {0x6c, 0x36, 0x8C},
    {0x6c, 0x37, 0x0A},
    {0x6c, 0x38, 0xD0},
    {0x6c, 0x39, 0x8A},
    {0x6c, 0x3A, 0x20},
    {0x6c, 0x3B, 0xE0},
    {0x6c, 0x3C, 0x2D},
    {0x6c, 0x3D, 0x10},
    {0x6c, 0x3E, 0x10},
    {0x6c, 0x3F, 0x3E},
    {0x6c, 0x40, 0x96},
    {0x6c, 0x41, 0x00},
    {0x6c, 0x42, 0x81},
    {0x6c, 0x43, 0x60},
    {0x6c, 0x44, 0x00},
    {0x6c, 0x45, 0x00},
    {0x6c, 0x46, 0x00},
    {0x6c, 0x47, 0x18},
    {0x6c, 0x48, 0x01},
    {0x6c, 0x49, 0x1D},
    {0x6c, 0x4A, 0x80},
    {0x6c, 0x4B, 0x18},
    {0x6c, 0x4C, 0x71},
    {0x6c, 0x4D, 0x1C},
    {0x6c, 0x4E, 0x16},
    {0x6c, 0x4F, 0x20},
    {0x6c, 0x50, 0x58},
    {0x6c, 0x51, 0x2C},
    {0x6c, 0x52, 0x25},
    {0x6c, 0x53, 0x00},
    {0x6c, 0x54, 0x81},
    {0x6c, 0x55, 0x49},
    {0x6c, 0x56, 0x00},
    {0x6c, 0x57, 0x00},
    {0x6c, 0x58, 0x00},
    {0x6c, 0x59, 0x9E},
    {0x6c, 0x5A, 0x00},
    {0x6c, 0x5B, 0x00},
    {0x6c, 0x5C, 0x00},
    {0x6c, 0x5D, 0xFC},
    {0x6c, 0x5E, 0x00},
    {0x6c, 0x5F, 0x56},
    {0x6c, 0x60, 0x41},
    {0x6c, 0x61, 0x2D},
    {0x6c, 0x62, 0x31},
    {0x6c, 0x63, 0x38},
    {0x6c, 0x64, 0x30},
    {0x6c, 0x65, 0x39},
    {0x6c, 0x66, 0x41},
    {0x6c, 0x67, 0x0A},
    {0x6c, 0x68, 0x20},
    {0x6c, 0x69, 0x20},
    {0x6c, 0x6A, 0x20},
    {0x6c, 0x6B, 0x20},
    {0x6c, 0x6C, 0x00},
    {0x6c, 0x6D, 0x00},
    {0x6c, 0x6E, 0x00},
    {0x6c, 0x6F, 0xFD},
    {0x6c, 0x70, 0x00},
    {0x6c, 0x71, 0x17},
    {0x6c, 0x72, 0x3D},
    {0x6c, 0x73, 0x0D},
    {0x6c, 0x74, 0x2E},
    {0x6c, 0x75, 0x11},
    {0x6c, 0x76, 0x00},
    {0x6c, 0x77, 0x0A},
    {0x6c, 0x78, 0x20},
    {0x6c, 0x79, 0x20},
    {0x6c, 0x7A, 0x20},
    {0x6c, 0x7B, 0x20},
    {0x6c, 0x7C, 0x20},
    {0x6c, 0x7D, 0x20},
    {0x6c, 0x7E, 0x01},
    {0x6c, 0x7F, 0x1C},
    {0x6c, 0x80, 0x02},
    {0x6c, 0x81, 0x03},
    {0x6c, 0x82, 0x34},
    {0x6c, 0x83, 0x71},
    {0x6c, 0x84, 0x4D},
    {0x6c, 0x85, 0x82},
    {0x6c, 0x86, 0x05},
    {0x6c, 0x87, 0x04},
    {0x6c, 0x88, 0x01},
    {0x6c, 0x89, 0x10},
    {0x6c, 0x8A, 0x11},
    {0x6c, 0x8B, 0x14},
    {0x6c, 0x8C, 0x13},
    {0x6c, 0x8D, 0x1F},
    {0x6c, 0x8E, 0x06},
    {0x6c, 0x8F, 0x15},
    {0x6c, 0x90, 0x03},
    {0x6c, 0x91, 0x12},
    {0x6c, 0x92, 0x35},
    {0x6c, 0x93, 0x0F},
    {0x6c, 0x94, 0x7F},
    {0x6c, 0x95, 0x07},
    {0x6c, 0x96, 0x17},
    {0x6c, 0x97, 0x1F},
    {0x6c, 0x98, 0x38},
    {0x6c, 0x99, 0x1F},
    {0x6c, 0x9A, 0x07},
    {0x6c, 0x9B, 0x30},
    {0x6c, 0x9C, 0x2F},
    {0x6c, 0x9D, 0x07},
    {0x6c, 0x9E, 0x72},
    {0x6c, 0x9F, 0x3F},
    {0x6c, 0xA0, 0x7F},
    {0x6c, 0xA1, 0x72},
    {0x6c, 0xA2, 0x57},
    {0x6c, 0xA3, 0x7F},
    {0x6c, 0xA4, 0x00},
    {0x6c, 0xA5, 0x37},
    {0x6c, 0xA6, 0x7F},
    {0x6c, 0xA7, 0x72},
    {0x6c, 0xA8, 0x83},
    {0x6c, 0xA9, 0x4F},
    {0x6c, 0xAA, 0x00},
    {0x6c, 0xAB, 0x00},
    {0x6c, 0xAC, 0x67},
    {0x6c, 0xAD, 0x03},
    {0x6c, 0xAE, 0x0C},
    {0x6c, 0xAF, 0x00},
    {0x6c, 0xB0, 0x10},
    {0x6c, 0xB1, 0x00},
    {0x6c, 0xB2, 0x88},
    {0x6c, 0xB3, 0x2D},
    {0x6c, 0xB4, 0x00},
    {0x6c, 0xB5, 0x00},
    {0x6c, 0xB6, 0x00},
    {0x6c, 0xB7, 0xFF},
    {0x6c, 0xB8, 0x00},
    {0x6c, 0xB9, 0x0A},
    {0x6c, 0xBA, 0x20},
    {0x6c, 0xBB, 0x20},
    {0x6c, 0xBC, 0x20},
    {0x6c, 0xBD, 0x20},
    {0x6c, 0xBE, 0x20},
    {0x6c, 0xBF, 0x20},
    {0x6c, 0xC0, 0x20},
    {0x6c, 0xC1, 0x20},
    {0x6c, 0xC2, 0x20},
    {0x6c, 0xC3, 0x20},
    {0x6c, 0xC4, 0x20},
    {0x6c, 0xC5, 0x20},
    {0x6c, 0xC6, 0x00},
    {0x6c, 0xC7, 0x00},
    {0x6c, 0xC8, 0x00},
    {0x6c, 0xC9, 0xFF},
    {0x6c, 0xCA, 0x00},
    {0x6c, 0xCB, 0x0A},
    {0x6c, 0xCC, 0x20},
    {0x6c, 0xCD, 0x20},
    {0x6c, 0xCE, 0x20},
    {0x6c, 0xCF, 0x20},
    {0x6c, 0xD0, 0x20},
    {0x6c, 0xD1, 0x20},
    {0x6c, 0xD2, 0x20},
    {0x6c, 0xD3, 0x20},
    {0x6c, 0xD4, 0x20},
    {0x6c, 0xD5, 0x20},
    {0x6c, 0xD6, 0x20},
    {0x6c, 0xD7, 0x20},
    {0x6c, 0xD8, 0x00},
    {0x6c, 0xD9, 0x00},
    {0x6c, 0xDA, 0x00},
    {0x6c, 0xDB, 0xFF},
    {0x6c, 0xDC, 0x00},
    {0x6c, 0xDD, 0x0A},
    {0x6c, 0xDE, 0x20},
    {0x6c, 0xDF, 0x20},
    {0x6c, 0xE0, 0x20},
    {0x6c, 0xE1, 0x20},
    {0x6c, 0xE2, 0x20},
    {0x6c, 0xE3, 0x20},
    {0x6c, 0xE4, 0x20},
    {0x6c, 0xE5, 0x20},
    {0x6c, 0xE6, 0x20},
    {0x6c, 0xE7, 0x20},
    {0x6c, 0xE8, 0x20},
    {0x6c, 0xE9, 0x20},
    {0x6c, 0xEA, 0x00},
    {0x6c, 0xEB, 0x00},
    {0x6c, 0xEC, 0x00},
    {0x6c, 0xED, 0x00},
    {0x6c, 0xEE, 0x00},
    {0x6c, 0xEF, 0x00},
    {0x6c, 0xF0, 0x00},
    {0x6c, 0xF1, 0x00},
    {0x6c, 0xF2, 0x00},
    {0x6c, 0xF3, 0x00},
    {0x6c, 0xF4, 0x00},
    {0x6c, 0xF5, 0x00},
    {0x6c, 0xF6, 0x00},
    {0x6c, 0xF7, 0x00},
    {0x6c, 0xF8, 0x00},
    {0x6c, 0xF9, 0x00},
    {0x6c, 0xFA, 0x00},
    {0x6c, 0xFB, 0x00},
    {0x6c, 0xFC, 0x00},
    {0x6c, 0xFD, 0x00},
    {0x6c, 0xFE, 0x00},
    {0x6c, 0xFF, 0xDA},
    {0x64, 0x77, 0x00},    //Set, the Most Significant Bit of the SPA location to 0
    {0x64, 0x52, 0x20},    //Set the SPA for port B.
    {0x64, 0x53, 0x00},    //Set the SPA for port B.
    {0x64, 0x70, 0x9E},    //Set the Least Significant Byte of the SPA location
    {0x64, 0x74, 0x03},    //Enable the Internal EDID for Ports
    {0xFF, 0xFF, 0xFF}     //End flag
};

static unsigned char hdmi_default_settings[][3] =
{
    /* Default settings: 1080p, SAV and EAV.
    /* 1080p Any Color Space In (YCrCb 444 24bit from ADV761x) Through HDMI Out 444 YCrCb VIC[16,31,32]: */
    {0x98, 0x01, 0x06},    //Prim_Mode =110b HDMI-GR
    {0x98, 0x02, 0xF5},    //Auto CSC, YCrCb out, Set op_656 bit
    {0x98, 0x03, 0x80},    //16 bit SDR 422 Mode 0
    {0x98, 0x05, 0x2C},    //AV Codes Off
    {0x98, 0x06, 0xAE},    //Invert VS,HS pins with clock and DE
    {0x98, 0x0B, 0x44},    //Power up part
    {0x98, 0x0C, 0x42},    //Power up part
    {0x98, 0x14, 0x7F},    //Max Drive Strength
    {0x98, 0x15, 0x80},    //Disable Tristate of Pins
    {0x98, 0x19, 0x83},    //LLC DLL phase
    {0x98, 0x33, 0x40},    //LLC DLL enable
    {0x44, 0xBA, 0x00},    //Set HDMI FreeRun Disbale
    {0x64, 0x40, 0x81},    //Disable HDCP 1.1 features
    {0x68, 0x9B, 0x03},    //ADI recommended setting
    {0x68, 0xC1, 0x01},    //ADI recommended setting
    {0x68, 0xC2, 0x01},    //ADI recommended setting
    {0x68, 0xC3, 0x01},    //ADI recommended setting
    {0x68, 0xC4, 0x01},    //ADI recommended setting
    {0x68, 0xC5, 0x01},    //ADI recommended setting
    {0x68, 0xC6, 0x01},    //ADI recommended setting
    {0x68, 0xC7, 0x01},    //ADI recommended setting
    {0x68, 0xC8, 0x01},    //ADI recommended setting
    {0x68, 0xC9, 0x01},    //ADI recommended setting
    {0x68, 0xCA, 0x01},    //ADI recommended setting
    {0x68, 0xCB, 0x01},    //ADI recommended setting
    {0x68, 0xCC, 0x01},    //ADI recommended setting
    {0x68, 0x00, 0x00},    //Set HDMI Input Port A
    {0x68, 0x83, 0xFE},    //Enable clock terminator for port A
    {0x68, 0x6F, 0x0C},    //ADI recommended setting
    {0x68, 0x85, 0x1F},    //ADI recommended setting
    {0x68, 0x87, 0x70},    //ADI recommended setting
    {0x68, 0x8D, 0x04},    //LFG
    {0x68, 0x8E, 0x1E},    //HFG
    {0x68, 0x1A, 0x8A},    //unmute audio
    {0x68, 0x57, 0xDA},    //ADI recommended setting
    {0x68, 0x58, 0x01},    //ADI recommended setting
    {0x68, 0x03, 0x98},    //DIS_I2C_ZERO_COMPR
    {0x68, 0x75, 0x10},    //DDC drive strength
    {0xFF, 0xFF, 0xFF}     //End flag
};

static unsigned char adv_i2c_addr_table[][3] =
{
    //{0x98, 0xFF, 0x80},                       //I2C reset
    {0x98, 0xF4, RX_I2C_CEC_MAP_ADDR},          //CEC
    {0x98, 0xF5, RX_I2C_INFOFRAME_MAP_ADDR},    //INFOFRAME
    {0x98, 0xF8, RX_I2C_AFE_DPLL_MAP_ADDR},     //DPLL
    {0x98, 0xF9, RX_I2C_REPEATER_MAP_ADDR},     //KSV
    {0x98, 0xFA, RX_I2C_EDID_MAP_ADDR},         //EDID
    {0x98, 0xFB, RX_I2C_HDMI_MAP_ADDR},         //HDMI
    {0x98, 0xFD, RX_I2C_CP_MAP_ADDR},           //CP
    {0xFF, 0xFF, 0xFF}                          //End flag
};

static STRU_ADV7611Format g_ADV7611SupportedFormatArray[] =
{
    {720,  480,  60},
    {1280, 720,  30},
    {1280, 720,  50},
    {1280, 720,  60},
    {1920, 1080, 30},
    {1920, 1080, 50},
    {1920, 1080, 60},
};

static STRU_ADV7611Status g_ADV7611Status = {0};

static void ADV_7611_Delay(unsigned int count)
{
    volatile unsigned int i = count;
    while (i--);
}

static void ADV_7611_I2CInitial(void)
{
    static uint8_t i2c_initialized = 0;
    if (i2c_initialized == 0)
    {
        I2C_Init(ADV_7611_I2C_COMPONENT_NUM, I2C_Master_Mode, RX_I2C_IO_MAP_ADDR >> 1, I2C_Fast_Speed);
        ADV_7611_Delay(100);
        ADV_7611_WriteByte(0x98, 0x1B, 0x01);
        ADV_7611_Delay(100);
        i2c_initialized = 1;
    }
}

void ADV_7611_WriteByte(uint8_t slv_addr, uint8_t sub_addr, uint8_t val)
{
    unsigned char sub_addr_tmp = sub_addr;
    unsigned char val_tmp = val;
    I2C_Master_Write_Data(ADV_7611_I2C_COMPONENT_NUM, slv_addr >> 1, &sub_addr_tmp, 1, &val_tmp, 1);
}

uint8_t ADV_7611_ReadByte(uint8_t slv_addr, uint8_t sub_addr)
{
    unsigned char sub_addr_tmp = sub_addr;
    unsigned char val = 0;
    I2C_Master_Read_Data(ADV_7611_I2C_COMPONENT_NUM, slv_addr >> 1, &sub_addr_tmp, 1, &val, 1);
    return val;
}

#define MAX_TABLE_ITEM_COUNT 2000
static void ADV_7611_WriteTable(uint8_t index, unsigned char(*reg_table)[3])
{
    unsigned int i = 0;
    unsigned char slv_addr_offset = (index == 0) ? 0 : 2; 
    
    while (i < MAX_TABLE_ITEM_COUNT)
    {
        if ((reg_table[i][0] == 0xFF) && (reg_table[i][1] == 0xFF) && (reg_table[i][2] == 0xFF))
        {
            break;
        }

        if ((reg_table[i][0] == 0) && (reg_table[i][1] == 0) && (reg_table[i][2] == 0))
        {
            break;
        }

        if (adv_i2c_addr_table == reg_table)
        {
            ADV_7611_WriteByte(reg_table[i][0] + slv_addr_offset, reg_table[i][1], reg_table[i][2] + slv_addr_offset);
        }
        else
        {
            ADV_7611_WriteByte(reg_table[i][0] + slv_addr_offset, reg_table[i][1], reg_table[i][2]);
        }
        
        i++;
    }
}

static void ADV_7611_GenericInitial(uint8_t index)
{
    ADV_7611_WriteTable(index, adv_i2c_addr_table);
    ADV_7611_WriteTable(index, hdmi_edid_table);
    ADV_7611_Delay(1000);
    ADV_7611_WriteTable(index, hdmi_default_settings);
}

static uint8_t ADV_7611_CheckVideoFormatSupportOrNot(uint32_t width, uint32_t hight, uint32_t framerate)
{
    uint8_t i = 0;
    uint8_t array_size = sizeof(g_ADV7611SupportedFormatArray)/sizeof(g_ADV7611SupportedFormatArray[0]);

    for (i = 0; i < array_size; i++)
    {
        if ((width == g_ADV7611SupportedFormatArray[i].width) &&
            (hight == g_ADV7611SupportedFormatArray[i].hight) &&
            (framerate == g_ADV7611SupportedFormatArray[i].framerate))
        {
            break;
        }
    }

    if (i < array_size)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static uint8_t ADV_7611_CheckVideoFormatChangeOrNot(uint8_t index, uint32_t width, uint32_t hight, uint32_t framerate)
{
    if (index >= 2)
    {
        return FALSE;
    }
    
    if ((g_ADV7611Status.video_format[index].width != width) ||
        (g_ADV7611Status.video_format[index].hight != hight) ||
        (g_ADV7611Status.video_format[index].framerate!= framerate))
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

static void ADV_7611_CheckFormatStatus(uint8_t index, uint8_t no_diff_check)
{
    static uint8_t format_not_support_count = 0;
    uint32_t width, hight, framerate;
    ADV_7611_GetVideoFormat(index, &width, &hight, &framerate);
    if (ADV_7611_CheckVideoFormatSupportOrNot(width, hight, framerate) == TRUE)
    {
        format_not_support_count = 0;
        if ((no_diff_check == TRUE) || (ADV_7611_CheckVideoFormatChangeOrNot(index, width, hight, framerate) == TRUE))
        {
            STRU_SysEvent_ADV7611FormatChangeParameter p;
            p.index = index;
            p.width = width;
            p.hight = hight;
            p.framerate = framerate;
            SYS_EVENT_Notify(SYS_EVENT_ID_ADV7611_FORMAT_CHANGE, (void*)&p);

            g_ADV7611Status.video_format[index].width = width;
            g_ADV7611Status.video_format[index].hight = hight;
            g_ADV7611Status.video_format[index].framerate = framerate;
        }
    }
    else
    {
        // Format not supported
        if (format_not_support_count <= FORMAT_NOT_SUPPORT_COUNT_MAX)
        {
            format_not_support_count++;
        }

        if (format_not_support_count == FORMAT_NOT_SUPPORT_COUNT_MAX)
        {
            STRU_SysEvent_ADV7611FormatChangeParameter p;
            p.index = index;
            p.width = 0;
            p.hight = 0;
            p.framerate = 0;
            SYS_EVENT_Notify(SYS_EVENT_ID_ADV7611_FORMAT_CHANGE, (void*)&p);

            g_ADV7611Status.video_format[index].width = 0;
            g_ADV7611Status.video_format[index].hight = 0;
            g_ADV7611Status.video_format[index].framerate = 0;
        }
    }
}

static void ADV_7611_IdleCallback(void *paramPtr)
{
    if (g_ADV7611Status.device_mask & ADV7611_0_DEVICE_ENABLE_MASK)
    {
        ADV_7611_CheckFormatStatus(0, FALSE);
    }
    
    if (g_ADV7611Status.device_mask & ADV7611_1_DEVICE_ENABLE_MASK)
    {
        ADV_7611_CheckFormatStatus(1, FALSE);
    }
}

void ADV_7611_Initial(uint8_t index)
{
    ADV_7611_I2CInitial();
    ADV_7611_GenericInitial(index);
    SYS_EVENT_RegisterHandler(SYS_EVENT_ID_IDLE, ADV_7611_IdleCallback);
    g_ADV7611Status.device_mask |= (1 << index);
    dlog_info("HDMI ADV7611 %d init finished!", index);
}

void ADV_7611_GetVideoFormat(uint8_t index, uint32_t* widthPtr, uint32_t* hightPtr, uint32_t* framteratePtr)
{
    uint32_t val1 = 0;
    uint32_t val2 = 0;
    uint32_t width = 0;
    uint32_t hight = 0;
    uint32_t frame_rate = 0;

    uint32_t bl_clk = 0;
    uint32_t hfreq = 0;
    uint32_t field0_hight = 0,  field1_hight = 0, field_hight = 0;
    uint32_t vfreq = 0;

    uint8_t hdmi_i2c_addr = (index == 0) ? RX_I2C_HDMI_MAP_ADDR : (RX_I2C_HDMI_MAP_ADDR + 2);
    uint8_t cp_i2c_addr = (index == 0) ? RX_I2C_CP_MAP_ADDR : (RX_I2C_CP_MAP_ADDR + 2);

    val1 = ADV_7611_ReadByte(hdmi_i2c_addr, 0x07) & 0x1F;
    val2 = ADV_7611_ReadByte(hdmi_i2c_addr, 0x08);
    width = (val1 << 8) + val2;
 
    val1 = ADV_7611_ReadByte(hdmi_i2c_addr, 0x09) & 0x1F;
    val2 = ADV_7611_ReadByte(hdmi_i2c_addr, 0x0a);
    hight = (val1 << 8) + val2;
    
    field0_hight = ((ADV_7611_ReadByte(hdmi_i2c_addr, 0x26) & 0x3f) << 8) | ADV_7611_ReadByte(hdmi_i2c_addr, 0x27);
    field1_hight = ((ADV_7611_ReadByte(hdmi_i2c_addr, 0x28) & 0x3f) << 8) | ADV_7611_ReadByte(hdmi_i2c_addr, 0x29);
    field_hight = (field0_hight + field1_hight) / 4;
    bl_clk = ((ADV_7611_ReadByte(cp_i2c_addr, 0xb1) & 0x3f) << 8) | ADV_7611_ReadByte(cp_i2c_addr, 0xb2);

    if ((field_hight != 0) && (bl_clk != 0))
    {
        hfreq = (ADV761x_CRYSTAL_CLK * 8) / bl_clk;
        vfreq = hfreq / field_hight;
        frame_rate = vfreq;
     }
     
     *widthPtr = width;
     *hightPtr = hight;
     *framteratePtr = frame_rate;
}

void ADV_7611_DumpOutEdidData(uint8_t index)
{
    dlog_info("Edid Data:");

    unsigned char slv_addr_offset = (index == 0) ? 0 : 2; 

    unsigned int i;
    unsigned char val = 0;
    for (i = 0; ; i++)
    {
        if ((hdmi_edid_table[i][0] == 0xFF) && (hdmi_edid_table[i][1] == 0xFF) && (hdmi_edid_table[i][2] == 0xFF))
        {
            break;
        }

        if ((hdmi_edid_table[i][0] == 0) && (hdmi_edid_table[i][1] == 0) && (hdmi_edid_table[i][2] == 0))
        {
            break;
        }

        val = ADV_7611_ReadByte(hdmi_edid_table[i][0] + slv_addr_offset, hdmi_edid_table[i][1]);
        
        if (val == hdmi_edid_table[i][2])
        {
            dlog_info("0x%x, 0x%x, 0x%x", hdmi_edid_table[i][0] + slv_addr_offset, hdmi_edid_table[i][1], val);
        }
        else
        {
            dlog_info("0x%x, 0x%x, 0x%x, Error: right value 0x%x!", hdmi_edid_table[i][0] + slv_addr_offset, hdmi_edid_table[i][1], val, hdmi_edid_table[i][2]);
        }
    }
}

void ADV_7611_DumpOutDefaultSettings(uint8_t index)
{
    unsigned int i;
    unsigned char val = 0;
    unsigned char slv_addr_offset = (index == 0) ? 0 : 2; 

    dlog_info("I2C Address Table:");
    for (i = 1; ; i++)
    {
        if ((adv_i2c_addr_table[i][0] == 0xFF) && (adv_i2c_addr_table[i][1] == 0xFF) && (adv_i2c_addr_table[i][2] == 0xFF))
        {
            break;
        }

        if ((adv_i2c_addr_table[i][0] == 0) && (adv_i2c_addr_table[i][1] == 0) && (adv_i2c_addr_table[i][2] == 0))
        {
            break;
        }

        val = ADV_7611_ReadByte(adv_i2c_addr_table[i][0] + slv_addr_offset, adv_i2c_addr_table[i][1]);
        
        if (val == (adv_i2c_addr_table[i][2] + slv_addr_offset))
        {
            dlog_info("0x%x, 0x%x, 0x%x", adv_i2c_addr_table[i][0] + slv_addr_offset, adv_i2c_addr_table[i][1], val);
        }
        else
        {
            dlog_info("0x%x, 0x%x, 0x%x, Error: right value 0x%x!", adv_i2c_addr_table[i][0] + slv_addr_offset, adv_i2c_addr_table[i][1], val, adv_i2c_addr_table[i][2] + slv_addr_offset);            
        }
    }
 
    dlog_info("Default Settings:"); 
    for (i = 0; ; i++)
    {
        if ((hdmi_default_settings[i][0] == 0xFF) && (hdmi_default_settings[i][1] == 0xFF) && (hdmi_default_settings[i][2] == 0xFF))
        {
            break;
        }

        if ((hdmi_default_settings[i][0] == 0) && (hdmi_default_settings[i][1] == 0) && (hdmi_default_settings[i][2] == 0))
        {
            break;
        }

        val = ADV_7611_ReadByte(hdmi_default_settings[i][0] + slv_addr_offset, hdmi_default_settings[i][1]);

        if (val == hdmi_default_settings[i][2])
        {
            dlog_info("0x%x, 0x%x, 0x%x", hdmi_default_settings[i][0] + slv_addr_offset, hdmi_default_settings[i][1], val);
        }
        else
        {
            dlog_info("0x%x, 0x%x, 0x%x, Error: right value 0x%x!", hdmi_default_settings[i][0] + slv_addr_offset, hdmi_default_settings[i][1], val, hdmi_default_settings[i][2]);
        }
    }
}

