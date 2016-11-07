#include <stdint.h>
#include <string.h>
#include "serial.h"
#include "debuglog.h"
#include "command.h"
#include "interrupt.h"
#include "quad_spi_ctrl.h"
#include "systicks.h"

#define ITCM0_START                0x00000000
#define ITCM1_START                0x44100000
#define ITCM2_START                0xB0000000
#define APPLICATION_IMAGE_START    0x10010000
#define MCU2_CPU_WAIT              0x40B000CC  /* ENABLE CPU1 */
#define MCU3_CPU_WAIT              0xA0030088  /* ENABLE CPU2 */
#define MCU0_VECTOR_TABLE_REG      0xE000ED08

#define GET_WORD_FROM_ANY_ADDR(any_addr) ((uint32_t)(*any_addr) | \
                                         (((uint32_t)(*(any_addr+1))) << 8) | \
                                         (((uint32_t)(*(any_addr+2))) << 16) | \
                                         ((uint32_t)((*(any_addr+3))) << 24))

uint8_t g_bootloadmode = 0;

static void console_init(uint32_t uart_num, uint32_t baut_rate)
{
    serial_init(uart_num, baut_rate);
    dlog_init(uart_num);
    command_init(uart_num);
}

static void boot_app(void)
{
    uint32_t iCount = 0;

    uint8_t* cpu0_app_size_addr = (uint8_t*)APPLICATION_IMAGE_START;
    uint32_t cpu0_app_size = GET_WORD_FROM_ANY_ADDR(cpu0_app_size_addr);
    uint32_t cpu0_app_start_addr = APPLICATION_IMAGE_START + 4;

    uint8_t* cpu1_app_size_addr = (uint8_t*)(cpu0_app_start_addr + cpu0_app_size);
    uint32_t cpu1_app_size = GET_WORD_FROM_ANY_ADDR(cpu1_app_size_addr);
    uint32_t cpu1_app_start_addr = cpu0_app_start_addr + cpu0_app_size + 4;

    uint8_t* cpu2_app_size_addr = (uint8_t*)(cpu1_app_start_addr + cpu1_app_size);
    uint32_t cpu2_app_size = GET_WORD_FROM_ANY_ADDR(cpu2_app_size_addr);
    uint32_t cpu2_app_start_addr = cpu1_app_start_addr + cpu1_app_size + 4;

    dlog_info("cpu0_app_start_addr 0x%x", cpu0_app_start_addr);
    dlog_info("cpu0_app_size 0x%x", cpu0_app_size);

    dlog_info("cpu1_app_start_addr 0x%x", cpu1_app_start_addr);
    dlog_info("cpu1_app_size 0x%x", cpu1_app_size);

    dlog_info("cpu2_app_start_addr 0x%x", cpu2_app_start_addr);
    dlog_info("cpu2_app_size 0x%x", cpu2_app_size);

    dlog_output(1000);

    memcpy((void*)ITCM0_START, (void*)cpu0_app_start_addr, cpu0_app_size);
    memcpy((void*)ITCM1_START, (void*)cpu1_app_start_addr, cpu1_app_size);
    memcpy((void*)ITCM2_START, (void*)cpu2_app_start_addr, cpu2_app_size);

    dlog_info("Enable cpu1 ...");
    dlog_output(200);
    *((volatile uint32_t*)MCU2_CPU_WAIT) = 0;

    dlog_info("Enable cpu2 ...");
    dlog_output(200);
    *((volatile uint32_t*)MCU3_CPU_WAIT) = 0;

    dlog_info("Jump to cpu0 ...");
    dlog_output(200);
    *((uint32_t*)MCU0_VECTOR_TABLE_REG) = 0;
    (*((void(*)())((*((uint32_t*)(ITCM0_START+4))))))();
}

/**
  * @brief  Main program
  * @param  None
  * @retval None
  */
int main(void)
{
    /* initialize the uart */
    console_init(0, 360000); //115200 in 200M CPU clock
    dlog_info("bootload start!!! \n");
    dlog_output(100);

    SysTicks_Init(64000);
    QUAD_SPI_SetSpeed(QUAD_SPI_SPEED_50M);

    SysTicks_DelayMS(100);

    if (g_bootloadmode == 0)
    {
        SysTicks_UnInit(); 
        boot_app();
    }

    dlog_info("Running in bootload mode");
    dlog_output(100);

    for( ;; )
    {
        if (command_getEnterStatus() == 1)
        {
            command_fulfill();
        }

        dlog_output(100);
        SysTicks_DelayMS(20);
    }

} 

