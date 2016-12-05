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

	g_stCanPar.br = strtoul(br, NULL, 0);
	g_stCanPar.acode = strtoul(acode, NULL, 0);
	g_stCanPar.amask = strtoul(amask, NULL, 0);
	g_stCanPar.rtie = strtoul(rtie, NULL, 0);
	g_stCanPar.format = strtoul(format, NULL, 0);

	//CAN_ConnectIsr();//CAN isr

	CAN_InitSt((unsigned char)u32_ch, &g_stCanPar);
	
	dlog_info("g_stCanPar: ch br acode amask rtie format:%d %d 0x%x 0x%x 0x%x 0x%x\n",
						u32_ch, 
						g_stCanPar.br, 
						g_stCanPar.acode, 
						g_stCanPar.amask, 
						g_stCanPar.rtie,
						g_stCanPar.format);
}

void command_TestCanTx(unsigned char *ch, unsigned char *id, unsigned char *len, unsigned char *tbuf,unsigned char *format, unsigned char *type)
{
	//unsigned int u32_data[] = {0x789,0x08,0x11223344,0x55667788};
	unsigned int u32_data1;
	unsigned char u8_unframe[30];
	unsigned char u8_ch = strtoul(ch, NULL, 0);
	g_stCanSendMsg.id = strtoul(id, NULL, 0);//id
	g_stCanSendMsg.len = strtoul(len, NULL, 0);//len
	g_stCanSendMsg.ch = u8_ch;//ch	
	g_stCanSendMsg.format = strtoul(format, NULL, 0);//format
	g_stCanSendMsg.type = strtoul(type,  NULL, 0);//type

	memcpy(u8_unframe,tbuf,10);
	u32_data1 = strtoul(u8_unframe, NULL, 0);
	*(unsigned int*)(&(g_stCanSendMsg.data[0])) = (((u32_data1<<24)&0xFF000000) | 
							((u32_data1<<8)&0x00FF0000) | 
							((u32_data1>>8)&0x0000FF00) | 
							((u32_data1>>24)&0x000000FF));
	
	memcpy(u8_unframe+2,tbuf+10,8);
	u32_data1 = strtoul(u8_unframe, NULL, 0);
	*(unsigned int*)(&(g_stCanSendMsg.data[4])) = (((u32_data1<<24)&0xFF000000) | 
							((u32_data1<<8)&0x00FF0000) | 
							((u32_data1>>8)&0x0000FF00) | 
							((u32_data1>>24)&0x000000FF));

	CAN_SendSt(u8_ch, &g_stCanSendMsg);
	
	/*if(CAN_FORMAT_STD == g_stCanSendMsg.format)
	{
		u32_data[1] &= ~(CAN_TBUF_IDE);
	}
	else
	{
		u32_data[1] |= (CAN_TBUF_IDE);
	}

	if(CAN_TYPE_DATA == g_stCanSendMsg.type)
	{
		u32_data[1] &= ~(CAN_TBUF_RTR);
	}
	else
	{
		u32_data[1] |= (CAN_TBUF_RTR);
	}

	can_send_ar(u8_ch,u32_data);*/
}
