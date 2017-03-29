#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include "hal_bb.h"
#include "memory_config.h"
#include "debuglog.h"
#include "test_bbuartcom.h"


static void rcvDataHandler(void *p)
{
    uint8_t data_buf_proc[128];
    uint32_t u32_rcvLen = 0;
    
    HAL_BB_UartComReceiveMsg(BB_UART_COM_SESSION_1, data_buf_proc, sizeof(data_buf_proc), &u32_rcvLen);
    uint32_t i = 0;
    dlog_info("rcv: %d", u32_rcvLen);
    for(i = 0; i < u32_rcvLen; i++)
    {
        dlog_info("%d %d", i, data_buf_proc[i]);
    }
}

void command_test_BB_uart(char *index_str)
{
    unsigned char opt = strtoul(index_str, NULL, 0);

    if (opt == 0)
    {
        HAL_BB_UartComRemoteSessionInit();
        HAL_BB_UartComRegisterSession(BB_UART_COM_SESSION_1, rcvDataHandler);
        dlog_info("init");
    }
    else if (opt == 1)
    {
        uint8_t data_buf[] = { 1,  2,  3,  4,  5};
        HAL_BB_UartComSendMsg(BB_UART_COM_SESSION_1, data_buf, sizeof(data_buf));
        dlog_info("send");
    }
    else if (opt == 2)
    {
        uint8_t data_buf_proc[128];
        uint32_t i = 0;
        for(i = 0; i < sizeof(data_buf_proc); i++)
        {
            data_buf_proc[i] = i;
        }
        
        HAL_BB_UartComSendMsg(BB_UART_COM_SESSION_1, data_buf_proc, sizeof(data_buf_proc));

        dlog_info("send \n");
        for(i = 0; i < sizeof(data_buf_proc); i++)
        {
            dlog_info("%d %d", i, data_buf_proc[i]);
        }
    }    
}
