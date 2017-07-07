#ifndef DEBUGLOG_H
#define DEBUGLOG_H

#ifdef  __cplusplus
extern "C"
{
#endif


#include <stdio.h>

#define LOG_LEVEL_CRITICAL 1
#define LOG_LEVEL_ERROR 2
#define LOG_LEVEL_WARNING 3
#define LOG_LEVEL_INFO 4

extern volatile uint8_t g_log_level;

#define DLOG_Critical(fmt, arg...) \
do \
{ \
    {if (g_log_level >= LOG_LEVEL_CRITICAL) \
    printf("CPU%d: %s\t" fmt "\n", *((int*)0x0000018C), __FUNCTION__, ##arg);} \
}while(0)

#define DLOG_Error(fmt, arg...) \
do \
{ \
    {if (g_log_level >= LOG_LEVEL_ERROR) \
    printf("CPU%d: %s\t" fmt "\n", *((int*)0x0000018C), __FUNCTION__, ##arg);} \
}while(0)

#define DLOG_Warning(fmt, arg...) \
do \
{ \
    {if (g_log_level >= LOG_LEVEL_WARNING)  \
    printf("CPU%d: %s\t" fmt "\n", *((int*)0x0000018C), __FUNCTION__, ##arg);} \
}while(0)

#define DLOG_Info(fmt, arg...) \
do \
{ \
    {if (g_log_level >= LOG_LEVEL_INFO) \
    printf("CPU%d: %s\t" fmt "\n", *((int*)0x0000018C), __FUNCTION__, ##arg);} \
}while(0)

#define DEBUG_LOG_USE_SRAM_OUTPUT_BUFFER
#define DEBUG_LOG_OUTPUT_CPU_AFTER_CPU
#define USE_SYS_EVENT_TRIGGER_DEBUG_LOG_PROCESS

#define DEBUG_LOG_UART_PORT        0

typedef enum
{
    DLOG_SERVER_PROCESSOR = 0,
    DLOG_CLIENT_PROCESSOR,
} ENUM_DLOG_PROCESSOR;

typedef void (*FUNC_CommandRun)(char *cmdArray[], uint32_t cmdNum);
typedef void (*FUNC_LogSave)(char *buf, uint32_t bytes);

void DLOG_Init(FUNC_CommandRun func_command, 
                  FUNC_LogSave func_log_sd,
                  ENUM_DLOG_PROCESSOR e_dlogProcessor);

unsigned int DLOG_Output(unsigned int byte_num);
void DLOG_Process(void* p);

#define dlog_init          DLOG_Init
#define dlog_output        DLOG_Output
#define dlog_info(...)     DLOG_Info(__VA_ARGS__)
#define dlog_warning(...)  DLOG_Warning(__VA_ARGS__)
#define dlog_error(...)    DLOG_Error(__VA_ARGS__)
#define dlog_critical(...) DLOG_Critical(__VA_ARGS__)

extern volatile uint8_t sd_mountStatus;


#ifdef  __cplusplus
}
#endif


#endif
