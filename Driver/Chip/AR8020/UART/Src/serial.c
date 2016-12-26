#include "stddef.h"
#include "serial.h"

/*static void UART0_entryFun(uint32_t u32_vectorNum);
static void UART1_entryFun(void);
static void UART2_entryFun(void);
static void UART3_entryFun(void);
static void UART4_entryFun(void);
static void UART5_entryFun(void);
static void UART6_entryFun(void);
static void UART7_entryFun(void);
static void UART8_entryFun(void);
static void UART9_entryFun(void);
static void UART10_entryFun(void);*/
static void UART0_entryFun(uint32_t u32_vectorNum);
static void UART1_entryFun(uint32_t u32_vectorNum);
static void UART2_entryFun(uint32_t u32_vectorNum);
static void UART3_entryFun(uint32_t u32_vectorNum);
static void UART4_entryFun(uint32_t u32_vectorNum);
static void UART5_entryFun(uint32_t u32_vectorNum);
static void UART6_entryFun(uint32_t u32_vectorNum);
static void UART7_entryFun(uint32_t u32_vectorNum);
static void UART8_entryFun(uint32_t u32_vectorNum);
static void UART9_entryFun(uint32_t u32_vectorNum);
static void UART10_entryFun(uint32_t u32_vectorNum);
static void UART_intrSrvc(uint8_t u8_uartCh);

UartRxFun g_pfun_uartUserFunTbl[UART_TOTAL_CHANNEL] =  
       { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

/*Irq_handler *s_pfun_uartIqrEntryTbl[UART_TOTAL_CHANNEL](uint32_t u32_vectorNum) = 
			{ UART0_entryFun, UART1_entryFun, UART2_entryFun, 
			  UART3_entryFun, UART4_entryFun, UART5_entryFun, 
			  UART6_entryFun, UART7_entryFun, UART8_entryFun, 
			  UART9_entryFun, UART10_entryFun };*/
Irq_handler s_pfun_uartIqrEntryTbl[UART_TOTAL_CHANNEL] = 
			{ UART0_entryFun, UART1_entryFun, UART2_entryFun, 
			  UART3_entryFun, UART4_entryFun, UART5_entryFun, 
			  UART6_entryFun, UART7_entryFun, UART8_entryFun, 
			  UART9_entryFun, UART10_entryFun };
/*********************************************************
 * Generic UART APIs
 *********************************************************/

static uart_type* get_uart_type_by_index(unsigned char index)
{
    uart_type *uart_regs;

    switch (index)
    {
    case 0:
        uart_regs = (uart_type *)UART0_BASE;
        break;
    case 1:
        uart_regs = (uart_type *)UART1_BASE;
        break;
    case 2:
        uart_regs = (uart_type *)UART2_BASE;
        break;
    case 3:
        uart_regs = (uart_type *)UART3_BASE;
        break;
    case 4:
        uart_regs = (uart_type *)UART4_BASE;
        break;
    case 5:
        uart_regs = (uart_type *)UART5_BASE;
        break;
    case 6:
        uart_regs = (uart_type *)UART6_BASE;
        break;
    case 7:
        uart_regs = (uart_type *)UART7_BASE;
        break;
    case 8:
        uart_regs = (uart_type *)UART8_BASE;
        break;
    case 9:
        uart_regs = (uart_type *)UART9_BASE;
        break;
    case 10:
        uart_regs = (uart_type *)UART10_BASE;
        break;
    default:
        uart_regs = (uart_type *)NULL;
        break;
    }

    return uart_regs;
}

void uart_init(unsigned char index, unsigned int baud_rate)
{
    int devisor;
    volatile uart_type *uart_regs;
    uart_regs = get_uart_type_by_index(index);

    if (uart_regs != NULL)
    {
        uart_regs->IIR_FCR = UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT | UART_FCR_TRIGGER_14;
        uart_regs->DLH_IER = 0x00000000;
        uart_regs->LCR = UART_LCR_WLEN8 & ~(UART_LCR_STOP | UART_LCR_PARITY);

        devisor = CLK_FREQ / (16 * baud_rate);
        uart_regs->LCR |= UART_LCR_DLAB;
        uart_regs->DLH_IER = (devisor >> 8) & 0x000000ff;
        uart_regs->RBR_THR_DLL = devisor & 0x000000ff;
        uart_regs->LCR &= ~UART_LCR_DLAB;
        uart_regs->DLH_IER = 0x1;
    }
}

void uart_putc(unsigned char index, char c)
{
    volatile uart_type *uart_regs;
    uart_regs = get_uart_type_by_index(index);

    if (uart_regs != NULL)
    {
        while ((uart_regs->LSR & UART_LSR_THRE) != UART_LSR_THRE);
        uart_regs->RBR_THR_DLL = c;
    }
}

void uart_puts(unsigned char index, const char *s)
{
    while (*s)
    {
        uart_putc(index, *s++);
    }
}

char uart_getc(unsigned char index)
{
    char tmp = 0;
    volatile uart_type *uart_regs = get_uart_type_by_index(index);

    if (uart_regs != NULL)
    {
        while ((uart_regs->LSR & UART_LSR_DATAREADY) != UART_LSR_DATAREADY); 
        tmp = uart_regs->RBR_THR_DLL;
    }

    return tmp;
}

static void UART0_entryFun(uint32_t u32_vectorNum)
{
    UART_intrSrvc(0);
}
static void UART1_entryFun(uint32_t u32_vectorNum)
{
    UART_intrSrvc(1);
}
static void UART2_entryFun(uint32_t u32_vectorNum)
{
    UART_intrSrvc(2);
}
static void UART3_entryFun(uint32_t u32_vectorNum)
{
    UART_intrSrvc(3);
}
static void UART4_entryFun(uint32_t u32_vectorNum)
{
    UART_intrSrvc(4);
}
static void UART5_entryFun(uint32_t u32_vectorNum)
{
    UART_intrSrvc(5);
}
static void UART6_entryFun(uint32_t u32_vectorNum)
{
    UART_intrSrvc(6);
}
static void UART7_entryFun(uint32_t u32_vectorNum)
{
    UART_intrSrvc(7);
}
static void UART8_entryFun(uint32_t u32_vectorNum)
{
    UART_intrSrvc(8);
}
static void UART9_entryFun(uint32_t u32_vectorNum)
{
    UART_intrSrvc(9);
}
static void UART10_entryFun(uint32_t u32_vectorNum)
{
    UART_intrSrvc(10);
}
static void UART_intrSrvc(uint8_t u8_uartCh)
{
    uint8_t u8_uartRxBuf[64];
    uint8_t u8_uartRxLen = 0;
    uint32_t u32_uartStatus;
    uint32_t u32_uartIsrType;
    volatile uart_type   *pst_uartRegs;
    
    pst_uartRegs = get_uart_type_by_index(u8_uartCh);

    u32_uartStatus = pst_uartRegs->LSR;
    u32_uartIsrType = pst_uartRegs->IIR_FCR;

    /* receive data irq, try to get the data */
    while (UART_IIR_RECEIVEDATA == (u32_uartIsrType & UART_IIR_RECEIVEDATA))
    {
        if ((u32_uartStatus & UART_LSR_DATAREADY) == UART_LSR_DATAREADY)
        {
            u8_uartRxBuf[u8_uartRxLen] = pst_uartRegs->RBR_THR_DLL;
	    u8_uartRxLen += 1;
        }
	if(u8_uartRxLen >= 64)
	{
	    break;
	}

        u32_uartStatus = pst_uartRegs->LSR;
        u32_uartIsrType = pst_uartRegs->IIR_FCR;
    }

    if(u8_uartRxLen > 0) // call user function
    {
        (g_pfun_uartUserFunTbl[u8_uartCh])(u8_uartRxBuf, u8_uartRxLen);
    }
}

/*********************************************************
 * Serial(UART0) APIs
 *********************************************************/

static unsigned char g_u8SerialUartIndex = 0xFF;
void serial_init(unsigned char index, unsigned int baud_rate)
{
    uart_init(index, baud_rate);
    g_u8SerialUartIndex = index;
}

void serial_putc(char c)
{
    if (c == '\n')
    {
        uart_putc(g_u8SerialUartIndex, '\r');
    }
    uart_putc(g_u8SerialUartIndex, c);
}

void serial_puts(const char *s)
{
    while (*s)
    {
        serial_putc(*s++);
    }
}

char serial_getc(void)
{
    return uart_getc(g_u8SerialUartIndex);
}


