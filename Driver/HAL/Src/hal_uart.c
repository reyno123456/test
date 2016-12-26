/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_uart.c
Description: The external HAL APIs to use the UART controller.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/12/20
History: 
        0.0.1    2016/12/20    The initial version of hal_uart.c
*****************************************************************************/


#include "hal_uart.h"
#include "serial.h"

static const uint32_t s_u32_uartBaudrTbl[] = 
                      {9600, 19200, 38400, 57600, 115200};

static const IRQ_type s_e_uartIntrVectorTbl[] = 
			  { UART_INTR0_VECTOR_NUM,
			    UART_INTR1_VECTOR_NUM,                      
			    UART_INTR2_VECTOR_NUM,                      
			    UART_INTR3_VECTOR_NUM,                      
			    UART_INTR4_VECTOR_NUM,                      
			    UART_INTR5_VECTOR_NUM,                      
			    UART_INTR6_VECTOR_NUM,                      
			    UART_INTR7_VECTOR_NUM,                      
			    UART_INTR8_VECTOR_NUM,                      
			    VIDEO_UART9_INTR_VECTOR_NUM,
			    VIDEO_UART10_INTR_VECTOR_NUM };



/**
* @brief  The UART initialization function which must be called 
*         before using the UART controller.
* @param  e_uartComponent   The UART controller number, the right number 
*                           should be 0-8 and totally 9 UART controllers 
*                           can be used by application.
*         e_uartBaudr       uart baud rate,can be 9600 19200 ... 115200.
*         pfun_rxFun        the uart receive data function.each time uart
*                           received data, it will be called.
*                           
* @retval HAL_OK            means the initializtion is well done.
*         HAL_UART_ERR_INIT means some error happens in the initializtion.
* @note   
*         pfun_rxFun be called in interrupt function,it should never block.
*         and try do less work,just read the data.
*/
HAL_RET_T HAL_UART_Init(ENUM_HAL_UART_COMPONENT e_uartComponent, 
		        ENUM_HAL_UART_BAUDR e_uartBaudr, 
			UartRxFun pfun_rxFun)
{
    uint32_t u32_uartBaudr;
    uint8_t u8_uartCh;

    if (e_uartComponent > HAL_UART_COMPONENT_8)
    {
        return HAL_UART_ERR_INIT;
    }
    if (e_uartBaudr > HAL_UART_BAUDR_115200)
    {
        return HAL_UART_ERR_INIT;
    }
    if (NULL == pfun_rxFun)
    {
        return HAL_UART_ERR_INIT;
    }
    u8_uartCh = (uint8_t)(e_uartComponent);

    //record user function
    g_pfun_uartUserFunTbl[u8_uartCh] = pfun_rxFun;

    // uart hadrware init.
    u32_uartBaudr = s_u32_uartBaudrTbl[(uint8_t)(e_uartBaudr)];
    uart_init(u8_uartCh, u32_uartBaudr);
    
   //connect uart interrupt service function
   reg_IrqHandle(s_e_uartIntrVectorTbl[u8_uartCh], 
                 g_pfun_uartIqrEntryTbl[u8_uartCh], 
		 NULL);
   INTR_NVIC_EnableIRQ(s_e_uartIntrVectorTbl[u8_uartCh]);

    return HAL_OK;
}

/**
* @brief  The uart data send function which can be used to send out uart 
*         data by the uart controller.
* @param  e_uartComponent         The UART controller number, the right number 
*                                 should be 0-8 and totally 9 UART controllers 
*                                 can be used by application.
*         pu8_txBuf               The transmit buffer pointer to be sent out by uart.
*         u16_len                 The transmit buffer size in byte. 
* @retval HAL_OK                  means the UART data write is well done.
*         HAL_UART_ERR_WRITE_DATA means some error happens in the UART data .
* @note   None.
*/
HAL_RET_T HAL_UART_TxData(ENUM_HAL_UART_COMPONENT e_uartComponent, 
		          uint8_t *pu8_txBuf, 
			  uint16_t u16_len)
{
    uint8_t u8_uartCh;
    uint16_t u16_uartTxCnt = 0;
    
    if (e_uartComponent > HAL_UART_COMPONENT_8)
    {
        return HAL_UART_ERR_WRITE_DATA;
    }
    if (NULL == pu8_txBuf)
    {
        return HAL_UART_ERR_WRITE_DATA;
    }
    if (0 == u16_len)
    {
        return HAL_UART_ERR_WRITE_DATA;
    }
    u8_uartCh = (uint8_t)(e_uartComponent);

    while(u16_uartTxCnt < u16_len)
    {
        uart_putc(u8_uartCh, pu8_txBuf[u16_uartTxCnt]);    
	u16_uartTxCnt += 1;
    }

    return HAL_OK;
}


