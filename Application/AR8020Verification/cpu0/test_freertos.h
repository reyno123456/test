#ifndef __TEST__FUNC__H
#define __TEST__FUNC__H

#include "stm32f746xx.h"

#define m7_malloc pvPortMalloc
#define m7_free vPortFree

void command_TestTask(void);
void command_TestTaskQuit(void);

/* creat by minzhao */
void vTask1(void const * argument);
void vTask2(void const * argument);
void vSendTask(void const *argument);
void vReceiveTask(void const *argument);
void PrintfMutex(void *argument);
void TestQueue(void);
void TestTask(void);
void TestSemaphore(void);
void TestMutex(void);
void TestMem(void);


#endif
