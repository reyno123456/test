#include "serial.h"

void serial_init()
{
  int devisor;
  uart_type *uart0_regs;
  uart0_regs = (uart_type *)UART0_BASE;

  //uart0_regs->IIR_FCR = UART_FCR_ENABLE_FIFO | UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT | UART_FCR_TRIGGER_14;
  uart0_regs->IIR_FCR = UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT | UART_FCR_TRIGGER_14;
  uart0_regs->DLH_IER = 0x00000000;
  uart0_regs->LCR = UART_LCR_WLEN8 & ~(UART_LCR_STOP | UART_LCR_PARITY);

  devisor = 50000000 / (16 * 115200);
  uart0_regs->LCR |= UART_LCR_DLAB;
  uart0_regs->DLH_IER = (devisor >> 8) & 0x000000ff;
  uart0_regs->RBR_THR_DLL = devisor & 0x000000ff;
  uart0_regs->LCR &= ~UART_LCR_DLAB;
  uart0_regs->DLH_IER = 0x1;
}

void serial_putc(char c)
{
  volatile uart_type *uart0_regs;
  uart0_regs = (uart_type *) UART0_BASE;
  if (c == '\n')
    serial_putc('\r');
  while ((uart0_regs->LSR & UART_LSR_THRE) != UART_LSR_THRE);
  uart0_regs->RBR_THR_DLL = c;
}

/* change the DEC into HEX*/
void print_str(unsigned int n)
{
  int i;
  char tmp;
  for (i = 0; i < 8; i++)
  {
    tmp = (n >> ((7 - i) * 4)) & 0xf;
    if (tmp >= 0xa && tmp <= 0xf)
      tmp = tmp - 0xa + 'a';
    else
      tmp = tmp + '0';
    serial_putc(tmp);
  }
}

void serial_puts(const char *s)
{
  while (*s)
    serial_putc(*s++);
}

/* add by minzhao
*  print the int
*/
void serial_int(unsigned int n)
{
  unsigned int temp = n;
  unsigned int chartmp;
  char str[10];
  unsigned int count = 0;
  unsigned int i;
  while (n)
  {
    if (n < 10)
    {
      n = n + '0';
      str[count] = n;
      count++;
      str[count] = '\0';
      for (i = count - 1; i >= 0; i--)
      {
        serial_putc(str[i]);
      }
      break;
    }
    temp = n / 10;
    chartmp = n - temp * 10;
    chartmp = chartmp + '0';
    n = temp;
    str[count] = chartmp;
    count++;
  }
}

char serial_getc(unsigned int UART_BASE)
{
  char tmp;
  uart_type *uart0_regs;
  uart0_regs = (uart_type *) UART_BASE;
  while ((uart0_regs->LSR & UART_LSR_DR) != UART_LSR_DR);
  tmp = uart0_regs->RBR_THR_DLL;
  return tmp;
}


