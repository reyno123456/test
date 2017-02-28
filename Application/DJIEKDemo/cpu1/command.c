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
#include "hal_ret_type.h"
#include "hal_nvic.h"

void command_run(char *cmdArray[], uint32_t cmdNum)
{
    if (memcmp(cmdArray[0], "help", strlen("help")) == 0)
    {
        dlog_error("Please use the commands like:");
    }
}

unsigned int command_str2uint(char *str)
{
    return strtoul(str, NULL, 0); 
}


