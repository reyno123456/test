#ifndef __COMMAND__H
#define __COMMAND__H
#include <stdint.h>

extern uint32_t g_sendUSBFlag;

void command_init(void);
unsigned char command_getEnterStatus(void);
void command_fulfill(void);

#endif
