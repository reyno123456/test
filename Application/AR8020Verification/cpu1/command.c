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
#include "test_spi.h"
#include "test_timer.h"
#include "test_gpio.h"
#include "test_i2c_adv7611.h"

static unsigned char g_commandPos;
static char g_commandLine[50];
static unsigned char g_commandEnter = 0;

uint32_t UartNum = 0;

void command_readMemory(char *addr);
void command_writeMemory(char *addr, char *value);
void command_initSdcard();
void command_readSdcard(char *Dstaddr, char *BlockNum);
void command_writeSdcard(char *Dstaddr, char *BlockNum, char *SrcAddr);
void command_eraseSdcard(char *startBlock, char *blockNum);
void command_startBypassVideo(void);
void command_stopBypassVideo(void);
void command_upgrade(void);
void command_sendCtrl(void);
void command_sendVideo(void);

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
    INTR_NVIC_SetIRQPriority(vector_num, 1);
    INTR_NVIC_EnableIRQ(vector_num);
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
    /* read sdcard: "readsd $(startBlock) $(blockNum)" */
    else if (memcmp(cmdArray[0], "readsd", 6) == 0)
    {
        command_readSdcard(cmdArray[1], cmdArray[2]);
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
    else if (memcmp(cmdArray[0], "test_timerall", strlen("test_timerall")) == 0)
    {
       command_TestTimAll();
    }
    else if (memcmp(cmdArray[0], "test_timerused", strlen("test_timerused")) == 0)
    {
       command_TestTimUsed();
    }
    else if (memcmp(cmdArray[0], "test_timer", strlen("test_timer")) == 0)
    {
       command_TestTim(cmdArray[1], cmdArray[2], cmdArray[3]);
    }    
    else if (memcmp(cmdArray[0], "test_pwmall", strlen("test_pwmall")) == 0)
    {
       command_TestPwmAll();
    }
    else if (memcmp(cmdArray[0], "test_pwm", strlen("test_pwm")) == 0)
    {
       command_TestPwm(cmdArray[1], cmdArray[2], cmdArray[3], cmdArray[4]);
    }    
    else if (memcmp(cmdArray[0], "test_spi_id", strlen("test_spi_id")) == 0)
    {
       command_WbFlashID(cmdArray[1]);
    }
    else if (memcmp(cmdArray[0], "test_spi_write_read", strlen("test_spi_write_read")) == 0)
    {        
        command_TestWbFlash(cmdArray[1]);                
    }
    else if (memcmp(cmdArray[0], "test_spi_erase", strlen("test_spi_erase")) == 0)
    {
       command_TestWbBlockErase(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    else if (memcmp(cmdArray[0], "test24c256write", strlen("test24c256write")) == 0)
    {
        command_Test24C256Write(cmdArray[1], cmdArray[1]);
    }
    else if (memcmp(cmdArray[0], "test24c256read", strlen("test24c256read")) == 0)
    {
        command_Test24C256Read(cmdArray[1]);
    }
    else if (memcmp(cmdArray[0], "test_TestGpioNormalRange", strlen("test_TestGpioNormalRange")) == 0)
    {
        command_TestGpioNormalRange(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    else if (memcmp(cmdArray[0], "test_TestGpioNormal", strlen("test_TestGpioNormal")) == 0)
    {
        command_TestGpioNormal(cmdArray[1], cmdArray[2]);
    }
    else if (memcmp(cmdArray[0], "test_TestGpioInterrupt", strlen("test_TestGpioInterrupt")) == 0)
    {
        command_TestGpioInterrupt(cmdArray[1], cmdArray[2], cmdArray[3]);
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
        dlog_error("test_timer <TIM Group> <TIM Num> <TIM Count>");
        dlog_error("test_timerall");
        dlog_error("test_timerused");
        dlog_error("test_pwm <PWM Group> <PWM Num> <PWM low> <PWM high>");
        dlog_error("test_pwmall");
        dlog_error("test_spi_id <spi_port>");
        dlog_error("test_spi_write_read <spi_port>");
        dlog_error("test_spi_erase <spi_port>");
        dlog_error("test_nor_flash_all <flash start address> <size> <value>");
        dlog_error("test_TestGpioNormal <gpionum> <highorlow>");
        dlog_error("test_TestGpioNormalRange <gpionum1> <gpionum2> <highorlow>");
        dlog_error("test_TestGpioInterrupt <gpionum> <inttype> <polarity>");
        dlog_error("test24c256write <i2c port> <i2c_value>");
        dlog_error("test24c256read <i2c port>");
    }

    /* must init to receive new data from serial */
    command_init();
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

void command_readSdcard(char *Dstaddr, char *BytesNum)
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





