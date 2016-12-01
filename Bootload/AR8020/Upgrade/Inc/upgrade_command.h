#ifndef UPGRADE_COMMAND_H
#define UPGRADE_COMMAND_H

void UPGRADE_CommandInit(uint8_t uart_num);
unsigned char UPGRADE_CommandGetEnterStatus(void);
void UPGRADE_CommandFulfill(void);
#endif
