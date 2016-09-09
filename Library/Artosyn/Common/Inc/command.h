#ifndef __COMMAND__H
#define __COMMAND__H
#include <stdint.h>

extern unsigned char g_commandPos;
extern char g_commandLine[50];
extern uint32_t g_sendUSBFlag;

extern void command_init(void);
extern void command_parse(char *cmd);
extern void command_run(char *cmdArray[], unsigned int cmdNum);
extern void command_readMemory(char *addr);
extern void command_writeMemory(char *addr, char *value);
extern unsigned int command_str2uint(char *str);
extern void command_readSdcard(char *Dstaddr, char *BytesNum, char *SrcAddr);
extern void command_writeSdcard(char *Dstaddr, char *BytesNum, char *SrcAddr);
extern void command_eraseSdcard(char *Dstaddr, char * BlockNum);

extern void delay_ms(uint32_t num);
extern void write_reg32(uint32_t *addr, uint32_t data);
extern uint32_t read_reg32(uint32_t *addr);

#endif
