/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_i2c.c
Description: The external HAL APIs to use the I2C controller.
Author: Artosy Software Team
Version: 0.0.1
Date: 2016/12/20
History: 
        0.0.1    2016/12/20    The initial version of hal_i2c.h
*****************************************************************************/

#include <stdint.h>
#include "i2c.h"
#include "hal_i2c.h"
#include "hal_ret_type.h"

/**
* @brief  The I2C initialization function which must be called before using the I2C controller.
* @param  e_i2cComponent    The I2C controller number, the right number should be 0-4 and totally
*                           5 I2C controllers can be used by application.
*         u16_i2cAddr       16 bit I2C address of the target device.
*         e_i2cSpeed        The I2C speed of the I2C clock of the I2C controller, the right value
*                           should be standard (<100Kb/s), fast (<400Kb/s) and high(<3.4Mb/s).
* @retval HAL_OK            means the initializtion is well done.
*         HAL_I2C_ERR_INIT  means some error happens in the initializtion.
* @note   Master mode only
*         High speed mode has some system dependency and is especially affected by the circuit capacity
*         and the I2C slave device ability.
*/

HAL_RET_T HAL_I2C_MasterInit(ENUM_HAL_I2C_COMPONENT e_i2cComponent, 
                             uint16_t u16_i2cAddr,
                             ENUM_HAL_I2C_SPEED e_i2cSpeed)
{
    return HAL_OK;
}

/**
* @brief  The I2C data write function which can be used to send out I2C data by the I2C controller.
* @param  e_i2cComponent          The I2C controller number, the right number should be 0-4 and totally
*                                 5 I2C controllers can be used by application.
*         u16_i2cAddr             16 bit I2C address of the target device.
*         pu8_wrData              The transmit buffer pointer to be sent out by I2C bus. In normal case, the first 1 
*                                 or 2 bytes should be the sub address to be accessed.
*         u16_wrSize              The transmit buffer size in byte. 
* @retval HAL_OK                  means the I2C data write is well done.
*         HAL_I2C_ERR_WRITE_DATA  means some error happens in the I2C data write.
* @note   High speed mode has some system dependency and is especially affected by the circuit capacity.
*/

HAL_RET_T HAL_I2C_MasterWriteData(ENUM_HAL_I2C_COMPONENT e_i2cComponent, 
                                  uint16_t u16_i2cAddr,
                                  uint8_t *pu8_wrData,
                                  uint16_t u16_wrSize)
{
    return HAL_OK;
}

/**
* @brief  The I2C data read function which can be used to read I2C data by the I2C controller.
* @param  e_i2cComponent          The I2C controller number, the right number should be 0-4 and totally
*                                 5 I2C controllers can be used by application.
*         u16_i2cAddr             16 bit I2C address of the target device.
*         pu8_wrData              The transmit buffer pointer to be sent out by I2C bus before read starts.
                                  In normal case, the first 1 or 2 bytes should be the sub address to be accessed.
*         u16_wrSize              The transmit buffer size in byte. 
          pu8_rdData              The receive buffer pointer to hold the data in read operation.
          u16_rdSize              The receive buffer size in byte.
* @retval HAL_OK                  means the initializtion is well done.
*         HAL_I2C_ERR_READ_DATA   means some error happens in the data read.
* @note   High speed mode has some system dependency and is especially affected by the circuit capacity.
*/

HAL_RET_T HAL_I2C_MasterReadData(ENUM_HAL_I2C_COMPONENT e_i2cComponent, 
                                 uint16_t u16_i2cAddr,
                                 uint8_t *pu8_wrData,
                                 uint8_t  u8_wrSize,
                                 uint8_t *pu8_rdData,
                                 uint16_t u16_rdSize)
{
    return HAL_OK;
}


