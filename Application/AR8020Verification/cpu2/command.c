#include <string.h>
#include <stdlib.h>
#include "command.h"
#include "debuglog.h"
#include "interrupt.h"
#include "serial.h"
#include "cmsis_os.h"
#include "bb_ctrl.h"
#include "test_BB.h"
#include "test_timer.h"
#include "test_h264_encoder.h"
#include "test_i2c_adv7611.h"
#include "test_sysevent.h"
#include "hal_ret_type.h"
#include "hal_nvic.h"
#include "test_float.h"

extern void BB_uart10_spi_sel(uint32_t sel_dat);

void command_run(char *cmdArray[], uint32_t cmdNum)
{
    extern int BB_add_cmds(uint8_t type, uint32_t param0, uint32_t param1, uint32_t param2);
    
    if (memcmp(cmdArray[0], "BB_data_buf_full", strlen("BB_data_buf_full")) == 0)
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
    else if(memcmp(cmdArray[0], "BB_add_cmds", strlen("BB_add_cmds")) == 0)
    {
        BB_add_cmds(strtoul(cmdArray[1], NULL, 0),  //type
                    strtoul(cmdArray[2], NULL, 0),  //param0
                    strtoul(cmdArray[3], NULL, 0),  //param1
                    strtoul(cmdArray[4], NULL, 0)   //param2
                    );
    }
    else if (memcmp(cmdArray[0], "help", strlen("help")) == 0) 
    {
        dlog_error("Please use commands like:");
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
        dlog_error("BB_add_cmds <type> <param0> <param1> <param2>");		
        dlog_output(1000);	
    }
}

