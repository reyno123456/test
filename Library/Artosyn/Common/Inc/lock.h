#ifndef LOCK_H
#define LOCK_H

#include <stdint.h>

typedef uint32_t lock_type;

static inline void Lock(uint32_t* lock)
{
    uint32_t tmp;
    
    __asm volatile (
        "1: ldrex %0, [%1] \n"
        "teq %0, #0 \n"
        "strexeq %0, %2, [%1] \n"
        "teqeq %0, #0 \n"
        "bne 1b"
        :"=&r" (tmp)
        :"r" (lock), "r"(1)
        :"cc");
}

static inline void UnLock(uint32_t* lock)
{
    __asm volatile (
        "str %1, [%0] \n"
        :
        :"r" (lock), "r"(0)
        :"cc");
}

static inline void Lock_IRQ(uint32_t* lock)
{
    uint32_t tmp;
    
    __asm volatile (
        "cpsid i \n"
        "1: ldrex %0, [%1] \n"
        "teq %0, #0 \n"
        "strexeq %0, %2, [%1] \n"
        "teqeq %0, #0 \n"
        "bne 1b"
        :"=&r" (tmp)
        :"r" (lock), "r"(1)
        :"cc");
}

static inline void UnLock_IRQ(uint32_t* lock)
{
    __asm volatile (
        "str %1, [%0] \n"
        "cpsie i \n"
        :
        :"r" (lock), "r"(0)
        :"cc");
}

#endif

