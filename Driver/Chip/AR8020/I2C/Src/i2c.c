#include "data_type.h"
#include "i2c.h"
#include "i2c_ll.h"
#include "debuglog.h"

static uint8_t I2C_Master_Write_Byte(EN_I2C_COMPONENT en_component, uint16_t i2cAddr, uint8_t value)
{
    STRU_I2C_Controller* dev = I2C_LL_GetI2CController(en_component);
    
    if (dev)
    {
        uint32_t value_tmp = i2cAddr;
        I2C_LL_IOCtl(dev, I2C_CMD_SET_M_TARGET_ADDRESS, &value_tmp);
        
        value_tmp = value;
        if (I2C_LL_IOCtl(dev, I2C_CMD_SET_M_WRITE_DATA, &value_tmp) == TRUE)
        {
            return TRUE;
    	}
    	else
    	{
    		dlog_error("I2C_CMD_SET_M_WRITE_DATA error!");
            return FALSE;
    	}
    }
    else
    {
    	dlog_error("Can not get right i2c controller!");
    }

    return FALSE;
}

static uint8_t I2C_Master_Read_Byte(EN_I2C_COMPONENT en_component, uint16_t i2cAddr, uint8_t* p_value)
{
    STRU_I2C_Controller* dev = I2C_LL_GetI2CController(en_component);
    
    if (p_value && dev)
    {
        uint32_t value_tmp = i2cAddr;
        I2C_LL_IOCtl(dev, I2C_CMD_SET_M_TARGET_ADDRESS, &value_tmp);

        uint32_t value;
        if (I2C_LL_IOCtl(dev, I2C_CMD_GET_M_READ_DATA, &value) == TRUE)
        {
            *p_value = (uint8_t)value;
            return TRUE;
        }
        else
        {
        	dlog_error("I2C_CMD_GET_M_READ_DATA error!");
            return FALSE;
        }
    }
    else
    {
    	if (dev == NULL)
            dlog_error("Can not get right i2c controller!");
    	else
    		dlog_error("p_value is %p\n", p_value);
    }

    return FALSE;
}

static void I2C_Master_Wait_Till_Idle(EN_I2C_COMPONENT en_component)
{
    uint32_t idle;
    STRU_I2C_Controller* dev = I2C_LL_GetI2CController(en_component);

    if (dev)
    {
    	idle = 0;
        while(idle != 1)
        {
            I2C_LL_IOCtl(dev, I2C_CMD_GET_M_IDLE, &idle);
            I2C_LL_Delay(5000);
        }
    }
    else
    {
    	dlog_error("Can not get right i2c controller!");
    }
}

static void I2C_Master_Wait_Till_TX_FIFO_Empty(EN_I2C_COMPONENT en_component)
{
    uint32_t idle;
    STRU_I2C_Controller* dev = I2C_LL_GetI2CController(en_component);

    if (dev)
    {
        idle = 0;
        while(idle != 1)
        {
            I2C_LL_IOCtl(dev, I2C_CMD_GET_M_TXFIFO_EMPTY, &idle);
            I2C_LL_Delay(5000);
        }
    }
    else
    {
    	dlog_error("Can not get right i2c controller!");
    }
}

/************************************************************************************
 * I2C APIs
 ************************************************************************************/

uint8_t I2C_Init(EN_I2C_COMPONENT en_component, ENUM_I2C_Mode en_i2cMode, uint16_t u16_i2cAddr, ENUM_I2C_Speed en_i2cSpeed)
{
    STRU_I2C_Controller* dev = I2C_LL_GetI2CController(en_component);
    
    if (dev)
    {
    	uint32_t tmpValue;
        if (en_i2cMode == I2C_Master_Mode)
        {        	
        	tmpValue = u16_i2cAddr;
        	I2C_LL_IOCtl(dev, I2C_CMD_SET_M_TARGET_ADDRESS, &tmpValue);
        	tmpValue = en_i2cSpeed;
        	I2C_LL_IOCtl(dev, I2C_CMD_SET_M_SPEED, &tmpValue);
        	tmpValue = en_i2cMode;
        	I2C_LL_IOCtl(dev, I2C_CMD_SET_MODE, &tmpValue);
        }
        else if (en_i2cMode == I2C_Slave_Mode)
        {
        	tmpValue = u16_i2cAddr;
        	I2C_LL_IOCtl(dev, I2C_CMD_SET_S_SLAVE_ADDRESS, &tmpValue);
        	tmpValue = en_i2cMode;
        	I2C_LL_IOCtl(dev, I2C_CMD_SET_MODE, &tmpValue);
        }
        
        return TRUE;
    }
    
    return FALSE;
}

uint8_t I2C_Master_Write_Data(EN_I2C_COMPONENT en_component, uint16_t u16_i2cAddr, uint8_t* ptr_subAddr, uint8_t u8_subAddrSize, uint8_t* ptr_data, uint32_t u32_dataSize)
{
    if (ptr_subAddr && ptr_data)
    {
    	while (u8_subAddrSize)
        {
            I2C_Master_Write_Byte(en_component, u16_i2cAddr, ptr_subAddr[u8_subAddrSize-1]); // High address first
            u8_subAddrSize--;
        }

        uint32_t i = 0;
        while (i < u32_dataSize)
        {
            I2C_Master_Write_Byte(en_component, u16_i2cAddr, ptr_data[i]);
            i++;
        }
        
        I2C_Master_Wait_Till_Idle(en_component);
        I2C_Master_Wait_Till_TX_FIFO_Empty(en_component);

        return TRUE;
    }

    return FALSE;
}

uint8_t I2C_Master_Read_Data(EN_I2C_COMPONENT en_component, uint16_t u16_i2cAddr, uint8_t* ptr_subAddr, uint8_t u8_subAddrSize, uint8_t* ptr_data, uint32_t u32_dataSize)
{
    if (ptr_subAddr && ptr_data)
    {
        while (u8_subAddrSize)
        {
        	I2C_Master_Write_Byte(en_component, u16_i2cAddr, ptr_subAddr[u8_subAddrSize-1]); // High address first
            u8_subAddrSize--;
        }

        uint32_t i = 0;
        while (i < u32_dataSize)
        {
            I2C_Master_Read_Byte(en_component, u16_i2cAddr, &(ptr_data[i]));
            I2C_Master_Wait_Till_Idle(en_component);
            i++;
        }

        return TRUE;
    }

    return FALSE;
}


