#include "debuglog.h"
#include "command.h"
#include "sys_event.h"
#include "serial.h"
#include "stm32f746xx.h"
#include "test_usbh.h"
#include "hal_usb_host.h"
#include "hal_sys_ctl.h"

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
/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    HAL_SYS_CTL_Init(NULL);
    
    /* initialize the uart */
    console_init(1,115200);
    dlog_info("cpu1 start!!! \n");

    CPU_CACHE_Enable();

    HAL_USB_InitHost(HAL_USB_HOST_PORT_1, HAL_USB_HOST_CLASS_UVC);

    /* We should never get here as control is now taken by the scheduler */
    for( ;; )
    {
        SYS_EVENT_Process();

        HAL_USB_HostProcess();

        USBH_ProcUVC();

        if (command_getEnterStatus() == 1)
        {
            command_fulfill();
        }

        dlog_output(100);
    }
} 

