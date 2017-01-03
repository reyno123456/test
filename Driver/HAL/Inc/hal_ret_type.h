#ifndef __HAL_RET_TYPE_H__
#define __HAL_RET_TYPE_H__

typedef uint32_t HAL_RET_T;

typedef uint8_t HAL_BOOL_T;

#define HAL_OK                                      (0)

#define HAL_TRUE                                    (1)
#define HAL_FALSE                                   (0)

#define HAL_I2C_ERR_MASK                            (0x10000)
#define HAL_I2C_ERR_UNKNOWN                         (HAL_I2C_ERR_MASK | 0x1)
#define HAL_I2C_ERR_INIT                            (HAL_I2C_ERR_MASK | 0x2)
#define HAL_I2C_ERR_WRITE_DATA                      (HAL_I2C_ERR_MASK | 0x3)
#define HAL_I2C_ERR_READ_DATA                       (HAL_I2C_ERR_MASK | 0x4)

#define HAL_SYS_CTL_ERR_MASK                        (0x60000)
#define HAL_SYS_CTL_ERR_UNKNOWN                     (HAL_SYS_CTL_ERR_MASK | 0x1)
#define HAL_SYS_CTL_ERR_SET_CPU_CLK                 (HAL_SYS_CTL_ERR_MASK | 0x2)
#define HAL_SYS_CTL_ERR_FPU_ENABLE                  (HAL_SYS_CTL_ERR_MASK | 0x3)
#define HAL_SYS_CTL_ERR_INIT                        (HAL_SYS_CTL_ERR_MASK | 0x4)
#define HAL_SYS_CTL_ERR_SYS_TICK_INIT               (HAL_SYS_CTL_ERR_MASK | 0x5)

#define HAL_HDMI_RX_ERR_MASK                        (0x70000)
#define HAL_HDMI_RX_ERR_UNKNOWN                     (HAL_HDMI_RX_ERR_MASK | 0x1)
#define HAL_HDMI_RX_ERR_INIT                        (HAL_HDMI_RX_ERR_MASK | 0x2)
#define HAL_HDMI_RX_ERR_UNINIT                      (HAL_HDMI_RX_ERR_MASK | 0x3)
#define HAL_HDMI_RX_ERR_GET_VIDEO_FORMAT            (HAL_HDMI_RX_ERR_MASK | 0x4)
#define HAL_HDMI_RX_ERR_VIDEO_FORMAT_NOT_SUPPORT    (HAL_HDMI_RX_ERR_MASK | 0x5)
#define HAL_HDMI_RX_ERR_INDEX_LARGER_THAN_MAX       (HAL_HDMI_RX_ERR_MASK | 0x6)

#define HAL_SRAM_ERR_MASK                           (0x20000)
#define HAL_SRAM_ERR_CHANNEL_INVALID                (HAL_HDMI_RX_ERR_MASK | 0x1)

#define HAL_GPIO_ERR_MASK                           (0x30000)
#define HAL_GPIO_ERR_UNKNOWN                        (HAL_GPIO_ERR_MASK | 0x1)

#define HAL_TIMER_ERR_MASK                          (0x40000)
#define HAL_TIMER_ERR_UNKNOWN                       (HAL_TIMER_ERR_MASK | 0x1)

#define HAL_PWM_ERR_MASK                            (0x50000)
#define HAL_PWM_ERR_UNKNOWN                         (HAL_PWM_ERR_MASK | 0x1)

#define HAL_SOFTPWM_ERR_MASK                        (0x80000)
#define HAL_SOFTPWM_ERR_GPIOMAX                     (HAL_SOFTPWM_ERR_MASK | 0x1)

#define HAL_USB_ERR_MASK                            (0x90000)
#define HAL_USB_ERR_PORT_INVALID                    (HAL_USB_ERR_MASK | 0x1)
#define HAL_USB_ERR_BUFF_IS_EMPTY                   (HAL_USB_ERR_MASK | 0x2)


#define HAL_SPI_ERR_MASK                            (0xA0000)
#define HAL_SPI_ERR_UNKNOWN                         (HAL_SPI_ERR_MASK | 0x1)
#define HAL_SPI_ERR_INIT                            (HAL_SPI_ERR_MASK | 0x2)
#define HAL_SPI_ERR_WRITE_DATA                      (HAL_SPI_ERR_MASK | 0x3)
#define HAL_SPI_ERR_READ_DATA                       (HAL_SPI_ERR_MASK | 0x4)
#define HAL_SPI_ERR_COMPONENT                       (HAL_SPI_ERR_MASK | 0x5)

#define HAL_UART_ERR_MASK                           (0xB0000)
#define HAL_UART_ERR_UNKNOWN                        (HAL_UART_ERR_MASK | 0x1)
#define HAL_UART_ERR_INIT                           (HAL_UART_ERR_MASK | 0x2)
#define HAL_UART_ERR_WRITE_DATA                     (HAL_UART_ERR_MASK | 0x3)
#define HAL_UART_ERR_READ_DATA                      (HAL_UART_ERR_MASK | 0x4)

#define HAL_NVIC_ERR_MASK                           (0xC000)



#define HAL_BB_ERR_MASK                             (0xB000)
#define HAL_BB_ERR_INIT                             (HAL_BB_ERR_MASK | 0x1)
#define HAL_BB_ERR_EVENT_NOTIFY                     (HAL_BB_ERR_MASK | 0x2)
#define HAL_BB_ERR_INIT_SESSION                     (HAL_BB_ERR_MASK | 0x3)
#define HAL_BB_ERR_SESSION_OCCUPIED                 (HAL_BB_ERR_MASK | 0x4)
#define HAL_BB_ERR_UNREGISTER_SESSION               (HAL_BB_ERR_MASK | 0x5)
#define HAL_BB_ERR_SESSION_SEND                     (HAL_BB_ERR_MASK | 0x6)
#define HAL_BB_ERR_SESSION_RCV                      (HAL_BB_ERR_MASK | 0x7)
#define HAL_BB_ERR_SPI_WRITE                        (HAL_BB_ERR_MASK | 0x8)
#define HAL_BB_ERR_SPI_READ                         (HAL_BB_ERR_MASK | 0x9)




#define HAL_SD_ERR_MASK								(0xC0000)
#define HAL_SD_ERR_ERROR                            (HAL_SD_ERR_MASK | 0x1)
#define HAL_SD_ERR_BUSY                             (HAL_SD_ERR_MASK | 0x1) 
#define HAL_SD_ERR_TIMEOUT                          (HAL_SD_ERR_MASK | 0x1)     

#endif

