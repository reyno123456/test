#include <string.h>
#include <stdlib.h>
#include "command.h"
#include "debuglog.h"
#include "interrupt.h"
#include "serial.h"
#include "cmsis_os.h"
#include "bb_ctrl.h"
#include "test_BB.h"
#include "test_spi.h"
#include "test_timer.h"
#include "test_h264_encoder.h"
#include "test_i2c_adv7611.h"
#include "test_sysevent.h"
#include "test_float.h"

static unsigned char g_commandPos;
static char g_commandLine[50];
static unsigned char g_commandEnter = 0;
uint32_t UartNum;

extern void BB_uart10_spi_sel(uint32_t sel_dat);

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
                g_commandLine[g_commandPos++] = c;
            }
        }
    }
}

void command_init()
{
    g_commandPos = 0;
    memset(g_commandLine, '\0', 50);
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
    INTR_NVIC_EnableIRQ(vector_num);
    INTR_NVIC_SetIRQPriority(vector_num, 1);
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
    else if (memcmp(cmdArray[0], "BB_data_buf_full", strlen("BB_data_buf_full")) == 0)
    {
        command_TestSysEventInterCore(cmdArray[1]);
    }
    else if (memcmp(cmdArray[0], "BB_uart10_spi_sel", strlen("BB_uart10_spi_sel")) == 0)
    {
        BB_uart10_spi_sel( strtoul(cmdArray[1], NULL, 0) );
    }
    else if (memcmp(cmdArray[0], "test_SysEventIdle", strlen("test_SysEventIdle")) == 0)
    {
        command_TestSysEventIdle();
    }

    else if(memcmp(cmdArray[0], "command_test_BB_uart", strlen("command_test_BB_uart")) == 0)
    {
        command_test_BB_uart(cmdArray[1]);
    }
    else if (memcmp(cmdArray[0], "test_float_calculate_pi", strlen("test_float_calculate_pi")) == 0)
    {
        test_float_calculate_pi();
    }
    //else if(memcmp(cmdArray[0], "grd_add_cmds", strlen("grd_add_cmds")) == 0)
    //{
    //    grd_add_spi_cmds(strtoul(cmdArray[1], NULL, 0), strtoul(cmdArray[2], NULL, 0));
    //}	
    else 
    {
        dlog_error("Command not found! Please use commands like:\n");
        dlog_error("encoder_dump_brc");
        dlog_error("encoder_update_brc <br>");
        dlog_error("encoder_update_video_format <W> <H> <F>");
        dlog_error("BB_data_buf_full <R>");
        dlog_error("BB_debug_print_init_sky");
        dlog_error("BB_debug_print_init_grd");
        dlog_error("BB_SPI_ReadByte <page> <addr>");
        dlog_error("BB_SPI_WriteByte <page> <addr> <value>");
        dlog_error("BB_uart10_spi_sel <value>");
        dlog_error("test_nor_flash_all <flash start address> <size> <value>");
        dlog_error("test_SysEventIdle");
        dlog_error("command_test_BB_uart");
        dlog_error("test_float_calculate_pi");
        dlog_error("grd_add_spi_cmds <type> <value>");		
    }

    /* must reset to receive new data from serial */
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

