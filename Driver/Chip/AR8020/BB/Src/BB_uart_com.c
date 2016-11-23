#include <stdint.h>
#include "BB_uart_com.h"
#include "debuglog.h"
#include "serial.h"
#include "interrupt.h"

static uint8_t header[] = {0xFF, 0x5A, 0xA5};
static BBUARTComFunction g_BBComCallback = NULL;

static void BB_UARTComPacketDataAnalyze(uint8_t chData)
{
    static uint8_t rx_state = BB_UART_COM_RX_HEADER;
    static uint8_t header_buf[4];
    static uint8_t header_buf_index = 0;
    static uint8_t data_length = 0;
    static uint8_t data_buf[128];
    static uint8_t data_buf_index = 0;
    static uint8_t check_sum = 0;

    //dlog_info("rx_state: %d, chData: 0x%x", rx_state, chData);

    switch (rx_state)
    {
    case BB_UART_COM_RX_HEADER:
        if (chData == header[0])    // Reset flag
        {
            header_buf_index = 0;
            header_buf[header_buf_index++] = chData;
        }
        else if (header_buf_index < sizeof(header))    // Get header
        {
            header_buf[header_buf_index++] = chData;

            if ((header_buf_index == sizeof(header)) && (memcmp(header, header_buf, sizeof(header)) == 0))
            {
                header_buf_index = 0;
                rx_state = BB_UART_COM_RX_DATALENGTH;
            }
        }
        break;
    case BB_UART_COM_RX_DATALENGTH:
        if (chData < sizeof(data_buf))
        {
            data_length = chData;
            rx_state = BB_UART_COM_RX_DATABUFFER;
            data_buf_index = 0;
        }
        else
        {
            rx_state = BB_UART_COM_RX_HEADER;
            dlog_error("BBCom RX data length is too long > %d !", sizeof(data_buf));
        }
        break;
    case BB_UART_COM_RX_DATABUFFER:
        if (data_buf_index < data_length)
        {
            data_buf[data_buf_index++] = chData;

            if (data_buf_index == data_length)
            {
                uint8_t i = 0;
                check_sum = 0;
                for (i = 0; i < data_length; i++)
                {
                    check_sum += data_buf[i];
                }

                rx_state = BB_UART_COM_RX_CHECKSUM;
            }
        }
        break;
    case BB_UART_COM_RX_CHECKSUM:
        if (check_sum == chData)
        {
            dlog_info("Get right BB UARTCom RX data, run callback!");

            if (g_BBComCallback != NULL)
            {
                g_BBComCallback(data_buf, data_length);
            }
        }

        header_buf_index = 0;
        rx_state = BB_UART_COM_RX_HEADER;
        break;
    default:
        header_buf_index = 0;
        rx_state = BB_UART_COM_RX_HEADER;
        break;
    }
}

static void BB_UARTComUART10IRQHandler(void)
{
    char                 c;
    unsigned int         status;
    unsigned int         isrType;
    volatile uart_type   *uart_regs = UART10_BASE;
    status     = uart_regs->LSR;
    isrType    = uart_regs->IIR_FCR;

    /* receive data irq, try to get the data */
    if (UART_IIR_RECEIVEDATA == (isrType & UART_IIR_RECEIVEDATA))
    {
        if ((status & UART_LSR_DATAREADY) == UART_LSR_DATAREADY)
        {
            c = uart_regs->RBR_THR_DLL;
            /* receive normal data */
            BB_UARTComPacketDataAnalyze(c);
        }
    }
}

void BB_UARTComInit(void)
{
    uart_init(BBCOM_UART_INDEX, ((BBCOM_UART_BAUDRATE * 100) / 166));
    reg_IrqHandle(VIDEO_UART10_INTR_VECTOR_NUM, BB_UARTComUART10IRQHandler);
    INTR_NVIC_EnableIRQ(VIDEO_UART10_INTR_VECTOR_NUM);
    INTR_NVIC_SetIRQPriority(VIDEO_UART10_INTR_VECTOR_NUM, 1);
}

void BB_UARTComRegisterRXCallback(BBUARTComFunction callback)
{
    g_BBComCallback = callback;
}

uint8_t BB_UARTComSendMsg(uint8_t* data_buf, uint8_t length)
{
    uint8_t iCnt = 0;
    uint8_t check_sum = 0;
    uint8_t iTotalCnt = 0;

    if (data_buf == NULL)
    {
        return 0;
    }

    // Header
    for (iCnt = 0; iCnt < sizeof(header); iCnt++)
    {
        uart_putc(BBCOM_UART_INDEX, header[iCnt]);
        iTotalCnt++;
    }

    // Data length
    uart_putc(BBCOM_UART_INDEX, length);
    iTotalCnt++;
    
    // Data
    check_sum = 0;
    for (iCnt = 0; iCnt < length; iCnt++)
    {
        uart_putc(BBCOM_UART_INDEX, data_buf[iCnt]);
        check_sum += data_buf[iCnt];
        iTotalCnt++;
    }

    // Checksum
    uart_putc(BBCOM_UART_INDEX, check_sum);
    iTotalCnt++;

    iTotalCnt = 16 - (iTotalCnt % 16);
    for (iCnt = 0; iCnt < iTotalCnt; iCnt++)
    {
        uart_putc(BBCOM_UART_INDEX, 0);
    }

    return 1;
}


