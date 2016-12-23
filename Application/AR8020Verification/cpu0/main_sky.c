#include "debuglog.h"
#include "serial.h"
#include "test_freertos.h"
#include "test_i2c_adv7611.h"
#include "pll_ctrl.h"
#include "command.h"
#include "test_usbh.h"
#include "cmsis_os.h"
#include "sys_event.h"
#include "inter_core.h"
#include "bb_spi.h"
#include "systicks.h"
#include "bb_ctrl_proxy.h"
#include "hal_hdmi_rx.h"

void *malloc(size_t size)
{
    return pvPortMalloc(size);
}

void free(void* p)
{
    vPortFree(p);
}

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
    dlog_init(uart_num);
    UartNum = uart_num;
    command_init();
}


static void IO_Task(void const *argument)
{
    while (1)
    {
        SYS_EVENT_Process();
        if (command_getEnterStatus() == 1)
        {
            command_fulfill();
        }

        dlog_output(100);
    }
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    BB_SetBoardMODE(SFR_TRX_MODE_SKY);

    BB_SPI_init();

    PLLCTRL_SetCoreClk(CPU0_CPU1_CORE_PLL_CLK, CPU0_ID);
    PLLCTRL_SetCoreClk(CPU2_CORE_PLL_CLK, CPU2_ID);

    /* initialize the uart */
    console_init(0,115200);
    dlog_info("cpu0 start!!! \n");

    InterCore_Init();

    /* Enable the CPU Cache */
    CPU_CACHE_Enable();

    SysTicks_Init(200000);

    HAL_HDMI_RX_Init(HAL_HDMI_RX_0);
    HAL_HDMI_RX_Init(HAL_HDMI_RX_1);

    USBD_ApplicationInit();

    /* Create Main Task */
    osThreadDef(USBMAIN_Task, USB_MainTask, osPriorityBelowNormal, 0, 4 * 128);
    osThreadCreate(osThread(USBMAIN_Task), NULL);

    osThreadDef(USBHStatus_Task, USBH_USBHostStatus, osPriorityNormal, 0, 4 * 128);
    osThreadCreate(osThread(USBHStatus_Task), NULL);

    osThreadDef(IOTask, IO_Task, osPriorityIdle, 0, 4 * 128);
    osThreadCreate(osThread(IOTask), NULL);

    osMessageQDef(osqueue, 1, uint16_t);
    g_usbhAppCtrl.usbhAppEvent  = osMessageCreate(osMessageQ(osqueue),NULL);

    osKernelStart();

    /* We should never get here as control is now taken by the scheduler */
    for( ;; )
    {
    }
} 

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/
