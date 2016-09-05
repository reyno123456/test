#include "serial.h"
#include "stddef.h"
#include "command.h"

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
    default:
        uart_regs = (uart_type *)NULL;
        break;
    }

    return uart_regs;
}

void uart_init(unsigned char index, unsigned int baud_rate)
{
    int devisor;
    uart_type *uart_regs;
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

char uart_getc(unsigned char index)
{
    char tmp = 0;
    uart_type *uart_regs = get_uart_type_by_index(index);

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

/* added by xiongjiangjiang */
extern unsigned char g_commandPos;
extern char g_commandLine[50];
void Drv_UART0_IRQHandler(void)
{
    char                  c;
    unsigned int          status;
    unsigned int          isrType;
    volatile uart_type   *uart0_regs;

    uart0_regs = get_uart_type_by_index(0);
    status     = uart0_regs->LSR;
    isrType    = uart0_regs->IIR_FCR;

    /* receive data irq, try to get the data */
    if (UART_IIR_RECEIVEDATA == (isrType & UART_IIR_RECEIVEDATA))
    {
        if ((status & UART_LSR_DATAREADY) == UART_LSR_DATAREADY)
        {
            c = uart0_regs->RBR_THR_DLL;
            /* receive "enter" key */
            if (c == '\r')
            {
                serial_putc('\n');

                /* if g_commandLine is not empty, go to parse command */
                if (g_commandPos > 0)
                    command_parse(g_commandLine);
            }
            /* receive "backspace" key */
            else if (c == '\b')
            {
                if (g_commandPos > 1)
                    g_commandLine[--g_commandPos] = '\0';
                serial_putc('\b');
                serial_putc(' ');
                serial_putc('\b');
            }
            /* receive normal data */
            else
            {
                serial_putc(c);
                g_commandLine[g_commandPos++] = c;
            }
        }
    }
}

void serial_init(void)
{
    uart_init(0, 115200);
}

void serial_putc(char c)
{
    if (c == '\n')
    {
        uart_putc(0, '\r');
    }
    uart_putc(0, c);
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
    return uart_getc(0);
}


