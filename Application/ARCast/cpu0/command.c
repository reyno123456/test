#include <string.h>
#include <stdlib.h>
#include "command.h"
#include "debuglog.h"
#include "test_hal_nv.h"
#include "test_storagedatausb.h"
#include "test_i2c_adv7611.h"

void command_readMemory(char *addr);
void command_writeMemory(char *addr, char *value);

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
    else if (memcmp(cmdArray[0], "storageaudio", strlen("storageaudio")) == 0)
    {
        writeAudioPcm(cmdArray[1]);
    }
    else if (memcmp(cmdArray[0], "sky_auto_search_rc_id", strlen("sky_auto_search_rc_id")) == 0)
    {
        command_TestNvSkyAutoSearhRcId();
    }
    else if (memcmp(cmdArray[0], "NvResetBbRcId", strlen("NvResetBbRcId")) == 0)
    {
        command_TestNvResetBbRcId();
    }
    else if (memcmp(cmdArray[0], "NvSetBbRcId", strlen("NvSetBbRcId")) == 0)
    {
        command_TestNvSetBbRcId(cmdArray[1],cmdArray[2],cmdArray[3],cmdArray[4],cmdArray[5]);
    }
    else if (memcmp(cmdArray[0], "hdmiread", strlen("hdmiread")) == 0)
    {
        command_readADV7611(cmdArray[1], cmdArray[2]);
    }
    else if (memcmp(cmdArray[0], "hdmiwrite", strlen("hdmiwrite")) == 0)
    {
        command_writeADV7611(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    else if (memcmp(cmdArray[0], "help", strlen("help")) == 0)
    {
        dlog_error("Please use the commands like:");
        dlog_error("read <address>");
        dlog_error("write <address> <data>");
        dlog_error("storageaudio <d1>");
        dlog_error("sky_auto_search_rc_id");
        dlog_error("NvResetBbRcId");
        dlog_error("NvSetBbRcId <id1> <id2> <id3> <id4> <id5>");
        dlog_error("hdmiread <slv address> <reg address>");
        dlog_error("hdmiwrite <slv address> <reg address> <reg value>");
        dlog_output(1000);
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

