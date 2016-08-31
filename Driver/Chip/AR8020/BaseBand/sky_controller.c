/**
  ******************************************************************************
  * @file    sky_controller c file.
  * @author  Artosyn AE/FAE Team
  * @version V1.0
  * @date    03-21-2016
  * @brief
  *
  *
  ******************************************************************************
  */
#include "sky_controller.h"

#ifdef BASEBAND_SKY

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "config_baseband_register.h"
#include "config_baseband_frqdata.h"
#include "config_rcfrq_pattern.h"
#include "sys_peripheral_communication.h"
#include "stm32f7xx_hal_tim.h"
#include "stm32f7xx_hal_flash.h"
#include "stm32f7xx_hal_flash_ex.h"

extern Sys_FlagTypeDef  SysState;
extern TIM_HandleTypeDef  Tim2Handle;
extern TIM_HandleTypeDef  Tim3Handle;
extern struct RC_FRQ_CHANNEL Rc_frq[];
extern struct IT_FRQ_CHANNEL It_frq[];

Sky_FlagTypeDef SkyState;
Sky_HanlderTypeDef SkyStruct;
IDRx_TypeDef IdStruct;

uint8_t  Timer2_Delay_Cnt  = 0;
uint8_t  Timer3_Delay1_Cnt = 0;
uint32_t Timer3_Delay2_Cnt = 0;
uint32_t Device_ID[6]={0};//flash stored.

extern uint8_t Txosd_Buffer[];

/**
      * @brief  Sky Controller Parm Init.
      *
      * @illustrate
      *
         (+) Sky_FlagTypeDef.
         (+) Sky_HandleTypeDef .
      * @{
      *
      * @}
      */
void Sky_Parm_Initial(void)
{
  SkyStruct.RCChannel=9;
  SkyStruct.ITChannel=0;
  SkyStruct.Timerirqcnt=0 ;
  SkyStruct.OptID=0;
  SkyStruct.FindIDcnt=0;
  SkyStruct.IDsearchcnt =0;
  SkyStruct.IDmatcnt=0;

  SkyState.Rcmissing = DISABLE;
  SkyState.CmdsearchID = ENABLE;
  SkyState.Cmdtestmode = DISABLE;
}


/**
  * @brief  Program words (32-bit) to flash at a specified address.
  *
  * @note   If an erase and a program operations are requested simultaneously,    
  *         the erase operation is performed before the program one.
  *  
  * @param  Address: specifies the address to be programmed.
  * @param  Data: specifies the data to be programmed.
  * @retval None
  */
HAL_StatusTypeDef Flash_Write(uint32_t Start_Addr, uint32_t *p_data, uint32_t size)
{
	HAL_StatusTypeDef status = HAL_OK;
	
	HAL_FLASH_Unlock();

	//Erase page
	uint32_t err;
	FLASH_EraseInitTypeDef initType = {
		.TypeErase = FLASH_TYPEERASE_SECTORS,
		.Sector = Start_Addr,
		.NbSectors = ((size *4) >> 10), //???? yongbohe
		.VoltageRange = FLASH_VOLTAGE_RANGE_3
	};
	
	//FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);

	status = HAL_FLASHEx_Erase(&initType, &err);
	if(status != HAL_OK)
	{
		goto RET;
	}
	
	uint8_t i = 0;
	while( (i < size) && (status == HAL_OK))
	{
		status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, Start_Addr+i*4, *(p_data+i));
		i ++;
	}

	//Check whether the data is wrong
	i = 0;
	while( (status == HAL_OK) && (i < size) )
	{
		if( (*(uint32_t*)(Start_Addr + i*4)) != p_data[i++])
		{
			//debug print..
			goto RET;
		}
		i ++;
	}

RET:
	HAL_FLASH_Lock(); //lock flash after writing.
	return status;
}

/**
  * @brief  read words (32-bit) from a specified address.
  *
  *  
  * @param  Address: specifies the address to be read.
  * @param  Data: specifies the data to be read.
  * @retval None
  */
void Flash_Read(uint32_t Start_Addr, uint32_t *p_data, uint32_t size)
{
	uint8_t i = 0;
	while(i < size)
	{
		p_data[i++] = (*(uint32_t*) (Start_Addr + i*4));
		i ++;
	}
}

/**
      * @brief  Sky Remote Controller Section.
      *
      * @illustrate
      *
         (+) Grd_Write_Rcfrq().
         (+) Grd_Id_Initial().
         (+) Grd_RC_Controller().
      */

void Sky_Write_Rcfrq(uint8_t frqchannel)
{
	uint8_t page=0;

	page = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
	Spi_Baseband_ReadWrite(spiWrite, FSM_0, page & PAGE_2);  // page2

	#if defined( SKY_RF8003_2P3) || defined( SKY_RF8003_2P4)

	Spi_Baseband_ReadWrite(spiWrite, AGC3_b, Rc_frq[frqchannel].frq1);
	Spi_Baseband_ReadWrite(spiWrite, AGC3_a, Rc_frq[frqchannel].frq2);
	Spi_Baseband_ReadWrite(spiWrite, AGC3_9, Rc_frq[frqchannel].frq3);
	Spi_Baseband_ReadWrite(spiWrite, AGC3_8, Rc_frq[frqchannel].frq4);
	Spi_Baseband_ReadWrite(spiWrite, FSM_0 , page);

	#else

	Spi_Baseband_ReadWrite(spiWrite, AGC3_a, Rc_frq[frqchannel].frq1);
	Spi_Baseband_ReadWrite(spiWrite, AGC3_b, Rc_frq[frqchannel].frq2);
	Spi_Baseband_ReadWrite(spiWrite, AGC3_c, Rc_frq[frqchannel].frq3);
	Spi_Baseband_ReadWrite(spiWrite, AGC3_d, Rc_frq[frqchannel].frq4);
	Spi_Baseband_ReadWrite(spiWrite, AGC3_e, Rc_frq[frqchannel].frq5);
	Spi_Baseband_ReadWrite(spiWrite, FSM_0 , page);

	#endif
}

void Sky_Write_Rchopfrq(void)
{
	SkyStruct.RCChannel += 1;

	if(SkyStruct.RCChannel > 9)
	{
		SkyStruct.RCChannel = 0;
	}
	
	Txosd_Buffer[2]=SkyStruct.RCChannel;
	Sky_Write_Rcfrq(RCFH_PATTERN[SkyStruct.RCChannel]);
	// Sky_Judge_Rc_Frequency_Pattern_Change()
}


void Sky_Write_Itfrq(uint8_t itfrqcnt)
{
   uint8_t page=0;

   page = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
   Spi_Baseband_ReadWrite(spiWrite, FSM_0 , page & PAGE_2);  // page2

   #if defined(SKY_RF8003_2P3) || defined(SKY_RF8003_2P4)

   Spi_Baseband_ReadWrite(spiWrite, AGC3_3, It_frq[itfrqcnt].frq1);
   Spi_Baseband_ReadWrite(spiWrite, AGC3_2, It_frq[itfrqcnt].frq2);
   Spi_Baseband_ReadWrite(spiWrite, AGC3_1, It_frq[itfrqcnt].frq3);
   Spi_Baseband_ReadWrite(spiWrite, AGC3_0, It_frq[itfrqcnt].frq4);
   Spi_Baseband_ReadWrite(spiWrite, FSM_0 , page);

   #else

   Spi_Baseband_ReadWrite(spiWrite, AGC3_0, It_frq[itfrqcnt].frq1);
   Spi_Baseband_ReadWrite(spiWrite, AGC3_1, It_frq[itfrqcnt].frq2);
   Spi_Baseband_ReadWrite(spiWrite, AGC3_2, It_frq[itfrqcnt].frq3);
   Spi_Baseband_ReadWrite(spiWrite, AGC3_3, It_frq[itfrqcnt].frq4);
   Spi_Baseband_ReadWrite(spiWrite, AGC3_4, It_frq[itfrqcnt].frq5);
   Spi_Baseband_ReadWrite(spiWrite, FSM_0 , page);

   #endif
}

void Sky_Id_Initial(void)        //天空端初始化
{
	#ifdef FUNC_FLASH_ENABLE

	uint8_t  hData=0;
	uint32_t FLASH_ID[6]={0};
	Flash_Read(FLASH_START_PAGE_ADDRESS,FLASH_ID,5);

	hData = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
	Spi_Baseband_ReadWrite(spiWrite, FSM_0 , hData & PAGE_2);
	Spi_Baseband_ReadWrite(spiWrite, FEC_7 , FLASH_ID[0]&0x00FF);
	Spi_Baseband_ReadWrite(spiWrite, FEC_8 , FLASH_ID[1]&0x00FF);
	Spi_Baseband_ReadWrite(spiWrite, FEC_9 , FLASH_ID[2]&0x00FF);
	Spi_Baseband_ReadWrite(spiWrite, FEC_10, FLASH_ID[3]&0x00FF);
	Spi_Baseband_ReadWrite(spiWrite, FEC_11, FLASH_ID[4]&0x00FF);
	Spi_Baseband_ReadWrite(spiWrite, FSM_0 , hData);

	#else

	uint8_t  hData2=0;
	hData2 = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
	Spi_Baseband_ReadWrite(spiWrite, FSM_0 , hData2 & PAGE_2);
	Spi_Baseband_ReadWrite(spiWrite, FEC_7 , SKY_ID_BIT39_32);   // High --> Low
	Spi_Baseband_ReadWrite(spiWrite, FEC_8 , SKY_ID_BIT31_24);
	Spi_Baseband_ReadWrite(spiWrite, FEC_9 , SKY_ID_BIT23_16);
	Spi_Baseband_ReadWrite(spiWrite, FEC_10, SKY_ID_BIT15_08);
	Spi_Baseband_ReadWrite(spiWrite, FEC_11, SKY_ID_BIT07_00);
	Spi_Baseband_ReadWrite(spiWrite, FSM_0 , hData2);
	#endif

	Sky_Write_Rcfrq(SKY_RC_FRQ_INIT);
	Baseband_Soft_Reset();
}

/**
      * @brief  Sky Baseband reg[E9] value.
      *
      * @illustrate
      *
         (+) Reg[E9].bit0 -- Id_Match.
         (+) Reg[E9].bit1 -- Crc_Check_Ok.
         (+) Reg[E9].bit7 -- Rc_Err_Flg.
         (+) Reg[E9]=0   <--> no 14ms irq.
      * @{
      *
      * @}
      */

uint8_t Sky_Id_Match(void)
{
	uint8_t page = 0;
	uint8_t data = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
	Spi_Baseband_ReadWrite(spiWrite, FSM_0, data & PAGE_2);

	page = Spi_Baseband_ReadWrite (spiRead, FEC_4_RD, 0x00);
	Spi_Baseband_ReadWrite(spiWrite, FSM_0, data);

	return (0x03 == (page & 0x03)); 
}

uint8_t Sky_Crc_Check_Ok(void)
{
	uint8_t jData_temp=0;
	uint8_t jData2=0;

	jData2 = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
	Spi_Baseband_ReadWrite(spiWrite, FSM_0 , jData2 &PAGE_2);

	jData_temp = Spi_Baseband_ReadWrite (spiRead, FEC_4_RD, 0x00);
	Spi_Baseband_ReadWrite(spiWrite,FSM_0 ,jData2);
	
	return (0x02== (jData_temp & 0x02));
}

uint8_t Sky_Rc_Err_Flag(void)      //0xE9[]:0x80
{
   uint8_t hData=0;
   uint8_t hData_temp=0;

   hData =  Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
   Spi_Baseband_ReadWrite(spiWrite, FSM_0 , hData & PAGE_2);
   hData_temp = Spi_Baseband_ReadWrite (spiRead, FEC_4_RD, 0x00);
   Spi_Baseband_ReadWrite(spiWrite,FSM_0 ,hData);
   if(0x80 == hData_temp) return 1;
   else return 0;
}

uint8_t Sky_Rc_Zero(void)      //0xE9[]:0x00
{
   uint8_t kData=0;
   uint8_t kData_temp=0;

   kData =  Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
   Spi_Baseband_ReadWrite(spiWrite, FSM_0 , kData & PAGE_2);

   kData_temp = Spi_Baseband_ReadWrite (spiRead, FEC_4_RD, 0x00);
   Spi_Baseband_ReadWrite(spiWrite,FSM_0 ,kData);
   if(0x00 == kData_temp) return 1;
   else return 0;
}

uint8_t  Min_of_Both(uint8_t a, uint8_t b ){return((a < b) ? a : b);}


/**
      * @brief Record the ID and power if Reg[E9]bit1=1 , stored in IdStruct[].
 */

void Sky_Record_Idpower( uint8_t i )
{
   uint8_t kData;
   IdStruct[i].num = i ;
   kData = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
   Spi_Baseband_ReadWrite(spiWrite, FSM_0 , kData & PAGE_2);
   IdStruct[i].rcid5 = Spi_Baseband_ReadWrite(spiRead, FEC_1_RD,   0x00);
   IdStruct[i].rcid4 = Spi_Baseband_ReadWrite(spiRead, FEC_2_RD_1, 0x00);
   IdStruct[i].rcid3 = Spi_Baseband_ReadWrite(spiRead, FEC_2_RD_2, 0x00);
   IdStruct[i].rcid2 = Spi_Baseband_ReadWrite(spiRead, FEC_2_RD_3, 0x00);
   IdStruct[i].rcid1 = Spi_Baseband_ReadWrite(spiRead, FEC_2_RD_4, 0x00);
   IdStruct[i].powerc5= Spi_Baseband_ReadWrite(spiRead, AAGC_2_RD,  0x00);
   IdStruct[i].powerc6 = Spi_Baseband_ReadWrite (spiRead, AAGC_3_RD,  0x00);
   Spi_Baseband_ReadWrite(spiWrite, FSM_0 , kData);

   if((0 == IdStruct[i].powerc5)||(0 == IdStruct[i].powerc6))
   {
     if(0 == IdStruct[i].powerc5)
      {
        IdStruct[i].powermin = IdStruct[i].powerc6;
      }
     else if (0 == IdStruct[i].powerc6)
     {
        IdStruct[i].powermin = IdStruct[i].powerc5;
     }
   }
   else
   {
       IdStruct[i].powermin = Min_of_Both(IdStruct[i].powerc5, IdStruct[i].powerc6);
   }
}

uint8_t Sky_Getopt_Id(void)
{
    uint8_t inum=0;
    uint8_t mData=0;
    uint8_t Optid=0;
    mData = IdStruct[0].powermin;
    for(inum = 0; inum<= SkyStruct.IDmatcnt; inum++)
    {
        if((IdStruct[inum].powermin <= mData)&&(IdStruct[inum].powermin > 0))
        {
           mData = IdStruct[inum].powermin;
           Optid = inum;
        }
    }
    return Optid;
}

void Sky_Search_Right_ID(void)
{
   uint8_t page=0;
   SkyStruct.Rcunlockcnt=0;

   if(DISABLE==SkyState.Rcmissing)
   {
     if(Sky_Rc_Err_Flag()){ Baseband_Soft_Reset();}
     if(Sky_Crc_Check_Ok())
     {
      Sky_Record_Idpower(SkyStruct.IDmatcnt);
      Sky_Write_Rchopfrq();
      SkyStruct.IDmatcnt++;
      Baseband_Soft_Reset();
     }
     if(SkyStruct.IDsearchcnt >= ID_SEARCH_MAX_TIMES)
     {
        SkyStruct.IDsearchcnt =0;
        Sky_Write_Rchopfrq();
     }
     else
     {
       SkyStruct.IDsearchcnt++;
     }
     if( ID_MATCH_MAX_TIMES <= SkyStruct.IDmatcnt )
       {

         SkyStruct.OptID = Sky_Getopt_Id();
         page = Spi_Baseband_ReadWrite(spiRead , FSM_0, 0x00);
         Spi_Baseband_ReadWrite(spiWrite, FSM_0 , page & PAGE_2);
         Spi_Baseband_ReadWrite(spiWrite, FEC_7 , IdStruct[SkyStruct.OptID].rcid5);
         Spi_Baseband_ReadWrite(spiWrite, FEC_8 , IdStruct[SkyStruct.OptID].rcid4);
         Spi_Baseband_ReadWrite(spiWrite, FEC_9 , IdStruct[SkyStruct.OptID].rcid3);
         Spi_Baseband_ReadWrite(spiWrite, FEC_10, IdStruct[SkyStruct.OptID].rcid2);
         Spi_Baseband_ReadWrite(spiWrite, FEC_11, IdStruct[SkyStruct.OptID].rcid1);
         Spi_Baseband_ReadWrite(spiWrite, FSM_0 , page);

         Device_ID[0] = Device_ID[0]|IdStruct[SkyStruct.OptID].rcid5;
         Device_ID[1] = Device_ID[1]|IdStruct[SkyStruct.OptID].rcid4;
         Device_ID[2] = Device_ID[2]|IdStruct[SkyStruct.OptID].rcid3;
         Device_ID[3] = Device_ID[3]|IdStruct[SkyStruct.OptID].rcid2;
         Device_ID[4] = Device_ID[4]|IdStruct[SkyStruct.OptID].rcid1;

       //  Device ID stored in FLASH
       //  Sky_Flash_Write( FLASH_START_PAGE_ADDRESS,Device_ID,5);
       //  Sky_Flash_Read(FLASH_START_PAGE_ADDRESS,read_id,5);
         SkyState.CmdsearchID = DISABLE;
         SkyStruct.IDsearchcnt=0;
         SkyStruct.IDmatcnt=0;

       }
     else
       {
         SkyState.CmdsearchID = ENABLE;
       }
   }
   else
   {

     if (Sky_Rc_Err_Flag()) {Baseband_Soft_Reset();}
      if(Sky_Id_Match())
       {
         Sky_Write_Rchopfrq();
        SkyStruct.IDsearchcnt =0;
        SkyState.CmdsearchID = DISABLE;
      }
      else if(SkyStruct.IDsearchcnt>=ID_SEARCH_MAX_TIMES)
      {
        Sky_Write_Rchopfrq();
        Baseband_Soft_Reset();
        SkyStruct.IDsearchcnt =0;
      }
     else
      {
        SkyStruct.IDsearchcnt ++;
      }
   }
}

void Sky_Rc_Hopping (void)
{
    uint8_t IMAGE_TANS_FLAG;
    uint8_t page;

  #ifdef QAM_CHANGE
    uint8_t iData2;
    uint8_t iData9;
  #endif

    Sky_Write_Rchopfrq();
    if(Sky_Id_Match())
    {
         //HAL_TIM_Base_Stop_IT(&Tim3Handle);//关560ms时钟

         HAL_NVIC_DisableIRQ(TIM3_IRQn);
         SkyStruct.Rcunlockcnt=0;
         //image transmission working frq.
         page = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
         Spi_Baseband_ReadWrite(spiWrite, FSM_0, page|PAGE_1);
         IMAGE_TANS_FLAG = Spi_Baseband_ReadWrite(spiRead, IT_FREQ_TX, 0x00);
         Spi_Baseband_ReadWrite(spiWrite,FSM_0,page);
         if(0x0E == (IMAGE_TANS_FLAG >> 4))
          {
           Sky_Write_Itfrq(IMAGE_TANS_FLAG & 0x0F);
          }

          #ifdef QAM_CHANGE
           iData2 = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
             Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData2|PAGE_1);
             SkyStruct.Changeqammode= Spi_Baseband_ReadWrite(spiRead,QAM_CHANGE,0x00);
             Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData2);

             if(0xF1==SkyStruct.Changeqammode)
              {
                 iData9= Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
                 Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData9|PAGE_1);
                 Spi_Baseband_ReadWrite(spiWrite, TX_2, QAM_BPSK);
                 Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData9);
              }
            else if(0xF3==SkyStruct.Changeqammode)
              {
                 iData9= Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
                 Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData9|PAGE_1);
                 Spi_Baseband_ReadWrite(spiWrite, TX_2, QAM_4QAM);
                 Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData9);
              }

            else if(0xF7==SkyStruct.Changeqammode)
              {
                 iData9= Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
                 Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData9|PAGE_1);
                 Spi_Baseband_ReadWrite(spiWrite, TX_2, QAM_16QAM);
                 Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData9);
              }
            else if(0xF9==SkyStruct.Changeqammode)
              {
                 iData9= Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
                 Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData9|PAGE_1);
                 Spi_Baseband_ReadWrite(spiWrite, TX_2, QAM_64QAM);
                 Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData9);
              }
          #endif
     }
    else
    {
      if(Sky_Rc_Err_Flag()){SkyStruct.Rcunlockcnt ++ ;}
      if(RC_FRQ_NUM <= SkyStruct.Rcunlockcnt )
      {
         SkyStruct.Rcunlockcnt=0;
         SkyState.CmdsearchID = ENABLE;
         SkyState.Rcmissing = ENABLE;
         SkyStruct.IDmatcnt = ID_SEARCH_MAX_TIMES;
     }
    }
}
void Sky_Adjust_AGCGain(void)
{
     uint8_t kData1=0 ;
     uint8_t fData1=0 ;
     uint8_t gData1=0 ;

     uint8_t VALUE_C5=0 ;
     uint8_t VALUE_C6=0 ;

     kData1 = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);        //page2,R[12][13]30->4C
     Spi_Baseband_ReadWrite(spiWrite, FSM_0 , kData1 & PAGE_2);
     VALUE_C5 = Spi_Baseband_ReadWrite (spiRead, AAGC_2_RD,  0x00 );
     VALUE_C6  = Spi_Baseband_ReadWrite (spiRead, AAGC_3_RD,  0x00 );
     Spi_Baseband_ReadWrite(spiWrite, FSM_0 , kData1);

    if((VALUE_C5 >= POWER_GATE)&&(VALUE_C6 >= POWER_GATE))
     //if(VALUE_C6 >= POWER_GATE)
     {
        fData1 = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
        Spi_Baseband_ReadWrite(spiWrite, FSM_0 , fData1 & PAGE_2);
        Spi_Baseband_ReadWrite(spiWrite, AGC_2 , AAGC_GAIN_FAR);
        Spi_Baseband_ReadWrite(spiWrite, AGC_3 , AAGC_GAIN_FAR);
        Spi_Baseband_ReadWrite(spiWrite, FSM_0 , fData1);
     }
     if(((VALUE_C5 < POWER_GATE)&&(VALUE_C6 < POWER_GATE))&&(VALUE_C5 > 0x00)&&(VALUE_C6 >0x00))
     {
       gData1 = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
       Spi_Baseband_ReadWrite(spiWrite, FSM_0 , gData1 & PAGE_2);
       Spi_Baseband_ReadWrite(spiWrite, AGC_2, AAGC_GAIN_NEAR);
       Spi_Baseband_ReadWrite(spiWrite, AGC_3 ,AAGC_GAIN_NEAR);
       Spi_Baseband_ReadWrite(spiWrite, FSM_0 , gData1);
     }
}

void Sky_Adjust_AGCGain_SearchID(uint8_t i)
{
    uint8_t hData0=0 ;
    uint8_t hData1=0 ;
     switch(i)
     {
       case 0:
       {
         hData0 = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
         Spi_Baseband_ReadWrite(spiWrite, FSM_0 , hData0 & PAGE_2);
         Spi_Baseband_ReadWrite(spiWrite, AGC_2, AAGC_GAIN_FAR);
         Spi_Baseband_ReadWrite(spiWrite, AGC_3 ,AAGC_GAIN_FAR);
         Spi_Baseband_ReadWrite(spiWrite, FSM_0 , hData0);
       };break;
       case 1:
       {
         hData1 = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
         Spi_Baseband_ReadWrite(spiWrite, FSM_0 , hData1 & PAGE_2);
         Spi_Baseband_ReadWrite(spiWrite, AGC_2, AAGC_GAIN_NEAR);
         Spi_Baseband_ReadWrite(spiWrite, AGC_3 ,AAGC_GAIN_NEAR);
         Spi_Baseband_ReadWrite(spiWrite, FSM_0 , hData1);
       };break;
     }
}
void Sky_Hanlde_SpecialIrq(void)
{
  if(Sky_Rc_Zero())
  {
    if(SkyStruct.CntAGCGain>2)
    {
      SkyStruct.CntAGCGain=0;
      Sky_Adjust_AGCGain_SearchID(0);
    }
    else
    {
      SkyStruct.CntAGCGain++;
      Sky_Adjust_AGCGain_SearchID(1);
    }
   Sky_Write_Rchopfrq();
   Baseband_Soft_Reset();
  }
}
void Sky_Osdmsg_Ptf(void)
{
  uint8_t iData_page_temp=0;
  /*
  uint8_t iData_C5_temp;
  uint8_t iData_C6_temp;
  uint8_t iData_C3_temp;
  uint8_t iData_C7_temp;
  uint8_t iData_C4_temp;
  uint8_t iData_Eb_temp;

  uint8_t iData_E0_temp;
  uint8_t iData_EA_temp;
  uint8_t iData_SNRH_temp;
  uint8_t iData_SNRL_temp;
  uint8_t iData_E7_temp;
  uint8_t iData_E8_temp;
  uint8_t iData_EE_temp;
  uint8_t iData_EF_temp;
   */
  Txosd_Buffer[0]=0x55;
  Txosd_Buffer[1]=0xaa;
  iData_page_temp= Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);   //page2
  Spi_Baseband_ReadWrite(spiWrite, FSM_0 , iData_page_temp& PAGE_2);

  Txosd_Buffer[3]=0;
  Txosd_Buffer[7]=0;
  Txosd_Buffer[8]/*iData_C3_temp = Spi_Baseband_ReadWrite(spiRead, AAGC_0_RD, 0x00);*/=0;
  Txosd_Buffer[9]/*iData_C7_temp = Spi_Baseband_ReadWrite(spiRead, AAGC_4_RD, 0x00);*/=0;
  Txosd_Buffer[10]/*iData_C4_temp= Spi_Baseband_ReadWrite(spiRead, AAGC_1_RD, 0x00);*/=0;
  Txosd_Buffer[4]/*iData_C5_temp*/= Spi_Baseband_ReadWrite(spiRead, AAGC_2_RD, 0x00);
  Txosd_Buffer[5]/*iData_C6_temp*/= Spi_Baseband_ReadWrite(spiRead, AAGC_3_RD, 0x00);
  Txosd_Buffer[6]/*iData_Eb_temp*/= Spi_Baseband_ReadWrite(spiRead, FEC_4_RD , 0x00);
  Txosd_Buffer[11]/*iData_E0_temp= Spi_Baseband_ReadWrite(spiRead, FEC_0_RD , 0x00);*/=0;

  Txosd_Buffer[13]/*iData_SNRH_temp = Spi_Baseband_ReadWrite(spiRead, CE_9_RD, 0x00);*/ =0;
  Txosd_Buffer[14]/*iData_SNRL_temp= Spi_Baseband_ReadWrite(spiRead, CE_A_RD, 0x00);*/ =0;

  Txosd_Buffer[15]/*iData_E7_temp = Spi_Baseband_ReadWrite(spiRead, FEC_3_RD_1, 0x00);*/=0;
  Txosd_Buffer[16]/*iData_E8_temp= Spi_Baseband_ReadWrite(spiRead, FEC_3_RD_2, 0x00);*/=0;
  Txosd_Buffer[17]/*iData_EE_temp = Spi_Baseband_ReadWrite(spiRead, FEC_9_RD, 0x00);*/=0;
  Txosd_Buffer[18]/*iData_EF_temp = Spi_Baseband_ReadWrite(spiRead, FEC_10_RD, 0x00);*/=0;
  Spi_Baseband_ReadWrite(spiWrite, FSM_0 , iData_page_temp);

  Txosd_Buffer[19]=0;
  SysState.TxcyosdOnly=ENABLE;

}

//*********************TX RX initial(14ms irq)**************


void TIM2_IRQHandler(void)
{
    //__HAL_TIM_CLEAR_IT(&Tim2Handle, TIM_IT_UPDATE);
    switch (Timer2_Delay_Cnt)
    {
       case 0:
        {
           Timer2_Delay_Cnt ++ ;

        } break;

       case 1:
        {
          Timer2_Delay_Cnt ++ ;
        } break;
       case 2:
        {
          Timer2_Delay_Cnt=0;
          if( ENABLE == SkyState.CmdsearchID)
           {
             Sky_Search_Right_ID();
           }
          else                                   // 不对频
           {
             SkyState.CmdsearchID = DISABLE;
             SkyState.Rcmissing = ENABLE;
             Sky_Rc_Hopping();
           }
           Sky_Adjust_AGCGain();
           Hanlde_Cy7c_Msg();
           Sky_Osdmsg_Ptf();
           //HAL_TIM_Base_Stop_IT(&Tim2Handle);//?1ms??
           HAL_NVIC_DisableIRQ(TIM2_IRQn);
        } break;
    }
}
void TIM3_IRQHandler(void)
{
   //__HAL_TIM_CLEAR_IT(&Tim3Handle, TIM_IT_UPDATE);
   if(Timer3_Delay2_Cnt < 560){Timer3_Delay2_Cnt ++;}
   else
     {
       Sky_Hanlde_SpecialIrq();
       Timer3_Delay2_Cnt  = 0;
     }
}
#endif
