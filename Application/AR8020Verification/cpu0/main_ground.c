#include "debuglog.h"
#include "test_freertos.h"
#include "test_i2c_adv7611.h"
#include "command.h"
#include "serial.h"
#include "hal_sram.h"
#include "cmsis_os.h"
#include "sys_event.h"
#include "upgrade.h"
#include "hal_bb.h"
#include "test_usbh.h"
#include "hal_usb_device.h"
#include "hal_sys_ctl.h"
#include "wireless_interface.h"

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

    HAL_USB_InitDevice(HAL_USB_DEVICE_PORT_0);

    HAL_SRAM_ReceiveVideoConfig();

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
