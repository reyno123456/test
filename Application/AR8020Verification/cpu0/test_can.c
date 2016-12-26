#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "debuglog.h"
#include "can.h"
#include "test_can.h"
#include "interrupt.h"

extern unsigned long strtoul(const char *cp, char **endp, unsigned int base);

void command_TestCanInit(unsigned char *ch, unsigned char *br, unsigned char *acode,unsigned char *amask, unsigned char *rtie, unsigned char *format)
{
	unsigned int u32_ch = strtoul(ch, NULL, 0);

	g_st_canPar.u32_br = strtoul(br, NULL, 0);
	g_st_canPar.u32_acode = strtoul(acode, NULL, 0);
	g_st_canPar.u32_amask = strtoul(amask, NULL, 0);
	g_st_canPar.u8_rtie = strtoul(rtie, NULL, 0);
	g_st_canPar.u8_format = strtoul(format, NULL, 0);

	//CAN_ConnectIsr();//CAN isr

	CAN_InitSt((unsigned char)u32_ch, &g_st_canPar);
	
	dlog_info("g_st_canPar: ch br acode amask rtie format:%d %d 0x%x 0x%x 0x%x 0x%x\n",
						u32_ch, 
						g_st_canPar.u32_br, 
						g_st_canPar.u32_acode, 
						g_st_canPar.u32_amask, 
						g_st_canPar.u8_rtie,
						g_st_canPar.u8_format);
}

void command_TestCanTx(unsigned char *ch, unsigned char *id, unsigned char *len, unsigned char *tbuf,unsigned char *format, unsigned char *type)
{
	//unsigned int u32_data[] = {0x789,0x08,0x11223344,0x55667788};
	unsigned int u32_data1;
	unsigned char u8_unframe[30];
	unsigned char u8_ch = strtoul(ch, NULL, 0);
	g_st_canSendMsg.u32_id = strtoul(id, NULL, 0);//id
	g_st_canSendMsg.u8_len = strtoul(len, NULL, 0);//len
	g_st_canSendMsg.u8_ch = u8_ch;//ch	
	g_st_canSendMsg.u8_format = strtoul(format, NULL, 0);//format
	g_st_canSendMsg.u8_type = strtoul(type,  NULL, 0);//type

	memcpy(u8_unframe,tbuf,10);
	u32_data1 = strtoul(u8_unframe, NULL, 0);
	*(unsigned int*)(&(g_st_canSendMsg.u8_dataArray[0])) = (((u32_data1<<24)&0xFF000000) | 
							       ((u32_data1<<8)&0x00FF0000) | 
							       ((u32_data1>>8)&0x0000FF00) | 
							       ((u32_data1>>24)&0x000000FF));
	
	memcpy(u8_unframe+2,tbuf+10,8);
	u32_data1 = strtoul(u8_unframe, NULL, 0);
	*(unsigned int*)(&(g_st_canSendMsg.u8_dataArray[4])) = (((u32_data1<<24)&0xFF000000) | 
							       ((u32_data1<<8)&0x00FF0000) | 
							       ((u32_data1>>8)&0x0000FF00) | 
							       ((u32_data1>>24)&0x000000FF));

	CAN_SendSt(u8_ch, &g_st_canSendMsg);
	
	/*if(CAN_FORMAT_STD == g_st_canSendMsg.format)
	{
		u32_data[1] &= ~(CAN_TBUF_IDE);
	}
	else
	{
		u32_data[1] |= (CAN_TBUF_IDE);
	}

	if(CAN_TYPE_DATA == g_st_canSendMsg.type)
	{
		u32_data[1] &= ~(CAN_TBUF_RTR);
	}
	else
	{
		u32_data[1] |= (CAN_TBUF_RTR);
	}

	can_send_ar(u8_ch,u32_data);*/
}

void command_TestCanRx(void)
{
    if (1 == (g_st_canRcvMsg.u8_isNewMsg))
    {
        g_st_canRcvMsg.u8_isNewMsg = 0;
        
        dlog_info("id:%x \ndata1~4:0x%08x data5~8:0x%08x \nlen:%x \nformat:%x \ntype:%x \n ",
                    g_st_canRcvMsg.u32_id, 
                    *(uint32_t*)(&g_st_canRcvMsg.u8_dataArray[0]),
                    *(uint32_t*)(&g_st_canRcvMsg.u8_dataArray[4]),
		    g_st_canRcvMsg.u8_len,
		    g_st_canRcvMsg.u8_format,
		    g_st_canRcvMsg.u8_type);
    }
    else
    {
        dlog_info("no new msg.");
    }
}
