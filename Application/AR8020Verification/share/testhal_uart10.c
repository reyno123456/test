#include <stdlib.h>
#include <string.h>
#include "debuglog.h"
#include "serial.h"
#include "hal_bb.h"
#include "bb_uart_com.h"

static void rcvDataHandler_uart10(void *p)
{
    uint8_t data_buf_proc[128];
    uint32_t u32_rcvLen = 0;
    
    HAL_BB_UartComReceiveMsg(BB_UART_COM_SESSION_2, data_buf_proc, sizeof(data_buf_proc), &u32_rcvLen);
    uint32_t i = 0;
    dlog_info("rcv: %d", u32_rcvLen);
    for(i = 0; i < u32_rcvLen; i++)
    {
        dlog_info("%d %d", i, data_buf_proc[i]);
    }
}

void commandhal_Uart10(char* index)
{
    unsigned char u8_index = strtoul(index, NULL, 0); 
    char testBuff[23] = {0};
    char i = 0;
    for (i = 0; i < 23; i++)
    {
        testBuff[i] =  i;
    }
    switch (u8_index)
    {
        case 0:
            uart_init(10,115200);
            break;
        case 1:
            uart_putdata(10,  testBuff, 23);
            break;
        case 2:
            HAL_BB_UartComRemoteSessionInit();
            HAL_BB_UartComRegisterSession(BB_UART_COM_SESSION_2, rcvDataHandler_uart10); 
            break;                
        case 3:
            BB_UARTComSendMsg(BB_UART_COM_SESSION_2, testBuff, 23);
            break; 
    }
}