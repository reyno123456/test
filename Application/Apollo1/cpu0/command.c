#include <string.h>
#include <stdlib.h>
#include "command.h"
#include "debuglog.h"
#include "serial.h"
#include "interrupt.h"
#include "upgrade.h"
#include "cmsis_os.h"

static unsigned char g_commandPos;
static char g_commandLine[50];
static unsigned char g_commandEnter = 0;
uint32_t UartNum;

void command_readMemory(char *addr);
void command_writeMemory(char *addr, char *value);

/* added by xiongjiangjiang */
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
    /* read memory: "read $(address)" */
    if ((memcmp(cmdArray[0], "read", 4) == 0) && (cmdNum == 2))
    {
        command_readMemory(cmdArray[1]);
    }
    /* write memory: "write $(address) $(data)" */
    else if ((memcmp(cmdArray[0], "write", 5) == 0) && (cmdNum == 3))
    {
        command_writeMemory(cmdArray[1], cmdArray[2]);
    }
	else if (memcmp(cmdArray[0], "upgrade", strlen("upgrade")) == 0)
    {
        char path[128];
        memset(path,'\0',128);
        if(strlen(cmdArray[1])>127)
        {
            command_reset();
            return;
        }
        memcpy(path,cmdArray[1],strlen(cmdArray[1]));
        path[strlen(cmdArray[1])]='\0';
        osThreadDef(UsbUpgrade, UPGRADE_Upgrade, osPriorityNormal, 0, 15 * 128);
        osThreadCreate(osThread(UsbUpgrade), path);
        vTaskDelay(100);       
    }
    /* error command */
    else
    {
        dlog_error("Command not found. Please use the commands like:");
        dlog_error("read <address>");
        dlog_error("write <address> <data>");
        dlog_error("upgrade <filename>");
		dlog_output(1000);
    }

    /* must init to receive new data from serial */
    command_reset();
}

void command_parse(char *cmd)
{
    unsigned char cmdIndex;
    char *tempCommand[7];

    cmdIndex = 0;
    memset(tempCommand, 0, 5);

    while (cmdIndex < 7)
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

unsigned int command_str2uint(char *str)
{
    unsigned int ret;
    unsigned char strSize;
    unsigned char i;

    ret = 0;

    /* the format of param can be: 0x12345678 or 12345678 */
    if ((str[0] == '0') && (str[1] == 'x'))
    {
        str += 2;
    }

    strSize = strlen(str);

    /* the number should not exceed 8 bits */
    if (strSize > 8)
    {
        return ret;
    }

    for (i = 0; i < strSize; i++)
    {
        ret = ret << 4;

        if ((str[i] <= '9') && (str[i] >= '0'))
        {
            ret += (str[i] - '0');
        }
        else if ((str[i] <= 'f') && (str[i] >= 'a'))
        {
            ret += ((str[i] - 'a') + 0xa);
        }
        else if ((str[i] <= 'F') && (str[i] >= 'A'))
        {
            ret += ((str[i] - 'A') + 0xa);
        }
        else
        {
            ret = 0xFFFFFFFF;
            break;
        }
    }

    return ret;
}

void command_readMemory(char *addr)
{
    unsigned int readAddress;
    unsigned char row;
    unsigned char column;

    readAddress = command_str2uint(addr);

    if (readAddress == 0xFFFFFFFF)
    {

        dlog_error("read address is illegal\n");

        return;
    }

    /* align to 4 bytes */
    readAddress -= (readAddress % 4);

    /* print to serial */
    for (row = 0; row < 8; row++)
    {
        /* new line */
        dlog_info("0x%08x: 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x\n ", 
                  readAddress,
                  *(uint32_t *)readAddress,
                  *(uint32_t *)(readAddress + 4),
                  *(uint32_t *)(readAddress + 8),
                  *(uint32_t *)(readAddress + 12),
                  *(uint32_t *)(readAddress + 16),
                  *(uint32_t *)(readAddress + 20),
                  *(uint32_t *)(readAddress + 24),
                  *(uint32_t *)(readAddress + 28));

        readAddress += 32;
    }
}

void command_writeMemory(char *addr, char *value)
{
    unsigned int writeAddress;
    unsigned int writeValue;

    writeAddress = command_str2uint(addr);

    if (writeAddress == 0xFFFFFFFF)
    {

        dlog_error("write address is illegal\n");

        return;
    }

    writeValue   = command_str2uint(value);

    *((unsigned int *)(writeAddress)) = writeValue;
}


