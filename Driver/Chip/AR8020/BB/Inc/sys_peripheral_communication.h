#ifndef __SYS_PERIPHERAL_COMMUNICATION_H
#define __SYS_PERIPHERAL_COMMUNICATION_H

#include "config_functions_sel.h"
#define __IO volatile
#include <stdint.h>
typedef struct
{
   uint8_t SkyIrq;              /*!< Specifies the sky 14ms irq had delayed.
                                 This parameter can be a value of @ref <enable or disable>   */
   uint8_t SkySpecialIrq;       /*!< Specifies the 560ms irq from the timer.
                                 This parameter can be a value of @ref <enable or disable> */
   uint8_t GrdIrq;              /*!< Specifies the ground 14ms irq had delayed.
                                 This parameter can be a value of @ref <enable or disable>   */
   uint8_t GrdGetSnrA;          /*!< Specifies the flag of getting snr 8 times in every 14ms.
                                 This parameter can be a value of @ref <enable or disable>   */
   uint8_t GrdGetSnrB;
   uint8_t GrdGetSnrC;
   uint8_t GrdGetSnrD;
   uint8_t GrdGetSnrE;
   uint8_t GrdGetSnrF;
   uint8_t GrdGetSnrG;
   uint8_t GrdGetSnrH;

   uint8_t TxcyregOnly;       /*!< Specifies the flag that plot reg value to cy7c and ensure run one time in every 14ms irq.
                                 This parameter can be a value of @ref <enable,disable>  */
   uint8_t TxcyosdOnly;       /*!< Specifies the flag that plot osd msg to cy7c and ensure run one time in every 14ms irq.
                                 This parameter can be a value of @ref <enable,disable>  */
}Sys_FlagTypeDef;
typedef struct
{
  uint8_t CmdsearchID;        /*!< Specifies command of searching ID from PC.
                                 This parameter can be a value of @ref <enable or disable> */
  uint8_t CmdEXTI;            /*!< Specifies command of enable/disable exteral irq from PC.
                                 This parameter can be a value of @ref <enable or disable> */
}PC_FlagTypeDef;

//=======================================Stm32 system=================================================
void Sys_Parm_Init(void);
void PC_Parm_Init(void);

//=======================================Stm32 & Baseband=================================================
#endif
