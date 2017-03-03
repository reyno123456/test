/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: hal_softi2s.h
Description: this module contains the helper fucntions necessary to control the general
             purpose softi2s block.softi2s use gpio to read i2s data.
             audio data buff limit 1M (AUDIO_SDRAM_END-AUDIO_SDRAM_START).
Author: Artosy Software Team
Version: 0.0.1
Date: 2017/02/21
History:
         0.0.1    2016/12/19    The initial version of hal_softi2s.h
*****************************************************************************/

#ifndef __HAL_SOFTI2S_H__
#define __HAL_SOFTI2S_H__

#include "hal_gpio.h"

#define AUDIO_SDRAM_START                (0x81F00000)
#define AUDIO_SDRAM_END                  (0x82000000) //SDRAM 0x81000000 - 0x81FFFFFF
//#define AUDIO_SDRAM_END                  (0x81FFE400) //SDRAM 0x81000000 - 0x81FFFFFF
#define AUDIO_BYPASS_START               (0xB1800000)
#define ADUIO_DATA_BUFF_LENGHT           (1024*4/sizeof(uint16_t)) 
//#define PLL_CLK_200M_48K                 (20048000)
//#define PLL_CLK_200M_44p1K               (10044100)
//#define PLL_CLK_100M_48K                 (10048000)
#define PLL_CLK_100M_44p1K               (10044100)


typedef struct
{
    uint32_t u32_audioDataAddress;
    ENUM_HAL_GPIO_NUM e_audioLeftGpioNum;
    ENUM_HAL_GPIO_NUM e_audioRightGpioNum;
    ENUM_HAL_GPIO_NUM e_audioDataGpioNum;    
} STRU_HAL_SOFTI2S_INIT;

/**
* @brief    soft i2s initialization
* @param    STRU_HAL_SOFTI2S_INIT: I2S initialization.
* @retval   HAL_OK    means the initialization is well done.
* @retval   HAL_SOFTI2S_ERR_INIT    means interrput gpio comflict.
* @note     none
*/
HAL_RET_T HAL_SOFTI2S_Init(STRU_HAL_SOFTI2S_INIT *st_i2sInit);

/**
* @brief    soft i2s run function
* @param    none
* @retval   none
* @note     none
*/
void HAL_SOFTI2S_Funct(void);
void IRQHandlerLeftAudio(void);
void IRQHandlerRightAudio(void);

#endif /*__HAL_SOFTI2S_H__END*/
