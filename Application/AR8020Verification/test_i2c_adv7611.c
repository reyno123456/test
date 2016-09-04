#include "debuglog.h"
#include "i2c.h"

#define TAR_24C256_ADDR       0x051

#define TAR_ADDR TAR_24C256_ADDR

void I2C_Initial(void)
{
    I2C_Init(I2C_Component_1, I2C_Master_Mode, TAR_ADDR, I2C_Fast_Speed);
}

void test_adv7611(void)
{

}

void test_24c256(void)
{
    unsigned short wr_start_addr = 0;
    unsigned short rd_start_addr = 0;
    unsigned int size = 60;
	
    I2C_Initial();
	
    dlog_info("I2C_Initial finished!\n");

    int i;
    unsigned char data_src[] = {61, 62, 63, 64, 65, 66};
    unsigned char data_chk[64];

    for (i = 0; i < size; i += 6)
    {
        data_src[0] = i; data_src[1] = i+1; data_src[2] = i+2; data_src[3] = i+3; data_src[4] = i+4; data_src[5] = i+5;
        dlog_info("%d, %d,%d,%d,%d,%d,%d\n", wr_start_addr, data_src[0],  data_src[1], data_src[2], data_src[3], data_src[4], data_src[5]);
        I2C_Master_Write_Data(I2C_Component_1, (uint8_t*)&wr_start_addr, 2, data_src, 6);
        wr_start_addr += 6;
    }

    dlog_info("%d, %d,%d,%d,%d,%d,%d\n", wr_start_addr, data_src[0],  data_src[1], data_src[2], data_src[3], data_src[4], data_src[5]);
    data_src[0] = i; data_src[1] = i+1; data_src[2] = i+2; data_src[3] = i+3; data_src[4] = i+4; data_src[5] = i+5;
    I2C_Master_Write_Data(I2C_Component_1, (uint8_t*)&wr_start_addr, 2, data_src, 4);

    dlog_info("I2C_Master_Write_Data finished!\n");

    I2C_Master_Read_Data(I2C_Component_1, (uint8_t*)&rd_start_addr, 2, data_chk, size);
    for(i = 0; i < size; i++)
    {
        dlog_info("%d, data_chk = %d\n", rd_start_addr+i, data_chk[i]);
    }
    dlog_info("I2C_Master_Read_Data finished!\n");
}

