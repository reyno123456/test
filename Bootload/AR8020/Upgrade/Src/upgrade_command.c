#include <stdint.h>
#include <string.h>
#include "upgrade_command.h"
#include "serial.h"
#include "debuglog.h"
#include "interrupt.h"
#include "upgrade_sd.h"
#include "upgrade_uart.h"

static uint8_t g_8CommandPos;
static int8_t g_8CommandLine[50];
static uint8_t g_u8CommandEnter = 0;

void UPGRADE_UartIRQHandler(void)
{
    char                  c;
    uint32_t          status;
    uint32_t          isrType;
    volatile uart_type   *uart_regs = (uart_type *)UART0_BASE;
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

                /* if g_8CommandLine is not empty, go to parse command */
                if (g_8CommandPos > 0)
                {
                    g_u8CommandEnter = 1;
                }
            }
            /* receive "backspace" key */
            else if (c == '\b')
            {
                if (g_8CommandPos > 1)
                    g_8CommandLine[--g_8CommandPos] = '\0';
                serial_putc('\b');
                serial_putc(' ');
                serial_putc('\b');
            }
            /* receive normal data */
            else
            {
                serial_putc(c);
                if (g_8CommandPos < sizeof(g_8CommandLine))
                {
                    g_8CommandLine[g_8CommandPos++] = c;
                }
            }
        }
    }
}

void UPGRADE_CommandInit(uint8_t uart_num)
{
    g_8CommandPos = 0;
    memset(g_8CommandLine, '\0', 50);
    reg_IrqHandle(UART_INTR0_VECTOR_NUM, UPGRADE_UartIRQHandler);
    INTR_NVIC_SetIRQPriority(UART_INTR0_VECTOR_NUM, 1);
    INTR_NVIC_EnableIRQ(UART_INTR0_VECTOR_NUM);
}

void UPGRADE_CommandReset(void)
{
    g_8CommandPos = 0;
    memset(g_8CommandLine, '\0', 50);
}
unsigned char UPGRADE_CommandGetEnterStatus(void)
{
    return g_u8CommandEnter;
}

void UPGRADE_CommandRun(char *cmdArray[], uint32_t cmdNum)
{

    if (memcmp(cmdArray[0], "sd_upgradeapp", strlen("sd_upgradeapp")) == 0)
    {
        UPGRADE_UpdataFromSDToNor();
    }
    /*else if (memcmp(cmdArray[0], "sd_boot", strlen("sd_boot")) == 0)
    {
        UPGRADE_BootFromSD();
    }*/
    else if (memcmp(cmdArray[0], "uart_upgradeapp", strlen("uart_upgradeapp")) == 0)
    {
        UPGRADE_APPFromUart();
    }
    else if (memcmp(cmdArray[0], "uart_boot", strlen("uart_boot")) == 0)
    {
        UPGRADE_BootloadFromUart();
    }
    else
    {
        dlog_error("%s Command not found. Please use the commands like:",cmdArray[0]);
        dlog_error("sd_upgradeapp");
       // dlog_error("sd_boot");
        dlog_error("uart_upgradeapp");
        dlog_error("uart_boot");
    }
    UPGRADE_CommandReset();

}

void UPGRADE_CommandParse(char *cmd)
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

    UPGRADE_CommandRun(tempCommand, cmdIndex);
}

void UPGRADE_CommandFulfill(void)
{
    UPGRADE_CommandParse(g_8CommandLine);
    g_u8CommandEnter = 0;
}

void delay_ms(uint32_t num)
{
    volatile int i;
    for (i = 0; i < num * 100; i++);
}
