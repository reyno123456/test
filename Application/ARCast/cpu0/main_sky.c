#include "debuglog.h"
#include "serial.h"
#include "command.h"
#include "cmsis_os.h"
#include "sys_event.h"
#include "bb_spi.h"
#include "hal.h"
#include "hal_gpio.h"
#include "hal_bb.h"
#include "hal_hdmi_rx.h"
#include "hal_usb_otg.h"
#include "hal_sys_ctl.h"
#include "wireless_interface.h"
#include "stm32f746xx.h"
#include "hal_nv.h"

/**
 * @brief  CPU L1-Cache enable.
 * @param  None
 * @retval None
 */
static void CPU_CACHE_Enable(void)
{
    /* Enable I-Cache */
    SCB_EnableICache();

    /* Enable D-Cache */
    SCB_EnableDCache();
}

void console_init(uint32_t uart_num, uint32_t baut_rate)
{
    serial_init(uart_num, baut_rate);
    dlog_init(command_run);
}

void HDMI_powerOn(void)
{
    HAL_GPIO_OutPut(63);
    HAL_GPIO_SetPin(63, HAL_GPIO_PIN_SET);
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    STRU_HAL_SYS_CTL_CONFIG *pst_cfg;
    HAL_SYS_CTL_GetConfig( &pst_cfg);
    pst_cfg->u8_workMode = 0;
    HAL_SYS_CTL_Init(pst_cfg);

    /* initialize the uart */
    console_init(0,115200);
    dlog_info("cpu0 start!!! \n");

    /* Enable the CPU Cache */
    CPU_CACHE_Enable();

    HAL_USB_ConfigPHY();

    HDMI_powerOn();
    
    STRU_HDMI_CONFIGURE        st_configure;
    st_configure.e_getFormatMethod = HAL_HDMI_POLLING;
    st_configure.u8_interruptGpio = HAL_GPIO_NUM64;
    st_configure.u8_hdmiToEncoderCh = 1;
    HAL_HDMI_RX_Init(HAL_HDMI_RX_0, &st_configure);

    st_configure.u8_interruptGpio = HAL_GPIO_NUM65;
    st_configure.u8_hdmiToEncoderCh = 0;
    HAL_HDMI_RX_Init(HAL_HDMI_RX_1, &st_configure);

    HAL_USB_InitOTG(HAL_USB_PORT_0);

    HAL_NV_Init();

    /* We should never get here as control is now taken by the scheduler */
    for( ;; )
    {
        SYS_EVENT_Process();

        DLOG_Process(NULL);

        HAL_Delay(20);
    }
} 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
