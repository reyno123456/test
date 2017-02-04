/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_hdmi_rx.c
Description: The external HAL APIs to use HDMI RX.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/12/20
History: 
        0.0.1    2016/12/20    The initial version of hal_hdmi_rx.c
*****************************************************************************/

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "sys_event.h"
#include "adv_7611.h"
#include "hal_hdmi_rx.h"
#include "hal_gpio.h"
#include "gpio.h"
#include "hal_ret_type.h"
#include "interrupt.h"
#include "debuglog.h"
#include "hal_nvic.h"


static STRU_HDMI_RX_STATUS s_st_hdmiRxStatus[HAL_HDMI_RX_MAX] = {0};
static void HAL_HDMI_RX_IrqHandler0(uint32_t u32_vectorNum);
static void HAL_HDMI_RX_IrqHandler1(uint32_t u32_vectorNum);

static STRU_HDMI_RX_OUTPUT_FORMAT s_st_hdmiRxSupportedOutputFormat[] =
{
    {720,  480,  60},
    {1280, 720,  30},
    {1280, 720,  50},
    {1280, 720,  60},
    {1920, 1080, 30},
    {1920, 1080, 50},
    {1920, 1080, 60},
};

static uint8_t HDMI_RX_MapToDeviceIndex(ENUM_HAL_HDMI_RX e_hdmiIndex)
{
    return (e_hdmiIndex == HAL_HDMI_RX_0) ? 0 : 1;
}

static HAL_BOOL_T HDMI_RX_CheckVideoFormatSupportOrNot(uint16_t u16_width, uint16_t u16_hight, uint8_t u8_framerate)
{
    uint8_t i = 0;
    uint8_t array_size = sizeof(s_st_hdmiRxSupportedOutputFormat)/sizeof(s_st_hdmiRxSupportedOutputFormat[0]);

    for (i = 0; i < array_size; i++)
    {
        if ((u16_width == s_st_hdmiRxSupportedOutputFormat[i].u16_width) &&
            (u16_hight == s_st_hdmiRxSupportedOutputFormat[i].u16_hight) &&
            (u8_framerate == s_st_hdmiRxSupportedOutputFormat[i].u8_framerate))
        {
            break;
        }
    }

    if (i < array_size)
    {
        return HAL_TRUE;
    }
    else
    {
        return HAL_FALSE;
    }
}

static HAL_BOOL_T HDMI_RX_CheckVideoFormatChangeOrNot(ENUM_HAL_HDMI_RX e_hdmiIndex, 
                                                      uint16_t u16_width, 
                                                      uint16_t u16_hight, 
                                                      uint8_t u8_framerate)
{
    if (e_hdmiIndex >= HAL_HDMI_RX_MAX)
    {
        return HAL_FALSE;
    }
    
    if ((s_st_hdmiRxStatus[e_hdmiIndex].u8_devEnable == 1) &&
        ((s_st_hdmiRxStatus[e_hdmiIndex].st_videoFormat.u16_width != u16_width) ||
         (s_st_hdmiRxStatus[e_hdmiIndex].st_videoFormat.u16_hight != u16_hight) ||
         (s_st_hdmiRxStatus[e_hdmiIndex].st_videoFormat.u8_framerate != u8_framerate)))
    {
        return HAL_TRUE;
    }
    else
    {
        return HAL_FALSE;
    }
}

static void HDMI_RX_CheckFormatStatus(ENUM_HAL_HDMI_RX e_hdmiIndex, HAL_BOOL_T b_noDiffCheck)
{
    static uint8_t s_u8_formatNotSupportCount = 0;

    uint16_t u16_width;
    uint16_t u16_hight;
    uint8_t u8_framerate;

    uint8_t u8_7611Index = HDMI_RX_MapToDeviceIndex(e_hdmiIndex);
    ADV_7611_GetVideoFormat(u8_7611Index, &u16_width, &u16_hight, &u8_framerate);
    if (HDMI_RX_CheckVideoFormatSupportOrNot(u16_width, u16_hight, u8_framerate) == HAL_TRUE)
    {
        s_u8_formatNotSupportCount = 0;
        if ((b_noDiffCheck == HAL_TRUE) || 
            (HDMI_RX_CheckVideoFormatChangeOrNot(e_hdmiIndex, u16_width, u16_hight, u8_framerate) == HAL_TRUE))
        {
            STRU_SysEvent_H264InputFormatChangeParameter p;
            p.index = (e_hdmiIndex == HAL_HDMI_RX_0) ? 0 : 1;
            p.width = u16_width;
            p.hight = u16_hight;
            p.framerate = u8_framerate;
            SYS_EVENT_Notify(SYS_EVENT_ID_H264_INPUT_FORMAT_CHANGE, (void*)&p);

            s_st_hdmiRxStatus[e_hdmiIndex].st_videoFormat.u16_width    = u16_width;
            s_st_hdmiRxStatus[e_hdmiIndex].st_videoFormat.u16_hight    = u16_hight;
            s_st_hdmiRxStatus[e_hdmiIndex].st_videoFormat.u8_framerate = u8_framerate;
        }
    }
    else
    {
        if (HAL_HDMI_POLLING == s_st_hdmiRxStatus[e_hdmiIndex].st_configure.e_getFormatMethod)
        {
             // Format not supported
            if (s_u8_formatNotSupportCount <= HDMI_RX_FORMAT_NOT_SUPPORT_COUNT_MAX)
            {
                s_u8_formatNotSupportCount++;
            }            
        }
        else
        {
            s_u8_formatNotSupportCount = HDMI_RX_FORMAT_NOT_SUPPORT_COUNT_MAX;
        }

        if (s_u8_formatNotSupportCount == HDMI_RX_FORMAT_NOT_SUPPORT_COUNT_MAX)
        {
            STRU_SysEvent_H264InputFormatChangeParameter p;
            p.index = HDMI_RX_MapToDeviceIndex(e_hdmiIndex);
            p.width = 0;
            p.hight = 0;
            p.framerate = 0;
            SYS_EVENT_Notify(SYS_EVENT_ID_H264_INPUT_FORMAT_CHANGE, (void*)&p);

            s_st_hdmiRxStatus[e_hdmiIndex].st_videoFormat.u16_width    = 0;
            s_st_hdmiRxStatus[e_hdmiIndex].st_videoFormat.u16_hight    = 0;
            s_st_hdmiRxStatus[e_hdmiIndex].st_videoFormat.u8_framerate = 0;
        }
       
    }
}

static void HDMI_RX_IdleCallback0(void *paramPtr)
{
    if (s_st_hdmiRxStatus[HAL_HDMI_RX_0].u8_devEnable == 1)
    {
        HDMI_RX_CheckFormatStatus(HAL_HDMI_RX_0, HAL_FALSE);
    }
}

static void HDMI_RX_IdleCallback1(void *paramPtr)
{
    if (s_st_hdmiRxStatus[HAL_HDMI_RX_1].u8_devEnable == 1)
    {
        HDMI_RX_CheckFormatStatus(HAL_HDMI_RX_1, HAL_FALSE);
    }
}

/**
* @brief  The HDMI RX init function.
* @param  e_hdmiIndex       The HDMI RX index number, the right number should be 0-1 and totally
*                           2 HDMI RX can be supported.
* @param  pst_hdmiConfigure hdmiconfigure include polling or interrupr.
* @retval HAL_OK            means the HDMI RX init is well done.
*         HAL_I2C_ERR_INIT  means some error happens in the HDMI RX init.
*         HAL_HDMI_GET_ERR_GORMAT_METHOD means get format method error.
*         HAL_HDMI_INPUT_SOURCE means the nunber of hdmi input error.
* @note   None.
*/

HAL_RET_T HAL_HDMI_RX_Init(ENUM_HAL_HDMI_RX e_hdmiIndex, STRU_HDMI_CONFIGURE *pst_hdmiConfigure)
{
    s_st_hdmiRxStatus[e_hdmiIndex].u8_devEnable = 1;
    memcpy(&(s_st_hdmiRxStatus[e_hdmiIndex].st_configure),pst_hdmiConfigure,sizeof(STRU_HDMI_CONFIGURE));

    if (s_st_hdmiRxStatus[e_hdmiIndex].st_configure.e_getFormatMethod == HAL_HDMI_INTERRUPT)
    {
        HAL_NVIC_SetPriority(GPIO_INTR_N0_VECTOR_NUM + ((pst_hdmiConfigure->u8_interruptGpio)>>5),5,0);
        switch (e_hdmiIndex)
        {
            case HAL_HDMI_RX_0:
            {
                HAL_GPIO_RegisterInterrupt(s_st_hdmiRxStatus[e_hdmiIndex].st_configure.u8_interruptGpio, 
                                           HAL_GPIO_ACTIVE_HIGH, HAL_GPIO_EDGE_SENUMSITIVE, HAL_HDMI_RX_IrqHandler0);
                break;
            }
            case HAL_HDMI_RX_1:
            {
                HAL_GPIO_RegisterInterrupt(s_st_hdmiRxStatus[e_hdmiIndex].st_configure.u8_interruptGpio, 
                                           HAL_GPIO_ACTIVE_HIGH, HAL_GPIO_EDGE_SENUMSITIVE, HAL_HDMI_RX_IrqHandler1);
                break;
            }
            default :
            {
                return HAL_HDMI_INPUT_COUNT;
            }

        }
    }
    else if (s_st_hdmiRxStatus[e_hdmiIndex].st_configure.e_getFormatMethod == HAL_HDMI_POLLING)
    {
        SYS_EVENT_RegisterHandler(SYS_EVENT_ID_IDLE, e_hdmiIndex == HAL_HDMI_RX_0 ? HDMI_RX_IdleCallback0 : HDMI_RX_IdleCallback1);
    }
    else
    {
        return HAL_HDMI_GET_ERR_GORMAT_METHOD;
    }

    ADV_7611_Initial(HDMI_RX_MapToDeviceIndex(e_hdmiIndex));
    
    return HAL_OK;
}

/**
* @brief  The HDMI RX uninit function.
* @param  e_hdmiIndex       The HDMI RX index number, the right number should be 0-1 and totally
*                           2 HDMI RX can be supported.
* @retval HAL_OK            means the HDMI RX init is well done.
*         HAL_I2C_ERR_INIT  means some error happens in the HDMI RX init.
* @note   None.
*/

HAL_RET_T HAL_HDMI_RX_UnInit(ENUM_HAL_HDMI_RX e_hdmiIndex)
{
    s_st_hdmiRxStatus[e_hdmiIndex].u8_devEnable = 0;

    SYS_EVENT_UnRegisterHandler(SYS_EVENT_ID_IDLE, e_hdmiIndex == HAL_HDMI_RX_0 ? HDMI_RX_IdleCallback0 : HDMI_RX_IdleCallback1);

    return HAL_OK;
}


/**
* @brief  The HDMI RX video format retrieve function.
* @param  e_hdmiIndex         The HDMI RX index number, the right number should be 0-1 and totally
*                             2 HDMI RX can be supported.
* @param  pu16_width          The pointer to the video width value.
* @param  pu16_hight          The pointer to the video hight value.
* @param  pu8_framterate      The pointer to the video framerate value.
* @retval HAL_OK                            means the HDMI RX init is well done.
*         HAL_HDMI_RX_ERR_GET_VIDEO_FORMAT  means some error happens in the HDMI RX init.
* @note   None.
*/

HAL_RET_T HAL_HDMI_RX_GetVideoFormat(ENUM_HAL_HDMI_RX e_hdmiIndex, 
                                     uint16_t *pu16_width, 
                                     uint16_t *pu16_hight, 
                                     uint8_t *pu8_framterate)
{
    ADV_7611_GetVideoFormat(HDMI_RX_MapToDeviceIndex(e_hdmiIndex), pu16_width, pu16_hight, pu8_framterate);
        
    return HAL_OK;
}


static void HAL_HDMI_RX_IrqHandler0(uint32_t u32_vectorNum)
{
    if (ADV_7611_IrqHandler0())
    {
        HDMI_RX_IdleCallback0(NULL);
    }
 
}

static void HAL_HDMI_RX_IrqHandler1(uint32_t u32_vectorNum)
{
    
    if (ADV_7611_IrqHandler1())
    {
        HDMI_RX_IdleCallback1(NULL);
    }
}
