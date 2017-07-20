/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: test_os_message.c
Description: 
Author: Wumin @ Artosy Software Team
Version: 0.0.1
Date: 2017/17/19
History:
         0.0.1    2017/07/19    test_os_message
*****************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include "cmsis_os.h"
#include "debuglog.h"
#include "hal.h"
#include "test_os_message.h"

typedef struct {
    uint32_t    voltage;
    uint32_t    current;
    uint32_t    counter;
} message_t;
 
static osPoolId  mpool; 
static osMessageQId  queue;
 
static void os_message_task1 (void const *args) 
{
    uint32_t i = 0;
    while (1) 
    {
        i++;
        message_t *message = (message_t*)osPoolAlloc(mpool);
        message->voltage = i * 33; 
        message->current = i * 11;
        message->counter = i;
        osMessagePut(queue, (uint32_t)message, osWaitForever);
        HAL_Delay(1000);
    }
}
  
static void os_message_task2 (void const *args)
{
    while (1) 
    {
        osEvent evt = osMessageGet(queue, osWaitForever);
        if (evt.status == osEventMessage) 
        {
            message_t *message = (message_t*)evt.value.p;
            dlog_info("Voltage: %d V"   , message->voltage);
            dlog_info("Current: %d A"     , message->current);
            dlog_info("Number of cycles: %u", message->counter);
            osPoolFree(mpool, message);
        }
    }
}

void test_os_message(void)
{
    osPoolDef(mpool, 16, message_t);
    osMessageQDef(queue, 16, message_t);

    mpool = osPoolCreate(osPool(mpool));
    queue = osMessageCreate(osMessageQ(queue), NULL);

    osThreadDef(OS_MESSAGE_task1, os_message_task1, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(OS_MESSAGE_task1), NULL);

    osThreadDef(OS_MESSAGE_task2, os_message_task2, osPriorityNormal, 0, configMINIMAL_STACK_SIZE);
    osThreadCreate(osThread(OS_MESSAGE_task2), NULL);
}
