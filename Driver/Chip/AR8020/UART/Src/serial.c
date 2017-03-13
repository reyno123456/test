#include "stddef.h"
#include "serial.h"
#include "pll_ctrl.h"
#include "cpu_info.h"
#include <stdint.h>


//the user CallBack for uart receive data.
static UART_RxHandler s_pfun_uartUserHandlerTbl[UART_TOTAL_CHANNEL] =  
       { NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL };

volatile static uint8_t s_u8_uartTflArray[UART_TOTAL_CHANNEL] = 
                                          {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
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

static uint32_t get_uart_clock_by_index(unsigned char index)
{
    uint16_t u16_pllClk = 64;

    switch (index)
    {
    case 0:
    case 1:
    case 2:
    case 3:
    case 4:
    case 5:
    case 6:
    case 7:
    case 8:
        PLLCTRL_GetCoreClk(&u16_pllClk, ENUM_CPU0_ID);
        u16_pllClk = u16_pllClk >> 1;
        break;
    case 9:
    case 10:
        PLLCTRL_GetCoreClk(&u16_pllClk, ENUM_CPU2_ID);
        break;
    default:
        break;
    }

    return (uint32_t)u16_pllClk;
}

void uart_init(unsigned char index, unsigned int baud_rate)
{
    int devisor;
    volatile uart_type *uart_regs;
    uart_regs = get_uart_type_by_index(index);

    if (uart_regs != NULL)
    {
        uart_regs->IIR_FCR = UART_FCR_CLEAR_RCVR | UART_FCR_CLEAR_XMIT | UART_FCR_TRIGGER_14 | UART_FCR_ENABLE_FIFO;
        uart_regs->DLH_IER = 0x00000000;
        uart_regs->LCR = UART_LCR_WLEN8 & ~(UART_LCR_STOP | UART_LCR_PARITY);

        devisor = (get_uart_clock_by_index(index) * 1000000) / (16 * baud_rate);
        uart_regs->LCR |= UART_LCR_DLAB;
        uart_regs->DLH_IER = (devisor >> 8) & 0x000000ff;
        uart_regs->RBR_THR_DLL = devisor & 0x000000ff;
        uart_regs->LCR &= ~UART_LCR_DLAB;
        uart_regs->DLH_IER = 0x3;
    }
}

void uart_putc(unsigned char index, char c)
{
    volatile uart_type *uart_regs;
    uart_regs = get_uart_type_by_index(index);

    if (uart_regs != NULL)
    {
        while (s_u8_uartTflArray[index] >= UART_TFL_MAX);
        
        uart_regs->RBR_THR_DLL = c;
        s_u8_uartTflArray[index] += 1;
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

/**
* @brief  uart interrupt servive function.just handled data reception.  
* @param  u32_vectorNum           Interrupt number.
* @retval None.
* @note   None.
*/
void UART_IntrSrvc(uint32_t u32_vectorNum)
{
    uint8_t u8_uartRxBuf[64];
    uint8_t u8_uartRxLen = 0;
    uint8_t u8_uartCh;
    uint32_t u32_uartStatus;
    uint32_t u32_uartIsrType;
    uint32_t u32_uartIsrType2;
    volatile uart_type   *pst_uartRegs;

    if (VIDEO_UART9_INTR_VECTOR_NUM == u32_vectorNum)
    {
        u8_uartCh = 9;
    }
    else if (VIDEO_UART10_INTR_VECTOR_NUM == u32_vectorNum)
    {
        u8_uartCh = 10;
    }
    else
    {
        u8_uartCh = u32_vectorNum - UART_INTR0_VECTOR_NUM;
    }
 
    pst_uartRegs = get_uart_type_by_index(u8_uartCh);

    u32_uartStatus = pst_uartRegs->LSR;
    u32_uartIsrType = pst_uartRegs->IIR_FCR;
    u32_uartIsrType2 = u32_uartIsrType;

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
        if(NULL != (s_pfun_uartUserHandlerTbl[u8_uartCh]))
        {
            (s_pfun_uartUserHandlerTbl[u8_uartCh])(u8_uartRxBuf, u8_uartRxLen);
        }
    }

    // TX empty interrupt.
    if (UART_IIR_THR_EMPTY == (u32_uartIsrType2 & UART_IIR_THR_EMPTY))
    {
        s_u8_uartTflArray[u8_uartCh] = 0;
    }
}

/**
* @brief  register user function for uart recevie data.called in interrupt
*         service function.
* @param  u8_uartCh           uart channel, 0 ~ 10.
* @param  userHandle          user function for uart recevie data.
* @retval 
*         -1                  register user function failed.
*         0                   register user function sucessed.
* @note   None.
*/
int32_t UART_RegisterUserRxHandler(uint8_t u8_uartCh, UART_RxHandler userHandle)
{
    if(u8_uartCh >= UART_TOTAL_CHANNEL)
    {
        return -1;
    }

    s_pfun_uartUserHandlerTbl[u8_uartCh] = userHandle;

    return 0;
}

/**
* @brief  unregister user function for uart recevie data.
* @param  u8_uartCh           uart channel, 0 ~ 10.
* @retval 
*         -1                  unregister user function failed.
*         0                   unregister user function sucessed.
* @note   None.
*/
int32_t UART_UnRegisterUserRxHandler(uint8_t u8_uartCh)
{
    if(u8_uartCh >= UART_TOTAL_CHANNEL)
    {
        return -1;
    }

    s_pfun_uartUserHandlerTbl[u8_uartCh] = NULL;

    return 0;
}

/**
* @brief  clear uart fifo count.
* @param  u8_uartCh           uart channel, 0 ~ 10.
* @retval 
*         -1                  unregister user function failed.
*         0                   unregister user function sucessed.
* @note   None.
*/
int32_t UART_ClearTflCnt(uint8_t u8_uartCh)
{
    if(u8_uartCh < UART_TOTAL_CHANNEL)
    {
        s_u8_uartTflArray[u8_uartCh] = 0;
        return 0;
    }
    else
    {
        return -1;
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


