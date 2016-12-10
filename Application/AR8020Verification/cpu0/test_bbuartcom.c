#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "BB_uart_com.h"
#include "memory_config.h"
#include "debuglog.h"
#include "test_bbuartcom.h"

void command_test_BB_uart(char *index_str)
{
    unsigned char opt = strtoul(index_str, NULL, 0);

    if (opt == 0)
    {
        BB_UARTComRemoteSessionInit();
        BB_UARTComRegisterSession(BB_UART_COM_SESSION_1);
        dlog_info("init");
    }
    else if (opt == 1)
    {
        uint8_t data_buf[] = { 1,  2,  3,  4,  5,  6,  7,  8,  9,  10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22};
        BB_UARTComSendMsg(BB_UART_COM_SESSION_1, data_buf, sizeof(data_buf));
        dlog_info("send");
    }
    else if (opt == 2)
    {
        static uint8_t data_buf_proc[128];
        uint32_t cnt = BB_UARTComReceiveMsg(BB_UART_COM_SESSION_1, data_buf_proc, sizeof(data_buf_proc));
        uint32_t i = 0;
        for(i = 0; i < cnt; i++)
        {
            dlog_info("%d,", data_buf_proc[i]);
        }
    }    
}
