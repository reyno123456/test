#ifndef BB_UARTCOM_H
#define BB_UARTCOM_H

#define BBCOM_UART_INDEX                    10
#define BBCOM_UART_BAUDRATE                 256000
#define BBCOM_UART_RX_BUF_SIZE              128

typedef enum
{
    BB_UART_COM_RX_HEADER = 0,
    BB_UART_COM_RX_SESSION_ID,
    BB_UART_COM_RX_DATALENGTH,
    BB_UART_COM_RX_DATABUFFER,
    BB_UART_COM_RX_CHECKSUM,
} ENUM_BBUartComRxState;

typedef enum
{
    BB_UART_COM_SESSION_0 = 0,
    BB_UART_COM_SESSION_1,
    BB_UART_COM_SESSION_2,
    BB_UART_COM_SESSION_3,
    BB_UART_COM_SESSION_4,
    BB_UART_COM_SESSION_MAX
} ENUM_BBUartComSessionID;

typedef struct
{
    volatile uint32_t in_use;
    volatile uint32_t rx_buf_wr_pos;
    volatile uint32_t rx_buf_rd_pos;
} STRU_BBUartComSessionRxBufferHeader;

typedef struct
{
    STRU_BBUartComSessionRxBufferHeader header;
    volatile uint8_t data[1];
} STRU_BBUartComSessionRxBuffer;

typedef struct
{
    STRU_BBUartComSessionRxBuffer* rx_buf;
    uint32_t data_max_size;
} STRU_BBUartComSession;

void BB_UARTComInit(void);
void BB_UARTComRemoteSessionInit(void);
uint8_t BB_UARTComRegisterSession(ENUM_BBUartComSessionID session_id);
void BB_UARTComUnRegisterSession(ENUM_BBUartComSessionID session_id);
uint8_t BB_UARTComSendMsg(ENUM_BBUartComSessionID session_id, uint8_t* data_buf, uint32_t length);
uint32_t BB_UARTComReceiveMsg(ENUM_BBUartComSessionID session_id, uint8_t* data_buf, uint32_t length_max);

#endif
