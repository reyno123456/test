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
#include "hal_nvic.h"
#include "interrupt.h"

/**
* @brief  Config the Buffer in SRAM to Receive Video Data from SKY.
* @param  void
* @retval   void
* @note  
*/
void HAL_SRAM_ReceiveVideoConfig(ENUM_HAL_SRAM_DATA_PATH e_dataPathReverse)
{
    SRAM_GROUND_ReceiveVideoConfig();

    /* register the irq handler */
    reg_IrqHandle(BB_SRAM_READY_IRQ_0_VECTOR_NUM, SRAM_Ready0IRQHandler, NULL);
    INTR_NVIC_SetIRQPriority(BB_SRAM_READY_IRQ_0_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_SRAM0,0));

    /* enable the SRAM_READY_0 IRQ */
    INTR_NVIC_EnableIRQ(BB_SRAM_READY_IRQ_0_VECTOR_NUM);

    /* register the irq handler */
    reg_IrqHandle(BB_SRAM_READY_IRQ_1_VECTOR_NUM, SRAM_Ready1IRQHandler, NULL);
    INTR_NVIC_SetIRQPriority(BB_SRAM_READY_IRQ_1_VECTOR_NUM,INTR_NVIC_EncodePriority(NVIC_PRIORITYGROUP_5,INTR_NVIC_PRIORITY_SRAM1,0));

    /* enable the SRAM_READY_1 IRQ */
    INTR_NVIC_EnableIRQ(BB_SRAM_READY_IRQ_1_VECTOR_NUM);

    g_u8DataPathReverse = e_dataPathReverse;
}


/**
* @brief  Enable Sky Video Bypass Function
* @param  e_sramVideoCh                                     indicate which video channel to use
* @retval   HAL_OK                                               reset buffer success
*               HAL_SRAM_ERR_CHANNEL_INVALID      the input channel number is not correct
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
* @brief  Disable Sky Video Bypass Function
* @param  e_sramVideoCh                                     indicate which video channel to use
* @retval   HAL_OK                                               reset buffer success
*               HAL_SRAM_ERR_CHANNEL_INVALID      the input channel number is not correct
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


void HAL_SRAM_CheckChannelTimeout(void)
{
    SRAM_CheckTimeout();
}

