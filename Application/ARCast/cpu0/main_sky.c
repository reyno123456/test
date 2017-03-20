#include "debuglog.h"
#include "serial.h"
#include "command.h"
#include "test_usbh.h"
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
#include "hal_nv.h"
#include "hal_usb_host.h"
#include "hal_encodemp3.h"
#include "hal_softi2s.h"
#include "systicks.h"


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
    dlog_init(command_run, DLOG_SERVER_PROCESSOR);
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

    st_configure.u8_interruptGpio = HAL_GPIO_NUM64;
    st_configure.u8_hdmiToEncoderCh = 0;
    HAL_HDMI_RX_Init(HAL_HDMI_RX_1, &st_configure);

    STRU_MP3_ENCODE_CONFIGURE_WAVE st_audioConfig;
    st_audioConfig.e_samplerate = HAL_MP3_ENCODE_48000;
    st_audioConfig.e_modes = HAL_MP3_ENCODE_STEREO;
    st_audioConfig.u32_rawDataAddr = AUDIO_DATA_START;
    st_audioConfig.u32_rawDataLenght = AUDIO_DATA_BUFF_SIZE;
    st_audioConfig.u32_encodeDataAddr = MPE3_ENCODER_DATA_ADDR;
    st_audioConfig.u32_newPcmDataFlagAddr = AUDIO_DATA_READY_ADDR;
    st_audioConfig.u8_channel = 2;

    HAL_USB_InitOTG(HAL_USB_PORT_0);

    HAL_NV_Init();

    USBH_MountUSBDisk();
    HAL_MP3EncodePcmInit(&st_audioConfig);
    /* We should never get here as control is now taken by the scheduler */
    for( ;; )
    {
        
        HAL_USB_HostProcess();
		HAL_MP3EncodePcm();
		SYS_EVENT_Process();
        DLOG_Process(NULL);
    }
} 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
