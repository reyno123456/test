#include "interrupt.h"
#include "serial.h"
#include "command.h"

/* added by xiongjiangjiang */
void UART0_IRQHandler(void)
{
    char                  c;
    unsigned int          status;
    unsigned int          isrType;
    volatile uart_type   *uart0_regs;

    uart0_regs = (uart_type *)UART0_BASE;
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

void UART1_IRQHandler(void)
{
    serial_puts("UART1_IRQHandler\n");
}

void UART2_IRQHandler(void)
{
    serial_puts("UART2_IRQHandler\n");
}

void UART3_IRQHandler(void)
{
    serial_puts("UART3_IRQHandler\n");
}

void UART4_IRQHandler(void)
{
    serial_puts("UART4_IRQHandler\n");
}

void UART5_IRQHandler(void)
{
    serial_puts("UART5_IRQHandler\n");
}

void UART6_IRQHandler(void)
{
    serial_puts("UART6_IRQHandler\n");
}

void UART7_IRQHandler(void)
{
    serial_puts("UART7_IRQHandler\n");
}

void UART8_IRQHandler(void)
{
    serial_puts("UART8_IRQHandler\n");
}

void TIMER0_0_IRQHandler(void)
{
    serial_puts("TIMER0_0_IRQHandler\n");
}

void TIMER0_1_IRQHandler(void)
{
    serial_puts("TIMER0_1_IRQHandler\n");
}

void TIMER0_2_IRQHandler(void)
{
    serial_puts("TIMER0_2_IRQHandler\n");
}

void TIMER0_3_IRQHandler(void)
{
    serial_puts("TIMER0_3_IRQHandler\n");
}

void TIMER0_4_IRQHandler(void)
{
    serial_puts("TIMER0_4_IRQHandler\n");
}

void TIMER0_5_IRQHandler(void)
{
    serial_puts("TIMER0_5_IRQHandler\n");
}

void TIMER0_6_IRQHandler(void)
{
    serial_puts("TIMER0_6_IRQHandler\n");
}

void TIMER0_7_IRQHandler(void)
{
    serial_puts("TIMER0_7_IRQHandler\n");
}

void TIMER1_0_IRQHandler(void)
{
    serial_puts("TIMER1_0_IRQHandler\n");
}

void TIMER1_1_IRQHandler(void)
{
    serial_puts("TIMER1_1_IRQHandler\n");
}

void TIMER1_2_IRQHandler(void)
{
    serial_puts("TIMER1_2_IRQHandler\n");
}

void TIMER1_3_IRQHandler(void)
{
    serial_puts("TIMER1_3_IRQHandler\n");
}

void TIMER1_4_IRQHandler(void)
{
    serial_puts("TIMER1_4_IRQHandler\n");
}

void TIMER1_5_IRQHandler(void)
{
    serial_puts("TIMER1_5_IRQHandler\n");
}

void TIMER1_6_IRQHandler(void)
{
    serial_puts("TIMER1_6_IRQHandler\n");
}

void TIMER1_7_IRQHandler(void)
{
    serial_puts("TIMER1_7_IRQHandler\n");
}

void TIMER2_0_IRQHandler(void)
{
    serial_puts("TIMER2_0_IRQHandler\n");
}

void TIMER2_1_IRQHandler(void)
{
    serial_puts("TIMER2_1_IRQHandler\n");
}

void TIMER2_2_IRQHandler(void)
{
    serial_puts("TIMER2_2_IRQHandler\n");
}

void TIMER2_3_IRQHandler(void)
{
    serial_puts("TIMER2_3_IRQHandler\n");
}

void TIMER2_4_IRQHandler(void)
{
    serial_puts("TIMER2_4_IRQHandler\n");
}

void TIMER2_5_IRQHandler(void)
{
    serial_puts("TIMER2_5_IRQHandler\n");
}

void TIMER2_6_IRQHandler(void)
{
    serial_puts("TIMER2_6_IRQHandler\n");
}

void TIMER2_7_IRQHandler(void)
{
    serial_puts("TIMER2_7_IRQHandler\n");
}

void SSI_0_IRQHandler(void)
{
    serial_puts("SSI_0_IRQHandler\n");
}

void SSI_1_IRQHandler(void)
{
    serial_puts("SSI_1_IRQHandler\n");
}

void SSI_2_IRQHandler(void)
{
    serial_puts("SSI_2_IRQHandler\n");
}

void SSI_3_IRQHandler(void)
{
    serial_puts("SSI_3_IRQHandler\n");
}

void SSI_4_IRQHandler(void)
{
    serial_puts("SSI_4_IRQHandler\n");
}

void SSI_5_IRQHandler(void)
{
    serial_puts("SSI_5_IRQHandler\n");
}

void SSI_6_IRQHandler(void)
{
    serial_puts("SSI_6_IRQHandler\n");
}

void I2C_MASTER0_IRQHandler(void)
{
    serial_puts("I2C_MASTER0_IRQHandler\n");
}

void I2C_MASTER1_IRQHandler(void)
{
    serial_puts("I2C_MASTER1_IRQHandler\n");
}

void I2C_MASTER2_IRQHandler(void)
{
    serial_puts("I2C_MASTER2_IRQHandler\n");
}

void I2C_MASTER3_IRQHandler(void)
{
    serial_puts("I2C_MASTER3_IRQHandler\n");
}

void CAN_BUS0_IRQHandler(void)
{
    serial_puts("CAN_BUS0_IRQHandler\n");
}

void CAN_BUS1_IRQHandler(void)
{
    serial_puts("CAN_BUS1_IRQHandler\n");
}

void CAN_BUS2_IRQHandler(void)
{
    serial_puts("CAN_BUS2_IRQHandler\n");
}

void CAN_BUS3_IRQHandler(void)
{
    serial_puts("CAN_BUS3_IRQHandler\n");
}

void WDT_0_IRQHandler(void)
{
    serial_puts("WDT_0_IRQHandler\n");
}

void WDT_1_IRQHandler(void)
{
    serial_puts("WDT_1_IRQHandler\n");
}

void GPIO_0_IRQHandler(void)
{
    serial_puts("GPIO_0_IRQHandler\n");
}

void GPIO_1_IRQHandler(void)
{
    serial_puts("GPIO_1_IRQHandler\n");
}

void GPIO_2_IRQHandler(void)
{
    serial_puts("GPIO_2_IRQHandler\n");
}

void GPIO_3_IRQHandler(void)
{
    serial_puts("GPIO_3_IRQHandler\n");
}

void I2C_SLAVE_IRQHandler(void)
{
    serial_puts("I2C_SLAVE_IRQHandler\n");
}

void RTC_IRQHandler(void)
{
    serial_puts("RTC_IRQHandler\n");
}

void USB_OTG0_IRQHandler(void)
{
    serial_puts("USB_OTG0_IRQHandler\n");
}

void USB_OTG1_IRQHandler(void)
{
    serial_puts("USB_OTG1_IRQHandler\n");
}

void SD_CTRL_IRQHandler(void)
{
    serial_puts("SD_CTRL_IRQHandler\n");
}

void DMA_IRQHandler(void)
{
    serial_puts("DMA_IRQHandler\n");
}

void UART9_IRQHandler(void)
{
    serial_puts("UART9_IRQHandler\n");
}

void VIDEO_ENC_IRQHandler(void)
{
    serial_puts("VIDEO_ENC_IRQHandler\n");
}

void UART10_IRQHandler(void)
{
    serial_puts("UART10_IRQHandler\n");
}

void VIDEO_I2C_IRQHandler(void)
{
    serial_puts("VIDEO_I2C_IRQHandler\n");
}

void VIDEO_SPI_IRQHandler(void)
{
    serial_puts("VIDEO_SPI_IRQHandler\n");
}

void WDT_2_IRQHandler(void)
{
    serial_puts("WDT_2_IRQHandler\n");
}

void VIDEO_SSI_IRQHandler(void)
{
    serial_puts("VIDEO_SSI_IRQHandler\n");
}

void VIDEO_AXI_WR_CH0_IRQHandler(void)
{
    serial_puts("VIDEO_AXI_WR_CH0_IRQHandler\n");
}

void VIDEO_AXI_WR_CH1_IRQHandler(void)
{
    serial_puts("VIDEO_AXI_WR_CH1_IRQHandler\n");
}

void GLOBAL_REG0_IRQHandler(void)
{
    serial_puts("GLOBAL_REG0_IRQHandler\n");
}

void GLOBAL_REG1_IRQHandler(void)
{
    serial_puts("GLOBAL_REG1_IRQHandler\n");
}

void SRAM_READY_0_IRQHandler(void)
{
    serial_puts("SRAM_READY_0_IRQHandler\n");
}

void SRAM_READY_1_IRQHandler(void)
{
    serial_puts("SRAM_READY_1_IRQHandler\n");
}

void WIMAX_TX_EN_IRQHandler(void)
{
    serial_puts("WIMAX_TX_EN_IRQHandler\n");
}

void WIMAX_RX_EN_IRQHandler(void)
{
    serial_puts("WIMAX_RX_EN_IRQHandler\n");
}




