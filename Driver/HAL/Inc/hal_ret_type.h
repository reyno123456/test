#ifndef __HAL_RET_TYPE_H__
#define __HAL_RET_TYPE_H__

typedef uint32_t HAL_RET_T;

#define HAL_OK                      (0)

#define HAL_I2C_ERR_MASK            (0x1000)
#define HAL_I2C_ERR_UNKNOWN         (HAL_I2C_ERR_MASK | 0x1)
#define HAL_I2C_ERR_INIT            (HAL_I2C_ERR_MASK | 0x2)
#define HAL_I2C_ERR_WRITE_DATA      (HAL_I2C_ERR_MASK | 0x3)
#define HAL_I2C_ERR_READ_DATA       (HAL_I2C_ERR_MASK | 0x4)

#endif

