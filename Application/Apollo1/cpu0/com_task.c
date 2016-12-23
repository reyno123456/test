#include <stdint.h>
#include "com_task.h"
#include "cmsis_os.h"
#include "interrupt.h"
#include "bb_uart_com.h"
#include "serial.h"
#include "debuglog.h"
#include "systicks.h"

static osMessageQId com_queue0_id = 0;
static uint8_t tx_buf0_index = 0;
static uint8_t tx_buf0[32];
static uint8_t rx_buf0[32];
static volatile uint8_t tx_last_byte_timeout0 = 0;
static osMutexId tx_buf_mutex0_id = 0;

static osMessageQId com_queue1_id = 0;
static uint8_t tx_buf1_index = 0;
static uint8_t tx_buf1[32];
static uint8_t rx_buf1[32];
static volatile uint8_t tx_last_byte_timeout1 = 0;
static osMutexId tx_buf_mutex1_id = 0;

static void COMTASK_SendDataToQueue(osMessageQId QueueId, uint8_t Data)
{
    if (QueueId != 0)
    {
        osMessagePut(QueueId, Data, 0);
    }
}

static void COMTASK_FlushTxBufferToUart(osMessageQId QueueId)
{
    if (QueueId != 0)
    {
        if (QueueId == com_queue0_id)
        {
            osMutexWait(tx_buf_mutex0_id, osWaitForever);
            if (tx_buf0_index != 0)
            {
                BB_UARTComSendMsg(BB_UART_COM_SESSION_1, tx_buf0, tx_buf0_index);
                tx_buf0_index = 0;
            }
            osMutexRelease(tx_buf_mutex0_id);
        }
        else if (QueueId == com_queue1_id)
        {
            osMutexWait(tx_buf_mutex1_id, osWaitForever);
            if (tx_buf1_index != 0)
            {
                BB_UARTComSendMsg(BB_UART_COM_SESSION_2, tx_buf1, tx_buf1_index);
                tx_buf1_index = 0;
            }
            osMutexRelease(tx_buf_mutex1_id);
        }
    }
}

static void COMTASK_UartIrqHandler3(uint32_t u32_vectorNum)
{
    char                  c;
    unsigned int          status;
    unsigned int          isrType;
    volatile uart_type   *uart_regs = (uart_type *)UART3_BASE;

    status     = uart_regs->LSR;
    isrType    = uart_regs->IIR_FCR;

    /* receive data irq, try to get the data */
    if (UART_IIR_RECEIVEDATA == (isrType & UART_IIR_RECEIVEDATA))
    {
        if ((status & UART_LSR_DATAREADY) == UART_LSR_DATAREADY)
        {
            c = uart_regs->RBR_THR_DLL;
            COMTASK_SendDataToQueue(com_queue0_id, c);
            tx_last_byte_timeout0 = 0;
        }
    }
}

static void COMTASK_UartIrqHandler4(uint32_t u32_vectorNum)
{
    char                  c;
    unsigned int          status;
    unsigned int          isrType;
    volatile uart_type   *uart_regs = (uart_type *)UART4_BASE;

    status     = uart_regs->LSR;
    isrType    = uart_regs->IIR_FCR;

    /* receive data irq, try to get the data */
    if (UART_IIR_RECEIVEDATA == (isrType & UART_IIR_RECEIVEDATA))
    {
        if ((status & UART_LSR_DATAREADY) == UART_LSR_DATAREADY)
        {
            c = uart_regs->RBR_THR_DLL;
            COMTASK_SendDataToQueue(com_queue1_id, c);
            tx_last_byte_timeout1 = 0;
        }
    }
}

static void COMTASK_UartInit0(void)
{
    uart_init(3, 115200);
    reg_IrqHandle(UART_INTR3_VECTOR_NUM, COMTASK_UartIrqHandler3, NULL);
    INTR_NVIC_EnableIRQ(UART_INTR3_VECTOR_NUM);
    INTR_NVIC_SetIRQPriority(UART_INTR3_VECTOR_NUM, 1);
}

static void COMTASK_UartInit1(void)
{
    uart_init(4, 115200);
    reg_IrqHandle(UART_INTR4_VECTOR_NUM, COMTASK_UartIrqHandler4, NULL);
    INTR_NVIC_EnableIRQ(UART_INTR4_VECTOR_NUM);
    INTR_NVIC_SetIRQPriority(UART_INTR4_VECTOR_NUM, 1);
}

static void COMTASK_Tx0Function(void const *argument)
{
    COMTASK_UartInit0();

    osMessageQDef(com_queue0, 128, uint8_t);
    com_queue0_id  = osMessageCreate(osMessageQ(com_queue0), NULL);

    osMutexDef(tx_mutex0);
    tx_buf_mutex0_id = osMutexCreate (osMutex(tx_mutex0));

    BB_UARTComRegisterSession(BB_UART_COM_SESSION_1);

    while (1)
    {
        osEvent event0 = osMessageGet(com_queue0_id, osWaitForever);
        if (event0.status == osEventMessage)
        {
            if (tx_buf0_index < sizeof(tx_buf0))
            {
                tx_buf0[tx_buf0_index++] = event0.value.v;
            }

            if (tx_buf0_index == sizeof(tx_buf0))
            {
                COMTASK_FlushTxBufferToUart(com_queue0_id);
            }
        }
    }
}

static void COMTASK_Tx1Function(void const *argument)
{
    COMTASK_UartInit1();

    osMessageQDef(com_queue1, 128, uint8_t);
    com_queue1_id  = osMessageCreate(osMessageQ(com_queue1), NULL);

    osMutexDef(tx_mutex1);
    tx_buf_mutex1_id = osMutexCreate (osMutex(tx_mutex1));

    BB_UARTComRegisterSession(BB_UART_COM_SESSION_2);

    while (1)
    {
        osEvent event1 = osMessageGet(com_queue1_id, osWaitForever);
        if (event1.status == osEventMessage)
        {
            if (tx_buf1_index < sizeof(tx_buf1))
            {
                tx_buf1[tx_buf1_index++] = event1.value.v;
            }

            if (tx_buf1_index == sizeof(tx_buf1))
            {
                COMTASK_FlushTxBufferToUart(com_queue1_id);
            }
        }
    }
}

static void COMTASK_Rx0Function(void const *argument)
{
    uint8_t i = 0;
    uint8_t cnt = 0;

    static uint8_t tx_last_byte_timeout0_bak = 0;
    
    while (1)
    {
        COMTASK_FlushTxBufferToUart(com_queue0_id);
    
        i = 0;
        cnt = BB_UARTComReceiveMsg(BB_UART_COM_SESSION_1, rx_buf0, sizeof(rx_buf0));

        while (i < cnt)
        {
            uart_putc(3, rx_buf0[i]);
            i++;
        }
        osDelay(20);
    }
}

static void COMTASK_Rx1Function(void const *argument)
{
    uint8_t i = 0;
    uint8_t cnt = 0;
    
    while (1)
    {
        COMTASK_FlushTxBufferToUart(com_queue1_id);
        
        i = 0;
        cnt = BB_UARTComReceiveMsg(BB_UART_COM_SESSION_2, rx_buf1, sizeof(rx_buf1));

        while (i < cnt)
        {
            uart_putc(4, rx_buf1[i]);
            i++;
        }
        osDelay(20);
    }
}

void COMTASK_Init(void)
{
    COMTASK_UartInit0();
    COMTASK_UartInit1();

    BB_UARTComRemoteSessionInit();

    osThreadDef(COMTASK_Tx0, COMTASK_Tx0Function, osPriorityIdle, 0, 4 * 128);
    osThreadCreate(osThread(COMTASK_Tx0), NULL);

    osThreadDef(COMTASK_Tx1, COMTASK_Tx1Function, osPriorityIdle, 0, 4 * 128);
    osThreadCreate(osThread(COMTASK_Tx1), NULL);

    osThreadDef(COMTASK_Rx0, COMTASK_Rx0Function, osPriorityIdle, 0, 4 * 128);
    osThreadCreate(osThread(COMTASK_Rx0), NULL);

    osThreadDef(COMTASK_Rx1, COMTASK_Rx1Function, osPriorityIdle, 0, 4 * 128);
    osThreadCreate(osThread(COMTASK_Rx1), NULL);
}


