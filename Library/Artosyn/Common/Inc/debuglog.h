#ifndef DEBUGLOG_H
#define DEBUGLOG_H

#ifdef  __cplusplus
extern "C"
{
#endif


#include <stdio.h>

#define LOG_LEVEL_CRITICAL 0
#define LOG_LEVEL_ERROR 1
#define LOG_LEVEL_WARNING 2
#define LOG_LEVEL_INFO 3

#define LOG_LEVEL_DEFAULT LOG_LEVEL_INFO

#define LOG_LEVEL LOG_LEVEL_DEFAULT

#define _print_log(fmt, arg...)  \
    do \
    { \
        printf("CPU%d: %s\t" fmt "\n", *((int*)0x0000018C), __FUNCTION__, ##arg);   \
    }while(0)

#if(LOG_LEVEL_CRITICAL <= LOG_LEVEL)
    #define DLOG_Critical(fmt, arg...)    _print_log(fmt, ##arg)
#else
    #define DLOG_Critical(fmt, arg...)    do{} while(0)
#endif
 
#if(LOG_LEVEL_ERROR <= LOG_LEVEL)
    #define DLOG_Error(fmt, arg...)   _print_log(fmt, ##arg)
#else
    #define DLOG_Error(fmt, arg...)   do{} while(0)
#endif

#if(LOG_LEVEL_WARNING <= LOG_LEVEL)
    #define DLOG_Warning(fmt, arg...)   _print_log(fmt, ##arg)
#else
    #define DLOG_Warning(fmt, arg...)   do{} while(0)
#endif

#if(LOG_LEVEL_INFO <= LOG_LEVEL)
    #define DLOG_Info(fmt, arg...)   _print_log(fmt, ##arg)
#else
    #define DLOG_Info(fmt, arg...)   do{} while(0)
#endif

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

void DLOG_Init(FUNC_CommandRun func, ENUM_DLOG_PROCESSOR e_dlogProcessor);
unsigned int DLOG_Output(unsigned int byte_num);
void DLOG_Process(void* p);

#define dlog_init          DLOG_Init
#define dlog_output        DLOG_Output
#define dlog_info(...)     DLOG_Info(__VA_ARGS__)
#define dlog_warning(...)  DLOG_Warning(__VA_ARGS__)
#define dlog_error(...)    DLOG_Error(__VA_ARGS__)
#define dlog_critical(...) DLOG_Critical(__VA_ARGS__)

#ifdef  __cplusplus
}
#endif


#endif
