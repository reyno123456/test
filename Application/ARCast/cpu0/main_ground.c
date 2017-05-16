#include "debuglog.h"
#include "command.h"
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
#include "hal_gpio.h"
#include "hal_uart.h"
#include "arcast_appcommon.h"

void CONSOLE_Init(void)
{
    HAL_UART_Init(DEBUG_LOG_UART_PORT, HAL_UART_BAUDR_115200, NULL);
    DLOG_Init(command_run, DLOG_SERVER_PROCESSOR);
}

static void GenericInitial(void const *argument)
{
    Common_AVFORMATSysEventGroundInit();
    vTaskDelete(NULL);
}

static void IO_Task(void const *argument)
{
    while (1)
    {
        SYS_EVENT_Process();
    }
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
    CONSOLE_Init();
    dlog_info("cpu0 start!!! \n");

    HAL_GPIO_InPut(HAL_GPIO_NUM99);

    HAL_USB_ConfigPHY();

    HAL_USB_InitOTG(HAL_USB_PORT_0);

    HAL_USB_InitOTG(HAL_USB_PORT_1);

    HAL_SRAM_ReceiveVideoConfig(ENUM_HAL_SRAM_DATA_PATH_REVERSE);
    

    HAL_NV_Init();

    osThreadDef(GenericInitialTask, GenericInitial, osPriorityNormal, 0, 4 * 128);
    osThreadCreate(osThread(GenericInitialTask), NULL);

    osThreadDef(USBHStatus_Task, USBH_USBHostStatus, osPriorityNormal, 0, 4 * 128);
    osThreadCreate(osThread(USBHStatus_Task), NULL);

    osThreadDef(IOTask, IO_Task, osPriorityIdle, 0, 16 * 128);
    osThreadCreate(osThread(IOTask), NULL);

    Wireless_TaskInit(WIRELESS_USE_RTOS);

    osKernelStart();

    /* We should never get here as control is now taken by the scheduler */
    for( ;; )
    {
    }
}
