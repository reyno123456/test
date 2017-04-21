#include "command.h"
#include "debuglog.h"
#include "interrupt.h"
#include "serial.h"
#include "stm32f746xx.h"
#include "debuglog.h"
#include <string.h>
#include <stdlib.h>
#include "cmsis_os.h"
#include "hal_sd.h"
#include "test_timer.h"
#include "test_gpio.h"
#include "test_i2c_adv7611.h"
#include "test_simulatepwm.h"
#include "hal_ret_type.h"
#include "hal_nvic.h"
#include "testhal_dma.h"

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

void command_run(char *cmdArray[], uint32_t cmdNum)
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
    //else if (memcmp(cmdArray[0], "readsd", 6) == 0)
    //{
    //    command_readSdcard(cmdArray[1], cmdArray[2]);
    //}
    /* write sdcard: "writesd $startBlock) $(blockNum) $(data)" */
    //else if (memcmp(cmdArray[0], "writesd", 7) == 0)
    //{
    //    command_writeSdcard(cmdArray[1], cmdArray[2], cmdArray[3]);
    //}
    //else if (memcmp(cmdArray[0], "erasesd", 7) == 0)
    //{
    //    command_eraseSdcard(cmdArray[1], cmdArray[2]);
    //}
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
    else if (memcmp(cmdArray[0], "test_dma_cpu1", strlen("test_dma_cpu1")) == 0)
    {
        command_dma(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    /* error command */
    else if (memcmp(cmdArray[0], "help", strlen("help")) == 0)
    {
        dlog_error("Please use the commands like:");
        dlog_error("read <address>");
        dlog_error("write <address> <data>");
        //dlog_error("readsd <startBlock> <blockNum>");
        //dlog_error("writesd <startBlock> <blockNum> <data>");
        //dlog_error("erasesd");
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
        dlog_error("test_nor_flash_all <flash start address> <size> <value>");
        dlog_error("test_TestGpioNormal <gpionum> <highorlow>");
        dlog_error("test_TestGpioNormalRange <gpionum1> <gpionum2> <highorlow>");
        dlog_error("test_TestGpioInterrupt <gpionum> <inttype> <polarity>");
        dlog_error("test_dma_cpu1 <src> <dst> <byte_num>");
    }
}

unsigned int command_str2uint(char *str)
{
    return strtoul(str, NULL, 0); 
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
    HAL_SD_Read(&sdhandle, bufferPos, iBytesNum, iSrcAddr);

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
    HAL_SD_Write(&sdhandle, iDstAddr, iBytesNum, writeSdcardBuff);
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
    // HAL_SD_Erase(&sdhandle, iStartBlock, iBlockNum);
    dlog_info("blockNum = 0x%08x\n", iBlockNum);
}

void delay_ms(uint32_t num)
{
    volatile int i;
    for (i = 0; i < num * 100; i++);
}





