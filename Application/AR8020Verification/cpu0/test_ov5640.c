#include "test_ov5640.h"

void command_TestOv5640(void)
{
	OV5640_init();	
}

void command_TestOv5640Write(unsigned char *subAddr, unsigned char *value)
{
	uint16_t reg = strtoul(subAddr, NULL, 0);
	uint8_t val = strtoul(value, NULL, 0);
	uint32_t delay = 200;
	uint8_t u8_val;

	OV5640_read_reg(reg, &u8_val);
	dlog_info("before write red:%x val:%x",reg,u8_val);
	
	OV5640_write_reg(reg, val);
		
	OV5640_read_reg(reg, &u8_val);
	dlog_info("after write red:%x val:%x",reg,u8_val);
}

void command_TestOv5640Read(unsigned char *subAddr)
{
	uint16_t reg = strtoul(subAddr, NULL, 0);
	uint8_t u8_val;
		
	OV5640_read_reg(reg, &u8_val);
	dlog_info("red:%x val:%x",reg,u8_val);

}

