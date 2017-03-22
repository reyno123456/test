#include "data_type.h"
#include "i2c.h"
#include "i2c_ll.h"
#include "debuglog.h"
#include "reg_rw.h"

static uint8_t I2C_Master_UpdateTargetAddress(EN_I2C_COMPONENT en_component, uint16_t i2cAddr)
{
    STRU_I2C_Controller* dev = I2C_LL_GetI2CController(en_component);
    
    if (dev)
    {
        uint32_t value_tmp = i2cAddr;
       
        if (I2C_LL_IOCtl(dev, I2C_CMD_SET_M_TARGET_ADDRESS, &value_tmp) == TRUE)
        {
            return TRUE;
        }
        else
        {
            dlog_error("I2C_CMD_SET_M_TARGET_ADDRESS error!");
            return FALSE;
        }
    }
    else
    {
        dlog_error("Can not get right i2c controller!");
    }

    return FALSE;
}

static uint8_t I2C_Master_WriteByte(EN_I2C_COMPONENT en_component, uint8_t value)
{
    STRU_I2C_Controller* dev = I2C_LL_GetI2CController(en_component);
    
    if (dev)
    {
        uint32_t value_tmp = value;
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

static uint8_t I2C_Master_ReadLaunch(EN_I2C_COMPONENT en_component)
{
    STRU_I2C_Controller* dev = I2C_LL_GetI2CController(en_component);
    
    if (dev)
    {
        uint32_t value;
        if (I2C_LL_IOCtl(dev, I2C_CMD_SET_M_READ_LAUNCH, &value) == TRUE)
        {
            return TRUE;
        }
        else
        {
            dlog_error("I2C_CMD_SET_M_READ_DATA_LAUNCH error!");
            return FALSE;
        }
    }
    else
    {
        dlog_error("Can not get right i2c controller!");
    }

    return FALSE;
}

static uint8_t I2C_Master_ReadByte(EN_I2C_COMPONENT en_component)
{
    STRU_I2C_Controller* dev = I2C_LL_GetI2CController(en_component);
    
    if (dev)
    {
        uint32_t value;
        if (I2C_LL_IOCtl(dev, I2C_CMD_GET_M_RX_FIFO_DATA, &value) == TRUE)
        {
            return (uint8_t)value;
        }
        else
        {
            dlog_error("I2C_CMD_GET_M_RX_FIFO_DATA error!");
            return 0;
        }
    }
    else
    {
        dlog_error("Can not get right i2c controller!");
    }

    return 0;
}

static void I2C_Master_WaitTillIdle(EN_I2C_COMPONENT en_component)
{
    ENUM_I2C_MASTER_ACTIVITY e_MastActivity;
    STRU_I2C_Controller* dev = I2C_LL_GetI2CController(en_component);

    if (dev)
    {
        uint32_t value;
        e_MastActivity = I2C_MASTER_ACTIVE;
        while(e_MastActivity != I2C_MASTER_IDLE)
        {
            I2C_LL_Delay(1000); // Need some time to get the real activity status
            I2C_LL_IOCtl(dev, I2C_CMD_GET_M_IDLE, &value);
            e_MastActivity = (ENUM_I2C_MASTER_ACTIVITY)value;
        }
    }
    else
    {
        dlog_error("Can not get right i2c controller!");
    }
}

static uint8_t I2C_Master_GetTxFifoLength(EN_I2C_COMPONENT en_component)
{
    uint32_t u32_FiFoLength;
    STRU_I2C_Controller* dev = I2C_LL_GetI2CController(en_component);

    if (dev)
    {
        I2C_LL_IOCtl(dev, I2C_CMD_GET_M_TX_FIFO_LENGTH, &u32_FiFoLength);
        return (uint8_t)u32_FiFoLength;
    }
    else
    {
        dlog_error("Can not get right i2c controller!");
    }
}

static uint8_t I2C_Master_GetRxFifoLength(EN_I2C_COMPONENT en_component)
{
    uint32_t u32_FiFoLength;
    STRU_I2C_Controller* dev = I2C_LL_GetI2CController(en_component);

    if (dev)
    {
        I2C_LL_IOCtl(dev, I2C_CMD_GET_M_RX_FIFO_LENGTH, &u32_FiFoLength);
        return (uint8_t)u32_FiFoLength;
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

    // IO MUX
    if (en_component == I2C_Component_2)
    {
        Reg_Write32_Mask(SFR_PAD_CTRL7_REG, 0, BIT(14) | BIT(15) | BIT(16) | BIT(17));
    }
    else if (en_component == I2C_Component_5)
    {
        Reg_Write32_Mask(SFR_PAD_CTRL7_REG, BIT(14) | BIT(16), BIT(14) | BIT(15) | BIT(16) | BIT(17));
    }

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

uint8_t I2C_Master_WriteData(EN_I2C_COMPONENT en_component, uint16_t u16_i2cAddr, uint8_t* ptr_data, uint32_t u32_dataSize)
{
    I2C_Master_UpdateTargetAddress(en_component, u16_i2cAddr);

    if (ptr_data)
    {
        uint32_t i = 0;
        while (i < u32_dataSize)
        {
            if (I2C_Master_GetTxFifoLength(en_component) < I2C_TX_FIFO_BUFFER_DEPTH)
            {
                I2C_Master_WriteByte(en_component, ptr_data[i]);
                i++;
            }
        }
        
        I2C_Master_WaitTillIdle(en_component);

        return TRUE;
    }

    return FALSE;
}

uint8_t I2C_Master_ReadData(EN_I2C_COMPONENT en_component, uint16_t u16_i2cAddr, uint8_t* ptr_subAddr, uint8_t u8_subAddrSize, uint8_t* ptr_data, uint32_t u32_dataSize)
{
    uint8_t u8_i = 0;
    
    I2C_Master_UpdateTargetAddress(en_component, u16_i2cAddr);
    
    if (ptr_subAddr)
    {
        while (u8_i < u8_subAddrSize)
        {
            // The sub address size should be less than I2C_TX_FIFO_BUFFER_DEPTH, so no check here.
            I2C_Master_WriteByte(en_component, ptr_subAddr[u8_i++]); // High address first
        }
    }
    
    if (ptr_data)
    {
        uint32_t u32_LaunchNumber = 0;
        uint32_t u32_ReadNumber = 0;
        uint32_t u32_NoValidDataCount = 0;
        while (u32_ReadNumber < u32_dataSize)
        {
            if ((u32_LaunchNumber < u32_dataSize) && ((u32_LaunchNumber - u32_ReadNumber) < (I2C_RX_FIFO_BUFFER_DEPTH - 1)))
            {
                I2C_Master_ReadLaunch(en_component);
                u32_LaunchNumber++;
            }

            if (I2C_Master_GetRxFifoLength(en_component) > 0)
            {
                ptr_data[u32_ReadNumber] = I2C_Master_ReadByte(en_component);
                u32_ReadNumber++;
                u32_NoValidDataCount = 0;
            }
            else
            {
                u32_NoValidDataCount++;
            }

            // Patch to process RX data error and wait too long time
            if (u32_NoValidDataCount > 50000)
            {
                DLOG_Error("I2C %d 0x%x: Read I2C data time out!", en_component, u16_i2cAddr);
                break;
            }
        }

        I2C_Master_WaitTillIdle(en_component);

        return TRUE;
    }

    return FALSE;
}


