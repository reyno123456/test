#include "command.h"
#include "debuglog.h"
#include "interrupt.h"
#include "serial.h"
#include "debuglog.h"
#include <string.h>
#include <stdlib.h>
#include "test_i2c_adv7611.h"
#include "test_freertos.h"
#include "test_timer.h"
#include "test_can.h"
#include "test_sd.h"
#include "hal_sd.h"
#include "test_spi.h"
#include "test_quadspi.h"
#include "test_gpio.h"
#include "test_usbh.h"
#include "test_float.h"
#include "test_ov5640.h"
#include "hal_dma.h"
#include "upgrade.h"
#include "test_bbuartcom.h"
#include "testhal_gpio.h"
#include "testhal_timer.h"
#include "testhal_pwm.h"
#include "testhal_softpwm.h"
#include "testhal_i2c.h"
#include "test_hal_uart.h"
#include "test_hal_spi.h"
#include "memory.h"

static unsigned char g_commandPos;
static char g_commandLine[50];
static unsigned char g_commandEnter = 0;
uint32_t UartNum;


void command_readMemory(char *addr);
void command_writeMemory(char *addr, char *value);
void command_readSdcard(char *Dstaddr, char *BlockNum);
void command_writeSdcard(char *Dstaddr, char *BlockNum, char *SrcAddr);
void command_eraseSdcard(char *startBlock, char *blockNum);
void command_startBypassVideo(void);
void command_stopBypassVideo(void);
void command_upgrade(void);
void command_sendCtrl(void);
void command_sendVideo(void);
void command_dma(char *,char *,char *);

/* added by xiongjiangjiang */
void Drv_UART_IRQHandler(uint32_t u32_vectorNum)
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
    reg_IrqHandle(vector_num, Drv_UART_IRQHandler, NULL);
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
    /* initialize sdcard: "initsd" */
    else if (memcmp(cmdArray[0], "initsd", 6) == 0)
    {
        command_initSdcard();
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
    else if (memcmp(cmdArray[0], "test_sdfs", 9) == 0)
    {
        command_SdcardFatFs();
    }
    else if (memcmp(cmdArray[0], "startbypassvideo", strlen("startbypassvideo")) == 0)
    {
        command_startBypassVideo();
    }
    else if (memcmp(cmdArray[0], "stopbypassvideo", strlen("stopbypassvideo")) == 0)
    {
        command_stopBypassVideo();
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
    else if (memcmp(cmdArray[0], "hdmiinit", strlen("hdmiinit")) == 0)
    {
        command_initADV7611(cmdArray[1]);
    }
    else if (memcmp(cmdArray[0], "hdmidump", strlen("hdmidump")) == 0)
    {
        command_dumpADV7611Settings(cmdArray[1]);
    }
    else if (memcmp(cmdArray[0], "hdmigetvideoformat", strlen("hdmigetvideoformat")) == 0)
    {
        uint16_t width, hight;
        uint8_t framterate;
        command_readADV7611VideoFormat(cmdArray[1], &width, &hight, &framterate);
        dlog_info("width %d, hight %d, framterate %d\n", width, hight, framterate);
    }
    else if (memcmp(cmdArray[0], "hdmiread", strlen("hdmiread")) == 0)
    {
        command_readADV7611(cmdArray[1], cmdArray[2]);
    }
    else if (memcmp(cmdArray[0], "hdmiwrite", strlen("hdmiwrite")) == 0)
    {
        command_writeADV7611(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    else if (memcmp(cmdArray[0], "freertos_task", strlen("freertos_task")) == 0)
    {
        command_TestTask();
    }
    else if (memcmp(cmdArray[0], "freertos_taskquit", strlen("freertos_taskquit")) == 0)
    {
        command_TestTaskQuit();
    }
    else if (memcmp(cmdArray[0], "test24c256write", strlen("test24c256write")) == 0)
    {
        command_Test24C256Write(cmdArray[1], cmdArray[1]);
    }
    else if (memcmp(cmdArray[0], "test24c256read", strlen("test24c256read")) == 0)
    {
        command_Test24C256Read(cmdArray[1]);
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
    else if (memcmp(cmdArray[0], "test_quadspi_speed", strlen("test_quadspi_speed")) == 0)
    {
        command_setQuadSPISpeed(cmdArray[1]);
    }
    else if (memcmp(cmdArray[0], "test_quadspi_data", strlen("test_quadspi_data")) == 0)
    {
        command_testQuadSPISpeedData();
    }
    else if (memcmp(cmdArray[0], "test_init_flash", strlen("test_init_flash")) == 0)
    {
        command_initWinbondNorFlash();
    }
    else if (memcmp(cmdArray[0], "test_erase_flash", strlen("test_erase_flash")) == 0)
    {
        command_eraseWinbondNorFlash(cmdArray[1], cmdArray[2]);
    }
    else if (memcmp(cmdArray[0], "test_write_flash", strlen("test_write_flash")) == 0)
    {
        command_writeWinbondNorFlash(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    else if (memcmp(cmdArray[0], "test_read_flash", strlen("test_read_flash")) == 0)
    {
        command_readWinbondNorFlash(cmdArray[1], cmdArray[2]);
    }
    else if (memcmp(cmdArray[0], "test_nor_flash_all", strlen("test_nor_flash_all")) == 0)
    {
        command_testAllNorFlashOperations(cmdArray[1], cmdArray[2], cmdArray[3]);
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
    else if (memcmp(cmdArray[0], "test_float_calculate_pi", strlen("test_float_calculate_pi")) == 0)
    {
        test_float_calculate_pi();
    }
    else if (memcmp(cmdArray[0], "test_can_init", strlen("test_can_init")) == 0)
    {
         command_TestCanInit(cmdArray[1], cmdArray[2], cmdArray[3], cmdArray[4], cmdArray[5], cmdArray[6]);
    } 
    else if (memcmp(cmdArray[0], "test_can_tx", strlen("test_can_tx")) == 0)
    {
        command_TestCanTx(cmdArray[1], cmdArray[2], cmdArray[3], cmdArray[4], cmdArray[5], cmdArray[6]);
    }
    else if (memcmp(cmdArray[0], "test_can_rx", strlen("test_can_rx")) == 0)
    {
        command_TestCanRx();
    }
    else if(memcmp(cmdArray[0], "test_ov5640", strlen("test_ov5640")) == 0)
    {
        command_TestOv5640();
    }
    else if(memcmp(cmdArray[0], "test_write_ov5640", strlen("test_write_ov5640")) == 0)
    {
        command_TestOv5640Write(cmdArray[1], cmdArray[2]);
    }
    else if(memcmp(cmdArray[0], "test_read_ov5640", strlen("test_ov5640_read")) == 0)
    {
        command_TestOv5640Read(cmdArray[1]);
    }
	else if(memcmp(cmdArray[0], "command_test_BB_uart", strlen("command_test_BB_uart")) == 0)
    {
        command_test_BB_uart(cmdArray[1]);
    }
    else if (memcmp(cmdArray[0], "testhal_TestGpioNormal", strlen("testhal_TestGpioNormal")) == 0)
    {
        commandhal_TestGpioNormal(cmdArray[1], cmdArray[2]);
    }
    else if (memcmp(cmdArray[0], "testhal_TestGpioInterrupt", strlen("testhal_TestGpioInterrupt")) == 0)
    {
        commandhal_TestGpioInterrupt(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    else if (memcmp(cmdArray[0], "testhal_Testtimer", strlen("testhal_Testtimer")) == 0)
    {
        commandhal_TestTim(cmdArray[1], cmdArray[2]);
    }
    else if (memcmp(cmdArray[0], "testhal_Testtiemrall", strlen("testhal_Testtiemrall")) == 0)
    {
        commandhal_TestTimAll();
    }
    else if (memcmp(cmdArray[0], "testhal_Testpwm", strlen("testhal_Testpwm")) == 0)
    {
        commandhal_TestPwm(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    else if (memcmp(cmdArray[0], "testhal_pwmall", strlen("testhal_pwmall")) == 0)
    {
        commandhal_TestPwmAll();
    }
    else if (memcmp(cmdArray[0], "testhal_simulatepwm", strlen("testhal_simulatepwm")) == 0)
    {
        commandhal_TestSimulatePwm();
    }
     else if (memcmp(cmdArray[0], "testhal24c256write", strlen("testhal24c256write")) == 0)
    {
        commandhal_Test24C256Write(cmdArray[1], cmdArray[2]);
    }
    else if (memcmp(cmdArray[0], "testhal24c256read", strlen("testhal24c256read")) == 0)
    {
        commandhal_Test24C256Read(cmdArray[1]);
    }
    else if (memcmp(cmdArray[0], "test_hal_uart_init", strlen("test_hal_uart_init")) == 0)
    {
        command_TestHalUartInit(cmdArray[1], cmdArray[2]);
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
    /* dma transfer: "transdma $(startBlock) $(blockNum)" */
    else if (memcmp(cmdArray[0], "test_dma", 8) == 0)
    {
        command_dma(cmdArray[1], cmdArray[2], cmdArray[3]);
    }
    else if (memcmp(cmdArray[0], "configure", 8) == 0)
    {
        setting_configure *configure=NULL;
        uint32_t i =0;
        uint32_t j =0;
        configure=(setting_configure *)get_configure_from_flash();
        dlog_error("****************        HDMI       ******************");
        dlog_error("%02x %02x %02x",configure->hdmi_configure[0][0],configure->hdmi_configure[0][1],configure->hdmi_configure[0][2]);
        dlog_error("%02x %02x %02x",configure->hdmi_configure[262][0],configure->hdmi_configure[262][1],configure->hdmi_configure[262][2]);
        dlog_error("****************        bb_sky       ******************");
        for(j=0;j<4;j++)
        {
            for(i=0;i<16;i++)
            {
                dlog_error("%02x",configure->bb_sky_configure[j][i*16]);
                dlog_output(100);
            }
            dlog_error("***********************************************");
        }
        dlog_error("***************      bb_grd            ********************");
        for(j=0;j<4;j++)
        {
            for(i=0;i<16;i++)
            {
                dlog_error("%02x",configure->bb_grd_configure[j][i*16]);
                dlog_output(100);
            }
            dlog_error("***********************************************");
        }
        dlog_error("*********        rf           **************");
        for(i=0;i<8;i++)
        {
            dlog_error("%02x",configure->rf_configure[i*16]);
            dlog_output(100);
        }
        /*for(i=0;i<263;i++)
        {
            dlog_error("%x %x %x",configure->hdmi_configure[i][0],configure->hdmi_configure[i][1],configure->hdmi_configure[i][2]);
            dlog_output(100);
        }
        dlog_error("***********************************************");
        for(i=0;i<263;i++)
        {
            dlog_error("%x %x %x",configure->hdmi_configure1[i][0],configure->hdmi_configure1[i][1],configure->hdmi_configure1[i][2]);
            dlog_output(100);
        }*/
    }
    /* error command */
    else
    {
        dlog_error("Command not found. Please use the commands like:");
        dlog_error("read <address>");
        dlog_error("write <address> <data>");
        dlog_error("initsd");
        dlog_error("readsd <SrcAddr:0x> <SectorNum:0x>");
        dlog_error("writesd <DstAddr:0x> <SectorNum:0x> <Srcaddr:0x>");
        dlog_error("erasesd <startSector> <SectorNum>");
        dlog_error("test_sdfs");
        dlog_error("hdmiinit <index>");
        dlog_error("hdmidump <index>");
        dlog_error("hdmigetvideoformat <index>");
        dlog_error("hdmiread <slv address> <reg address>");
        dlog_error("hdmiwrite <slv address> <reg address> <reg value>");
        dlog_error("freertos_task");
        dlog_error("freertos_taskquit");
        dlog_error("test24c256write <i2c port> <i2c_value>");
        dlog_error("test24c256read <i2c port>");
        dlog_error("test_timer <TIM Group> <TIM Num> <TIM Count>");
        dlog_error("test_timerused");
        dlog_error("test_timerall");
        dlog_error("test_pwm <PWM Group> <PWM Num> <PWM low> <PWM high>");
        dlog_error("test_pwmall");
        dlog_error("test_spi_id <spi_port>");
        dlog_error("test_spi_write_read <spi_port>");
        dlog_error("test_spi_erase <spi_port>");
        dlog_error("test_quadspi_speed <speed enum>");
        dlog_error("test_quadspi_data"); 
        dlog_error("test_init_flash");
        dlog_error("test_erase_flash <erase type> <flash start address>");
        dlog_error("test_write_flash <flash start address> <size> <value>");
        dlog_error("test_read_flash <flash start address> <size>");
        dlog_error("test_nor_flash_all <flash start address> <size> <value>");
        dlog_error("test_TestGpioNormal <gpionum> <highorlow>");
        dlog_error("test_TestGpioNormalRange <gpionum1> <gpionum2> <highorlow>");
        dlog_error("test_TestGpioInterrupt <gpionum> <inttype> <polarity>");
        dlog_output(1000);
        dlog_error("startbypassvideo");
        dlog_error("stopbypassvideo");
        dlog_error("test_float_calculate_pi");
		dlog_error("command_test_BB_uart <option>");
        dlog_error("upgrade <filename>");
        dlog_error("test_can_init <ch> <br> <acode> <amsk> <rtie> <format>");
        dlog_error("test_can_tx <ch> <id> <len> <data(hex)> <format> <type>");
        dlog_error("test_can_rx");
        dlog_error("test_ov5640");
        dlog_error("test_write_ov5640 <subAddr(hex)> <value>(hex)");
        dlog_error("test_read_ov5640 <subAddr(hex)>");
        dlog_error("testhal_TestGpioNormal <gpionum> <highorlow>");
        dlog_error("testhal_TestGpioInterrupt <gpionum> <inttype> <polarity>");
        dlog_error("testhal_Testtimer <TIM Num> <TIM Count>");
        dlog_error("testhal_Testtiemrall");
        dlog_error("testhal_Testpwm <PWM Num> <PWM low> <PWM high>");
        dlog_error("testhal_pwmall");
        dlog_error("testhal_simulatepwm");
        dlog_error("testhal24c256write <i2c port> <i2c_value>");
        dlog_error("testhal24c256read <i2c port>");
        dlog_error("test_hal_uart_init <ch> <baudr>");
        dlog_error("test_hal_uart_tx <ch> <len>");
        dlog_error("test_hal_uart_rx <ch>");
        dlog_error("test_hal_spi_init <ch> <baudr> <polarity> <phase>");
        dlog_error("test_hal_spi_write <ch> <addr> <wdata>");
        dlog_error("test_hal_spi_read <ch> <addr>");
        dlog_error("configure");
        dlog_error("test_dma <src> <dst> <byte_num>");
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

void command_readSdcard(char *Dstaddr, char *BlockNum)
{
    unsigned int iDstAddr;
    unsigned int iBlockNum;
    unsigned int iSrcAddr;
    unsigned int rowIndex;
    unsigned int columnIndex;
    unsigned int blockIndex;
    char *readSdcardBuff;
    char *bufferPos;

    iSrcAddr   = command_str2uint(Dstaddr);
    iBlockNum  = command_str2uint(BlockNum);

    readSdcardBuff = m7_malloc(iBlockNum * 512);
    memset(readSdcardBuff, '\0', iBlockNum * 512);
    bufferPos = readSdcardBuff;

    // dlog_info("iSrcBlock = 0x%08x\n", iSrcAddr);
    // dlog_info("iBlockNum = 0x%08x\n", iBlockNum);
    // dlog_info("readSdcardBuff = 0x%08x\n", readSdcardBuff);

    /* read from sdcard */
    HAL_SD_Read((uint32_t)bufferPos, iSrcAddr, iBlockNum);

    /* print to serial */
    for (blockIndex = iSrcAddr; blockIndex < (iSrcAddr + iBlockNum); blockIndex++)
    {
        dlog_info("==================block: %d=================",blockIndex);
        for (rowIndex = 0; rowIndex < 16; rowIndex++)
        {
            /* new line */
            dlog_info("0x%x: ",(unsigned int)((rowIndex << 5) + (blockIndex << 9)));
            for (columnIndex = 0; columnIndex < 1; columnIndex++)
            {
                dlog_info("0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x 0x%08x", 
                           *((unsigned int *)bufferPos), 
                           *((unsigned int *)(bufferPos + 4)), 
                           *((unsigned int *)(bufferPos + 8)), 
                           *((unsigned int *)(bufferPos + 12)),
                           *((unsigned int *)(bufferPos + 16)),
                           *((unsigned int *)(bufferPos + 20)),
                           *((unsigned int *)(bufferPos + 24)),
                           *((unsigned int *)(bufferPos + 28)));
                bufferPos += 32;
            }
        }
        dlog_info("\n");
    }
    m7_free(readSdcardBuff);

}

void command_writeSdcard(char *Dstaddr, char *BlockNum, char *SrcAddr)
{
    unsigned int iDstAddr;
    unsigned int iBlockNum;
    unsigned int iSrcAddr;

    iDstAddr    = command_str2uint(Dstaddr);
    iBlockNum   = command_str2uint(BlockNum);
    iSrcAddr    = command_str2uint(SrcAddr);

    /* write to sdcard */
    HAL_SD_Write(iDstAddr, iSrcAddr, iBlockNum);

}

void command_eraseSdcard(char *startBlock, char *blockNum)
{
    unsigned int iStartBlock;
    unsigned int iBlockNum;
    iStartBlock = command_str2uint(startBlock);
    iBlockNum   = command_str2uint(blockNum);


    // dlog_info("startBlock = 0x%08x\n", iStartBlock);
    // dlog_info("blockNum = %d\n", iBlockNum);
    HAL_SD_Erase(iStartBlock, iBlockNum);
}

void delay_ms(uint32_t num)
{
    volatile int i;
    for (i = 0; i < num * 100; i++);
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

void command_dma(char * u32_src, char *u32_dst, char *u32_byteNum)
{
    unsigned int iSrcAddr;
    unsigned int iDstAddr;
    unsigned int iNum;

    iDstAddr    = command_str2uint(u32_dst);
    iSrcAddr    = command_str2uint(u32_src);
    iNum        = command_str2uint(u32_byteNum);

    dlog_info("src = 0x%08x\n", iSrcAddr);
    dlog_info("dst = 0x%08x\n", iDstAddr);
    HAL_DMA_Start(iSrcAddr, iDstAddr, iNum);
    dlog_info("num = %x\n", iNum);
}

