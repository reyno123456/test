#include "FreeRTOS.h"
#include "test_freertos.h"
#include "task.h"
#include "cmsis_os.h"
#include "debuglog.h"

#define m7_malloc pvPortMalloc
#define m7_free vPortFree

static QueueHandle_t xQueue = NULL;
static SemaphoreHandle_t xMutex;
static unsigned char task1_log_ignore = 1;
static unsigned char task2_log_ignore = 1;

void vTask1( void const * argument)
{
	volatile unsigned long ul;

	for (;;)
	{
        if (task1_log_ignore == 0)
	    {
            dlog_info("Task1 is running\n");
        }
		osDelay(1500);
	}
}

void vTask2( void const * argument)
{
	volatile unsigned long ul;

	for (;;)
	{
        if (task2_log_ignore == 0)
        {
            dlog_info("Task2 is running\n");
        }
		osDelay(3000);
	}
}

void vSendTask(void const *argument)
{
	char cValueToSend;
	portBASE_TYPE xStatus;
	cValueToSend = (char)argument;
	for (;; )
	{
		xStatus = xQueueSendToBack(xQueue, &cValueToSend, 0);
		if (xStatus != pdPASS)
		{
			dlog_info("Could not send to the Queue\n");
		}
		taskYIELD();
	}

}

void vReceiveTask(void const *argument)
{
	char cReceiveValue;
	portBASE_TYPE xStatus;
	const portTickType xTicksToWait = 100 / portTICK_RATE_MS;
	for (;;)
	{

		if (uxQueueMessagesWaiting(xQueue) != 0)
		{
			dlog_info("Queue should have been empty\n");
		}
		xStatus = xQueueReceive(xQueue, &cReceiveValue, xTicksToWait);
		if (xStatus == pdPASS)
		{
			dlog_info("receive value is: %d\n", cReceiveValue);
			serial_putc('\n');
		}
		else
		{
			dlog_info("Could not receive from the Queue\n");
		}
	}

}

void PrintfMutex(void *argument)
{
	for (; ;)
	{
		if (xSemaphoreTake(xMutex, portMAX_DELAY) == pdPASS)
		{
			dlog_info("Mutex argument %p", argument);
			serial_putc('\n');
		}

		if (xSemaphoreGive(xMutex) == pdTRUE)
		{
			dlog_info("Give the Mutex!\n");
		}
	}
}

void TestQueue(void)
{
	xQueue = xQueueCreate(5, sizeof(char));
	if (xQueue != NULL)
	{
		osThreadDef(Send1_Thread, vSendTask, osPriorityNormal, 0, 8 * configMINIMAL_STACK_SIZE);
		osThreadCreate(osThread(Send1_Thread), (void *)100);

		osThreadDef(Send2_Thread, vSendTask, osPriorityNormal, 0, 8 * configMINIMAL_STACK_SIZE);
		osThreadCreate(osThread(Send2_Thread), (void *)150);

		osThreadDef(Receive_Thread, vReceiveTask, osPriorityHigh, 0, 8 * configMINIMAL_STACK_SIZE);
		osThreadCreate(osThread(Receive_Thread), NULL);
		//xTaskCreate(vSendTask, "sender 1", 1000, (void *)'a', 1, NULL);
		//xTaskCreate(vSendTask, "sender 2", 1000, (void *)'b', 1, NULL);
		//xTaskCreate(vReceiveTask, "receive", 1000, NULL, 2, NULL);
		dlog_info("Create task succ!\n");
		osKernelStart();
	}
	else
	{
		dlog_info("Create Queue failed\n");
	}
}

void TestTask(void)
{
	osThreadDef(Task1_Thread, vTask1, osPriorityNormal, 0, 8 * configMINIMAL_STACK_SIZE);
	osThreadCreate(osThread(Task1_Thread), NULL);
	osThreadDef(Task2_Thread, vTask2, osPriorityNormal, 0, 8 * configMINIMAL_STACK_SIZE);
	osThreadCreate(osThread(Task2_Thread), NULL);
	osKernelStart();
	dlog_info("osKernelStart done \n");
}

void TestMutex(void)
{
	xMutex = xSemaphoreCreateMutex();
	if (xMutex == NULL)
	{
		dlog_info("Create the xMutex failed!\n");
	}
	else
	{
		//xTaskCreate(PrintfMutex, "Print1", 1000, "Task1 gets the Mutex!\n", 1, NULL);
		xTaskCreate(PrintfMutex, "Print1", 1000, "Task1 gets the Mutex!\n", 1, NULL);
		xTaskCreate(PrintfMutex, "Print1", 1000, "Task2 gets the Mutex!\n", 2, NULL);
		osKernelStart();
	}
	for (; ;);
}

void TestMem(void)
{
	unsigned int i;

	void* dynaAddr[5] = { NULL };
	for (i = 0; i < 5; i++)
	{
		dynaAddr[i] = m7_malloc(100);
		dlog_info("malloc addr is 0x%x\n", (unsigned int)dynaAddr[i]);
		serial_putc('\n');
	}

	for (i = 0; i < 5; i++)
	{
		m7_free(dynaAddr[i]);
		dynaAddr[i] = NULL;
	}

	for (;;);
}

void command_TestTask(void)
{
    task1_log_ignore = 0;
    task2_log_ignore = 0;
}

void command_TestTaskQuit(void)
{
    task1_log_ignore = 1;
    task2_log_ignore = 1;
}
