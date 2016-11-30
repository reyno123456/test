#include <stdint.h>
#include <string.h>
#include "command.h"
#include "serial.h"
#include "debuglog.h"
#include "interrupt.h"
#include "quad_spi_ctrl.h"
#include "systicks.h"
#include "sd_boot.h"
#include "boot.h"
static unsigned char g_commandPos;
static char g_commandLine[50];
static unsigned char g_commandEnter = 0;
static uint32_t UartNum;
extern uint8_t g_bootloadmode;
//uint32_t g_sendUSBFlag = 0;
extern void boot_app(void);
extern void check(void);
void Drv_UART_IRQHandler(void)
{
    char                  c;
    unsigned int          status;
    unsigned int          isrType;
    volatile uart_type   *uart_regs;
    switch(UartNum)
    {
        case 0:
            uart_regs = (uart_type *)UART0_BASE;
            break;
        case 1:
            uart_regs = (uart_type *)UART1_BASE;
            break;
        case 2:
            uart_regs = (uart_type *)UART2_BASE;
            break;
        case 3:
            uart_regs = (uart_type *)UART3_BASE;
            break;
        case 4:
            uart_regs = (uart_type *)UART4_BASE;
            break;
        case 5:
            uart_regs = (uart_type *)UART5_BASE;
            break;
        case 6:
            uart_regs = (uart_type *)UART6_BASE;
            break;
        case 7:
            uart_regs = (uart_type *)UART7_BASE;
            break;
        case 8:
            uart_regs = (uart_type *)UART8_BASE;
            break;
        case 9:
            uart_regs = (uart_type *)UART9_BASE;
            break;
        default:
            break;
    }
    status     = uart_regs->LSR;
    isrType    = uart_regs->IIR_FCR;

    /* receive data irq, try to get the data */
    if (UART_IIR_RECEIVEDATA == (isrType & UART_IIR_RECEIVEDATA))
    {
        if ((status & UART_LSR_DATAREADY) == UART_LSR_DATAREADY)
        {
            c = uart_regs->RBR_THR_DLL;
            /* receive "enter" key */
            if (c == '\r')
            {
                serial_putc('\n');

                /* if g_commandLine is not empty, go to parse command */
                if (g_commandPos > 0)
                {
                    g_commandEnter = 1;
                }
            }
            /* receive "backspace" key */
            else if (c == '\b')
            {
                if (g_commandPos > 1)
                    g_commandLine[--g_commandPos] = '\0';
                serial_putc('\b');
                serial_putc(' ');
                serial_putc('\b');
            }
            /* receive normal data */
            else
            {
                serial_putc(c);
                if (g_commandPos < sizeof(g_commandLine))
                {
                    g_commandLine[g_commandPos++] = c;
                }
            }

            if ((c == 't') && (g_bootloadmode == 0))
            {
                if (g_commandPos >= 3)
                {
                     if ((g_commandLine[g_commandPos-1] == 't') && (g_commandLine[g_commandPos-2] == 't') && (g_commandLine[g_commandPos-3] == 't'))
                     {
                         g_bootloadmode = 1;
                     }
                }
            }
        }
    }
}

void command_init(uint8_t uart_num)
{
    g_commandPos = 0;
    memset(g_commandLine, '\0', 50);
    UartNum = uart_num;
    IRQ_type vector_num;
    switch(UartNum)
    {
        case 0:
            vector_num = UART_INTR0_VECTOR_NUM;
            break;
        case 1:
            vector_num = UART_INTR1_VECTOR_NUM;
            break;
        case 2:
            vector_num = UART_INTR2_VECTOR_NUM;
            break;
        case 3:
            vector_num = UART_INTR3_VECTOR_NUM;
            break;
        case 4:
            vector_num = UART_INTR4_VECTOR_NUM;
            break;
        case 5:
            vector_num = UART_INTR5_VECTOR_NUM;
            break;
        case 6:
            vector_num = UART_INTR6_VECTOR_NUM;
            break;
        case 7:
            vector_num = UART_INTR7_VECTOR_NUM;
            break;
        case 8:
            vector_num = UART_INTR8_VECTOR_NUM;
            break;
        case 9:
            vector_num = VIDEO_UART9_INTR_VECTOR_NUM;
            break;
        default:
            break;
    }

    reg_IrqHandle(vector_num, Drv_UART_IRQHandler);
    INTR_NVIC_SetIRQPriority(vector_num, 1);
    INTR_NVIC_EnableIRQ(vector_num);
}

void command_uninit(void)
{
    IRQ_type vector_num;
    switch(UartNum)
    {
        case 0:
            vector_num = UART_INTR0_VECTOR_NUM;
            break;
        case 1:
            vector_num = UART_INTR1_VECTOR_NUM;
            break;
        case 2:
            vector_num = UART_INTR2_VECTOR_NUM;
            break;
        case 3:
            vector_num = UART_INTR3_VECTOR_NUM;
            break;
        case 4:
            vector_num = UART_INTR4_VECTOR_NUM;
            break;
        case 5:
            vector_num = UART_INTR5_VECTOR_NUM;
            break;
        case 6:
            vector_num = UART_INTR6_VECTOR_NUM;
            break;
        case 7:
            vector_num = UART_INTR7_VECTOR_NUM;
            break;
        case 8:
            vector_num = UART_INTR8_VECTOR_NUM;
            break;
        case 9:
            vector_num = VIDEO_UART9_INTR_VECTOR_NUM;
            break;
        default:
            break;
    }

    INTR_NVIC_DisableIRQ(vector_num);
}

void command_reset(void)
{
    g_commandPos = 0;
    memset(g_commandLine, '\0', 50);
}
unsigned char command_getEnterStatus(void)
{
    return g_commandEnter;
}

void command_run(char *cmdArray[], unsigned int cmdNum)
{

    if (memcmp(cmdArray[0], "run_app", strlen("run_app")) == 0)
    {
        BOOTLOAD_CopyFromNorToITCM();
        BOOTLOAD_BootApp();
    }
    else if (memcmp(cmdArray[0], "sd_upgradeapp", strlen("sd_upgradeapp")) == 0)
    {
        BOOTLOAD_UpdataFromSDToNor();
    }
    else if (memcmp(cmdArray[0], "sd_boot", strlen("sd_boot")) == 0)
    {
        BOOTLOAD_BootFromSD();
    }
    /*else if (memcmp(cmdArray[0], "sd_upgradeappwithcheck", strlen("sd_upgradeappwithcheck")) == 0)
    {
        BOOTLOAD_BootFromSD();
    }
    else if (memcmp(cmdArray[0], "sd_bootwithcheck", strlen("sd_bootwithcheck")) == 0)
    {
        BOOTLOAD_BootFromSD();
    }*/
    else
    {
        dlog_error("%s Command not found. Please use the commands like:",cmdArray[0]);
        dlog_error("run_app");
        dlog_error("sd_upgradeapp");
        dlog_error("sd_boot");
        /*
        dlog_error("uart_updataBoot");
        dlog_error("sd_updataBoot");
        dlog_error("usb_updataBoot");
        
        
        dlog_error("usb_updataApp");
        dlog_error("data");                       
        dlog_error("usb_boot");
        */
    }
    command_reset();

}

void command_parse(char *cmd)
{
    unsigned char cmdIndex;
    char *tempCommand[5];

    cmdIndex = 0;
    memset(tempCommand, 0, 5);

    while (cmdIndex < 5)
    {
        /* skip the sapce */
        while ((*cmd == ' ') || (*cmd == '\t'))
        {
            ++cmd;
        }

        /* end of the cmdline */
        if (*cmd == '\0')
        {
            tempCommand[cmdIndex] = 0;
            break;
        }

        tempCommand[cmdIndex++] = cmd;

        /* find the end of string */
        while (*cmd && (*cmd != ' ') && (*cmd != '\t'))
        {
            ++cmd;
        }

        /* no more command */
        if (*cmd == '\0')
        {
            tempCommand[cmdIndex] = 0;
            break;
        }

        /* current cmd is end */
        *cmd++ = '\0';
    }

    command_run(tempCommand, cmdIndex);
}

void command_fulfill(void)
{
    command_parse(g_commandLine);
    g_commandEnter = 0;
}

void delay_ms(uint32_t num)
{
    volatile int i;
    for (i = 0; i < num * 100; i++);
}
