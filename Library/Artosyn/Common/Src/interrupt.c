#include "interrupt.h"
#include "serial.h"
#include "command.h"
#include "cmsis_os.h"
#include "debuglog.h"

void SysTick_Handler(void)
{
    osSystickHandler();
}

void UART0_IRQHandler(void)
{
    Drv_UART0_IRQHandler();
}

void UART1_IRQHandler(void)
{
    dlog_info("UART1_IRQHandler\n");
}

void UART2_IRQHandler(void)
{
    dlog_info("UART2_IRQHandler\n");
}

void UART3_IRQHandler(void)
{
    dlog_info("UART3_IRQHandler\n");
}

void UART4_IRQHandler(void)
{
    dlog_info("UART4_IRQHandler\n");
}

void UART5_IRQHandler(void)
{
    dlog_info("UART5_IRQHandler\n");
}

void UART6_IRQHandler(void)
{
    dlog_info("UART6_IRQHandler\n");
}

void UART7_IRQHandler(void)
{
    dlog_info("UART7_IRQHandler\n");
}

void UART8_IRQHandler(void)
{
    dlog_info("UART8_IRQHandler\n");
}

void TIMER0_0_IRQHandler(void)
{
    dlog_info("TIMER0_0_IRQHandler\n");
}

void TIMER0_1_IRQHandler(void)
{
    dlog_info("TIMER0_1_IRQHandler\n");
}

void TIMER0_2_IRQHandler(void)
{
    dlog_info("TIMER0_2_IRQHandler\n");
}

void TIMER0_3_IRQHandler(void)
{
    dlog_info("TIMER0_3_IRQHandler\n");
}

void TIMER0_4_IRQHandler(void)
{
    dlog_info("TIMER0_4_IRQHandler\n");
}

void TIMER0_5_IRQHandler(void)
{
    dlog_info("TIMER0_5_IRQHandler\n");
}

void TIMER0_6_IRQHandler(void)
{
    dlog_info("TIMER0_6_IRQHandler\n");
}

void TIMER0_7_IRQHandler(void)
{
    dlog_info("TIMER0_7_IRQHandler\n");
}

void TIMER1_0_IRQHandler(void)
{
    dlog_info("TIMER1_0_IRQHandler\n");
}

void TIMER1_1_IRQHandler(void)
{
    dlog_info("TIMER1_1_IRQHandler\n");
}

void TIMER1_2_IRQHandler(void)
{
    dlog_info("TIMER1_2_IRQHandler\n");
}

void TIMER1_3_IRQHandler(void)
{
    dlog_info("TIMER1_3_IRQHandler\n");
}

void TIMER1_4_IRQHandler(void)
{
    dlog_info("TIMER1_4_IRQHandler\n");
}

void TIMER1_5_IRQHandler(void)
{
    dlog_info("TIMER1_5_IRQHandler\n");
}

void TIMER1_6_IRQHandler(void)
{
    dlog_info("TIMER1_6_IRQHandler\n");
}

void TIMER1_7_IRQHandler(void)
{
    dlog_info("TIMER1_7_IRQHandler\n");
}

void TIMER2_0_IRQHandler(void)
{
    dlog_info("TIMER2_0_IRQHandler\n");
}

void TIMER2_1_IRQHandler(void)
{
    dlog_info("TIMER2_1_IRQHandler\n");
}

void TIMER2_2_IRQHandler(void)
{
    dlog_info("TIMER2_2_IRQHandler\n");
}

void TIMER2_3_IRQHandler(void)
{
    dlog_info("TIMER2_3_IRQHandler\n");
}

void TIMER2_4_IRQHandler(void)
{
    dlog_info("TIMER2_4_IRQHandler\n");
}

void TIMER2_5_IRQHandler(void)
{
    dlog_info("TIMER2_5_IRQHandler\n");
}

void TIMER2_6_IRQHandler(void)
{
    dlog_info("TIMER2_6_IRQHandler\n");
}

void TIMER2_7_IRQHandler(void)
{
    dlog_info("TIMER2_7_IRQHandler\n");
}

void SSI_0_IRQHandler(void)
{
    dlog_info("SSI_0_IRQHandler\n");
}

void SSI_1_IRQHandler(void)
{
    dlog_info("SSI_1_IRQHandler\n");
}

void SSI_2_IRQHandler(void)
{
    dlog_info("SSI_2_IRQHandler\n");
}

void SSI_3_IRQHandler(void)
{
    dlog_info("SSI_3_IRQHandler\n");
}

void SSI_4_IRQHandler(void)
{
    dlog_info("SSI_4_IRQHandler\n");
}

void SSI_5_IRQHandler(void)
{
    dlog_info("SSI_5_IRQHandler\n");
}

void SSI_6_IRQHandler(void)
{
    dlog_info("SSI_6_IRQHandler\n");
}

void I2C_MASTER0_IRQHandler(void)
{
    dlog_info("I2C_MASTER0_IRQHandler\n");
}

void I2C_MASTER1_IRQHandler(void)
{
    dlog_info("I2C_MASTER1_IRQHandler\n");
}

void I2C_MASTER2_IRQHandler(void)
{
    dlog_info("I2C_MASTER2_IRQHandler\n");
}

void I2C_MASTER3_IRQHandler(void)
{
    dlog_info("I2C_MASTER3_IRQHandler\n");
}

void CAN_BUS0_IRQHandler(void)
{
    dlog_info("CAN_BUS0_IRQHandler\n");
}

void CAN_BUS1_IRQHandler(void)
{
    dlog_info("CAN_BUS1_IRQHandler\n");
}

void CAN_BUS2_IRQHandler(void)
{
    dlog_info("CAN_BUS2_IRQHandler\n");
}

void CAN_BUS3_IRQHandler(void)
{
    dlog_info("CAN_BUS3_IRQHandler\n");
}

void WDT_0_IRQHandler(void)
{
    dlog_info("WDT_0_IRQHandler\n");
}

void WDT_1_IRQHandler(void)
{
    dlog_info("WDT_1_IRQHandler\n");
}

void GPIO_0_IRQHandler(void)
{
    dlog_info("GPIO_0_IRQHandler\n");
}

void GPIO_1_IRQHandler(void)
{
    dlog_info("GPIO_1_IRQHandler\n");
}

void GPIO_2_IRQHandler(void)
{
    dlog_info("GPIO_2_IRQHandler\n");
}

void GPIO_3_IRQHandler(void)
{
    dlog_info("GPIO_3_IRQHandler\n");
}

void I2C_SLAVE_IRQHandler(void)
{
    dlog_info("I2C_SLAVE_IRQHandler\n");
}

void RTC_IRQHandler(void)
{
    dlog_info("RTC_IRQHandler\n");
}

void USB_OTG0_IRQHandler(void)
{
    dlog_info("USB_OTG0_IRQHandler\n");
}

void USB_OTG1_IRQHandler(void)
{
    dlog_info("USB_OTG1_IRQHandler\n");
}

void SD_CTRL_IRQHandler(void)
{
    dlog_info("SD_CTRL_IRQHandler\n");
}

void DMA_IRQHandler(void)
{
    dlog_info("DMA_IRQHandler\n");
}

void UART9_IRQHandler(void)
{
    dlog_info("UART9_IRQHandler\n");
}

void VIDEO_ENC_IRQHandler(void)
{
    dlog_info("VIDEO_ENC_IRQHandler\n");
}

void UART10_IRQHandler(void)
{
    dlog_info("UART10_IRQHandler\n");
}

void VIDEO_I2C_IRQHandler(void)
{
    dlog_info("VIDEO_I2C_IRQHandler\n");
}

void VIDEO_SPI_IRQHandler(void)
{
    dlog_info("VIDEO_SPI_IRQHandler\n");
}

void WDT_2_IRQHandler(void)
{
    dlog_info("WDT_2_IRQHandler\n");
}

void VIDEO_SSI_IRQHandler(void)
{
    dlog_info("VIDEO_SSI_IRQHandler\n");
}

void VIDEO_AXI_WR_CH0_IRQHandler(void)
{
    dlog_info("VIDEO_AXI_WR_CH0_IRQHandler\n");
}

void VIDEO_AXI_WR_CH1_IRQHandler(void)
{
    dlog_info("VIDEO_AXI_WR_CH1_IRQHandler\n");
}

void GLOBAL_REG0_IRQHandler(void)
{
    dlog_info("GLOBAL_REG0_IRQHandler\n");
}

void GLOBAL_REG1_IRQHandler(void)
{
    dlog_info("GLOBAL_REG1_IRQHandler\n");
}

void SRAM_READY_0_IRQHandler(void)
{
    dlog_info("SRAM_READY_0_IRQHandler\n");
}

void SRAM_READY_1_IRQHandler(void)
{
    dlog_info("SRAM_READY_1_IRQHandler\n");
}

void WIMAX_TX_EN_IRQHandler(void)
{
    dlog_info("WIMAX_TX_EN_IRQHandler\n");
}

void WIMAX_RX_EN_IRQHandler(void)
{
    dlog_info("WIMAX_RX_EN_IRQHandler\n");
}




