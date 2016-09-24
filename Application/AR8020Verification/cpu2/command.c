#include <string.h>
#include <stdlib.h>
#include "command.h"
#include "debuglog.h"
#include "interrupt.h"
#include "serial.h"
#include "stm32f746xx.h"
#include "cmsis_os.h"
#include "test_h264_encoder.h"

static unsigned char g_commandPos;
static char g_commandLine[50];

static void Drv_UART_IRQHandler(void)
{
    char                  c;
    unsigned int          status;
    unsigned int          isrType;
    volatile uart_type   *uart_regs;

    uart_regs = (uart_type *)UART9_BASE;
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
                    command_parse(g_commandLine);
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
                g_commandLine[g_commandPos++] = c;
            }
        }
    }
}

void command_init(void)
{
    g_commandPos = 0;
    memset(g_commandLine, '\0', 50);
    reg_IrqHandle(VIDEO_UART9_INTR_VECTOR_NUM, Drv_UART_IRQHandler);
    INTR_NVIC_EnableIRQ(VIDEO_UART9_INTR_VECTOR_NUM);
}

void command_reset(void)
{
    g_commandPos = 0;
    memset(g_commandLine, '\0', 50);
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

void command_run(char *cmdArray[], unsigned int cmdNum)
{
    if (memcmp(cmdArray[0], "encoder_dump_brc", strlen("encoder_dump_brc")) == 0)
    {
        command_encoder_dump_brc();
    }
    else if (memcmp(cmdArray[0], "encoder_update_brc", strlen("encoder_update_brc")) == 0)
    {
        command_encoder_update_brc(cmdArray[1]);
    }
    else if (memcmp(cmdArray[0], "encoder_update_video_format", strlen("encoder_update_video_format")) == 0)
    {
        command_encoder_update_video(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    else 
    {
        dlog_error("Command not found! Please use commands like:\n");
        dlog_error("encoder_dump_brc");
        dlog_error("encoder_update_brc <br>");
        dlog_error("encoder_update_video_format <W> <H> <F>");
    }

    /* must reset to receive new data from serial */
    command_reset();
}
