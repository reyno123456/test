/**
  ******************************************************************************
  * @file    ground_controller.h
  * @author  Artosyn AE/FAE Team
  * @version V1.0
  * @date    03-21-2016
  * @brief
  *
  *
  ******************************************************************************
  */
#ifndef __GRD_CONTROLLER_H
#define __GRD_CONTROLLER_H

#include "config_functions_sel.h"
#include <stdint.h>
#include "debuglog.h"


#ifdef BASEBAND_GRD
/**
  * @brief Ground Controller Structure definition
  */

typedef struct
{
   uint8_t ITManualmode;      /*!< Specifies the flag of image transmission working in manual.
                                 This parameter can be a value of @ref <enable,disable>  */
   uint8_t ITAutomode;        /*!< Specifies the flag of image transmission working in auto.
                                 This parameter can be a value of @ref <enable,disable>  */
   uint8_t FEClock;           /*!< Specifies the flag of image transmission FEC lock.
                                 This parameter can be a value of @ref <enable,disable>  */
   uint8_t GetITfrq;          /*!< Specifies the flag of getting image transmission frq from sweeping results.
                                 This parameter can be a value of @ref <enable,disable>  */
   uint8_t RegetITfrq;        /*!< Specifies the flag of getting image transmission frq from sweeping results again .
                                 This parameter can be a value of @ref <enable,disable>  */
   uint8_t Endsweep;          /*!< Specifies the flag of sweeping  is end .
                                 This parameter can be a value of @ref <enable,disable>  */
   uint8_t GetfrqOnly;        /*!< Specifies the flag of get image frq after sweeping only in manual mode.
                                 This parameter can be a value of @ref <enable,disable>  */
   uint8_t Allowjglock;       /*!< Specifies the flag of permitting to judge image tr lock.
                                 This parameter can be a value of @ref <enable,disable>  */
   uint8_t Ldpcjgflag;       /*!< Specifies the flag of satisfy thre of ldpc err num.
                                 This parameter can be a value of @ref <enable,disable>  */
} Grd_FlagTypeDef;

typedef struct
{
   uint8_t RCChannel;         /*!< Specifies the remote controller working in freq channel.
                                    This parameter can be a value of @ref <RC_FREQ_MODE>   */
   uint8_t ITManualChannel;   /*!< Specifies the image transmission in manual at frq.
                                    This parameter can be a value of @ref <IM_FREQ_MODE>  */
   uint8_t ITAutoChannel;     /*!< Specifies the image transmission in auto at frq.
                                 This parameter can be a value of @ref <IM_FREQ_MODE>  */
   uint8_t ITTxChannel;       /*!< Specifies the ground tx the image transmission frq to the sky.
                                 This parameter can be a value of @ref <IM_FREQ_MODE>  */
   uint8_t ITTxCnt;           /*!< Specifies the ground tx the image transmission frq counts.
                                 This parameter can be a value of @ref <1,2>  */
   uint8_t Sweepfrqunlock;    /*!< Specifies the sweeping frq before FEC lock .
                                 This parameter can be a value of @ref <IM_FREQ_MODE>  */
   uint8_t Sweepfrqlock;      /*!< Specifies the sweeping frq after FEC lock .
                                 This parameter can be a value of @ref <IM_FREQ_MODE>  */
   uint8_t Harqcnt;           /*!< Specifies the cnts of retransmissions.
                                 This parameter can be a value of @ref <Reg[0xEA]>  */
   uint8_t ReHarqcnt;         /*!< Specifies the cnts of retransmissions continuously.
                                 This parameter can be a value of @ref <Reg[0xEA]>  */
   uint8_t SweepCyccnt;       /*!< Specifies the cnts of sweeping cycle.
                                 This parameter can be a value of @ref <1,2,3...>  */
   uint32_t ITunlkcnt;         /*!< Specifies the cnts of FEC is unlock.
                                 This parameter can be a value of @ref <Reg[0xEB]>  */
   uint8_t CgITfrqspan;       /*!< Specifies the time span of change image transmission frq.
                                 This parameter can be a value of @ref <1,2,3...>  */
   uint8_t Ldpcreadcnt;       /*!< Specifies the cnt of statistics Ldpc err num .
                                 This parameter can be a value of @ref <Reg[E7][E8]>  */


}Grd_HandleTypeDef;

typedef struct
{
   uint8_t Upqpsk12;          /*!< Specifies the flag of changing QAM mode to qpsk 1/2.
                                 This parameter can be a value of @ref <enable,disable>  */
   uint8_t Up16qam12;         /*!< Specifies the flag of changing QAM mode to 16QAM 1/2.
                                 This parameter can be a value of @ref <enable,disable>  */
   uint8_t Up64qam12;         /*!< Specifies the flag of changing QAM mode to 64QAM 1/2.
                                 This parameter can be a value of @ref <enable,disable>  */

   uint8_t Down16qam12;       /*!< Specifies the flag of changing QAM mode to 16QAM 1/2.
                                 This parameter can be a value of @ref <enable,disable>  */
   uint8_t Downqpsk12;        /*!< Specifies the flag of changing QAM mode to qpsk 1/2.
                                 This parameter can be a value of @ref <enable,disable>  */
   uint8_t Downbpsk12;        /*!< Specifies the flag of changing QAM mode to qpsk 1/2.
                                 This parameter can be a value of @ref <enable,disable>  */

} Grd_QAMTypeDef;


//*************************  Data Structure Define  **********************


// stored sweeping power in baseband.
struct SWEEP_POWER
{
  uint8_t   frqchannel;  /*!< Specifies the frq channel of image transmissions.*/

  uint32_t  powerall1;   /*!< Specifies the power total of working frq channel.*/
  uint32_t  powerall2;   /*!< Specifies the power total of working frq channel.*/
  uint32_t  powerall3;   /*!< Specifies the power total of working frq channel.*/
  uint32_t  powerall4;   /*!< Specifies the power total of working frq channel.*/

  uint32_t  poweravr;   /*!< Specifies the power average of working frq channel.*/

  uint16_t  powerwave;  /*!< Specifies the power wave of working frq channel.*/

};

//The frequency sequency in method of sweeping power .
struct FRQ_SEQ_ORDER
{
  uint8_t   frqchannel; /*!< Specifies the working frq channel.*/
  uint32_t  poweravr;   /*!< Specifies the power average of working frq channel.*/
  uint32_t  powerwave;  /*!< Specifies the power wave of working frq channel.*/
};

//Data structure of working at current / alternative / others.
struct CURRENT_ALTER_OTHERS_FRQ
{
  uint8_t   frqchannel; /*!< Specifies the working frq channel.*/
  uint32_t  poweravr;   /*!< Specifies the power average of working frq channel.*/
  uint32_t  powerwave;  /*!< Specifies the power wave of working frq channel.*/
};

struct SNR_PERCYC
{
  uint8_t  num;        /*!< Specifies the cnt of reading snr values in baseband.*/
  uint32_t snrhgh;     /*!< Specifies the high 7 bits of snr values in baseband.*/
  uint32_t snrlow;     /*!< Specifies the low 7 bits of snr values in baseband.*/
  uint32_t snrall;     /*!< Specifies the total of snr values in baseband.*/
};

void Grd_Parm_Initial(void);
void Grd_Write_Rcfrq(uint8_t i);
void Grd_Write_Itworkfrq(uint8_t i);
void Grd_Write_Itsweepfrq(uint8_t i);
void Grd_Id_Initial(void);

// Read Reg[EB] of baseband , lock or not.
uint8_t Grd_Baseband_Fec_Lock(void);
void Grd_Sweeping_Energy_Statistic(uint8_t i);
void Grd_Itfrq_Sort(uint8_t num);   // sort  SP[]-->FSO[]
void Grd_Alterfrq_Updute(uint8_t Itfrqchannel);
void Grd_Sweeping_Before_Fec_Locked(void);
void Grd_Sweeping_After_Fec_Locked(void);

void Grd_Get_Itfrq(uint8_t iflag);
void Grd_Fecunlock_Getfrq(void);
void Grd_Txmsg_Frq_Change(uint8_t i);
void Grd_Getsnr(uint8_t i);  //get SNR value at present

void Grd_Frqsnr_Array(void);
void Grd_Qamsnr_Array(void);
void Grd_Frq_Snrblock_Determine(uint16_t iMCS);
uint8_t Grd_Sweeppower_Fluctuate_Average(void);     //扫频的波动性和平均值
void Grd_Ldpc_Err_Num_Statistics(void);             //2 // 2 sec
uint8_t Grd_Ldpc_Block_Determine(void);
void Grd_Itfrq_Hopping(void);
void Grd_Working_Qam_Change(void);
void Grd_Txmsg_Qam_Change(void);
void Grd_Qamflag_Clear(void);
void Grd_IT_Controller(void);
void Grd_Osdmsg_Ptf(void);

void TIM0_IRQHandler(void);
void TIM1_IRQHandler(void);

#endif
#endif
