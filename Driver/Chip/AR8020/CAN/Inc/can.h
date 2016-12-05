/* ----------------------------------------------------------------------
 * $Date:        2016.11.29
 * $Revision:    V0.01
 *
 * Project:      
 * Title:        can.h
 *
 * Version 0.01
 *       
 *  
 *----------------------------------------------------------------------------
 *
 * 
 *---------------------------------------------------------------------------*/

 /**
  ******************************************************************************
  * @file    can.h
  * @author  
  * @date    
  * @brief   
  ******************************************************************************
  * @attention
  *
  *
  *
  ******************************************************************************
  */ 



#ifndef  CAN_H
#define  CAN_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#include "debuglog.h"
#include "can.h"
#include "interrupt.h"





/*******************enum data type**************************/
/*typedef enum{
	CAN_TfWm_Ptb,		//just use PTB
	CAN_TfWm_Fifo,		//STB work in fifo mode
	CAN_TfWm_pri		//STB work int priority mode
}CAN_TfWm;*/

typedef enum{
	CAN_Par_br,	//baud rate
	CAN_Par_acode,	//acceptance code
	CAN_Par_amsdk,	//acceptance mask
	CAN_Par_rtie,	//receive and transmit interrupt
	//......	//......
}CAN_ParNo;



/*******************can init parament define**************************/
typedef struct{
	uint8_t		ch;		//CAN_CH_0 ~ CAN_CH_3
	uint32_t	br; 	//125k,250k,500k,1M
	uint32_t	acode;	//bit10~bit0 <---> ID10~ID0
	uint32_t	amask;	//bit10~bit0 <---> ID10~ID0	
	uint8_t		rtie;	//bit7~bit1 <---> RIE,ROIE,RFIE,RAFIE,TPIE,TSIE,EIE
	uint8_t		format;	//standard of extended
	//CAN_TfWm	tf_wm;	//TBUF work mode	
} CAN_Par;



/*******************can msg define**************************/
typedef struct{
	uint32_t	id;			//message identtifier
	uint8_t		data[8];	//message data
	uint8_t		len;		//number of data in message
	uint8_t		ch;			//can channel
	uint8_t		format;		//standard or extended
	uint8_t		type;		//data or remote frame
	bool		isNewMsg;	//true:msg is new,false:msg is old	
	//uint8_t		txPri;	//tx priority,place at PTB or STB
} CAN_Msg;



/*******************Function declaration**************************/

/**
* @brief 	create can rx buf,tx buf ,mutex 
* @param	ch:	CAN_CH_0 ~ CAN_CH_3
* @param	rx_len	can receive buffer size
* @param	tx_len	can send buffer size
* @retval 
* @note   
*/
int32_t CAN_Start(uint8_t ch, uint32_t rx_len, uint32_t tx_len);

/**
* @brief 	can init 
* @param	ch:	CAN_CH_0 ~ CAN_CH_3
* @param 	p_stCanPar	can init paraments 
* @retval 
* @note   
*/
int32_t CAN_InitSt(uint8_t ch, CAN_Par *p_stCanPar);

/**
* @brief 	set can parameter 
* @param	ch:		CAN_CH_0 ~ CAN_CH_3
* @param 	p_stCanPar	can parameter struct 
* @param 	no		parameter no 
* @param 	par		parameter value
* @retval 
* @note   
*/
int32_t CAN_IoctlSt(uint8_t ch, 
					CAN_Par *p_stCanPar, 
					CAN_ParNo no, 
					uint32_t par_val);

/**
* @brief 	can send msg 
* @param	ch:	CAN_CH_0 ~ CAN_CH_3
* @param 	p_stCanSendMsg	can msg 
* @retval 
* @note   
*/
int32_t CAN_SendSt(uint8_t ch, CAN_Msg *p_stCanSendMsg);

/**
* @brief 	can send msg 
* @param	ch:	CAN_CH_0 ~ CAN_CH_3
* @param 	p_stCanReceiveMsg	can msg 
* @retval 
* @note   
*/
int32_t CAN_ReceiveSt(uint8_t ch, CAN_Msg *p_stCanReceiveMsg);


/**
* @brief 	delete can rx buf,tx buf ,mutex 
* @param	ch:	CAN_CH_0 ~ CAN_CH_3
* @retval 
* @note   
*/
int32_t CAN_Close(uint8_t ch);



/*******************global variable**************************/
extern CAN_Par g_stCanPar;
extern CAN_Msg g_stCanSendMsg;
extern CAN_Msg g_stCanReceiveMsg;



#endif
