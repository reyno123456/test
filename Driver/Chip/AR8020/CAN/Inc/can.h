/*****************************************************************************
 * Copyright: 2016-2020, Artosyn. Co., Ltd.
 * File name: can.h
 * Description: can drive function declaration
 * Author: Artosyn FW
 * Version: V0.01 
 * Date: 2016.11.29
 * History: 
 * 2016.11.29 the first edition
 * *****************************************************************************/

#ifndef  __CAN_H__
#define  __CAN_H__



/**************************struct define****************************************/
//can init parament define
typedef struct{
        uint8_t         u8_ch;          //CAN_CH_0 ~ CAN_CH_3
        uint32_t        u32_br;         //125k,250k,500k,1M
        uint32_t        u32_acode;      //bit10~bit0 <---> ID10~ID0
        uint32_t        u32_amask;      //bit10~bit0 <---> ID10~ID0     
        uint8_t         u8_rtie;        //bit7~bit1 <---> RIE,ROIE,RFIE,RAFIE,TPIE,TSIE,EIE
        uint8_t         u8_format;      //standard of extended
} STRU_CAN_PAR;

//can msg define
typedef struct{
        uint32_t        u32_id;             //message identtifier
        uint8_t         u8_dataArray[8];    //message data
        uint8_t         u8_len;             //number of data in message
        uint8_t         u8_ch;              //can channel
        uint8_t         u8_format;          //standard or extended
        uint8_t         u8_type;            //data or remote frame
        uint8_t         u8_isNewMsg;        //1:msg is new,0:msg is old      
} STRU_CAN_MSG;


/**************************global variable declaration************************/
extern STRU_CAN_PAR g_st_canPar;
extern STRU_CAN_MSG g_st_canSendMsg;
extern STRU_CAN_MSG g_st_canRcvMsg;


/****************************Function declaration*****************************/
/**
* @brief    can init 
* @param    u8_ch: CAN_CH_0 ~ CAN_CH_3
* @param    pst_canPar  can init paraments 
* @retval 
* @note   
*/
int32_t CAN_InitSt(uint8_t u8_ch, STRU_CAN_PAR *pst_canPar);

/**
* @brief    can send msg 
* @param    u8_ch: CAN_CH_0 ~ CAN_CH_3
* @param    pst_canSendMsg  can msg 
* @retval 
* @note   
*/
int32_t CAN_SendSt(uint8_t u8_ch, STRU_CAN_MSG *pst_canSendMsg);

/**
* @brief    can send msg, if received a new msg,
*       pst_canRcvMsg->u8_isNewMsg will set to 1 
* @param    u8_ch: CAN_CH_0 ~ CAN_CH_3
* @param    pst_canRcvMsg   can msg 
* @retval 
* @note   
*/
int32_t CAN_RcvSt(uint8_t u8_ch, STRU_CAN_MSG *pst_canRcvMsg);


#endif
