#include <string.h>
#include <stdlib.h>
#include "command.h"
#include "debuglog.h"
#include "interrupt.h"
#include "serial.h"
#include "cmsis_os.h"
#include "bb_ctrl.h"
#include "hal_ret_type.h"
#include "hal_nvic.h"
#include "memory_config.h"

void command_run(char *cmdArray[], uint32_t cmdNum)
{
    if (memcmp(cmdArray[0], "BB_uart10_spi_sel", strlen("BB_uart10_spi_sel")) == 0)
    {
        BB_uart10_spi_sel( strtoul(cmdArray[1], NULL, 0) );
    }
    else if (memcmp(cmdArray[0], "help", strlen("help")) == 0) 
    {
        dlog_error("Please use commands like:");
        dlog_error("BB_uart10_spi_sel <value>");
    }
}

