#ifndef __HAL_I2C_H__
#define __HAL_I2C_H__

typedef enum
{
    HAL_I2C_COMPONENT_0 = 0,
    HAL_I2C_COMPONENT_1,
    HAL_I2C_COMPONENT_2,
    HAL_I2C_COMPONENT_3,
    HAL_I2C_COMPONENT_4,
    HAL_I2C_COMPONENT_MAX
} ENUM_HAL_I2C_COMPONENT;

typedef enum
{
    HAL_I2C_STANDARD_SPEED = 0,
    HAL_I2C_FAST_SPEED,
    HAL_I2C_HIGH_SPEED
} ENUM_HAL_I2C_SPEED;

HAL_RET_T HAL_I2C_MasterInit(ENUM_HAL_I2C_COMPONENT e_i2cComponent, 
                             uint16_t u16_i2cAddr,
                             ENUM_HAL_I2C_SPEED e_i2cSpeed);

HAL_RET_T HAL_I2C_MasterWriteData(ENUM_HAL_I2C_COMPONENT e_i2cComponent, 
                                  uint16_t u16_i2cAddr,
                                  uint8_t* pu8_wrData,
                                  uint16_t u16_wrSize);

HAL_RET_T HAL_I2C_MasterReadData(ENUM_HAL_I2C_COMPONENT e_i2cComponent, 
                                 uint16_t u16_i2cAddr,
                                 uint8_t* pu8_wrData,
                                 uint8_t  u8_wrSize,
                                 uint8_t* pu8_rdData,
                                 uint16_t u16_rdSize);

#endif

