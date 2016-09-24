#include <stdlib.h>
#include "debuglog.h"
#include "i2c.h"
#include "adv_7611.h"

#define TAR_24C256_ADDR       0x51
#define TAR_ADV7611_ADDR      (0x98 >> 1)

#define I2C_COMPONENT_NUM  I2C_Component_0
#define ADV_7611_I2C_COMPONENT_NUM I2C_Component_0

#define RX_I2C_IO_MAP_ADDR              (0x98 >> 1)
#define RX_I2C_CP_MAP_ADDR              (0x44 >> 1)
#define RX_I2C_VDP_MAP_ADDR             (0x48 >> 1)
#define RX_I2C_AFE_DPLL_MAP_ADDR        (0x4C >> 1)
#define RX_I2C_REPEATER_MAP_ADDR        (0x64 >> 1)
#define RX_I2C_HDMI_MAP_ADDR            (0x68 >> 1)
#define RX_I2C_EDID_MAP_ADDR            (0x6C >> 1)
#define RX_I2C_ESDP_MAP_ADDR            (0x70 >> 1)
#define RX_I2C_DPP_MAP_ADDR             (0x78 >> 1)
#define RX_I2C_AVLINK_MAP_ADDR          (0x84 >> 1)
#define RX_I2C_CEC_MAP_ADDR             (0x80 >> 1)
#define RX_I2C_INFOFRAME_MAP_ADDR       (0x7C >> 1)
#define RX_I2C_TEST_MAP1_ADDR           (0x60 >> 1)
#define RX_I2C_SDP_MAP_ADDR             (0x90 >> 1)
#define RX_I2C_SDP_IO_MAP_ADDR          (0x94 >> 1)

static void ADV_7611_WriteByte(unsigned char slv_addr, unsigned char sub_addr, unsigned char val)
{
    unsigned char sub_addr_tmp = sub_addr;
    unsigned char val_tmp = val;
    I2C_Master_Write_Data(ADV_7611_I2C_COMPONENT_NUM, slv_addr >> 1, &sub_addr_tmp, 1, &val_tmp, 1);
}

static unsigned char ADV_7611_ReadByte(unsigned char slv_addr, unsigned char sub_addr)
{
    unsigned char sub_addr_tmp = sub_addr;
    unsigned char val = 0;
    I2C_Master_Read_Data(ADV_7611_I2C_COMPONENT_NUM, slv_addr >> 1, &sub_addr_tmp, 1, &val, 1);
    return val;
}

void command_readSDV7611(char *slvAddr, char *regAddr)
{
    unsigned char slvAddrTmp = strtoul(slvAddr, NULL, 0);
    unsigned char regAddrTmp = strtoul(regAddr, NULL, 0);
    unsigned char val = ADV_7611_ReadByte(slvAddrTmp, regAddrTmp);
    dlog_info("Read: 0x%x, 0x%x. Val: 0x%x", slvAddrTmp, regAddrTmp, val);
}

void command_writeSDV7611(char *slvAddr, char *regAddr, char *regVal)
{
    unsigned char slvAddrTmp = strtoul(slvAddr, NULL, 0);
    unsigned char regAddrTmp = strtoul(regAddr, NULL, 0);
    unsigned char regValTmp  = strtoul(regVal, NULL, 0);
    ADV_7611_WriteByte(slvAddrTmp, regAddrTmp, regValTmp);
    dlog_info("Wrte: 0x%x, 0x%x, 0x%x", slvAddrTmp, regAddrTmp, regValTmp);
}

void command_readADV7611VideoFormat(uint32_t* widthPtr, uint32_t* hightPtr, uint32_t* framteratePtr)
{
    ADV_7611_GetVideoFormat(widthPtr, hightPtr, framteratePtr);
}

void command_initADV7611(void)
{
    ADV_7611_Initial();
    ADV_7611_DumpOutEdidData();
    ADV_7611_DumpOutDefaultSettings();
}

void test_24c256(void)
{
    unsigned short wr_start_addr = 0;
    unsigned short rd_start_addr = 0;
    unsigned int size = 60;
	
    I2C_Init(I2C_COMPONENT_NUM, I2C_Master_Mode, TAR_24C256_ADDR, I2C_Fast_Speed);
	
    dlog_info("I2C_Initial finished!\n");

    int i;
    unsigned char data_src[] = {61, 62, 63, 64, 65, 66};
    unsigned char data_chk[64];

    for (i = 0; i < size; i += 6)
    {
        data_src[0] = i; data_src[1] = i+1; data_src[2] = i+2; data_src[3] = i+3; data_src[4] = i+4; data_src[5] = i+5;
        dlog_info("%d, %d,%d,%d,%d,%d,%d\n", wr_start_addr, data_src[0],  data_src[1], data_src[2], data_src[3], data_src[4], data_src[5]);
        I2C_Master_Write_Data(I2C_COMPONENT_NUM, TAR_24C256_ADDR, (uint8_t*)&wr_start_addr, 2, data_src, 6);
        wr_start_addr += 6;
    }

    dlog_info("%d, %d,%d,%d,%d,%d,%d\n", wr_start_addr, data_src[0],  data_src[1], data_src[2], data_src[3], data_src[4], data_src[5]);
    data_src[0] = i; data_src[1] = i+1; data_src[2] = i+2; data_src[3] = i+3; data_src[4] = i+4; data_src[5] = i+5;
    I2C_Master_Write_Data(I2C_COMPONENT_NUM, TAR_24C256_ADDR, (uint8_t*)&wr_start_addr, 2, data_src, 4);

    dlog_info("I2C_Master_Write_Data finished!\n");

    I2C_Master_Read_Data(I2C_COMPONENT_NUM, TAR_24C256_ADDR, (uint8_t*)&rd_start_addr, 2, data_chk, size);
    for(i = 0; i < size; i++)
    {
        dlog_info("%d, data_chk = %d\n", rd_start_addr+i, data_chk[i]);
    }
    dlog_info("I2C_Master_Read_Data finished!\n");
}

