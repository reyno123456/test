#ifndef __TEST_HAL_UART_H__
#define __TEST_HAL_UART_H__

#include "hal_uart.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "debuglog.h"


void command_TestHalUartInit(unsigned char *ch, unsigned char *br);

void command_TestHalUartTx(unsigned char *ch, unsigned char *len);

void command_TestHalUartRx(unsigned char *ch);

uint32_t uartRxCallBack(uint8_t *pu8_rxBuf, uint8_t u8_len);

#endif
