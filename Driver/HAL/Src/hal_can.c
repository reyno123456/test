/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_can.c
Description: The external HAL APIs to use the CAN controller.
Author: Artosyn Software Team
Version: 0.0.1
Date: 2016/12/29
History: 
        0.0.1    2016/12/29    The initial version of hal_can.c
*****************************************************************************/
#include "hal_can.h"
#include "hal_nvic.h"
#include "can.h"
#include <stdio.h>

/**
* @brief   can controller initialization. 
* @param   st_halCanConfig        init need info.  
*                           
* @retval HAL_OK                  init can controller successed. 
*         HAL_CAN_ERR_INIT        init can controller failed.
*         HAL_CAN_ERR_COMPONENT   can channel error.init failed.
* @note   None. 
*/
HAL_RET_T HAL_CAN_Init(STRU_HAL_CAN_CONFIG *st_halCanConfig)
{
    uint8_t u8_canCh;

    if ((st_halCanConfig->e_halCanComponent) > HAL_CAN_COMPONENT_3)
    {
       return  HAL_CAN_ERR_COMPONENT;
    }

    u8_canCh = (uint8_t)(st_halCanConfig->e_halCanComponent);

    //first ,disable can_x interrupt.
    HAL_NVIC_DisableIrq(u8_canCh + HAL_NVIC_CAN_IRQ0_VECTOR_NUM);
    
    //record user function
    CAN_RegisterUserRxHandler(u8_canCh,\
                   (CAN_RcvMsgHandler)(st_halCanConfig->pfun_halCanRcvMsg));
    
    // hardware init
    CAN_InitHw((st_halCanConfig->e_halCanComponent), 
               (st_halCanConfig->e_halCanBaudr), 
               (st_halCanConfig->u32_halCanAcode), 
               (st_halCanConfig->u32_halCanAmask), 
               0x88,
               (st_halCanConfig->e_halCanFormat));

    //connect can interrupt service function
    HAL_NVIC_RegisterHandler(u8_canCh + HAL_NVIC_CAN_IRQ0_VECTOR_NUM, 
                           CAN_IntrSrvc, 
		           NULL);

    // enable can_x interrupt.
    if (NULL != (st_halCanConfig->pfun_halCanRcvMsg)) 
    {
        HAL_NVIC_EnableIrq(u8_canCh + HAL_NVIC_CAN_IRQ0_VECTOR_NUM);
    }

    return HAL_OK;
}

/**
* @brief  send can frame.include standard data frame,standard remote frame,
*         extended data frame, extended remote frame
* @param  st_halCanMsg       pointer to can message for send. 
*                           
* @retval HAL_OK                can message send successed. 
*         HAL_CAN_ERR_SEND_MSG  can message send failed.
* @note   None. 
*/
HAL_RET_T HAL_CAN_Send(STRU_HAL_CAN_MSG *st_halCanMsg)
{
    CAN_Send((st_halCanMsg->e_halCanComponent), 
              (st_halCanMsg->u32_halCanId), 
              &(st_halCanMsg->u8_halCanDataArray[0]), 
              (st_halCanMsg->u8_halCanDataLen), 
              (st_halCanMsg->e_halCanFormat), 
              (st_halCanMsg->e_halCanType));

    return HAL_OK;
}
