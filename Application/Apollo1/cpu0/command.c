#include <string.h>
#include <stdlib.h>
#include "command.h"
#include "debuglog.h"
#include "serial.h"
#include "interrupt.h"
#include "upgrade.h"
#include "cmsis_os.h"
#include "testhal_pwm.h"
#include "test_hal_uart.h"
#include "test_hal_spi.h"
#include "test_hal_i2c.h"


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
	else if (memcmp(cmdArray[0], "upgrade", strlen("upgrade")) == 0)
    {
        char path[128];
        memset(path,'\0',128);
        if(strlen(cmdArray[1])>127)
        {
            return;
        }
        memcpy(path,cmdArray[1],strlen(cmdArray[1]));
        path[strlen(cmdArray[1])]='\0';
        osThreadDef(UsbUpgrade, UPGRADE_Upgrade, osPriorityNormal, 0, 15 * 128);
        osThreadCreate(osThread(UsbUpgrade), path);
        vTaskDelay(100);       
    }
    else if (memcmp(cmdArray[0], "testhal_Testpwm", strlen("testhal_Testpwm")) == 0)
    {
        commandhal_TestPwm(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    else if (memcmp(cmdArray[0], "test_hal_uart_init", strlen("test_hal_uart_init")) == 0)
    {
        command_TestHalUartInit(cmdArray[1], cmdArray[2]);
    }
    else if (memcmp(cmdArray[0], "test_hal_uart_set_int", strlen("test_hal_uart_set_int")) == 0)
    {
        command_TestHalUartIntSet(cmdArray[1], cmdArray[2]);
    }
    else if (memcmp(cmdArray[0], "test_hal_uart_tx", strlen("test_hal_uart_tx")) == 0)
    {
        command_TestHalUartTx(cmdArray[1], cmdArray[2]);
    }
    else if (memcmp(cmdArray[0], "test_hal_uart_rx", strlen("test_hal_uart_rx")) == 0)
    {
        command_TestHalUartRx(cmdArray[1]);
    }
    else if (memcmp(cmdArray[0], "test_hal_spi_init", strlen("test_hal_spi_init")) == 0)
    {
        command_TestHalSpiInit(cmdArray[1], cmdArray[2], cmdArray[3], cmdArray[4]);
    }
    else if (memcmp(cmdArray[0], "test_hal_spi_write", strlen("test_hal_spi_write")) == 0)
    {
        command_TestHalSpiTx(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    else if (memcmp(cmdArray[0], "test_hal_spi_read", strlen("test_hal_spi_read")) == 0)
    {
        command_TestHalSpiRx(cmdArray[1], cmdArray[2]);
    }
    else if (memcmp(cmdArray[0], "test_hal_i2c_init", strlen("test_hal_i2c_init")) == 0)
    {
        command_TestHalI2cInit(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    else if (memcmp(cmdArray[0], "test_hal_i2c_write", strlen("test_hal_i2c_write")) == 0)
    {
        command_TestHalI2cWrite(cmdArray[1]);
    }
    else if (memcmp(cmdArray[0], "test_hal_i2c_read", strlen("test_hal_i2c_read")) == 0)
    {
        command_TestHalI2cRead(cmdArray[1]);
    }
    /* error command */
    else
    {
        dlog_error("Command not found. Please use the commands like:");
        dlog_error("read <address>");
        dlog_error("write <address> <data>");
        dlog_error("upgrade <filename>");
        dlog_error("testhal_Testpwm <PWM Num> <PWM low> <PWM high>");
        dlog_error("test_hal_uart_init <ch> <baudr>");
        dlog_error("test_hal_uart_set_int <ch> <flag>");
        dlog_error("test_hal_uart_tx <ch> <len>");
        dlog_error("test_hal_uart_rx <ch>");
        dlog_error("test_hal_spi_init <ch> <baudr> <polarity> <phase>");
        dlog_error("test_hal_spi_write <ch> <addr> <wdata>");
        dlog_error("test_hal_spi_read <ch> <addr>");
        dlog_error("test_hal_i2c_init <ch> <i2c_addr> <speed>");
        dlog_error("test_hal_i2c_write <subAddr + data>");
        dlog_error("test_hal_i2c_read <sub_addr>");
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


