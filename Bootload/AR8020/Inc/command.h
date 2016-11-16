#ifndef __COMMAND__H
#define __COMMAND__H
#include <stdint.h>

void command_init(uint8_t uart_num);
unsigned char command_getEnterStatus(void);
void command_fulfill(void);
void command_uninit(void);
#endif
