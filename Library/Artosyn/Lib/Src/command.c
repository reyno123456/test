#include "command.h"
#include "serial.h"
#include "sd_host.h"

unsigned char g_commandPos;
char g_commandLine[50];

void command_init(void)
{
    g_commandPos = 0;
    memory_set(g_commandLine, '\0', 50);
}

void command_parse(char *cmd)
{
    unsigned char cmdIndex;
    char *tempCommand[5];

    cmdIndex = 0;
    memory_set(tempCommand, 0, 5);

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
    serial_puts("command_run\n");
    serial_puts(cmdArray[0]);
    serial_putc('\n');
    /* read memory: "read $(address)" */
    if ((memory_cmp(cmdArray[0], "read", 4) == 0) && (cmdNum == 2))
    {
        command_readMemory(cmdArray[1]);
    }
    /* write memory: "write $(address) $(data)" */
    else if ((memory_cmp(cmdArray[0], "write", 5) == 0) && (cmdNum == 3))
    {
        command_writeMemory(cmdArray[1], cmdArray[2]);
    }
    /* read sdcard: "readsd $(startBlock) $(blockNum)" */
    else if (memory_cmp(cmdArray[0], "readsd", 6) == 0)
    {
        command_readSdcard(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    /* write sdcard: "writesd $startBlock) $(blockNum) $(data)" */
    else if (memory_cmp(cmdArray[0], "writesd", 7) == 0)
    {
        command_writeSdcard(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    else if (memory_cmp(cmdArray[0], "erasesd", 7) == 0)
    {
        command_eraseSdcard(cmdArray[1], cmdArray[2]);
    }
    /* error command */
    else
    {
        serial_puts("command not found!\n");
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

    strSize = memory_strlen(str);

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
        serial_puts("read address is illegal");
        return;
    }

    /* align to 4 bytes */
    readAddress -= (readAddress % 4);

    /* print to serial */
    for (row = 0; row < 8; row++)
    {
        /* new line */
        serial_puts("0x");
        print_str(readAddress);
        serial_putc(':');
        serial_putc(' ');

        for (column = 0; column < 8; column++)
        {
            print_str(*((unsigned int *)readAddress));
            serial_putc(' ');
            readAddress += 4;
        }
        serial_putc('\n');
    }
    serial_putc('\n');
}

void command_writeMemory(char *addr, char *value)
{
    unsigned int writeAddress;
    unsigned int writeValue;

    writeAddress = command_str2uint(addr);

    if (writeAddress == 0xFFFFFFFF)
    {
        serial_puts("write address is illegal");
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


    serial_puts("readSdcardBuff = ");
    print_str(readSdcardBuff);
    serial_putc('\n');

    serial_puts("iDstAddr = ");
    print_str(iDstAddr);
    serial_putc('\n');

    serial_puts("iBytesNum = ");
    print_str(iBytesNum);
    serial_putc('\n');

    serial_puts("iSrcAddr = ");
    print_str(iSrcAddr);
    serial_putc('\n');


#if 0
    readSdcardBuff = m7_malloc(iBytesNum);
    memory_set(readSdcardBuff, '\0', iBytesNum);
    bufferPos = readSdcardBuff;

    /* read from sdcard */
    sd_read(&sdhandle, bufferPos, iBytesNum, iSrcAddr);

    /* print to serial */
    for (blockIndex = iDstAddr; blockIndex <= (iDstAddr + iBytesNum / dma.BlockSize); blockIndex++)
    {
        serial_puts("==============block:");
        print_str(blockIndex);
        serial_puts("==============\n");

        for (rowIndex = 0; rowIndex < 16; rowIndex++)
        {
            /* new line */
            serial_puts("0x");
            print_str((unsigned int)((rowIndex << 5) + (blockIndex << 9)));
            serial_putc(':');
            serial_putc(' ');

            for (columnIndex = 0; columnIndex < 8; columnIndex++)
            {
                print_str(*((unsigned int *)bufferPos));
                serial_putc(' ');
                bufferPos += 4;
            }
            serial_putc('\n');
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
    memory_set(writeSdcardBuff, SrcAddr, iBytesNum);

    /* write to sdcard */
    sd_write(&sdhandle, iDstAddr, iBytesNum, writeSdcardBuff);
    m7_free(writeSdcardBuff);
#endif

    serial_puts("writeSdcardBuff = ");
    print_str(writeSdcardBuff);
    serial_putc('\n');

    serial_puts("iDstAddr = ");
    print_str(iDstAddr);
    serial_putc('\n');

    serial_puts("iBytesNum = ");
    print_str(iBytesNum);
    serial_putc('\n');

    serial_puts("iSrcAddr = ");
    print_str(iSrcAddr);
    serial_putc('\n');
}

void command_eraseSdcard(char *startBlock, char *blockNum)
{
    unsigned int iStartBlock;
    unsigned int iBlockNum;
    iStartBlock = command_str2uint(startBlock);
    iBlockNum   = command_str2uint(blockNum);

    serial_puts("startBlock = ");
    print_str(iStartBlock);
    serial_putc('\n');

    serial_puts("blockNum = ");
    print_str(iBlockNum);
    serial_putc('\n');

    //sd_erase(&sdhandle, iStartBlock, iBlockNum);

}

void delay_ms(uint32_t num)
{
    int i;
    for (i = 0; i < num * 100; i++);
}

void write_reg32(uint32_t *addr, uint32_t data)
{
#ifdef ECHO
    serial_puts("Write ADDR = ");
    print_str((uint32_t)addr);
    serial_putc('\n');
    serial_puts("Write data = ");
    print_str(data);
    serial_putc('\n');
#endif
    uint32_t *reg_addr = (uint32_t *)addr;
    *reg_addr = data;
}

uint32_t read_reg32(uint32_t *addr)
{
#ifdef ECHO
    serial_puts("Read ADDR = ");
    print_str((uint32_t)addr);
    serial_putc('\n');
#endif
    uint32_t *reg_addr = (uint32_t *)addr;
#ifdef ECHO
    serial_puts("Read data = ");
    print_str(*reg_addr);
    serial_putc('\n');
#endif
    return (*reg_addr);
}
