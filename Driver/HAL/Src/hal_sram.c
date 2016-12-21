/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_sram.c
Description: The external HAL APIs to use the SRAM.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/12/21
History: 
        0.0.1    2016/12/21    The initial version of hal_sram.c
*****************************************************************************/

#include <stdint.h>
#include "sram.h"
#include "hal_sram.h"
#include "interrupt.h"

/**
* @brief  Config the Buffer in SRAM to Receive Video Data from SKY.
* @param  void
* @retval   void
* @note  
*/
void HAL_SRAM_ReceiveVideoConfig(void)
{
    SRAM_GROUND_ReceiveVideoConfig();

    /* register the irq handler */
    reg_IrqHandle(BB_SRAM_READY_IRQ_0_VECTOR_NUM, SRAM_Ready0IRQHandler);

    /* enable the SRAM_READY_0 IRQ */
    INTR_NVIC_EnableIRQ(BB_SRAM_READY_IRQ_0_VECTOR_NUM);

    /* register the irq handler */
    reg_IrqHandle(BB_SRAM_READY_IRQ_1_VECTOR_NUM, SRAM_Ready1IRQHandler);

    /* enable the SRAM_READY_1 IRQ */
    INTR_NVIC_EnableIRQ(BB_SRAM_READY_IRQ_1_VECTOR_NUM);
}


/**
* @brief  Reset the SRAM Buffer to receive the next package video data.
* @param  void
* @retval   void
* @note  
*/
HAL_RET_T HAL_SRAM_ResetBuffer(ENUM_HAL_SRAM_VIDEO_CHANNEL e_sramVideoCh)
{
    HAL_RET_T   ret = HAL_OK;

    if (HAL_SRAM_VIDEO_CHANNEL_0 == e_sramVideoCh)
    {
        SRAM_Ready0Confirm();
    }
    else if (HAL_SRAM_VIDEO_CHANNEL_1 == e_sramVideoCh)
    {
        SRAM_Ready1Confirm();
    }
    else
    {
        ret = HAL_SRAM_ERR_CHANNEL_INVALID;
    }

    return ret;
}


/**
* @brief  Reset the SRAM Buffer1 to receive the next package video data.
* @param  void
* @retval   void
* @note  
*/
HAL_RET_T HAL_SRAM_EnableSkyBypassVideo(ENUM_HAL_SRAM_VIDEO_CHANNEL e_sramVideoCh)
{
    if (e_sramVideoCh > HAL_SRAM_VIDEO_CHANNEL_1)
    {
        return HAL_SRAM_ERR_CHANNEL_INVALID;
    }

    SRAM_SKY_EnableBypassVideoConfig(e_sramVideoCh);

    return HAL_OK;
}


/**
* @brief  Reset the SRAM Buffer1 to receive the next package video data.
* @param  void
* @retval   void
* @note  
*/
HAL_RET_T HAL_SRAM_DisableSkyBypassVideo(ENUM_HAL_SRAM_VIDEO_CHANNEL e_sramVideoCh)
{
    if (e_sramVideoCh > HAL_SRAM_VIDEO_CHANNEL_1)
    {
        return HAL_SRAM_ERR_CHANNEL_INVALID;
    }

    SRAM_SKY_DisableBypassVideoConfig(e_sramVideoCh);

    return HAL_OK;
}



