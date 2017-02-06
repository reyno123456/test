/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_mipi.c
Description: The external HAL APIs to use the mipi controller.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2017/01/23
History: 
        0.0.1    2017/01/23    The initial version of hal_mipi.c
*****************************************************************************/
#include "hal_mipi.h"
#include "mipi.h"
#include <stdio.h>


HAL_RET_T HAL_MIPI_Init(void)
{
    MIPI_Init();
}
