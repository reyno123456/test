/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_bb.c
Description: The external HAL APIs to use the I2C controller.
Author: Artosy Software Team
Version: 0.0.1
Date: 2016/12/20
History:
        0.0.1    2017/02/06    The initial version of hal_bb_sky.c
*****************************************************************************/


#include <stdio.h>
#include <stdint.h>

#include "bb_spi.h"
#include "bb_ctrl.h"
#include "bb_sky_ctrl.h"
#include "hal_bb.h"


/** 
 * @brief   init baseband to sky mode
 * @param   NONE
 * @return  HAL_OK:                         means init baseband 
 *          HAL_BB_ERR_INIT:                means some error happens in init session 
 */
HAL_RET_T HAL_BB_initSky( void )
{
    BB_init( BB_SKY_MODE );
    BB_SKY_start();

    return HAL_OK;
}

/** 
 * @brief       
 * @param   
 * @retval      
 * @note      
 */
HAL_RET_T HAL_BB_SetAutoSearchRcId(void)
{
    BB_SetAutoSearchRcId();

    return HAL_OK;
}
