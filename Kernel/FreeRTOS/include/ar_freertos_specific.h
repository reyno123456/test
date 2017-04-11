/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_rots.h
Description: The external HAL APIs to use the SDMMC controller.
Author: Wumin @Artosyn Software Team
Version: 0.1.1
Date: 2017/4/5
History: 
        0.1.1    2017/4/5    The initial version of ar_freertos_specific.h
*****************************************************************************/
#ifndef _AR_FREERTOS_SPECIFIC_H
#define _AR_FREERTOS_SPECIFIC_H 

#include "hal_ret_type.h"
#include <stdint.h>

/* for task traceability define  */
#define portCONFIGURE_TIMER_FOR_RUN_TIME_STATS()  rtos_feature_task_traceability_init()
#define portGET_RUN_TIME_COUNTER_VALUE() g_rtos_feature_task_traceability_cnt

/* used for freertos kernel */
uint32_t g_rtos_feature_task_traceability_cnt;
void rtos_feature_task_traceability_init(void);

/* public for user application */
void ar_top(void);
void ar_osDelay(uint32_t u32_ms);

#endif
