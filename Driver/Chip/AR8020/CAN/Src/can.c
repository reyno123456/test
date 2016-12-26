/*****************************************************************************
 * Copyright: 2016-2020, Artosyn. Co., Ltd.
 * File name: can.c
 * Description: can drive function implementation 
 * Author: Artosyn FW
 * Version: V0.01 
 * Date: 2016.11.29
 * History: 
 * 2016.11.29 the first edition.
 *     finished standard data frame,standard remote frame,
 *     extended data frame,extended remote frame receive and send.
 * *****************************************************************************/

#include "../Inc/canDef.h"
#include "../Inc/can.h"

/*******************global variable init**************************/
STRU_CAN_PAR g_st_canPar = {
    .u8_ch         = CAN_CH_0,         //u8_ch
    .u32_br        = CAN_BR_500K,      //u32_br
    .u32_acode     = 0x123,            //u32_acode
    .u32_amask     = 0x7FF,            //u32_amask 
    .u8_rtie       = 0x80,             //u8_rtie
    .u8_format     = CAN_FORMAT_STD    //u8_format
};

STRU_CAN_MSG g_st_canSendMsg = {
    .u32_id         = 0x111,        //u32_id
    .u8_dataArray   = {0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11}, //u8_dataArray
    .u8_len         = 8,            //u8_len
    .u8_ch          = CAN_CH_0,     //u8_ch
    .u8_format      = CAN_FORMAT_STD,   //u8_format
    .u8_type        = CAN_TYPE_DATA,    //u8_type
    .u8_isNewMsg    = 0     //u8_isNewMsg
};

STRU_CAN_MSG g_st_canRcvMsg = {
    .u32_id         = 0x0,      //u32_id
    .u8_dataArray   = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, //u8_dataArray
    .u8_len         = 0,        //u8_len
    .u8_ch          = CAN_CH_0, //u8_ch
    .u8_format      = CAN_FORMAT_STD,   //u8_format
    .u8_type        = CAN_TYPE_DATA,    //u8_type
    .u8_isNewMsg    = 0     //u8_isNewMsg
};

/*******************Function declaration**************************/

static STRU_CAN_TYPE * CAN_GetBaseAddrByCh(uint8_t u8_ch);

static int32_t CAN_InitHw(uint8_t u8_ch, 
                        uint32_t u32_br, 
                        uint32_t u32_acode, 
                        uint32_t u32_amask, 
                        uint8_t u8_rtie,
                        uint8_t u8_format);

static int32_t CAN_Send(uint8_t u8_ch, 
                        uint32_t u32_id, 
                        uint8_t u8_len, 
                        uint8_t *u32_txBuf, 
                        uint8_t u8_format, 
                        uint8_t u8_type);

static int32_t CAN_Rcv(uint8_t u8_ch, 
                       uint32_t *u32_id, 
                       uint8_t *u8_len, 
                       uint8_t *u8_rxBuf, 
                       uint8_t *u8_format, 
                       uint8_t *u8_type);

static int32_t CAN_SendArr(uint8_t u8_ch, uint8_t *u32_txBuf);

static int32_t CAN_RcvArr(uint8_t u8_ch, uint8_t *u8_rxBuf);

static void CAN_PrintRxMsg(void);

static void CAN0_Isr(uint32_t u32_vectorNum);

static void CAN1_Isr(uint32_t u32_vectorNum);

static void CAN2_Isr(uint32_t u32_vectorNum);

static void CAN3_Isr(uint32_t u32_vectorNum);

static void CAN_ConnectIsr(void);

static int32_t CAN_Start(uint8_t u8_ch, uint32_t u32_rxLen, uint32_t u32_txLen);

static int32_t CAN_Close(uint8_t u8_ch);

static int32_t CAN_IoctlSt(uint8_t u8_ch, 
                           STRU_CAN_PAR *pst_canPar, 
                           ENUM_CAN_PAR_NO e_no, 
                           uint32_t u32_parVal);



/*******************Function implementation**************************/

/**
* @brief    get can base addr  
* @param    u8_ch  can0~can3
* @retval   can base addr
* @note   
*/
static STRU_CAN_TYPE * CAN_GetBaseAddrByCh(uint8_t u8_ch)
{
    STRU_CAN_TYPE *pst_canReg;

    switch(u8_ch)
    {
        case CAN_CH_0:
	{
            pst_canReg = (STRU_CAN_TYPE *)BASE_ADDR_CAN0; 
            break;
	}
        case CAN_CH_1:
	{
            pst_canReg = (STRU_CAN_TYPE *)BASE_ADDR_CAN1; 
            break;
	}
        case CAN_CH_2:
	{
            pst_canReg = (STRU_CAN_TYPE *)BASE_ADDR_CAN2; 
            break;
	}
        case CAN_CH_3:
	{
            pst_canReg = (STRU_CAN_TYPE *)BASE_ADDR_CAN3; 
            break;
	}
        default:
	{
            //pst_canReg = (STRU_CAN_TYPE *)BASE_ADDR_CAN0;
            pst_canReg = (STRU_CAN_TYPE *)NULL;
            //dlog_error("u8_ch error!\n");
            break;
	}
    } 

    return pst_canReg;
} 

/**
* @brief    can init 
* @param    u8_ch  CAN_CH_0 ~ CAN_CH_3 
* @param    u32_br  125k,250k,500k,1M
* @param    u32_acode   bit10~bit0 <---> ID10~ID0 
* @param    u32_amask   bit10~bit0 <---> ID10~ID0
* @param    u8_rtie    bit7~bit1 <---> RIE,ROIE,RFIE,RAFIE,TPIE,TSIE,EIE 
* @param    u8_format  std or ext u8_format 
* @retval 
* @note   
*/
static int32_t CAN_InitHw(uint8_t u8_ch, 
                        uint32_t u32_br, 
                        uint32_t u32_acode, 
                        uint32_t u32_amask, 
                        uint8_t u8_rtie,
                        uint8_t u8_format)
{
    volatile STRU_CAN_TYPE *pst_canReg;
    unsigned int u32_tmpData;

    pst_canReg = CAN_GetBaseAddrByCh(u8_ch); 

    pst_canReg->u32_reg3 |= (1<<7); // CFG_STAT-->RESET=1

    //clear S_Seg_1,S_Seg_2,S_SJW
    u32_tmpData = (pst_canReg->u32_reg5) & (~(0x3F | (0x1F<<8) | (0xF<<16)));
    // set S_Seg_1=12,S_Seg_2=10,S_SJW=2, BT=((S_Seg_1+2)+(S_Seg_2+1))*TQ
    u32_tmpData |= ((0xC) | (0xA<<8) | (0x2<<16));     
    pst_canReg->u32_reg5 = u32_tmpData;

    pst_canReg->u32_reg6 &= (~0xFF);    //clear S_PRESC
    switch(u32_br)
    {
        case CAN_BR_125K:
	{
            pst_canReg->u32_reg6 |= 0x1F;          // S_PRESC = 31 
            break;
	}
        case CAN_BR_250K:
	{
            pst_canReg->u32_reg6 |= 0xF;          // S_PRESC = 15 
            break;
	}
        case CAN_BR_500K:
	{
            pst_canReg->u32_reg6 |= 0x7;          // S_PRESC = 7 
            break;
	}
        case CAN_BR_1M:
	{
            pst_canReg->u32_reg6 |= 0x3;          // S_PRESC = 3 
            break;
	}
        default:
	{
            pst_canReg->u32_reg6 |= 0x7;          // S_PRESC = 7 
            dlog_error("baud rate error,set default baud rate 500K.\n");
            break;
	}
    }
    // ACFCTRL-->SELMASK=0, register ACF_x point to acceptance code
    pst_canReg->u32_reg8 &= ~(1<<5); 
    if(CAN_FORMAT_STD == u8_format)
    {       
        pst_canReg->u32_reg9 &= ~(CAN_AMASK_ID10_0);
        pst_canReg->u32_reg9 |= (u32_acode & CAN_AMASK_ID10_0);
    }   
    else 
    {       
        pst_canReg->u32_reg9 &= ~(CAN_AMASK_ID28_0);
        pst_canReg->u32_reg9 |= (u32_acode & CAN_AMASK_ID28_0);
    }   
    // ACFCTRL-->SELMASK=1, register ACF_x point to acceptance mask
    pst_canReg->u32_reg8 |=(1<<5 | 0x1<<16); 
    if(CAN_FORMAT_STD == u8_format)
    {   
        //AIDE=0 acceptance filter accepts only standard frame
        pst_canReg->u32_reg9 &= ~(CAN_AMASK_ID10_0 | (1<<29));  
        //AIDEE=1 AIDE=0 acceptance filter accepts only standard frame
        pst_canReg->u32_reg9 |= ((u32_amask & CAN_AMASK_ID10_0)|(1<<30));
    }
    else
    {   
        pst_canReg->u32_reg9 &= ~(CAN_AMASK_ID28_0);    //clear ID28~ID0
        //AIDEE=1 AIDE=1 acceptance filter accepts only extended frame
        pst_canReg->u32_reg9 |= ((u32_amask & CAN_AMASK_ID28_0)|(3<<29));
    }
    
    pst_canReg->u32_reg4 &= ~(0xFE);//clear RTIE register
    pst_canReg->u32_reg4 |= (u8_rtie & 0xFE);
    
    pst_canReg->u32_reg3 &= ~(1<<7);    // CFG_STAT-->RESET=0
    
    return 0;
}

/**
* @brief    send can std data frame 
* @param    u8_ch: CAN_CH_0 ~ CAN_CH_3
* @param    u32_id: bit10~bit0 <---> ID10~ID0
* @param    u8_len:    data length
* @param    u32_txBuf:   send data buf
* @param    u8_format  std or ext frame
* @param    u8_type    data or remote frame
* @retval 
* @note   
*/
static int32_t CAN_Send(uint8_t u8_ch, 
                        uint32_t u32_id, 
                        uint8_t u8_len, 
                        uint8_t *u32_txBuf, 
                        uint8_t u8_format, 
                        uint8_t u8_type)
{
    uint8_t u8_i;
    uint8_t u8_dlc;
    volatile STRU_CAN_TYPE *pst_canReg;

    pst_canReg = CAN_GetBaseAddrByCh(u8_ch);

    //EDL=0 CAN2.0 frame
    pst_canReg->u32_txBuf[1] &= (~CAN_TBUF_EDL);
    
    if(CAN_FORMAT_STD == u8_format)// std
    {
        //IDE=0 standard u8_format
        pst_canReg->u32_txBuf[1] &= (~CAN_TBUF_IDE);
        
        // clear ID[10:0]
        pst_canReg->u32_txBuf[0] &= (~CAN_AMASK_ID10_0); 
        // set ID[10:0]
        pst_canReg->u32_txBuf[0] |= (u32_id & CAN_AMASK_ID10_0);
    }   
    else //ext
    {   
        //IDE=1 extended u8_format
        pst_canReg->u32_txBuf[1] |= (CAN_TBUF_IDE);
        
        // clear ID[28:0]
        pst_canReg->u32_txBuf[0] &= (~CAN_AMASK_ID28_0); 
        // set ID[28:0]
        pst_canReg->u32_txBuf[0] |= (u32_id & CAN_AMASK_ID28_0); 
    }
    
    // clear DLC, the number of payload bytes in a frame, valid max=8 for CAN2.0
    pst_canReg->u32_txBuf[1] &= (~CAN_FRAME_LEN_AMASK); 
    u8_dlc = (u8_len<=8) ? u8_len : 8;
    pst_canReg->u32_txBuf[1] |= u8_dlc;         
    
    if(CAN_TYPE_DATA == u8_type)//data frame
    {
        //RTR=0,data frame
        pst_canReg->u32_txBuf[1] &= (~CAN_TBUF_RTR);

        for(u8_i=0; u8_i<(u8_dlc+3)/4; u8_i++)
        {
            pst_canReg->u32_txBuf[2+u8_i] =*(uint32_t*)(u32_txBuf+u8_i*4);
        }
    }
    else//remote frame
    {
        //RTR=1,remote frame
        pst_canReg->u32_txBuf[1] |= (CAN_TBUF_RTR);
    }

    pst_canReg->u32_reg3 |= (1<<12);    // TCMD-->TPE = 1,Transmit Primary Enable

    return 0;
}

/**
* @brief    receive can frame 
* @param    u8_ch      CAN_CH_0 ~ CAN_CH_3
* @param    u32_id      bit10~bit0 <---> ID10~ID0
* @param    u8_len     data length
* @param    u8_rxBuf    receice buff
* @param    u8_format  std or ext frame
* @param    u8_type    data or remote frame
* @retval 
* @note   
*/
static int32_t CAN_Rcv(uint8_t u8_ch, 
                       uint32_t *u32_id, 
                       uint8_t *u8_len, 
                       uint8_t *u8_rxBuf, 
                       uint8_t *u8_format, 
                       uint8_t *u8_type)
{
    uint8_t u8_tmpLen;
    uint32_t u32_data;
    volatile STRU_CAN_TYPE *pst_canReg;

    pst_canReg = CAN_GetBaseAddrByCh(u8_ch); 
   
    //RSTAT(1:0) = 00b, receive buffer empty
    if(0 == ((pst_canReg->u32_reg3) & (3<<24)))
    {
        return -1;  //rx buffer is empty
    }
 
    //pst_canReg->u32_reg4 |= (0xf<<12);  // clear the receiver irq
    u32_data = pst_canReg->u32_rxBuf[1];

    if(0 == (u32_data & CAN_TBUF_IDE))//std frame
    {
        *u32_id = pst_canReg->u32_rxBuf[0] & CAN_AMASK_ID10_0;//u32_id
        *u8_format = CAN_FORMAT_STD;//u8_format
    }
    else//ext frame
    {
        *u32_id = pst_canReg->u32_rxBuf[0] & CAN_AMASK_ID28_0;//u32_id
        *u8_format = CAN_FORMAT_EXT;//u8_format
    }
    
    if(0 == (u32_data & CAN_TBUF_RTR))//data frame
    {
        *u8_type = CAN_TYPE_DATA;  
    }
    else//remote frame
    {
        *u8_type = CAN_TYPE_RMT;   
    }
    
    u8_tmpLen = u32_data & CAN_FRAME_LEN_AMASK;
    u8_tmpLen = (u8_tmpLen <= 8) ? u8_tmpLen : 8;
    
    //remote frame has no data
    if( (CAN_TYPE_RMT == (*u8_type)) || (0x00 == u8_tmpLen) )
    {
        *u8_len = 0;   
    }       
    else
    {
        *u8_len = u8_tmpLen;
        *(uint32_t *)(u8_rxBuf) = pst_canReg->u32_rxBuf[2];  //data1~data4
        *(uint32_t *)(u8_rxBuf + 4) = pst_canReg->u32_rxBuf[3];//data5~data8
    }


    pst_canReg->u32_reg3 |= (1<<28);       // RREL, release the RBUF

    return 0;
}

/**
* @brief    send std data frame
* @param    u8_ch: CAN_CH_0 ~ CAN_CH_3
* @param    u32_txBuf:   send data buffer
* @retval 
* @note   
*/
static int32_t CAN_SendArr(uint8_t u8_ch, uint8_t *u32_txBuf)
{
    volatile STRU_CAN_TYPE *pst_canReg;

    pst_canReg = CAN_GetBaseAddrByCh(u8_ch);

    pst_canReg->u32_txBuf[0] = *(uint32_t*)(u32_txBuf + 0);   //u32_id
    //EDL=0 CAN2.0 frame,BRS=0 nominal/slow bit rate    
    pst_canReg->u32_txBuf[1] = 0; 
    pst_canReg->u32_txBuf[1] |= *(uint32_t*)(u32_txBuf + 4);  //IDE RTR DLC 
    pst_canReg->u32_txBuf[2] = *(uint32_t*)(u32_txBuf + 8);
    pst_canReg->u32_txBuf[3] = *(uint32_t*)(u32_txBuf + 12);
    
    pst_canReg->u32_reg3 |= (1<<12);    // TCMD-->TPE = 1,Transmit Primary Enable

    return 0;
 
}

/**
* @brief    receive can frame 
* @param    u8_ch: CAN_CH_0 ~ CAN_CH_3
* @param    u8_rxBuf:   
* @retval 
* @note   
*/
static int32_t CAN_RcvArr(uint8_t u8_ch, uint8_t *u8_rxBuf)
{
    volatile STRU_CAN_TYPE *pst_canReg;

    pst_canReg = CAN_GetBaseAddrByCh(u8_ch);

    *(uint32_t*)(u8_rxBuf + 0) = pst_canReg->u32_rxBuf[0];   //ID
    *(uint32_t*)(u8_rxBuf + 4) = pst_canReg->u32_rxBuf[1];   //DLC
    *(uint32_t*)(u8_rxBuf + 8) = pst_canReg->u32_rxBuf[2];   //data1~data$
    *(uint32_t*)(u8_rxBuf + 12) = pst_canReg->u32_rxBuf[3];  //data5~data8

    pst_canReg->u32_reg3 |= (1<<28);       // RREL, release the RBUF
    
    return 0;
}

static void CAN_PrintRxMsg(void)
{
    if(CAN_FORMAT_STD == (g_st_canRcvMsg.u8_format))    
    {   
        dlog_info(" format:std");
    }   
    else if(CAN_FORMAT_EXT == (g_st_canRcvMsg.u8_format))   
    {   
        dlog_info(" format:ext");
    }   
    else    
    {   
        dlog_info(" format:error");
    }   
    
    if(CAN_TYPE_DATA == (g_st_canRcvMsg.u8_type))   
    {   
        dlog_info(" type:data ");
    }   
    else if(CAN_TYPE_RMT == (g_st_canRcvMsg.u8_type))   
    {   
        dlog_info(" type:rmt ");
    }   
    else    
    {   
        dlog_info(" type:error ");
    }
    
    dlog_info("id:%x len:%d  data1~4:0x%08x data5~8:0x%08x\n",
                    g_st_canRcvMsg.u32_id, 
                    g_st_canRcvMsg.u8_len,
                    *(uint32_t*)(&g_st_canRcvMsg.u8_dataArray[0]),
                    *(uint32_t*)(&g_st_canRcvMsg.u8_dataArray[4]));
}

/**
* @brief  
* @param  
* @retval 
* @note   
*/
static void CAN0_Isr(uint32_t u32_vectorNum)
{
    volatile STRU_CAN_TYPE *pst_canReg;
    uint32_t u32_id;
    int32_t s32_result;
    uint8_t u8_len;
    uint8_t u8_rbuf[8];
     pst_canReg = CAN_GetBaseAddrByCh(CAN_CH_0);
    
     //rx interrupt
    if(0x00 != ((pst_canReg->u32_reg4) & 0x8000))   
    {
        pst_canReg->u32_reg4 |= 0x8000; //clear flag
        
        CAN_RcvSt(CAN_CH_0, &g_st_canRcvMsg);
        if(1 == (g_st_canRcvMsg.u8_isNewMsg))
        {
            g_st_canRcvMsg.u8_isNewMsg = 0;
            CAN_PrintRxMsg();
        }
        else
        {
            dlog_info("!!!!!!!!!!error!!!!!!!!\n");
        }
    
    }
    //tx Primary or Secondary interrupt
    else if(0x00 != ((pst_canReg->u32_reg4) & 0x0C00))  
    {
        dlog_info("CAN0_Isr tx is ok!\n");  
        pst_canReg->u32_reg4 |= 0x0C00; //clear flag
    }
    else    //other
    {
    
    }
    
}

/**
* @brief  
* @param  
* @retval 
* @note   
*/
static void CAN1_Isr(uint32_t u32_vectorNum)
{
    volatile STRU_CAN_TYPE *pst_canReg;
    uint32_t u32_id;
    uint8_t u8_len;
    uint8_t u8_rbuf[8];
     pst_canReg = CAN_GetBaseAddrByCh(CAN_CH_1);
    
     //rx interrupt
    if(0x00 != ((pst_canReg->u32_reg4) & 0x8000))   
    {
        pst_canReg->u32_reg4 |= 0x8000; //clear flag
        
        CAN_RcvSt(CAN_CH_1, &g_st_canRcvMsg);
        
        if(1 == (g_st_canRcvMsg.u8_isNewMsg))
        {
            g_st_canRcvMsg.u8_isNewMsg = 0;
            CAN_PrintRxMsg();
        }
        else
        {
            dlog_info("!!!!!!!!!!error!!!!!!!!\n");
        }
    }
    //tx Primary or Secondary interrupt
    else if(0x00 != ((pst_canReg->u32_reg4) & 0x0C00))  
    {
        dlog_info("CAN1_Isr tx is ok!\n");  
        pst_canReg->u32_reg4 |= 0x0C00; //clear flag
    }
    else    //other
    {
    
    }
    
}

/**
* @brief  
* @param  
* @retval 
* @note   
*/
static void CAN2_Isr(uint32_t u32_vectorNum)
{
    volatile STRU_CAN_TYPE *pst_canReg;
    uint32_t u32_id;
    uint8_t u8_len;
    uint8_t u8_rbuf[8];
     pst_canReg = CAN_GetBaseAddrByCh(CAN_CH_2);
    
     //rx interrupt
    if(0x00 != ((pst_canReg->u32_reg4) & 0x8000))   
    {
        pst_canReg->u32_reg4 |= 0x8000; //clear flag
        
        CAN_RcvSt(CAN_CH_2, &g_st_canRcvMsg);
        
        if(1 == (g_st_canRcvMsg.u8_isNewMsg))
        {
            g_st_canRcvMsg.u8_isNewMsg = 0;
            CAN_PrintRxMsg();
        }
        else
        {
            dlog_info("!!!!!!!!!!error!!!!!!!!\n");
        }
    }
    //tx Primary or Secondary interrupt
    else if(0x00 != ((pst_canReg->u32_reg4) & 0x0C00))  
    {
        dlog_info("CAN2_Isr tx is ok!\n");  
        pst_canReg->u32_reg4 |= 0x0C00; //clear flag
    }
    else    //other
    {
    
    }
    
}/**
* @brief  
* @param  
* @retval 
* @note   
*/
static void CAN3_Isr(uint32_t u32_vectorNum)
{
    volatile STRU_CAN_TYPE *pst_canReg;
    uint32_t u32_id;
    uint8_t u8_len;
    uint8_t u8_rbuf[8];
     pst_canReg = CAN_GetBaseAddrByCh(CAN_CH_3);
    
     //rx interrupt
    if(0x00 != ((pst_canReg->u32_reg4) & 0x8000))   
    {
        pst_canReg->u32_reg4 |= 0x8000; //clear flag
        
        CAN_RcvSt(CAN_CH_3, &g_st_canRcvMsg);
        
        if(1 == (g_st_canRcvMsg.u8_isNewMsg))
        {
            g_st_canRcvMsg.u8_isNewMsg = 0;
            CAN_PrintRxMsg();
        }
        else
        {
            dlog_info("!!!!!!!!!!error!!!!!!!!\n");
        }
    }
    //tx Primary or Secondary interrupt
    else if(0x00 != ((pst_canReg->u32_reg4) & 0x0C00))  
    {
        dlog_info("CAN3_Isr tx is ok!\n");  
        pst_canReg->u32_reg4 |= 0x0C00; //clear flag
    }
    else    //other
    {
    
    }
    
}
/**
* @brief    connect can isr 
* @param  
* @retval 
* @note   
*/
static void CAN_ConnectIsr(void)
{
    /* register the irq handler */
    reg_IrqHandle(CAN_IRQ0_VECTOR_NUM, CAN0_Isr, NULL);
    INTR_NVIC_EnableIRQ(CAN_IRQ0_VECTOR_NUM);
    //INTR_NVIC_SetIRQPriority(CAN_IRQ0_VECTOR_NUM, 1);
    
    reg_IrqHandle(CAN_IRQ1_VECTOR_NUM, CAN1_Isr, NULL);
    INTR_NVIC_EnableIRQ(CAN_IRQ1_VECTOR_NUM);
    //INTR_NVIC_SetIRQPriority(CAN_IRQ1_VECTOR_NUM, 1);
    
    reg_IrqHandle(CAN_IRQ2_VECTOR_NUM, CAN2_Isr, NULL);
    INTR_NVIC_EnableIRQ(CAN_IRQ2_VECTOR_NUM);
    //INTR_NVIC_SetIRQPriority(CAN_IRQ2_VECTOR_NUM, 1);
    
    reg_IrqHandle(CAN_IRQ3_VECTOR_NUM, CAN3_Isr, NULL);
    INTR_NVIC_EnableIRQ(CAN_IRQ3_VECTOR_NUM);
    //INTR_NVIC_SetIRQPriority(CAN_IRQ3_VECTOR_NUM, 1);
}


/**
* @brief    can init 
* @param    u8_ch: CAN_CH_0 ~ CAN_CH_3
* @param    pst_canPar  can init paraments 
* @retval 
* @note   
*/
int32_t CAN_InitSt(uint8_t u8_ch, STRU_CAN_PAR *pst_canPar)
{
    int32_t s32_result;
    
    CAN_ConnectIsr();   //connect can isr
    
    s32_result = CAN_InitHw(u8_ch, 
                          pst_canPar->u32_br, 
                          pst_canPar->u32_acode, 
                          pst_canPar->u32_amask, 
                          pst_canPar->u8_rtie,
                          pst_canPar->u8_format);

    return s32_result;
}

/**
* @brief    can send msg 
* @param    u8_ch: CAN_CH_0 ~ CAN_CH_3
* @param    pst_canSendMsg  can msg 
* @retval 
* @note   
*/
int32_t CAN_SendSt(uint8_t u8_ch, STRU_CAN_MSG *pst_canSendMsg)
{
    uint32_t s32_result;

    //xSemaphoreTake(g_canMutex,0);

    s32_result = CAN_Send(u8_ch, 
                          pst_canSendMsg->u32_id, 
                          pst_canSendMsg->u8_len,
                          pst_canSendMsg->u8_dataArray,
                          pst_canSendMsg->u8_format,
                          pst_canSendMsg->u8_type);

    //xSemaphoreGive(g_canMutex);

    return s32_result;
}

/**
* @brief    can send msg, if received a new msg,
*       pst_canRcvMsg->u8_isNewMsg will set to 1 
* @param    u8_ch: CAN_CH_0 ~ CAN_CH_3
* @param    pst_canRcvMsg   can msg 
* @retval 
* @note   
*/
int32_t CAN_RcvSt(uint8_t u8_ch, STRU_CAN_MSG *pst_canRcvMsg)
{
    uint32_t s32_result;

    s32_result = CAN_Rcv(u8_ch, 
                         &(pst_canRcvMsg->u32_id), 
                         &(pst_canRcvMsg->u8_len), 
                         &(pst_canRcvMsg->u8_dataArray[0]), 
                         &(pst_canRcvMsg->u8_format), 
                         &(pst_canRcvMsg->u8_type));
    if(0 == s32_result)
    {
        //update new msg flag.
        pst_canRcvMsg->u8_isNewMsg = 1;
    }
    
    return s32_result;
}

/**
* @brief    create can rx buf,tx buf ,mutex 
* @param    u8_ch: CAN_CH_0 ~ CAN_CH_3
* @param    u32_rxLen  can receive buffer size
* @param    u32_txLen  can send buffer size
* @retval 
* @note   
*/
static int32_t CAN_Start(uint8_t u8_ch, uint32_t u32_rxLen, uint32_t u32_txLen)
{
    int32_t s32_result = 0;
    
    //create xMutex for can operation   
    /*g_canMutex = xSemaphoreCreateMutex(); 
    
    if(NULL == g_canMutex)
    {
        s32_result = -1;
    }*/

    //check buffer length
    if((u32_rxLen < sizeof(STRU_CAN_MSG)) || (u32_txLen < sizeof(STRU_CAN_MSG)))
    {
        s32_result = -1;
    }

    //create can receive buffer
    
    //create can send buffer

    return 0;
}

/**
* @brief    delete can rx buf,tx buf ,mutex 
* @param    u8_ch: CAN_CH_0 ~ CAN_CH_3
* @retval 
* @note   
*/
static int32_t CAN_Close(uint8_t u8_ch)
{
    //delete can receive buffer
    
    //delete can send buffer
}


/**
* @brief    set can parameter 
* @param    u8_ch:     CAN_CH_0 ~ CAN_CH_3
* @param    pst_canPar  can parameter struct 
* @param    no      parameter no 
* @param    par     parameter value
* @retval 
* @note   
*/
static int32_t CAN_IoctlSt(uint8_t u8_ch, 
                           STRU_CAN_PAR *pst_canPar, 
                           ENUM_CAN_PAR_NO e_no, 
                           uint32_t u32_parVal)
{
    
}
