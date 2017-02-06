/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_bb.c
Description: The external HAL APIs to use the I2C controller.
Author: Artosy Software Team
Version: 0.0.1
Date: 2016/12/20
History:
        0.0.1    2017/02/06    The initial version of hal_bb_ground.c
*****************************************************************************/


#include <stdio.h>
#include <stdint.h>

#include "bb_ctrl.h"
#include "bb_grd_ctrl.h"
#include "hal_bb.h"


/** 
 * @brief   init baseband to ground mode
 * @param   NONE
 * @return  HAL_OK:                         means init baseband 
 *          HAL_BB_ERR_INIT:                means some error happens in init session 
 */
HAL_RET_T HAL_BB_initGround( void )
{
    BB_init( BB_GRD_MODE );
	BB_GRD_start();

    return HAL_OK;
}