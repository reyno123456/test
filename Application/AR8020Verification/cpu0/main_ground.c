#include "debuglog.h"
#include "test_freertos.h"
#include "test_i2c_adv7611.h"
#include "command.h"
#include "serial.h"
#include "hal_sram.h"
#include "cmsis_os.h"
#include "sys_event.h"
#include "upgrade.h"
#include "hal.h"
#include "hal_bb.h"
#include "test_usbh.h"
#include "hal_usb_otg.h"
#include "hal_sys_ctl.h"
#include "wireless_interface.h"
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
    dlog_init(command_run, DLOG_CLIENT_PROCESSOR);
}

static void IO_Task(void const *argument)
{
    while (1)
    {
        SYS_EVENT_Process();

        DLOG_Process(NULL);
      
        HAL_Delay(20);
    }
}

/** 
 * @brief       API for set board SKY mode or GROUND mode
 * @param[in]   SFR_TRX_MODE_SKY or SFR_TRX_MODE_GROUND
 */
#define SFR_TRX_MODE_SEL (*(volatile uint32_t *)0x40B00068)

void BB_SetBoardMODE(uint8_t mode)
{
    SFR_TRX_MODE_SEL = mode;
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
    pst_cfg->u8_workMode = 1;
    HAL_SYS_CTL_Init(pst_cfg);

    /* initialize the uart */
    console_init(0,115200);
    dlog_info("cpu0 start!!! \n");

    /* Enable the CPU Cache */
    CPU_CACHE_Enable();

    HAL_USB_ConfigPHY();

    HAL_USB_InitOTG(HAL_USB_PORT_0);

    HAL_SRAM_ReceiveVideoConfig();

    HAL_NV_Init();

    osThreadDef(USBHStatus_Task, USBH_USBHostStatus, osPriorityNormal, 0, 4 * 128);
    osThreadCreate(osThread(USBHStatus_Task), NULL);

    osThreadDef(IOTask, IO_Task, osPriorityIdle, 0, 4 * 128);
    osThreadCreate(osThread(IOTask), NULL);

    Wireless_TaskInit();

    osKernelStart();

    /* We should never get here as control is now taken by the scheduler */
    for( ;; )
    {
    }
}

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
