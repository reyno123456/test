/* ----------------------------------------------------------------------
 * $Date:        2016.11.29
 * $Revision:    V0.01
 *
 * Project:      
 * Title:        can.c
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
  * @file    can.c
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


#include "../Inc/canDef.h"
#include "../Inc/can.h"

/*******************global variable init**************************/
CAN_Par g_stCanPar = {
	.ch 		= CAN_CH_0,	//ch
	.br 		= CAN_BR_500K,	//br
	.acode 		= 0x123,		//acode
	.amask 		= 0x7FF,		//amask	
	.rtie 		= 0x80,		//rtie
	.format 	= CAN_FORMAT_STD	//format
};

CAN_Msg g_stCanSendMsg = {
	.id 		= 0x111,		//id
	.data 		= {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11},	//data
	.len 		= 8,		//len
	.ch 		= CAN_CH_0,	//ch
	.format		= CAN_FORMAT_STD,	//format
	.type		= CAN_TYPE_DATA,	//type
	.isNewMsg	= false		//isNewMsg
};

CAN_Msg g_stCanReceiveMsg = {
	.id 		= 0x0,		//id
	.data 		= {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},	//data
	.len 		= 0,		//len
	.ch 		= CAN_CH_0,	//ch
	.format		= CAN_FORMAT_STD,	//format
	.type		= CAN_TYPE_DATA,	//type
	.isNewMsg	= false		//isNewMsg
};

//static SemaphoreHandle_t g_canMutex;



/*******************Function declaration**************************/
static CAN_Type * CAN_GetBaseAddrByCh(uint8_t ch);

static int32_t CAN_Init(uint8_t ch, 
		uint32_t br, 
		uint32_t acode, 
		uint32_t amask, 
		uint8_t rtie,
		uint8_t format);

static int32_t CAN_Send(uint8_t ch, 
				uint32_t id, 
				uint8_t len, 
				uint8_t *tbuf, 
				uint8_t format, 
				uint8_t type);

static int32_t CAN_Receive(uint8_t ch, 
					uint32_t *id, 
					uint8_t *len, 
					uint8_t *rbuf, 
					uint8_t *format, 
					uint8_t *type);

static int32_t CAN_SendArr(uint8_t ch, uint8_t *tbuf);

static int32_t CAN_ReceiveArr(uint8_t ch, uint8_t *rbuf);

static void CAN_ConnectIsr(void);

static void CAN_PrintRxMsg(void);

static void CAN0_Isr(void);

static void CAN1_Isr(void);

static void CAN2_Isr(void);

static void CAN3_Isr(void);

static CAN_Type * CAN_GetBaseAddrByCh(uint8_t ch);




/*******************Function implementation**************************/

/**
* @brief	get can base addr  
* @param  	ch	can0~can3
* @retval 	can base addr
* @note   
*/
static CAN_Type * CAN_GetBaseAddrByCh(uint8_t ch)
{
	CAN_Type *p_stCanReg;

	switch(ch)
	{
		case CAN_CH_0:
			p_stCanReg = (CAN_Type *)BASE_ADDR_CAN0; 
			break;
		case CAN_CH_1:
			p_stCanReg = (CAN_Type *)BASE_ADDR_CAN1; 
			break;
		case CAN_CH_2:
			p_stCanReg = (CAN_Type *)BASE_ADDR_CAN2; 
			break;
		case CAN_CH_3:
			p_stCanReg = (CAN_Type *)BASE_ADDR_CAN3; 
			break;
		default:
			//p_stCanReg = (CAN_Type *)BASE_ADDR_CAN0;
			p_stCanReg = (CAN_Type *)NULL;
			//dlog_error("ch error!\n");
			break;
	} 
	return p_stCanReg;
} 

/**
* @brief 	can init 
* @param 	ch	CAN_CH_0 ~ CAN_CH_3 
* @param 	br	125k,250k,500k,1M
* @param 	acode	bit10~bit0 <---> ID10~ID0 
* @param 	amask	bit10~bit0 <---> ID10~ID0
* @param 	rtie	bit7~bit1 <---> RIE,ROIE,RFIE,RAFIE,TPIE,TSIE,EIE 
* @param 	format	std or ext format 
* @retval 
* @note   
*/
static int32_t CAN_Init(uint8_t ch, 
		uint32_t br, 
		uint32_t acode, 
		uint32_t amask, 
		uint8_t rtie,
		uint8_t format)
{
	volatile CAN_Type *p_stCanReg;
	unsigned int u32_tmpdata;

	p_stCanReg = CAN_GetBaseAddrByCh(ch); 

	p_stCanReg->REG3 |= (1<<7); // CFG_STAT-->RESET=1

	//clear S_Seg_1,S_Seg_2,S_SJW
	u32_tmpdata = (p_stCanReg->REG5) & (~(0x3F | (0x1F<<8) | (0xF<<16)));
	// set S_Seg_1=12,S_Seg_2=10,S_SJW=2, BT=((S_Seg_1+2)+(S_Seg_2+1))*TQ
	u32_tmpdata |= ((0xC) | (0xA<<8) | (0x2<<16));     
	p_stCanReg->REG5 = u32_tmpdata;

	p_stCanReg->REG6 &= (~0xFF);	//clear S_PRESC
	switch(br)
	{
		case CAN_BR_125K:
        	p_stCanReg->REG6 |= 0x1F;          // S_PRESC = 31 
			break;
		case CAN_BR_250K:
        	p_stCanReg->REG6 |= 0xF;          // S_PRESC = 15 
			break;
		case CAN_BR_500K:
        	p_stCanReg->REG6 |= 0x7;          // S_PRESC = 7 
			break;
		case CAN_BR_1M:
        	p_stCanReg->REG6 |= 0x3;          // S_PRESC = 3 
			break;
		default:
        	p_stCanReg->REG6 |= 0x7;          // S_PRESC = 7 
			dlog_error("baud rate error,set default baud rate 500K.\n");
			break;
	}
	// ACFCTRL-->SELMASK=0, register ACF_x point to acceptance code
	p_stCanReg->REG8 &= ~(1<<5); 
	if(CAN_FORMAT_STD == format)
	{		
		p_stCanReg->REG9 &= ~(CAN_AMASK_ID10_0);
		p_stCanReg->REG9 |= (acode & CAN_AMASK_ID10_0);
	}	
	else 
	{		
		p_stCanReg->REG9 &= ~(CAN_AMASK_ID28_0);
		p_stCanReg->REG9 |= (acode & CAN_AMASK_ID28_0);
	}	
	// ACFCTRL-->SELMASK=1, register ACF_x point to acceptance mask
	p_stCanReg->REG8 |=(1<<5 | 0x1<<16); 
	if(CAN_FORMAT_STD == format)
	{	
		//AIDE=0 acceptance filter accepts only standard frame
		p_stCanReg->REG9 &= ~(CAN_AMASK_ID10_0 | (1<<29));	
		//AIDEE=1 AIDE=0 acceptance filter accepts only standard frame
		p_stCanReg->REG9 |= ((amask & CAN_AMASK_ID10_0)|(1<<30));
	}
	else
	{	
		p_stCanReg->REG9 &= ~(CAN_AMASK_ID28_0);	//clear ID28~ID0
		//AIDEE=1 AIDE=1 acceptance filter accepts only extended frame
		p_stCanReg->REG9 |= ((amask & CAN_AMASK_ID28_0)|(3<<29));
	}
	
	p_stCanReg->REG4 &= ~(0xFE);//clear RTIE register
	p_stCanReg->REG4 |= (rtie & 0xFE);
    
	p_stCanReg->REG3 &= ~(1<<7);	// CFG_STAT-->RESET=0
	
	return 0;
}

/**
* @brief	send can std data frame 
* @param	ch:	CAN_CH_0 ~ CAN_CH_3
* @param	id:	bit10~bit0 <---> ID10~ID0
* @param	len:	data length
* @param	tbuf:	send data buf
* @param	format	std or ext frame
* @param	type	data or remote frame
* @retval 
* @note   
*/
static int32_t CAN_Send(uint8_t ch, 
				uint32_t id, 
				uint8_t len, 
				uint8_t *tbuf, 
				uint8_t format, 
				uint8_t type)
{
	uint8_t u8_i;
	uint8_t u8_dlc;
	volatile CAN_Type *p_stCanReg;

	p_stCanReg = CAN_GetBaseAddrByCh(ch);

	//EDL=0 CAN2.0 frame
	p_stCanReg->TBUF[1] &= (~CAN_TBUF_EDL);
	
	if(CAN_FORMAT_STD == format)// std
	{
		//IDE=0 standard format
		p_stCanReg->TBUF[1] &= (~CAN_TBUF_IDE);
		
		// clear ID[10:0]
		p_stCanReg->TBUF[0] &= (~CAN_AMASK_ID10_0);	
		// set ID[10:0]
		p_stCanReg->TBUF[0] |= (id & CAN_AMASK_ID10_0);
	}	
	else //ext
	{	
		//IDE=1 extended format
		p_stCanReg->TBUF[1] |= (CAN_TBUF_IDE);
		
		// clear ID[28:0]
		p_stCanReg->TBUF[0] &= (~CAN_AMASK_ID28_0);	
		// set ID[28:0]
		p_stCanReg->TBUF[0] |= (id & CAN_AMASK_ID28_0);	
	}
	
	// clear DLC, the number of payload bytes in a frame, valid max=8 for CAN2.0
	p_stCanReg->TBUF[1] &= (~CAN_FRAME_LEN_AMASK); 
	u8_dlc = (len<=8) ? len : 8;
	p_stCanReg->TBUF[1] |= u8_dlc;         
	
	if(CAN_TYPE_DATA == type)//data frame
	{
		//RTR=0,data frame
		p_stCanReg->TBUF[1] &= (~CAN_TBUF_RTR);

		for(u8_i=0; u8_i<(u8_dlc+3)/4; u8_i++)
		{
			p_stCanReg->TBUF[2+u8_i] =*(uint32_t*)(tbuf+u8_i*4);
		}
	}
	else//remote frame
	{
		//RTR=1,remote frame
		p_stCanReg->TBUF[1] |= (CAN_TBUF_RTR);
	}

	p_stCanReg->REG3 |= (1<<12);    // TCMD-->TPE = 1,Transmit Primary Enable

	return 0;
}

/**
* @brief 	receive can frame 
* @param	ch		CAN_CH_0 ~ CAN_CH_3
* @param	id		bit10~bit0 <---> ID10~ID0
* @param	len		data length
* @param	rbuf	receice buff
* @param	format	std or ext frame
* @param	type	data or remote frame
* @retval 
* @note   
*/
static int32_t CAN_Receive(uint8_t ch, 
					uint32_t *id, 
					uint8_t *len, 
					uint8_t *rbuf, 
					uint8_t *format, 
					uint8_t *type)
{
	uint8_t u8_len;
	uint32_t u32_data;
	volatile CAN_Type *p_stCanReg;

	p_stCanReg = CAN_GetBaseAddrByCh(ch); 
   
	//RSTAT(1:0) = 00b, receive buffer empty
	if(0 == ((p_stCanReg->REG3) & (3<<24)))
	{
		return -1;	//rx buffer is empty
	}
 
	//p_stCanReg->REG4 |= (0xf<<12);  // clear the receiver irq
	u32_data = p_stCanReg->RBUF[1];

	if(0 == (u32_data & CAN_TBUF_IDE))//std frame
	{
		*id = p_stCanReg->RBUF[0] & CAN_AMASK_ID10_0;//id
		*format = CAN_FORMAT_STD;//format
	}
	else//ext frame
	{
		*id = p_stCanReg->RBUF[0] & CAN_AMASK_ID28_0;//id
		*format = CAN_FORMAT_EXT;//format
	}
	
	if(0 == (u32_data & CAN_TBUF_RTR))//data frame
	{
		*type = CAN_TYPE_DATA;	
	}
	else//remote frame
	{
		*type = CAN_TYPE_RMT;	
	}
	
	u8_len = u32_data & CAN_FRAME_LEN_AMASK;
	u8_len = (u8_len <= 8) ? u8_len : 8;
	
	//remote frame has no data
	if( (CAN_TYPE_RMT == (*type)) || (0x00 == u8_len) )
	{
		*len = 0;	
	}		
	else
	{
		*len = u8_len;
		*(uint32_t *)(rbuf) = p_stCanReg->RBUF[2];	//data1~data4
		*(uint32_t *)(rbuf + 4) = p_stCanReg->RBUF[3];//data5~data8
	}


	p_stCanReg->REG3 |= (1<<28);       // RREL, release the RBUF

	return 0;
}

/**
* @brief  	send std data frame
* @param	ch:	CAN_CH_0 ~ CAN_CH_3
* @param	tbuf:	send data buffer
* @retval 
* @note   
*/
static int32_t CAN_SendArr(uint8_t ch, uint8_t *tbuf)
{
	volatile CAN_Type *p_stCanReg;

	p_stCanReg = CAN_GetBaseAddrByCh(ch);

	p_stCanReg->TBUF[0] = *(uint32_t*)(tbuf + 0);	//id
	//EDL=0 CAN2.0 frame,BRS=0 nominal/slow bit rate	
	p_stCanReg->TBUF[1] = 0; 
	p_stCanReg->TBUF[1] |= *(uint32_t*)(tbuf + 4);	//IDE RTR DLC 
	p_stCanReg->TBUF[2] = *(uint32_t*)(tbuf + 8);
	p_stCanReg->TBUF[3] = *(uint32_t*)(tbuf + 12);
	
	p_stCanReg->REG3 |= (1<<12);    // TCMD-->TPE = 1,Transmit Primary Enable

	return 0;
 
}

/**
* @brief 	receive can frame 
* @param	ch:	CAN_CH_0 ~ CAN_CH_3
* @param	rbuf:	
* @retval 
* @note   
*/
static int32_t CAN_ReceiveArr(uint8_t ch, uint8_t *rbuf)
{
	volatile CAN_Type *p_stCanReg;

	p_stCanReg = CAN_GetBaseAddrByCh(ch);

	*(uint32_t*)(rbuf + 0) = p_stCanReg->RBUF[0];	//ID
	*(uint32_t*)(rbuf + 4) = p_stCanReg->RBUF[1];	//DLC
	*(uint32_t*)(rbuf + 8) = p_stCanReg->RBUF[2];	//data1~data$
	*(uint32_t*)(rbuf + 12) = p_stCanReg->RBUF[3];	//data5~data8

	p_stCanReg->REG3 |= (1<<28);       // RREL, release the RBUF
	
	return 0;
}

static void CAN_PrintRxMsg(void)
{
	if(CAN_FORMAT_STD == (g_stCanReceiveMsg.format))	
	{	
		dlog_info(" format:std");
	}	
	else if(CAN_FORMAT_EXT == (g_stCanReceiveMsg.format))	
	{	
		dlog_info(" format:ext");
	}	
	else	
	{	
		dlog_info(" format:error");
	}	
	
	if(CAN_TYPE_DATA == (g_stCanReceiveMsg.type))	
	{	
		dlog_info(" type:data ");
	}	
	else if(CAN_TYPE_RMT == (g_stCanReceiveMsg.type))	
	{	
		dlog_info(" type:rmt ");
	}	
	else	
	{	
		dlog_info(" type:error ");
	}
	
	dlog_info("id:%x len:%d  data1~4:0x%08x data5~8:0x%08x\n",
					g_stCanReceiveMsg.id, 
					g_stCanReceiveMsg.len,
					*(uint32_t*)(&g_stCanReceiveMsg.data[0]),
					*(uint32_t*)(&g_stCanReceiveMsg.data[4]));
}

/**
* @brief  
* @param  
* @retval 
* @note   
*/
static void CAN0_Isr(void)
{
	volatile CAN_Type *p_stCanReg;
	uint32_t u32_id;
	int32_t s32_result;
	uint8_t u8_len;
	uint8_t u8_rbuf[8];
	 p_stCanReg = CAN_GetBaseAddrByCh(CAN_CH_0);
	
	 //rx interrupt
	if(0x00 != ((p_stCanReg->REG4) & 0x8000))	
	{
		p_stCanReg->REG4 |= 0x8000;	//clear flag
		
		CAN_ReceiveSt(CAN_CH_0, &g_stCanReceiveMsg);
		if(true == (g_stCanReceiveMsg.isNewMsg))
		{
			g_stCanReceiveMsg.isNewMsg = false;
			CAN_PrintRxMsg();
		}
		else
		{
			dlog_info("!!!!!!!!!!error!!!!!!!!\n");
		}
	
	}
	//tx Primary or Secondary interrupt
	else if(0x00 != ((p_stCanReg->REG4) & 0x0C00))	
	{
		dlog_info("CAN0_Isr tx is ok!\n");	
		p_stCanReg->REG4 |= 0x0C00;	//clear flag
	}
	else	//other
	{
	
	}
	
}

/**
* @brief  
* @param  
* @retval 
* @note   
*/
static void CAN1_Isr(void)
{
	volatile CAN_Type *p_stCanReg;
	uint32_t u32_id;
	uint8_t u8_len;
	uint8_t u8_rbuf[8];
	 p_stCanReg = CAN_GetBaseAddrByCh(CAN_CH_1);
	
	 //rx interrupt
	if(0x00 != ((p_stCanReg->REG4) & 0x8000))	
	{
		p_stCanReg->REG4 |= 0x8000;	//clear flag
		
		CAN_ReceiveSt(CAN_CH_1, &g_stCanReceiveMsg);
		
		if(true == (g_stCanReceiveMsg.isNewMsg))
		{
			g_stCanReceiveMsg.isNewMsg = false;
			CAN_PrintRxMsg();
		}
		else
		{
			dlog_info("!!!!!!!!!!error!!!!!!!!\n");
		}
	}
	//tx Primary or Secondary interrupt
	else if(0x00 != ((p_stCanReg->REG4) & 0x0C00))	
	{
		dlog_info("CAN1_Isr tx is ok!\n");	
		p_stCanReg->REG4 |= 0x0C00;	//clear flag
	}
	else	//other
	{
	
	}
	
}

/**
* @brief  
* @param  
* @retval 
* @note   
*/
static void CAN2_Isr(void)
{
	volatile CAN_Type *p_stCanReg;
	uint32_t u32_id;
	uint8_t u8_len;
	uint8_t u8_rbuf[8];
	 p_stCanReg = CAN_GetBaseAddrByCh(CAN_CH_2);
	
	 //rx interrupt
	if(0x00 != ((p_stCanReg->REG4) & 0x8000))	
	{
		p_stCanReg->REG4 |= 0x8000;	//clear flag
		
		CAN_ReceiveSt(CAN_CH_2, &g_stCanReceiveMsg);
		
		if(true == (g_stCanReceiveMsg.isNewMsg))
		{
			g_stCanReceiveMsg.isNewMsg = false;
			CAN_PrintRxMsg();
		}
		else
		{
			dlog_info("!!!!!!!!!!error!!!!!!!!\n");
		}
	}
	//tx Primary or Secondary interrupt
	else if(0x00 != ((p_stCanReg->REG4) & 0x0C00))	
	{
		dlog_info("CAN2_Isr tx is ok!\n");	
		p_stCanReg->REG4 |= 0x0C00;	//clear flag
	}
	else	//other
	{
	
	}
	
}/**
* @brief  
* @param  
* @retval 
* @note   
*/
static void CAN3_Isr(void)
{
	volatile CAN_Type *p_stCanReg;
	uint32_t u32_id;
	uint8_t u8_len;
	uint8_t u8_rbuf[8];
	 p_stCanReg = CAN_GetBaseAddrByCh(CAN_CH_3);
	
	 //rx interrupt
	if(0x00 != ((p_stCanReg->REG4) & 0x8000))	
	{
		p_stCanReg->REG4 |= 0x8000;	//clear flag
		
		CAN_ReceiveSt(CAN_CH_3, &g_stCanReceiveMsg);
		
		if(true == (g_stCanReceiveMsg.isNewMsg))
		{
			g_stCanReceiveMsg.isNewMsg = false;
			CAN_PrintRxMsg();
		}
		else
		{
			dlog_info("!!!!!!!!!!error!!!!!!!!\n");
		}
	}
	//tx Primary or Secondary interrupt
	else if(0x00 != ((p_stCanReg->REG4) & 0x0C00))	
	{
		dlog_info("CAN3_Isr tx is ok!\n");	
		p_stCanReg->REG4 |= 0x0C00;	//clear flag
	}
	else	//other
	{
	
	}
	
}
/**
* @brief	connect can isr 
* @param  
* @retval 
* @note   
*/
static void CAN_ConnectIsr(void)
{
    /* register the irq handler */
	reg_IrqHandle(CAN_IRQ0_VECTOR_NUM, CAN0_Isr);
    INTR_NVIC_EnableIRQ(CAN_IRQ0_VECTOR_NUM);
    //INTR_NVIC_SetIRQPriority(CAN_IRQ0_VECTOR_NUM, 1);
    
	reg_IrqHandle(CAN_IRQ1_VECTOR_NUM, CAN1_Isr);
    INTR_NVIC_EnableIRQ(CAN_IRQ1_VECTOR_NUM);
    //INTR_NVIC_SetIRQPriority(CAN_IRQ1_VECTOR_NUM, 1);
    
	reg_IrqHandle(CAN_IRQ2_VECTOR_NUM, CAN2_Isr);
    INTR_NVIC_EnableIRQ(CAN_IRQ2_VECTOR_NUM);
    //INTR_NVIC_SetIRQPriority(CAN_IRQ2_VECTOR_NUM, 1);
    
	reg_IrqHandle(CAN_IRQ3_VECTOR_NUM, CAN3_Isr);
    INTR_NVIC_EnableIRQ(CAN_IRQ3_VECTOR_NUM);
    //INTR_NVIC_SetIRQPriority(CAN_IRQ3_VECTOR_NUM, 1);
}


/**
* @brief 	can init 
* @param	ch:	CAN_CH_0 ~ CAN_CH_3
* @param 	p_stCanPar	can init paraments 
* @retval 
* @note   
*/
int32_t CAN_InitSt(uint8_t ch, CAN_Par *p_stCanPar)
{
	int32_t s32_result;
	
	CAN_ConnectIsr();	//connect can isr
	
	s32_result = CAN_Init(ch, 
			p_stCanPar->br, 
			p_stCanPar->acode, 
			p_stCanPar->amask, 
			p_stCanPar->rtie,
			p_stCanPar->format);

	return s32_result;
}

/**
* @brief 	can send msg 
* @param	ch:	CAN_CH_0 ~ CAN_CH_3
* @param 	p_stCanSendMsg	can msg 
* @retval 
* @note   
*/
int32_t CAN_SendSt(uint8_t ch, CAN_Msg *p_stCanSendMsg)
{
	uint32_t s32_result;

	//xSemaphoreTake(g_canMutex,0);

	s32_result = CAN_Send(ch, 
					p_stCanSendMsg->id, 
					p_stCanSendMsg->len,
					p_stCanSendMsg->data,
					p_stCanSendMsg->format,
					p_stCanSendMsg->type);

	//xSemaphoreGive(g_canMutex);

	return s32_result;
}

/**
* @brief 	can send msg, if received a new msg,
* 		p_stCanReceiveMsg->isNewMsg will set to true 
* @param	ch:	CAN_CH_0 ~ CAN_CH_3
* @param 	p_stCanReceiveMsg	can msg 
* @retval 
* @note   
*/
int32_t CAN_ReceiveSt(uint8_t ch, CAN_Msg *p_stCanReceiveMsg)
{
	uint32_t s32_result;

	s32_result = CAN_Receive(ch, 
							&(p_stCanReceiveMsg->id), 
							&(p_stCanReceiveMsg->len), 
							&(p_stCanReceiveMsg->data[0]), 
							&(p_stCanReceiveMsg->format), 
							&(p_stCanReceiveMsg->type));
	if(0 == s32_result)
	{
		//update new msg flag.
		p_stCanReceiveMsg->isNewMsg = true;
	}
	
	return s32_result;
}

/**
* @brief 	create can rx buf,tx buf ,mutex 
* @param	ch:	CAN_CH_0 ~ CAN_CH_3
* @param	rx_len	can receive buffer size
* @param	tx_len	can send buffer size
* @retval 
* @note   
*/
int32_t CAN_Start(uint8_t ch, uint32_t rx_len, uint32_t tx_len)
{
	int32_t s32_result = 0;
	
	//create xMutex for can operation	
	/*g_canMutex = xSemaphoreCreateMutex(); 
	
	if(NULL == g_canMutex)
	{
		s32_result = -1;
	}*/

	//check buffer length
	if((rx_len < sizeof(CAN_Msg)) || (tx_len < sizeof(CAN_Msg)))
	{
		s32_result = -1;
	}

	//create can receive buffer
	
	//create can send buffer

	return 0;
}

/**
* @brief 	delete can rx buf,tx buf ,mutex 
* @param	ch:	CAN_CH_0 ~ CAN_CH_3
* @retval 
* @note   
*/
int32_t CAN_Close(uint8_t ch)
{
	//delete can receive buffer
	
	//delete can send buffer
}


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
					uint32_t par_val)
{
	
}
