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
#include "test_hal_i2c_24c256.h"
#include "test_hal_i2c.h"
#include "testhal_gpio.h"
#include "test_usbh.h"
#include "test_hal_nv.h"



void command_readMemory(char *addr);
void command_writeMemory(char *addr, char *value);
void command_startBypassVideo(void);
void command_stopBypassVideo(void);

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
    else if (memcmp(cmdArray[0], "testhal24c256", strlen("testhal24c256")) == 0)
    {
        commandhal_Test24C256(cmdArray[1], cmdArray[2]);
    }
    else if (memcmp(cmdArray[0], "test_hal_i2c_init", strlen("test_hal_i2c_init")) == 0)
    {
        command_TestHalI2cInit(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    else if (memcmp(cmdArray[0], "test_hal_i2c_write", strlen("test_hal_i2c_write")) == 0)
    {
        command_TestHalI2cWrite(cmdArray[1], cmdArray[2], cmdArray[3], cmdArray[4], cmdArray[5]);
    }
    else if (memcmp(cmdArray[0], "test_hal_i2c_read", strlen("test_hal_i2c_read")) == 0)
    {
        command_TestHalI2cRead(cmdArray[1], cmdArray[2], cmdArray[3], cmdArray[4]);
    }    
    else if (memcmp(cmdArray[0], "testhal_TestGpioNormal", strlen("testhal_TestGpioNormal")) == 0)
    {
        commandhal_TestGpioNormal(cmdArray[1], cmdArray[2]);
    }
    else if (memcmp(cmdArray[0], "testhal_TestGpioInterrupt", strlen("testhal_TestGpioInterrupt")) == 0)
    {
        commandhal_TestGpioInterrupt(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    else if (memcmp(cmdArray[0], "startbypassvideo", strlen("startbypassvideo")) == 0)
    {
        command_startBypassVideo();
    }
    else if (memcmp(cmdArray[0], "stopbypassvideo", strlen("stopbypassvideo")) == 0)
    {
        command_stopBypassVideo();
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
    /* error command */
    else if (memcmp(cmdArray[0], "help", strlen("help")) == 0)
    {
        dlog_error("Please use the commands like:");
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
        dlog_error("testhal24c256 <i2c port> <i2c_value>");
        dlog_error("test_hal_i2c_init <ch> <i2c_addr> <speed>");
        dlog_error("test_hal_i2c_write <ch> <subAddr> <subAddrLen> <data> <dataLen>");
        dlog_error("test_hal_i2c_read <ch> <subAddr> <subAddrLen> <dataLen>");
        dlog_error("testhal_TestGpioNormal <gpionum> <highorlow>");
        dlog_error("testhal_TestGpioInterrupt <gpionum> <inttype> <polarity>");
        dlog_error("startbypassvideo");
        dlog_error("stopbypassvideo");
        dlog_error("sky_auto_search_rc_id");
        dlog_error("NvResetBbRcId");
        dlog_error("NvSetBbRcId <id1> <id2> <id3> <id4> <id5>");
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

void command_startBypassVideo(void)
{
    USBH_APP_EVENT_DEF  usbhAppType;

    usbhAppType = USBH_APP_START_BYPASS_VIDEO;

    if (0 == g_usbhBypassVideoCtrl.taskActivate)
    {
        g_usbhBypassVideoCtrl.taskActivate  = 1;
        osMessagePut(g_usbhAppCtrl.usbhAppEvent, usbhAppType, 0);
    }
    else
    {
        dlog_error("Bypass Video Task is running\n");
    }
}

void command_stopBypassVideo(void)
{
    USBH_APP_EVENT_DEF  usbhAppType;

    usbhAppType = USBH_APP_STOP_BYPASS_VIDEO;

    if (1 == g_usbhBypassVideoCtrl.taskActivate)
    {
        g_usbhBypassVideoCtrl.taskActivate  = 0;
        osMessagePut(g_usbhAppCtrl.usbhAppEvent, usbhAppType, 0);
    }
    else
    {
        dlog_error("Bypass Video Task is not running\n");
    }
}
