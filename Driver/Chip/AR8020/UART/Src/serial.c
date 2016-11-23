#include "stddef.h"
#include "serial.h"

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


