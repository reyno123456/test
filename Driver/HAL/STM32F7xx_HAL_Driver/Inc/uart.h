typedef struct {
  unsigned int RBR_THR_DLL;
  unsigned int DLH_IER;
  unsigned int IIR_FCR;
  unsigned int LCR;
  unsigned int MCR;
  unsigned int LSR;
  unsigned int MSR;
  unsigned int SCR;
  unsigned int LPDLL;
  unsigned int LPDLH;
  unsigned int reserv[2];
  unsigned int SRBR_STHR[16];
  unsigned int FAR;
  unsigned int TFR;
  unsigned int RFW;
  unsigned int USR;
  unsigned int TFL;
  unsigned int RFL;
  unsigned int SRR;
  unsigned int SRTS;
  unsigned int SBCR;
  unsigned int SDMAM;
  unsigned int SFE;
  unsigned int SRT;
  unsigned int STET;
  unsigned int HTX;
  unsigned int DMASA;
  unsigned int reserv1[14];
  unsigned int CPR;
  unsigned int UCV;
  unsigned int CTR;
} uart_type;

/* Private define ------------------------------------------------------------*/
#define UART0_BASE 0x40500000
#define UART_FCR_ENABLE_FIFO     0x01
#define UART_FCR_CLEAR_RCVR      0x02
#define UART_FCR_CLEAR_XMIT      0x04
#define UART_FCR_TRIGGER_14      0xc0
#define UART_LCR_WLEN8           0x03
#define UART_LCR_STOP            0x04
#define UART_LCR_PARITY          0x08
#define UART_LCR_DLAB            0x80
#define UART_LSR_THRE            0x20

static void serial_putc(char c);
static void serial_puts(const char *s);
static void print_str(unsigned int n);

void serial_putc(char c)
{
    uart_type *uart0_regs;
    uart0_regs = (uart_type *) UART0_BASE;
    if (c == '\n')
      serial_putc('\r');
    while ((uart0_regs->LSR & UART_LSR_THRE) != UART_LSR_THRE);
    uart0_regs->RBR_THR_DLL = c;
}

void print_str(unsigned int n)
{
  int i;
  char tmp;
  for (i=0;i<8;i++)
  {
    tmp = (n >> ((7-i)*4)) & 0xf;
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
