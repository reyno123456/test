/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_rots.h
Description: The external HAL APIs to use the SDMMC controller.
Author: Wumin @Artosyn Software Team
Version: 0.1.1
Date: 2017/4/5
History: 
        0.1.1    2017/4/5    The initial version of ar_freertos_specific.c
                             added top command
*****************************************************************************/

#include <stdint.h>
#include "ar_freertos_specific.h"
#include "hal_timer.h"

#include "FreeRTOSConfig.h"
#include "portmacro.h"
#include "FreeRTOS.h"
#include "task.h"
#include "debuglog.h"

#define LOCAL_MAX_TASK_NUM 20
#define LOCAL_HAL_TIMER 21
#define LOCAL_HAL_TIMER_INTERVEL 100

uint32_t g_rtos_feature_task_traceability_cnt = 0;

static void rtos_feature_task_traceability_handler(void)
{
	g_rtos_feature_task_traceability_cnt++;
	// dlog_info("running...\n");
}

void rtos_feature_task_traceability_init(void)
{
	HAL_TIMER_RegisterTimer(LOCAL_HAL_TIMER, LOCAL_HAL_TIMER_INTERVEL, rtos_feature_task_traceability_handler);
	// dlog_info("rtos_feature_task_traceability_init called\n");
}

void ar_top(void) 
{
	const char task_state[]={'r','R','B','S','D'};  
	volatile UBaseType_t uxArraySize, x;
	uint32_t ulTotalRunTime;
	unsigned int ulStatsAsPercentage;
	TaskStatus_t pxTaskStatusArray[LOCAL_MAX_TASK_NUM];

	/* require the total tasks */
	uxArraySize = uxTaskGetNumberOfTasks();  
	
	if(uxArraySize>LOCAL_MAX_TASK_NUM)  
	{  
		dlog_info("too many tasks\n");
		return;
	}
	/* require each task struct information */
	uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, &ulTotalRunTime);  

	dlog_info("\n");
	dlog_info("name                    status ID Priority   stack   used\n");

	if( ulTotalRunTime > 0 )  
	{  
		for( x = 0; x < uxArraySize; x++ )  
        {  
        	char tmp[128];  
             
			/* caculate the total run time and total percent time */
            ulStatsAsPercentage =(uint64_t)(pxTaskStatusArray[x].ulRunTimeCounter)*100 / 
            ulTotalRunTime;  
   
            if( ulStatsAsPercentage > 0UL )  
            {
            	sprintf(tmp,"%-24s%-7c%-6d%-8d%-8d%d%%",pxTaskStatusArray[x].pcTaskName,
					task_state[pxTaskStatusArray[x].eCurrentState],  
					pxTaskStatusArray[x].xTaskNumber,pxTaskStatusArray[x].uxCurrentPriority,  
					pxTaskStatusArray[x].usStackHighWaterMark,ulStatsAsPercentage);  
            }  
            else  
            {  
				/* cpu used not more than 1 */
				sprintf(tmp,"%-24s%-7c%-6d%-8d%-8dt<1%%",pxTaskStatusArray[x].pcTaskName,
					task_state[pxTaskStatusArray[x].eCurrentState],  
					pxTaskStatusArray[x].xTaskNumber,pxTaskStatusArray[x].uxCurrentPriority,
					pxTaskStatusArray[x].usStackHighWaterMark); 
            }
			dlog_info("%s\n",tmp);  
        }  
    }  
	return;
}

