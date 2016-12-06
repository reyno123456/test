
/* ----------------------------------------------------------------------
 * $Date:        2016.11.29
 * $Revision:    V0.01
 *
 * Project:      
 * Title:        canDef.h
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
  * @file    canDef.h
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



#ifndef  CAN_DEF_H
#define  CAN_DEF_H

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>


/*******************Macro define**************************/
#define BASE_ADDR_CAN0 (0x40300000)
#define BASE_ADDR_CAN1 (0x40340000)
#define BASE_ADDR_CAN2 (0x40380000)
#define BASE_ADDR_CAN3 (0x403C0000)

#define	CAN_RX_BUF_LEN (16)	//4 ch,total is (4 * CAN_RX_BUF_LEN)

#define	CAN_CH_0	(0)
#define	CAN_CH_1	(1)
#define	CAN_CH_2	(2)
#define	CAN_CH_3	(3)

#define	CAN_BR_125K	(0)
#define	CAN_BR_250K	(1)
#define	CAN_BR_500K	(2)
#define	CAN_BR_1M	(3)


#define	CAN_FORMAT_STD	(0)	//standard frame
#define	CAN_FORMAT_EXT	(1)	//extended frame

#define	CAN_TYPE_DATA	(0)	//data frame
#define	CAN_TYPE_RMT	(1)	//remote frame


//define register RTIE
#define CAN_RTIE_RIE	(1<<7)
#define CAN_RTIE_ROIE	(1<<6)
#define CAN_RTIE_RFIE	(1<<5)
#define CAN_RTIE_RAFIE	(1<<4)
#define CAN_RTIE_TPIE	(1<<3)
#define CAN_RTIE_TSIE	(1<<2)
#define CAN_RTIE_EIE	(1<<1)

//define register TCMD
#define CAN_TCMD_TBSEL	(1<<7)
#define CAN_TCMD_LOM	(1<<6)
#define CAN_TCMD_STBY	(1<<5)
#define CAN_TCMD_TPE	(1<<4)
#define CAN_TCMD_TPA	(1<<3)
#define CAN_TCMD_TSONE	(1<<2)
#define CAN_TCMD_TSALL	(1<<1)
#define CAN_TCMD_TSA	(1<<0)

//define register TBUF
#define CAN_TBUF_IDE	(1<<7)	//std or ext
#define CAN_TBUF_RTR	(1<<6)	//data or remote
#define CAN_TBUF_EDL	(1<<5)
#define CAN_TBUF_BRS	(1<<4)

#define	CAN_AMASK_ID10_0	(0x7FF)
#define	CAN_UNAMASK_ID10_0	(0x0)
#define	CAN_FRAME_LEN_AMASK	(0xF)

#define	CAN_AMASK_ID28_0	(0x1FFFFFFF)
#define	CAN_UNAMASK_ID28_0	(0x0)


/*******************can register define**************************/
typedef struct{
    volatile uint32_t    RBUF[18];           // 0x00-0x47
    volatile uint32_t    TBUF[18];           // 0x48-0x8f
    volatile uint32_t    REG3;               // 0x90 --> RCTRL     TCTRL    TCMD      CFG_STAT
    volatile uint32_t    REG4;               // 0x94 --> LIMIT     ERRINT   RTIF      RTIE
    volatile uint32_t    REG5;               // 0x98 -->  --       BITTIME2 BITTIME1  BITTIME0
    volatile uint32_t    REG6;               // 0x9c -->  --       TDC      F_PRESC   S_PRESC
    volatile uint32_t    REG7;               // 0xa0 -->  TECNT    RECNT    --        EALCAP
    volatile uint32_t    REG8;               // 0xa4 -->  ACF_EN1  ACF_EN0  --        ACFCTRL
    volatile uint32_t    REG9;               // 0xa8 -->  ACF3     ACF2     ACF1      ACF0
    volatile uint32_t    REG10;              // 0xac -->  --       --       VER1      VER0
    volatile uint32_t    REG11;              // 0xb0 -->  REF_MSG3 REF_MSG2 REF_MSG1  REF_MSG0
    volatile uint32_t    REG12;              // 0xb4 -->  TT_TRIG1 TT_TRIG0 TRIG_CFG1 TRIG_CFG0
    volatile uint32_t    REG13;              // 0xb8 -->  --       --       TT_WTRIG1 TT_WTRIG0
} CAN_Type;

#endif
