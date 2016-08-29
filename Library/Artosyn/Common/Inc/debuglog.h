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
        printf("%s:%s:%d\t"fmt"\n", __FILE__, __FUNCTION__, __LINE__, ##arg);   \
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

#endif
