#include <stdint.h>
#include "i2c.h"
#include "hal_i2c.h"
#include "hal_ret_type.h"

HAL_RET_T HAL_I2C_MasterInit(ENUM_HAL_I2C_COMPONENT e_i2cComponent, 
                             uint16_t u16_i2cAddr,
                             ENUM_HAL_I2C_SPEED e_i2cSpeed)
{
    return HAL_OK;
}

HAL_RET_T HAL_I2C_MasterWriteData(ENUM_HAL_I2C_COMPONENT e_i2cComponent, 
                                  uint16_t u16_i2cAddr,
                                  uint8_t* pu8_wrData,
                                  uint16_t u16_wrSize)
{
    return HAL_OK;
}

HAL_RET_T HAL_I2C_MasterReadData(ENUM_HAL_I2C_COMPONENT e_i2cComponent, 
                                 uint16_t u16_i2cAddr,
                                 uint8_t* pu8_wrData,
                                 uint8_t  u8_wrSize,
                                 uint8_t* pu8_rdData,
                                 uint16_t u16_rdSize)
{
    return HAL_OK;
}


