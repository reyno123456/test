#ifndef __HAL_RET_TYPE_H__
#define __HAL_RET_TYPE_H__

typedef uint32_t HAL_RET_T;

#define HAL_OK                       (0)

#define HAL_I2C_ERR_MASK             (0x10000)
#define HAL_I2C_ERR_UNKNOWN          (HAL_I2C_ERR_MASK | 0x1)
#define HAL_I2C_ERR_INIT             (HAL_I2C_ERR_MASK | 0x2)
#define HAL_I2C_ERR_WRITE_DATA       (HAL_I2C_ERR_MASK | 0x3)
#define HAL_I2C_ERR_READ_DATA        (HAL_I2C_ERR_MASK | 0x4)

#define HAL_SYS_CTL_ERR_MASK         (0x60000)
#define HAL_SYS_CTL_ERR_UNKNOWN      (HAL_SYS_CTL_ERR_MASK | 0x1)
#define HAL_SYS_CTL_ERR_SET_CPU_CLK  (HAL_SYS_CTL_ERR_MASK | 0x2)
#define HAL_SYS_CTL_ERR_FPU_ENABLE   (HAL_SYS_CTL_ERR_MASK | 0x3)
#define HAL_SYS_CTL_ERR_INIT         (HAL_SYS_CTL_ERR_MASK | 0x4)

#define HAL_HDMI_RX_ERR_MASK                (0x70000)
#define HAL_HDMI_RX_ERR_UNKNOWN             (HAL_HDMI_RX_ERR_MASK | 0x1)
#define HAL_HDMI_RX_ERR_INIT                (HAL_HDMI_RX_ERR_MASK | 0x2)
#define HAL_HDMI_RX_ERR_GET_VIDEO_FORMAT    (HAL_HDMI_RX_ERR_MASK | 0x3)

#define HAL_SRAM_ERR_MASK                   (0x20000)
#define HAL_SRAM_ERR_CHANNEL_INVALID        (HAL_HDMI_RX_ERR_MASK | 0x1)


#endif

