/*****************************************************************************
Copyright: 2016-2020, Artosyn. Co., Ltd.
File name: testhal_pwm.c
Description: 
Author: Wumin @ Artosy Software Team
Version: 0.0.1
Date: 2016/12/19
History:
         0.0.1    2016/12/19    test pwm
         0.0.2    2017/3/27      seperated to share
*****************************************************************************/

#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "debuglog.h"
#include "hal_dma.h"
#include "md5.h"
#include "data_type.h"
#include "cmsis_os.h"


extern unsigned int command_str2uint(char *str);

void command_dma(char * u32_src, char *u32_dst, char *u32_byteNum)
{
    unsigned int iSrcAddr;
    unsigned int iDstAddr;
    unsigned int iNum;

    iDstAddr    = command_str2uint(u32_dst);
    iSrcAddr    = command_str2uint(u32_src);
    iNum        = command_str2uint(u32_byteNum);


    HAL_DMA_Start(iSrcAddr, iDstAddr, iNum, DMA_AUTO, DMA_LINK_LIST_ITEM);
	
	/* use to fake the dst data */
#if 0
    unsigned char *p_reg;
    p_reg = (unsigned char *)0x81800000;
    *p_reg = 0xAA;
#endif
    /***********************/
    
    #define MD5_SIZE 16
    uint8_t    md5_value[MD5_SIZE];
    int i = 0;
    MD5_CTX md5;
    MD5Init(&md5);
    MD5Update(&md5, (uint8_t *)iSrcAddr, iNum);
    MD5Final(&md5, md5_value);
    dlog_info("src MD5 = 0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++]);

    memset(&md5, 0, sizeof(MD5_CTX));
    MD5Init(&md5);
    MD5Update(&md5, (uint8_t *)iDstAddr, iNum);
    memset(&md5_value, 0, sizeof(md5_value));
    MD5Final(&md5, md5_value);
    i = 0;
    dlog_info("dst MD5 = 0x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", 
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++],
                                    md5_value[i++]);
}

void command_test_dma_loop(char * u32_src, char *u32_dst, char *u32_byteNum)
{
	unsigned int i = 0;
	
	while(1)
	{
		command_dma(u32_src, u32_dst, u32_byteNum);
		dlog_info("i = %d\n", i++);
	}
}


