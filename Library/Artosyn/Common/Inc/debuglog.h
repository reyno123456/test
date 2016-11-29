#include<stdio.h>
 
#ifndef DEBUGLOG_H
#define DEBUGLOG_H

enum
{
    LOG_LEVEL_CRITICAL = 0,
    LOG_LEVEL_ERROR,
    LOG_LEVEL_WARNING,
    LOG_LEVEL_INFO
};

#define LOG_LEVEL_DEFAULT LOG_LEVEL_INFO

#define LOG_LEVEL LOG_LEVEL_DEFAULT

#define _print_log(fmt, arg...)  \
    do \
    { \
        printf("%s\t"fmt"\n", __FUNCTION__, ##arg);   \
    }while(0)

#if(LOG_LEVEL_CRITICAL <= LOG_LEVEL)
    #define dlog_critical(fmt, arg...)    _print_log(fmt, ##arg)
#else
    #define dlog_critical(fmt, arg...)    do{} while(0)
#endif
 
#if(LOG_LEVEL_ERROR <= LOG_LEVEL)
    #define dlog_error(fmt, arg...)   _print_log(fmt, ##arg)
#else
    #define dlog_error(fmt, arg...)   do{} while(0)
#endif

#if(LOG_LEVEL_WARNING <= LOG_LEVEL)
    #define dlog_warning(fmt, arg...)   _print_log(fmt, ##arg)
#else
    #define dlog_warning(fmt, arg...)   do{} while(0)
#endif

#if(LOG_LEVEL_INFO <= LOG_LEVEL)
    #define dlog_info(fmt, arg...)   _print_log(fmt, ##arg)
#else
    #define dlog_info(fmt, arg...)   do{} while(0)
#endif

#define USE_SRAM_DEBUG_LOG_BUFFER

// 2K * 3 SRAM space
enum
{
    SRAM_DEBUG_LOG_SPACE_0 = 0,
    SRAM_DEBUG_LOG_SPACE_1,
    SRAM_DEBUG_LOG_SPACE_2,
    SRAM_DEBUG_LOG_SPACE_UNKNOWN,
};

#define DEBUG_LOG_BUFFER_WRITE_NO_OVERLAP

#define SRAM_DEBUG_LOG_BUFFER_ST_ADDR_0  0x210FE800
#define SRAM_DEBUG_LOG_BUFFER_END_ADDR_0 0x210FEFFF

#define SRAM_DEBUG_LOG_BUFFER_ST_ADDR_1  0x210FF000
#define SRAM_DEBUG_LOG_BUFFER_END_ADDR_1 0x210FF7FF

#define SRAM_DEBUG_LOG_BUFFER_ST_ADDR_2  0x210FF800
#define SRAM_DEBUG_LOG_BUFFER_END_ADDR_2 0x210FFFFF

void dlog_init(unsigned int index);
unsigned int dlog_output(unsigned int byte_num);

#endif
