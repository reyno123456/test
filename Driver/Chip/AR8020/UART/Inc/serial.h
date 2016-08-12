#ifndef __UART__H
#define __UART__H

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
#define UART0_BASE               0x40500000
#define UART_FCR_ENABLE_FIFO     0x01
#define UART_FCR_CLEAR_RCVR      0x02
#define UART_FCR_CLEAR_XMIT      0x04
#define UART_FCR_TRIGGER_14      0xc0
#define UART_LCR_WLEN8           0x03
#define UART_LCR_STOP            0x04
#define UART_LCR_PARITY          0x08
#define UART_LCR_DLAB            0x80
#define UART_LSR_THRE            0x20
#define UART_LSR_DATAREADY       0x01
#define UART_IIR_RECEIVEDATA     0x04
#define UART_LSR_DR              0x01


extern void serial_init(void);
extern void serial_putc(char c);
extern void serial_puts(const char *s);
extern char serial_getc(unsigned int UART_BASE);
extern void print_str(unsigned int n);
extern void serial_int(unsigned int n);
extern void Uart0_IRQHandler(void);

#endif