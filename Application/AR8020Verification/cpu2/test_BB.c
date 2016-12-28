#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "debuglog.h"
#include "interrupt.h"
#include "hal_bb.h"

void command_test_BB_uart(char *index_str)
{
    static uint8_t data_buf_proc[128];

    unsigned char opt = strtoul(index_str, NULL, 0);

    if (opt == 0)
    {
        HAL_BB_UartComRegisterSession(BB_UART_COM_SESSION_0);
    }
    else if (opt == 1)
    {
        uint8_t data_buf_tmp[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22};
        HAL_BB_UartComSendMsg(BB_UART_COM_SESSION_0, data_buf_tmp, sizeof(data_buf_tmp));
    }
    else if (opt == 2)
    {
        uint32_t len = 0;
        HAL_BB_UartComReceiveMsg(BB_UART_COM_SESSION_0, data_buf_proc, sizeof(data_buf_proc), &len);

        uint32_t i = 0;
        for(i = 0; i < len; i++)
        {
            dlog_info("%d,", data_buf_proc[i]);
        }
    }
}

