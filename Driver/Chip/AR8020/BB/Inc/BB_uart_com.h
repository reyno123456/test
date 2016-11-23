#ifndef BB_UARTCOM_H
#define BB_UARTCOM_H

#define BBCOM_UART_INDEX    10
#define BBCOM_UART_BAUDRATE 256000

typedef enum
{
    BB_UART_COM_RX_HEADER = 0,
    BB_UART_COM_RX_DATALENGTH,
    BB_UART_COM_RX_DATABUFFER,
    BB_UART_COM_RX_CHECKSUM,
} ENUM_BBUartComRxState;

typedef void (*BBUARTComFunction)(uint8_t* data_buf, uint8_t length);

void BB_UARTComInit(void);
void BB_UARTComRegisterRXCallback(BBUARTComFunction callback);
uint8_t BB_UARTComSendMsg(uint8_t* data_buf, uint8_t length);

#endif
