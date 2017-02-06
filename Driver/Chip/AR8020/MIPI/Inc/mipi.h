/*****************************************************************************
 * Copyright: 2016-2020, Artosyn. Co., Ltd.
 * File name: can.h
 * Description: mipi drive function declaration
 * Author: Artosyn FW
 * Version: V0.01 
 * Date: 2017.01.22
 * History: 
 * 2017.01.22 the first edition
 * *****************************************************************************/

#ifndef __MIPI_H__
#define __MIPI_H__


#include <stdint.h>



#define MIPI_BASE_ADDR       (0xA0050000)
#define ENCODER_BASE_ADDR    (0xA0010000)

typedef struct
{
    uint32_t u32_regAddr;
    uint32_t u32_val;
} STRU_REG_VALUE;


/**
* @brief    
* @param     
* @retval  
* @note    
*/
int32_t MIPI_Init(void);

#endif
