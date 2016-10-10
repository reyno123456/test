#include "command.h"
#include "sd_host.h"
#include "debuglog.h"
#include "interrupt.h"
#include "serial.h"
#include "sd_host.h"
#include "stm32f746xx.h"
#include "debuglog.h"
#include <string.h>
#include <stdlib.h>
#include "cmsis_os.h"
#include "test_freertos.h"

static unsigned char g_commandPos;
static char g_commandLine[50];
static unsigned char g_commandEnter = 0;

//QueueHandle_t xQueue = NULL;
osMessageQId usbVideoReadyEvent;
uint32_t g_sendUSBFlag = 0;

/* added by xiongjiangjiang */
void Drv_UART1_IRQHandler(void)
{
    char                  c;
    unsigned int          status;
    unsigned int          isrType;
    volatile uart_type   *uart0_regs;

    uart0_regs = (uart_type *)UART0_BASE; //get_uart_type_by_index(0);
    status     = uart0_regs->LSR;
    isrType    = uart0_regs->IIR_FCR;

    /* receive data irq, try to get the data */
    if (UART_IIR_RECEIVEDATA == (isrType & UART_IIR_RECEIVEDATA))
    {
        if ((status & UART_LSR_DATAREADY) == UART_LSR_DATAREADY)
        {
            c = uart0_regs->RBR_THR_DLL;
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

void command_init(void)
{
    g_commandPos = 0;
    memset(g_commandLine, '\0', 50);

    reg_IrqHandle(UART_INTR1_VECTOR_NUM, Drv_UART1_IRQHandler);
}

unsigned char command_getEnterStatus(void)
{
    return g_commandEnter;
}

void command_fulfill(void)
{
    command_parse(g_commandLine);
    g_commandEnter = 0;
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
    /* read sdcard: "readsd $(startBlock) $(blockNum)" */
    else if (memcmp(cmdArray[0], "readsd", 6) == 0)
    {
        command_readSdcard(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    /* write sdcard: "writesd $startBlock) $(blockNum) $(data)" */
    else if (memcmp(cmdArray[0], "writesd", 7) == 0)
    {
        command_writeSdcard(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    else if (memcmp(cmdArray[0], "erasesd", 7) == 0)
    {
        command_eraseSdcard(cmdArray[1], cmdArray[2]);
    }
    else if (memcmp(cmdArray[0], "sendusb", 7) == 0)
    {
        command_sendusb();
    }
    else if (memcmp(cmdArray[0], "freertos_task", strlen("freertos_task")) == 0)
    {
        command_TestTask();
    }
    else if (memcmp(cmdArray[0], "freertos_taskquit", strlen("freertos_taskquit")) == 0)
    {
        command_TestTaskQuit();
    }
    /* error command */
    else
    {
        dlog_error("Command not found. Please use the commands like:");
        dlog_error("read <address>");
        dlog_error("write <address> <data>");
        dlog_error("readsd <startBlock> <blockNum>");
        dlog_error("writesd <startBlock> <blockNum> <data>");
        dlog_error("erasesd");
        dlog_error("sendusb");
        dlog_error("hdmiinit");
        dlog_error("hdmigetvideoformat");
        dlog_error("hdmiread <slv address> <reg address>");
        dlog_error("freertos_task");
        dlog_error("freertos_taskquit");
    }

    /* must init to receive new data from serial */
    command_init();
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

        dlog_info("0x%08x: ", (unsigned int)readAddress);

        for (column = 0; column < 8; column++)
        {
            dlog_info("%08x ", *(unsigned int *)readAddress);

        dlog_info("0x%x: ", readAddress);

        for (column = 0; column < 8; column++)
        {
            dlog_info("0x%x: ", readAddress);

            readAddress += 4;
        }
        dlog_info("\n");
        }
        dlog_info("\n");
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

void command_readSdcard(char *Dstaddr, char *BytesNum, char *SrcAddr)
{
    unsigned int iDstAddr;
    unsigned int iBytesNum;
    unsigned int iSrcAddr;
    unsigned int rowIndex;
    unsigned int columnIndex;
    unsigned int blockIndex;
    char *readSdcardBuff;
    char *bufferPos;

    iDstAddr   = command_str2uint(Dstaddr);
    iBytesNum  = command_str2uint(BytesNum);
    iSrcAddr   = command_str2uint(SrcAddr);



    dlog_info("readSdcardBuff = 0x%08x\n", readSdcardBuff);

    dlog_info("iDstAddr = 0x%08x\n", iDstAddr);

    dlog_info("iBytesNum = 0x%08x\n", iBytesNum);

    dlog_info("iSrcAddr = 0x%08x\n", iSrcAddr);

#if 0
    readSdcardBuff = m7_malloc(iBytesNum);
    memset(readSdcardBuff, '\0', iBytesNum);
    bufferPos = readSdcardBuff;

    /* read from sdcard */
    sd_read(&sdhandle, bufferPos, iBytesNum, iSrcAddr);

    /* print to serial */
    for (blockIndex = iDstAddr; blockIndex <= (iDstAddr + iBytesNum / dma.BlockSize); blockIndex++)
    {
        dlog_info("block:");
        dlog_info("%x", blockIndex);
        dlog_info("==============\n");

        for (rowIndex = 0; rowIndex < 16; rowIndex++)
        {
            /* new line */
            dlog_info("0x");
            dlog_info("%x", (unsigned int)((rowIndex << 5) + (blockIndex << 9)));
            dlog_info(':');
            dlog_info(' ');

            for (columnIndex = 0; columnIndex < 8; columnIndex++)
            {
                dlog_info("%x", *((unsigned int *)bufferPos));
                dlog_info(' ');
                bufferPos += 4;
            }
            dlog_info('\n');
        }
    }
    m7_free(readSdcardBuff);
#endif

}

void command_writeSdcard(char *Dstaddr, char *BytesNum, char *SrcAddr)
{
    unsigned int iDstAddr;
    unsigned int iBytesNum;
    unsigned int iSrcAddr;
    char *writeSdcardBuff;

    iDstAddr    = command_str2uint(Dstaddr);
    iBytesNum   = command_str2uint(BytesNum);
    iSrcAddr    = command_str2uint(SrcAddr);
#if 0
    writeSdcardBuff = m7_malloc(iBytesNum);
    memset(writeSdcardBuff, SrcAddr, iBytesNum);

    /* write to sdcard */
    sd_write(&sdhandle, iDstAddr, iBytesNum, writeSdcardBuff);
    m7_free(writeSdcardBuff);
#endif


    dlog_info("writeSdcardBuff = 0x%08x\n", writeSdcardBuff);

    dlog_info("iDstAddr = 0x%08x\n", iDstAddr);

    dlog_info("iBytesNum = 0x%08x\n", iBytesNum);

    dlog_info("iSrcAddr = 0x%08x\n", iSrcAddr);

}

void command_eraseSdcard(char *startBlock, char *blockNum)
{
    unsigned int iStartBlock;
    unsigned int iBlockNum;
    iStartBlock = command_str2uint(startBlock);
    iBlockNum   = command_str2uint(blockNum);


    dlog_info("startBlock = 0x%08x\n", iStartBlock);
    //sd_erase(&sdhandle, iStartBlock, iBlockNum);
    dlog_info("blockNum = 0x%08x\n", iBlockNum);
}

void delay_ms(uint32_t num)
{
    volatile int i;
    for (i = 0; i < num * 100; i++);
}

void command_sendusb(void)
{
    uint32_t        ValueToSend = 0;
    portBASE_TYPE   xStatus = pdFALSE;

    ValueToSend++;

//    xStatus = xQueueSendFromISR(xQueue, &ValueToSend, &xStatus);
//    if (xStatus != pdPASS)
//    {
//        dlog_info("Could not send to the Queue\n");
//    }
    osMessagePut(usbVideoReadyEvent, ValueToSend, 0);

}




