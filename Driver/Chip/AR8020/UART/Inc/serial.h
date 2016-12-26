#ifndef __UART__H
#define __UART__H

#include <stdint.h>
#include "interrupt.h"

/* Private define ------------------------------------------------------------*/

#define UART0_BASE               0x40500000
#define	UART1_BASE               0x40510000
#define	UART2_BASE               0x40520000
#define	UART3_BASE               0x40530000
#define	UART4_BASE               0x40540000
#define	UART5_BASE               0x40550000
#define	UART6_BASE               0x40560000
#define	UART7_BASE               0x40570000
#define	UART8_BASE               0x40580000
#define UART9_BASE               0xA0000000
#define UART10_BASE              0xA0060000 


#define CLK_FREQ                 100000000    // clock frequency, 125MHz = half of Core Clk
//#define CLK_FREQ               80000000    // clock frequency

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

#define UART_TOTAL_CHANNEL       (11)

typedef uint32_t (*UartRxFun)(uint8_t *pu8_rxBuf, uint8_t u8_len);


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


extern UartRxFun g_pfun_uartUserFunTbl[UART_TOTAL_CHANNEL];
extern Irq_handler g_pfun_uartIqrEntryTbl[UART_TOTAL_CHANNEL];
 


void uart_init(unsigned char index, unsigned int baud_rate);
void uart_putc(unsigned char index, char c);
void uart_puts(unsigned char index, const char *s);
char uart_getc(unsigned char index);

void serial_init(unsigned char index, unsigned int baud_rate);
void serial_putc(char c);
void serial_puts(const char *s);
char serial_getc(void);
void Drv_UART0_IRQHandler(void);

#endif
