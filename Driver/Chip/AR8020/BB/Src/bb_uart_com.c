#include <stdint.h>
#include <string.h>
#include "bb_uart_com.h"
#include "debuglog.h"
#include "serial.h"
#include "interrupt.h"
#include "memory_config.h"
#include "lock.h"


static uint8_t header[] = {0xFF, 0x5A, 0xA5};
static STRU_BBUartComSession g_BBUARTComSessionArray[BB_UART_COM_SESSION_MAX] = {0};
static uint8_t g_BBUARTComSession0RxBuffer[128] = {0};

static void BB_UARTComLockAccquire(void)
{
    lock_type * lock_p = (lock_type*)SRAM_MODULE_LOCK_BB_UART_MUTEX_FLAG;
    Lock(lock_p);
}

static void BB_UARTComLockRelease(void)
{
    lock_type * lock_p = (lock_type*)SRAM_MODULE_LOCK_BB_UART_MUTEX_FLAG;
    UnLock(lock_p);
}

static void BB_UARTComWriteSessionRxBuffer(ENUM_BBUARTCOMSESSIONID session_id, uint8_t* data_buf, uint32_t data_size)
{
    if (g_BBUARTComSessionArray[session_id].rx_buf->header.in_use == 1)
    {
        uint32_t wr_pos = g_BBUARTComSessionArray[session_id].rx_buf->header.rx_buf_wr_pos;
        uint32_t rd_pos = g_BBUARTComSessionArray[session_id].rx_buf->header.rx_buf_rd_pos;
        uint32_t tail_pos = g_BBUARTComSessionArray[session_id].data_max_size;

        uint32_t cnt = 0;
        do
        {
            g_BBUARTComSessionArray[session_id].rx_buf->data[wr_pos] = data_buf[cnt];
            wr_pos++;
            cnt++;

            if (wr_pos >= tail_pos)
            {
                wr_pos = 0;
            }
        } while ((wr_pos != rd_pos) && (cnt < data_size));

        if (wr_pos == rd_pos)
        {
            dlog_error("FIFO is full!");
        }

        g_BBUARTComSessionArray[session_id].rx_buf->header.rx_buf_wr_pos = wr_pos;

        //notify the session
        {
            uint32_t u32_event = ( session_id == 0) ? SYS_EVENT_ID_UART_DATA_RCV_SESSION0 : 
                                 ((session_id == 1) ? SYS_EVENT_ID_UART_DATA_RCV_SESSION1 : 
                                 ((session_id == 2) ? SYS_EVENT_ID_UART_DATA_RCV_SESSION2 : SYS_EVENT_ID_UART_DATA_RCV_SESSION3));
            SYS_EVENT_Notify_From_ISR(u32_event, NULL);
            
            //dlog_info("Notify Event %d", u32_event);
        }
    }
}

uint32_t BB_UARTComPacketDataAnalyze(uint8_t *u8_uartRxBuf, uint8_t u8_uartRxLen)
{
    uint8_t i = 0;
    char chData = '\0';
    uint8_t u8_commandPos = 0;
    while (u8_uartRxLen)
    {
        chData = *(u8_uartRxBuf + i);

        static uint8_t rx_state = BB_UART_COM_RX_HEADER;
        static uint8_t header_buf[4];
        static uint8_t header_buf_index = 0;
        static ENUM_BBUARTCOMSESSIONID session_id = 0;
        static uint8_t data_length = 0;
        static uint8_t data_buf[BBCOM_UART_RX_BUF_SIZE];
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

                if ((header_buf_index == sizeof(header)) && (memcmp((void *)header, (void *)header_buf, sizeof(header)) == 0))
                {
                    rx_state = BB_UART_COM_RX_SESSION_ID;
                }
            }
            break;
        case BB_UART_COM_RX_SESSION_ID:
            session_id = chData;
            rx_state = BB_UART_COM_RX_DATALENGTH;
            break;
        case BB_UART_COM_RX_DATALENGTH:
            if (chData <= sizeof(data_buf))
            {
                data_length = chData;
                rx_state = BB_UART_COM_RX_DATABUFFER;
                data_buf_index = 0;
            }
            else
            {
                header_buf_index = 0;
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
                //dlog_info("Get BB UARTCom session %d data.", session_id);
                if (session_id < BB_UART_COM_SESSION_MAX)
                {
                    BB_UARTComWriteSessionRxBuffer(session_id, data_buf, data_length);
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

        i++;
        u8_uartRxLen--;
    }
}
#if 0
static void BB_UARTComUART10IRQHandler(uint32_t u32_vectorNum)
{
    char                 c;
    unsigned int         isrType;
		
    volatile uart_type   *uart_regs = (uart_type *)UART10_BASE;
    isrType    = uart_regs->IIR_FCR;


    /* receive data irq, try to get the data */
    if (UART_IIR_RECEIVEDATA == (isrType & 0xf))
    {
        
        int i = 14;
        while (i--)
        {
            c = uart_regs->RBR_THR_DLL;
            BB_UARTComPacketDataAnalyze(&c,1);
        }

    }

    if (UART_IIR_DATATIMEOUT == (isrType & 0xf))
    {        
        c = uart_regs->RBR_THR_DLL;
        BB_UARTComPacketDataAnalyze(&c,1);           

    }

    // TX empty interrupt.
    if (UART_IIR_THR_EMPTY == (isrType & 0xf))
    {       
        uart_putFifo(10);        
    }

}
#endif
void BB_UARTComInit(SYS_Event_Handler session0RcvDataHandler)
{
    *((lock_type*)(SRAM_MODULE_LOCK_BB_UART_MUTEX_FLAG)) = UNLOCK_STATE;
    *((volatile uint32_t*)(SRAM_MODULE_LOCK_BB_UART_INIT_FLAG)) = 0;

    uart_init(BBCOM_UART_INDEX, BBCOM_UART_BAUDRATE);
    reg_IrqHandle(VIDEO_UART10_INTR_VECTOR_NUM, UART_IntrSrvc, NULL);
    UART_RegisterUserRxHandler(BBCOM_UART_INDEX, BB_UARTComPacketDataAnalyze);
    INTR_NVIC_SetIRQPriority(VIDEO_UART10_INTR_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_VIDEO_UART10,0));
    INTR_NVIC_EnableIRQ(VIDEO_UART10_INTR_VECTOR_NUM);

    // Session 0 is registered by default
    g_BBUARTComSessionArray[0].rx_buf = (STRU_BBUartComSessionRxBuffer*)g_BBUARTComSession0RxBuffer;
    g_BBUARTComSessionArray[0].rx_buf->header.in_use = 1;
    g_BBUARTComSessionArray[0].rx_buf->header.rx_buf_wr_pos = 0;
    g_BBUARTComSessionArray[0].rx_buf->header.rx_buf_rd_pos = 0;
    g_BBUARTComSessionArray[0].data_max_size = sizeof(g_BBUARTComSession0RxBuffer) - sizeof(STRU_BBUartComSessionRxBufferHeader);
    if ( session0RcvDataHandler )
    {
        SYS_EVENT_RegisterHandler(SYS_EVENT_ID_UART_DATA_RCV_SESSION0, session0RcvDataHandler);
    }

    // Sessions 1-4 are registered dynamicly
    g_BBUARTComSessionArray[1].rx_buf = (STRU_BBUartComSessionRxBuffer*)SRAM_BB_UART_COM_SESSION_1_SHARE_MEMORY_ST_ADDR;
    g_BBUARTComSessionArray[1].rx_buf->header.in_use = 0;
    g_BBUARTComSessionArray[1].rx_buf->header.rx_buf_wr_pos = 0;
    g_BBUARTComSessionArray[1].rx_buf->header.rx_buf_rd_pos = 0;
    g_BBUARTComSessionArray[1].data_max_size = SRAM_BB_UART_COM_SESSION_1_SHARE_MEMORY_SIZE - sizeof(STRU_BBUartComSessionRxBufferHeader);

    g_BBUARTComSessionArray[2].rx_buf = (STRU_BBUartComSessionRxBuffer*)SRAM_BB_UART_COM_SESSION_2_SHARE_MEMORY_ST_ADDR;
    g_BBUARTComSessionArray[2].rx_buf->header.in_use = 0;
    g_BBUARTComSessionArray[2].rx_buf->header.rx_buf_wr_pos = 0;
    g_BBUARTComSessionArray[2].rx_buf->header.rx_buf_rd_pos = 0;
    g_BBUARTComSessionArray[2].data_max_size = SRAM_BB_UART_COM_SESSION_2_SHARE_MEMORY_SIZE - sizeof(STRU_BBUartComSessionRxBufferHeader);

    g_BBUARTComSessionArray[3].rx_buf = (STRU_BBUartComSessionRxBuffer*)SRAM_BB_UART_COM_SESSION_3_SHARE_MEMORY_ST_ADDR;
    g_BBUARTComSessionArray[3].rx_buf->header.in_use = 0;
    g_BBUARTComSessionArray[3].rx_buf->header.rx_buf_wr_pos = 0;
    g_BBUARTComSessionArray[3].rx_buf->header.rx_buf_rd_pos = 0;
    g_BBUARTComSessionArray[3].data_max_size = SRAM_BB_UART_COM_SESSION_3_SHARE_MEMORY_SIZE - sizeof(STRU_BBUartComSessionRxBufferHeader);

    g_BBUARTComSessionArray[4].rx_buf = (STRU_BBUartComSessionRxBuffer*)SRAM_BB_UART_COM_SESSION_4_SHARE_MEMORY_ST_ADDR;
    g_BBUARTComSessionArray[4].rx_buf->header.in_use = 0;
    g_BBUARTComSessionArray[4].rx_buf->header.rx_buf_wr_pos = 0;
    g_BBUARTComSessionArray[4].rx_buf->header.rx_buf_rd_pos = 0;
    g_BBUARTComSessionArray[4].data_max_size = SRAM_BB_UART_COM_SESSION_4_SHARE_MEMORY_SIZE - sizeof(STRU_BBUartComSessionRxBufferHeader);

    *((volatile uint32_t*)(SRAM_MODULE_LOCK_BB_UART_INIT_FLAG)) = 0x10A5A501;
}

void BB_UARTComRemoteSessionInit(void)
{
    // Wait for the init finish
    while (*((volatile uint32_t*)(SRAM_MODULE_LOCK_BB_UART_INIT_FLAG)) != 0x10A5A501) { }
    
    g_BBUARTComSessionArray[0].rx_buf = (STRU_BBUartComSessionRxBuffer*)g_BBUARTComSession0RxBuffer;
    g_BBUARTComSessionArray[0].rx_buf->header.in_use = 1;
    g_BBUARTComSessionArray[1].rx_buf = (STRU_BBUartComSessionRxBuffer*)SRAM_BB_UART_COM_SESSION_1_SHARE_MEMORY_ST_ADDR;
    g_BBUARTComSessionArray[1].data_max_size = SRAM_BB_UART_COM_SESSION_1_SHARE_MEMORY_SIZE - sizeof(STRU_BBUartComSessionRxBufferHeader);
    g_BBUARTComSessionArray[2].rx_buf = (STRU_BBUartComSessionRxBuffer*)SRAM_BB_UART_COM_SESSION_2_SHARE_MEMORY_ST_ADDR;
    g_BBUARTComSessionArray[2].data_max_size = SRAM_BB_UART_COM_SESSION_2_SHARE_MEMORY_SIZE - sizeof(STRU_BBUartComSessionRxBufferHeader);
    g_BBUARTComSessionArray[3].rx_buf = (STRU_BBUartComSessionRxBuffer*)SRAM_BB_UART_COM_SESSION_3_SHARE_MEMORY_ST_ADDR;
    g_BBUARTComSessionArray[3].data_max_size = SRAM_BB_UART_COM_SESSION_3_SHARE_MEMORY_SIZE - sizeof(STRU_BBUartComSessionRxBufferHeader);
    g_BBUARTComSessionArray[4].rx_buf = (STRU_BBUartComSessionRxBuffer*)SRAM_BB_UART_COM_SESSION_4_SHARE_MEMORY_ST_ADDR;
    g_BBUARTComSessionArray[4].data_max_size = SRAM_BB_UART_COM_SESSION_4_SHARE_MEMORY_SIZE - sizeof(STRU_BBUartComSessionRxBufferHeader);
}

uint8_t BB_UARTComRegisterSession(ENUM_BBUARTCOMSESSIONID session_id)
{
    if ((session_id != BB_UART_COM_SESSION_0) && (session_id < BB_UART_COM_SESSION_MAX))
    {
        if (g_BBUARTComSessionArray[session_id].rx_buf->header.in_use == 0)
        {
            g_BBUARTComSessionArray[session_id].rx_buf->header.in_use = 1;
            g_BBUARTComSessionArray[session_id].rx_buf->header.rx_buf_wr_pos = 0;
            g_BBUARTComSessionArray[session_id].rx_buf->header.rx_buf_rd_pos = 0;
            return 1;
        }
        else
        {
            dlog_error("Session %d occupied", session_id);
            return 0;
        }
    }
}

void BB_UARTComUnRegisterSession(ENUM_BBUARTCOMSESSIONID session_id)
{
    if ((session_id != BB_UART_COM_SESSION_0) && (session_id < BB_UART_COM_SESSION_MAX))
    {
        g_BBUARTComSessionArray[session_id].rx_buf->header.in_use = 0;
        g_BBUARTComSessionArray[session_id].rx_buf->header.rx_buf_wr_pos = 0;
        g_BBUARTComSessionArray[session_id].rx_buf->header.rx_buf_rd_pos = 0;
    }
}

uint8_t BB_UARTComSendMsg(ENUM_BBUARTCOMSESSIONID session_id, uint8_t* data_buf, uint32_t length)
{
    uint8_t iCnt = 0;
    uint8_t check_sum = 0;
    uint8_t iTotalCnt = 0;
    if (data_buf == NULL)
    {
        return 0;
    }

    uint8_t align_block_num = length / BBCOM_UART_RX_BUF_SIZE;
    uint8_t unalign_block_byte = length % BBCOM_UART_RX_BUF_SIZE;
    uint8_t length_tmp = 0;
    uint8_t* data_buf_tmp = data_buf;

    BB_UARTComLockAccquire();

    while (align_block_num || unalign_block_byte)
    {
        if (align_block_num == 0)
        {
            length_tmp = unalign_block_byte;
        }
        else
        {
            length_tmp = BBCOM_UART_RX_BUF_SIZE;
        }

        // Header
        iTotalCnt = 0;
        for (iCnt = 0; iCnt < sizeof(header); iCnt++)
        {
            uart_putc(BBCOM_UART_INDEX, header[iCnt]);
            iTotalCnt++;
        }

        // Session ID
        uart_putc(BBCOM_UART_INDEX, session_id);
        iTotalCnt++;

        // Data length
        uart_putc(BBCOM_UART_INDEX, length_tmp);
        iTotalCnt++;
        
        // Data
        check_sum = 0;
        for (iCnt = 0; iCnt < length_tmp; iCnt++)
        {
            uart_putc(BBCOM_UART_INDEX, data_buf_tmp[iCnt]);
            check_sum += data_buf_tmp[iCnt];
            iTotalCnt++;
        }

        // Checksum
        uart_putc(BBCOM_UART_INDEX, check_sum);
        iTotalCnt++;

        // Pad "0" when not aligned by 16 bytes
        iTotalCnt = 16 - (iTotalCnt % 16);
        for (iCnt = 0; iCnt < iTotalCnt; iCnt++)
        {
            uart_putc(BBCOM_UART_INDEX, 0);
        }

        if (align_block_num > 0)
        {
            data_buf_tmp += BBCOM_UART_RX_BUF_SIZE;
            align_block_num--;
        }
        else
        {
            break;
        }
    }

    BB_UARTComLockRelease();
    
    return 1;
}

uint32_t BB_UARTComReceiveMsg(ENUM_BBUARTCOMSESSIONID session_id, uint8_t* data_buf, uint32_t length_max)
{
    if (g_BBUARTComSessionArray[session_id].rx_buf->header.in_use == 1)
    {
        uint32_t wr_pos = g_BBUARTComSessionArray[session_id].rx_buf->header.rx_buf_wr_pos;
        uint32_t rd_pos = g_BBUARTComSessionArray[session_id].rx_buf->header.rx_buf_rd_pos;
        uint32_t tail_pos = g_BBUARTComSessionArray[session_id].data_max_size;

        uint32_t cnt = 0;
        while (rd_pos != wr_pos)
        {
            data_buf[cnt] = g_BBUARTComSessionArray[session_id].rx_buf->data[rd_pos];
            rd_pos++;
            cnt++;

            if (rd_pos >= tail_pos)
            {
                rd_pos = 0;
            }

            if (cnt == length_max)
            {
                break;
            }
        }

        g_BBUARTComSessionArray[session_id].rx_buf->header.rx_buf_rd_pos = rd_pos;

        return cnt;
    }
}

