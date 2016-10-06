/**
  ******************************************************************************
  * @file    sky_controller.h
  * @author  Artosyn AE/FAE Team
  * @version V1.0
  * @date    03-21-2016
  * @brief
  *
  *
  ******************************************************************************
  */
#ifndef __SKY_CONTROLLER_H
#define __SKY_CONTROLLER_H

#include "config_functions_sel.h"
#ifdef BASEBAND_SKY
#include <stdint.h>

/**
  * @brief Sky Controller Structure definition
  */

typedef struct
{
     uint8_t Rcmissing;         /*!< Specifies rc unlock state.
                                 This parameter can be a value of @ref <enable or disable> */
     uint8_t IDmat;             /*!< Specifies the flag of ID matching successfully.
                                 This parameter can be a value of @ref <enable or disable> */
     uint8_t CmdsearchID;        /*!< Specifies select the function of searching ID.
                                 This parameter can be a value of @ref <enable or disable> */
     uint8_t Cmdtestmode;       /*!< Specifies select the function of test mode.
                                 This parameter can be a value of @ref <enable or disable> */


}Sky_FlagTypeDef;

typedef struct
{
     uint8_t RCChannel;       /*!< Specifies the remote controller working in freq channel.
                                    This parameter can be a value of @ref <RC_FREQ_MODE>   */
     uint8_t ITChannel;       /*!< Specifies the image transmission working frq channel from ground.
                                    This parameter can be a value of @ref <IT_FREQ_MODE>   */
     uint8_t FindIDcnt;       /*!< Specifies the cnt of sky rx from the ground's id.
                                    This parameter can be a value of @ref <1,2,3...>   */
     uint8_t Timerirqcnt;     /*!< Specifies the cnt of 560ms irq.
                                    This parameter can be a value of @ref <1,2,3...>   */
     uint8_t OptID;           /*!< Specifies the optimum ID selecting of ID sequency.
                                    This parameter can be a value of @ref <1,2,3...>   */
     uint8_t IDsearchcnt;        /*!< Specifies the cnt of matching ID successfulling.
                                    This parameter can be a value of @ref <1,2,3...>   */
     uint8_t IDmatcnt;        /*!< Specifies the cnt of matching ID successfulling.
                                    This parameter can be a value of @ref <1,2,3...>   */
     uint8_t Rcunlockcnt;     /*!< Specifies the cnt of remote controller unlock.
                                    This parameter can be a value of @ref <1,2,3...>   */
     uint8_t Changeqammode;     /*!< Specifies the cnt of remote controller unlock.
                                    This parameter can be a value of @ref <1,2,3...>   */
     uint8_t CntAGCGain;     /*!< Specifies the cnt of remote controller unlock.
                                    This parameter can be a value of @ref <1,2,3...>   */


}Sky_HanlderTypeDef;


/**
  * @brief ID _ Data structure definition
  */
typedef struct
{
   uint8_t num;
   uint8_t powerc5;     /*!< Specifies the value of baseband Reg[C5].*/
   uint8_t powerc6;     /*!< Specifies the value of baseband Reg[C6].*/
   uint8_t powermin;    /*!< Specifies the min of <Reg[C5],Reg[C6]>.*/
   uint8_t rcid5;       /*!< Specifies the bit[39:32]of remote controller id.*/
   uint8_t rcid4;       /*!< Specifies the bit[31:24]of remote controller id.*/
   uint8_t rcid3;       /*!< Specifies the bit[23:16]of remote controller id.*/
   uint8_t rcid2;       /*!< Specifies the bit[15:08]of remote controller id.*/
   uint8_t rcid1;       /*!< Specifies the bit[07:00]of remote controller id.*/

} IDRx_TypeDef[ID_SEARCH_MAX_TIMES];
typedef enum
{
  FLASH_BUSY_1 = 1,
  FLASH_ERROR_PG_1,
  FLASH_ERROR_WRP_1,
  FLASH_COMPLETE_1,
  FLASH_TIMEOUT_1

}FLASH_Status;


void Sky_Parm_Initial(void);
void Sky_Write_Rcfrq(uint8_t frqchannel);
void Sky_Write_Rchopfrq(void);
void Grd_Write_Itfrq(uint8_t itfrqcnt);
void Sky_Id_Initial(void);
uint8_t Sky_Id_Match(void);
uint8_t Sky_Crc_Check_Ok(void);
uint8_t Sky_Rc_Err_Flag(void);      //0xE9[]:0x80
uint8_t Sky_Rc_Zero(void);      //0xE9[]:0x00
uint8_t  Min_of_Both(uint8_t a, uint8_t b );
void Sky_Record_Idpower( uint8_t i );
uint8_t Sky_Getopt_Id(void);
void Sky_Search_Right_ID(void);
void Sky_Rc_Hopping (void);
void Sky_Adjust_AGCGain(void);
void Sky_Adjust_AGCGain_SearchID(uint8_t i);
void Sky_Hanlde_SpecialIrq(void);
void TIM2_IRQHandler(void);
void TIM3_IRQHandler(void);
void wimax_vsoc_tx_isr(void);
void wimax_vsoc_rx_isr(void);

#endif
#endif