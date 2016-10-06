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
#include "config_functions_sel.h"
#ifdef BASEBAND_SKY
#include <timer.h>
#include "interrupt.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdint.h>
#include "config_baseband_register.h"
#include "config_baseband_frqdata.h"
#include "config_rcfrq_pattern.h"
#include "sys_peripheral_communication.h"
#include "BB_ctrl.h"
#include "sys_peripheral_init.h"
#include "debuglog.h"

extern Sys_FlagTypeDef  SysState;
extern struct RC_FRQ_CHANNEL Rc_frq[];
extern struct IT_FRQ_CHANNEL It_frq[];
extern uint8_t Txosd_Buffer[];
extern init_timer_st init_timer0_0;
extern init_timer_st init_timer0_1;
Sky_FlagTypeDef SkyState;
Sky_HanlderTypeDef SkyStruct;
IDRx_TypeDef IdStruct;
uint8_t  Timer2_Delay_Cnt  = 0;
uint8_t  Timer3_Delay1_Cnt = 0;
uint32_t Timer3_Delay2_Cnt = 0;
uint32_t Device_ID[6]={0};//flash stored.


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

    //when first to use the Rc or losed communication times upto
    #if 1
    Timer2_Init();
    Sky_Timer3_Init();
    #endif
    
    printf("Grd_Parm_Initial %d %d %d\r\n",init_timer0_0.base_time_group,init_timer0_0.time_num,init_timer0_0.ctrl);
    printf("Grd_Parm_Initial %d %d %d\r\n",init_timer0_1.base_time_group,init_timer0_1.time_num,init_timer0_1.ctrl);
    
    #if 1    
    reg_IrqHandle(BB_RX_ENABLE_VECTOR_NUM, wimax_vsoc_rx_isr);
    INTR_NVIC_EnableIRQ(BB_RX_ENABLE_VECTOR_NUM);
    #endif
//    while(1);
}

/**
      * @brief STORED ID in FLASH.
      *
      * @illustrate
      *
         (+) Flash_Write().
         (+) Flash_Read().
      */
#if 0
uint8_t Flash_Write(uint32_t Start_Addr,uint32_t *p_data,uint32_t size)
{
  volatile FLASH_Status FLASHStatus;
  uint32_t End_Addr=0;

  uint32_t NbrOfPage = 0;
  uint32_t EraseCounter = 0;
  uint32_t Address = 0;
  uint8_t i=0;
  uint8_t MemoryProgramStatus=1;


  End_Addr = Start_Addr + size*4;

  NbrOfPage=((End_Addr-Start_Addr) >> 10) + 1;  //How many pages of erased

  FLASHStatus = FLASH_COMPLETE_1;

  //The result is 1, then through

  //unlock function
  HAL_FLASH_Unlock();

  //erase page
  FLASH_ClearFlag(FLASH_FLAG_BSY | FLASH_FLAG_EOP | FLASH_FLAG_PGERR | FLASH_FLAG_WRPERR);


  for(EraseCounter=0;(EraseCounter < NbrOfPage)&&(FLASHStatus==FLASH_COMPLETE_1);EraseCounter++)
  {

    FLASHStatus=FLASH_ErasePage(Start_Addr+(PageSize*EraseCounter));

  }
  //write data
  Address = Start_Addr;
  i=0;
  while((Address<End_Addr)&&(FLASHStatus==FLASH_COMPLETE_1))
  {

    FLASHStatus = FLASH_ProgramWord(Address,p_data[i++]);
    Address=Address+4;

  }
  //Check whether the data is wrong
  Address = Start_Addr;
  i=0;
  while((Address < End_Addr) && (MemoryProgramStatus != 0))
  {
    if((*(uint32_t*) Address) != p_data[i++])
    {
        MemoryProgramStatus = 0;
        return 1;
    }
    Address += 4;
  }
  return 0;

}


uint8_t Flash_Read(uint32_t Start_Addr,uint32_t *p_data,uint32_t size)
{
  uint32_t EndAddr = Start_Addr + size*4;
  uint32_t Address = 0x0;
  uint8_t  i = 0;
  uint8_t  MemoryProgramStatus = 1;

  Address = Start_Addr;

  while((Address < EndAddr) && (MemoryProgramStatus != 0))
  {
    p_data[i++]=(*(uint32_t*) Address);
    Address += 4;
  }

  return 0;

}
#endif
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
    #if defined( SKY_RF8003_2P3) || defined( SKY_RF8003_2P4)
        /* Spi_Baseband_ReadWrite(spiWrite, AGC3_b, Rc_frq[frqchannel].frq1);
        Spi_Baseband_ReadWrite(spiWrite, AGC3_a, Rc_frq[frqchannel].frq2);
        Spi_Baseband_ReadWrite(spiWrite, AGC3_9, Rc_frq[frqchannel].frq3);
        Spi_Baseband_ReadWrite(spiWrite, AGC3_8, Rc_frq[frqchannel].frq4);
        Spi_Baseband_ReadWrite(spiWrite, FSM_0 , dpage);*/
        
        BB_SPI_WriteByte(PAGE2, AGC3_b, Rc_frq[frqchannel].frq1);
        BB_SPI_WriteByte(PAGE2, AGC3_a, Rc_frq[frqchannel].frq2);
        BB_SPI_WriteByte(PAGE2, AGC3_9, Rc_frq[frqchannel].frq3);
        BB_SPI_WriteByte(PAGE2, AGC3_8, Rc_frq[frqchannel].frq4);
    #else
        /*Spi_Baseband_ReadWrite(spiWrite, AGC3_a, Rc_frq[frqchannel].frq1);
        Spi_Baseband_ReadWrite(spiWrite, AGC3_b, Rc_frq[frqchannel].frq2);
        Spi_Baseband_ReadWrite(spiWrite, AGC3_c, Rc_frq[frqchannel].frq3);
        Spi_Baseband_ReadWrite(spiWrite, AGC3_d, Rc_frq[frqchannel].frq4);
        Spi_Baseband_ReadWrite(spiWrite, AGC3_e, Rc_frq[frqchannel].frq5);
        Spi_Baseband_ReadWrite(spiWrite, FSM_0 , dpage);*/
        
        BB_SPI_WriteByte(PAGE2, AGC3_a, Rc_frq[frqchannel].frq1);
        BB_SPI_WriteByte(PAGE2, AGC3_b, Rc_frq[frqchannel].frq2);
        BB_SPI_WriteByte(PAGE2, AGC3_c, Rc_frq[frqchannel].frq3);
        BB_SPI_WriteByte(PAGE2, AGC3_d, Rc_frq[frqchannel].frq4);
        BB_SPI_WriteByte(PAGE2, AGC3_e, Rc_frq[frqchannel].frq5);
    #endif
}

void Sky_Write_Rchopfrq(void)
{
    SkyStruct.RCChannel += 1;
    if(SkyStruct.RCChannel >=  MAX_RC_FRQ_SIZE)
    {
        SkyStruct.RCChannel = 0;
    }
    
    //Txosd_Buffer[2]=SkyStruct.RCChannel;
    Sky_Write_Rcfrq(RCFH_PATTERN[SkyStruct.RCChannel]);
}


static uint8_t workfrqcnt = 0xff;
void Sky_Write_Itfrq(uint8_t itfrqcnt)
{
    #if defined(SKY_RF8003_2P3) || defined(SKY_RF8003_2P4)
    if(workfrqcnt != itfrqcnt)
    {
        BB_SPI_WriteByte(PAGE2, AGC3_0, It_frq[itfrqcnt].frq1);
        BB_SPI_WriteByte(PAGE2, AGC3_1, It_frq[itfrqcnt].frq2);
        BB_SPI_WriteByte(PAGE2, AGC3_2, It_frq[itfrqcnt].frq3);
        BB_SPI_WriteByte(PAGE2, AGC3_3, It_frq[itfrqcnt].frq4);
        workfrqcnt = itfrqcnt;
        printf("S==>%d\r\n", itfrqcnt);
    }   
    #else
    #endif
}


void Sky_Id_Initial(void)        //天空端初始化
{

 #ifdef FUNC_FLASH_ENABLE

   uint32_t FLASH_ID[6]={0};
   Flash_Read(FLASH_START_PAGE_ADDRESS,FLASH_ID,5);//?
/*   
   hData = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
   Spi_Baseband_ReadWrite(spiWrite, FSM_0 , hData & PAGE_2);   
   Spi_Baseband_ReadWrite(spiWrite, FEC_7 , FLASH_ID[0]&0x00FF);
   Spi_Baseband_ReadWrite(spiWrite, FEC_8 , FLASH_ID[1]&0x00FF);
   Spi_Baseband_ReadWrite(spiWrite, FEC_9 , FLASH_ID[2]&0x00FF);
   Spi_Baseband_ReadWrite(spiWrite, FEC_10, FLASH_ID[3]&0x00FF);
   Spi_Baseband_ReadWrite(spiWrite, FEC_11, FLASH_ID[4]&0x00FF);
   Spi_Baseband_ReadWrite(spiWrite, FSM_0 , hData);
*/

   BB_SPI_WriteByte(PAGE2, FEC_7, FLASH_ID[0]&0x00FF);
   BB_SPI_WriteByte(PAGE2, FEC_8, FLASH_ID[0]&0x00FF);
   BB_SPI_WriteByte(PAGE2, FEC_9, FLASH_ID[0]&0x00FF);
   BB_SPI_WriteByte(PAGE2, FEC_10, FLASH_ID[0]&0x00FF);
   BB_SPI_WriteByte(PAGE2, FEC_11, FLASH_ID[0]&0x00FF);


 #else
/*
   uint8_t  hData2=0;
   hData2 = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
   Spi_Baseband_ReadWrite(spiWrite, FSM_0 , hData2 & PAGE_2);
   Spi_Baseband_ReadWrite(spiWrite, FEC_7 , SKY_ID_BIT39_32);   // High --> Low
   Spi_Baseband_ReadWrite(spiWrite, FEC_8 , SKY_ID_BIT31_24);
   Spi_Baseband_ReadWrite(spiWrite, FEC_9 , SKY_ID_BIT23_16);
   Spi_Baseband_ReadWrite(spiWrite, FEC_10, SKY_ID_BIT15_08);
   Spi_Baseband_ReadWrite(spiWrite, FEC_11, SKY_ID_BIT07_00);
   Spi_Baseband_ReadWrite(spiWrite, FSM_0 , hData2);
*/
   BB_SPI_WriteByte(PAGE2, FEC_7, SKY_ID_BIT39_32);
   BB_SPI_WriteByte(PAGE2, FEC_8, SKY_ID_BIT31_24);
   BB_SPI_WriteByte(PAGE2, FEC_9, SKY_ID_BIT23_16);
   BB_SPI_WriteByte(PAGE2, FEC_10, SKY_ID_BIT15_08);
   BB_SPI_WriteByte(PAGE2, FEC_11, SKY_ID_BIT07_00);

  #endif

  Sky_Write_Rcfrq(SKY_RC_FRQ_INIT);
  BB_softReset(BB_SKY_MODE );
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
   uint8_t iData_temp=0;
/*   
   uint8_t iData2=0;

   iData2 = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
   Spi_Baseband_ReadWrite(spiWrite, FSM_0 , iData2 & PAGE_2);
   iData_temp = Spi_Baseband_ReadWrite (spiRead, FEC_4_RD, 0x00);
   Spi_Baseband_ReadWrite(spiWrite,FSM_0 ,iData2);
   */
    iData_temp = BB_SPI_ReadByte(PAGE2, FEC_4_RD);
    
    return (iData_temp & 0x03) ? 1 : 0;
}

uint8_t Sky_Crc_Check_Ok(void)
{
    uint8_t jData_temp = BB_SPI_ReadByte(PAGE2, FEC_4_RD);   
    return (jData_temp & 0x02) ? 1 : 0;
}

uint8_t lock_cnt = 0;
uint8_t lock = 0;
uint8_t Sky_Rc_Err_Flag(void)      //0xE9[]:0x80
{
    uint8_t hData=0;
    uint8_t hData_temp=0;

    /*   hData =  Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
    Spi_Baseband_ReadWrite(spiWrite, FSM_0 , hData & PAGE_2);
    hData_temp = Spi_Baseband_ReadWrite (spiRead, FEC_4_RD, 0x00);
    Spi_Baseband_ReadWrite(spiWrite,FSM_0 ,hData);*/
    hData_temp = BB_SPI_ReadByte (PAGE2, FEC_4_RD);
    
    if( lock != hData_temp || lock_cnt ++ > 200)
    {
        printf("L %x\n", hData_temp);
        lock_cnt = 0;
        lock = hData_temp;
    }
    
    return (0x80 == hData_temp);
}

uint8_t Sky_Rc_Zero(void)      //0xE9[]:0x00
{   
   uint8_t kData = BB_SPI_ReadByte(PAGE2, FEC_4_RD);
   
   return (0x00 == kData);
}


uint8_t  Min_of_Both(uint8_t a, uint8_t b ){return((a < b) ? a : b);}


/**
      * @brief Record the ID and power if Reg[E9]bit1=1 , stored in IdStruct[].
 */

void Sky_Record_Idpower( uint8_t i )
{
   uint8_t kData;
   IdStruct[i].num = i ;
 /*  
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
*/
   IdStruct[i].rcid5 = BB_SPI_ReadByte(PAGE2, FEC_1_RD);
   IdStruct[i].rcid4 = BB_SPI_ReadByte(PAGE2, FEC_2_RD_1);
   IdStruct[i].rcid3 = BB_SPI_ReadByte(PAGE2, FEC_2_RD_2);
   IdStruct[i].rcid2 = BB_SPI_ReadByte(PAGE2, FEC_2_RD_3);
   IdStruct[i].rcid1 = BB_SPI_ReadByte(PAGE2, FEC_2_RD_4);
   IdStruct[i].powerc5= BB_SPI_ReadByte(PAGE2, AAGC_2_RD);
   IdStruct[i].powerc6 = BB_SPI_ReadByte (PAGE2, AAGC_3_RD);

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
    uint8_t iData=0;
    SkyStruct.Rcunlockcnt=0;

    if(DISABLE==SkyState.Rcmissing)
    {
        if(Sky_Rc_Err_Flag())
        {
            BB_softReset(BB_SKY_MODE );
        }

        if(Sky_Crc_Check_Ok())
        {
            Sky_Record_Idpower(SkyStruct.IDmatcnt);
            Sky_Write_Rchopfrq();
            SkyStruct.IDmatcnt++;
            printf("ch %d \r\n",SkyStruct.IDmatcnt);
            BB_softReset(BB_SKY_MODE );
        }
        
        if(SkyStruct.IDsearchcnt >= ID_SEARCH_MAX_TIMES)
        {
            SkyStruct.IDsearchcnt =0;
            Sky_Write_Rchopfrq();
            printf(">=\r\n");
        }
        else
        {
            SkyStruct.IDsearchcnt++;
        }

        if( ID_MATCH_MAX_TIMES <= SkyStruct.IDmatcnt )
        {
             SkyStruct.OptID = Sky_Getopt_Id();
            /* 
             iData = Spi_Baseband_ReadWrite(spiRead , FSM_0, 0x00);
             Spi_Baseband_ReadWrite(spiWrite, FSM_0 , iData & PAGE_2);
             Spi_Baseband_ReadWrite(spiWrite, FEC_7 , IdStruct[SkyStruct.OptID].rcid5);
             Spi_Baseband_ReadWrite(spiWrite, FEC_8 , IdStruct[SkyStruct.OptID].rcid4);
             Spi_Baseband_ReadWrite(spiWrite, FEC_9 , IdStruct[SkyStruct.OptID].rcid3);
             Spi_Baseband_ReadWrite(spiWrite, FEC_10, IdStruct[SkyStruct.OptID].rcid2);
             Spi_Baseband_ReadWrite(spiWrite, FEC_11, IdStruct[SkyStruct.OptID].rcid1);
             Spi_Baseband_ReadWrite(spiWrite, FSM_0 , iData);
            */
             BB_SPI_WriteByte(PAGE2, FEC_7 , IdStruct[SkyStruct.OptID].rcid5);
             BB_SPI_WriteByte(PAGE2, FEC_8 , IdStruct[SkyStruct.OptID].rcid4);
             BB_SPI_WriteByte(PAGE2, FEC_9 , IdStruct[SkyStruct.OptID].rcid3);
             BB_SPI_WriteByte(PAGE2, FEC_10, IdStruct[SkyStruct.OptID].rcid2);
             BB_SPI_WriteByte(PAGE2, FEC_11, IdStruct[SkyStruct.OptID].rcid1);

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
        if (Sky_Rc_Err_Flag()) 
        {
            BB_softReset(BB_SKY_MODE );
        }
        if(Sky_Id_Match())
        {
            Sky_Write_Rchopfrq();
            SkyStruct.IDsearchcnt =0;
            SkyState.CmdsearchID = DISABLE;
        }
        else if(SkyStruct.IDsearchcnt>=ID_SEARCH_MAX_TIMES)
        {
            Sky_Write_Rchopfrq();
            BB_softReset(BB_SKY_MODE );
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
    uint8_t iData3;

    #ifdef QAM_CHANGE
    uint8_t iData2;
    uint8_t iData9;
    #endif

 //   Sky_Write_Rchopfrq();
//    printf("Sky_Rc_Hopping Sky_Write_Rchopfrq \n");
    if(Sky_Id_Match())
    {
        INTR_NVIC_DisableIRQ(TIMER_INTR01_VECTOR_NUM);
        SkyStruct.Rcunlockcnt=0;
        //image transmission working frq.
        /*iData3 = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
        Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData3|PAGE_1);
        IMAGE_TANS_FLAG = Spi_Baseband_ReadWrite(spiRead, IT_FREQ_TX, 0x00);
        Spi_Baseband_ReadWrite(spiWrite,FSM_0,iData3);*/
        IMAGE_TANS_FLAG = BB_SPI_ReadByte(PAGE2, IT_FREQ_TX);
        if(0x0E == (IMAGE_TANS_FLAG >> 4))
        {
            Sky_Write_Itfrq(IMAGE_TANS_FLAG & 0x0F);
        }

        #if 0
        #ifdef QAM_CHANGE
        /*
        iData2 = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
        Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData2|PAGE_1);
        SkyStruct.Changeqammode= Spi_Baseband_ReadWrite(spiRead,QAM_CHANGE,0x00);
        Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData2);
        */
        SkyStruct.Changeqammode= BB_SPI_ReadByte(PAGE2,QAM_CHANGE);
        printf("SkyStruct.Changeqammode %d \n",SkyStruct.Changeqammode);
        if(0xF1==SkyStruct.Changeqammode)
        {
            /*iData9= Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
            Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData9|PAGE_1);
            Spi_Baseband_ReadWrite(spiWrite, TX_2, QAM_BPSK);
            Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData9);*/

            BB_SPI_WriteByte(PAGE2, TX_2, QAM_BPSK);

        }
        else if(0xF3==SkyStruct.Changeqammode)
        {
            /*iData9= Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
            Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData9|PAGE_1);
            Spi_Baseband_ReadWrite(spiWrite, TX_2, QAM_4QAM);
            Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData9);*/
            BB_SPI_WriteByte(PAGE2, TX_2, QAM_4QAM);
        }
        else if(0xF7==SkyStruct.Changeqammode)
        {
            /*iData9= Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
            Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData9|PAGE_1);
            Spi_Baseband_ReadWrite(spiWrite, TX_2, QAM_16QAM);
            Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData9);*/
            BB_SPI_WriteByte(PAGE2, TX_2, QAM_16QAM);
        }
        else if(0xF9==SkyStruct.Changeqammode)
        {
            /*iData9= Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
            Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData9|PAGE_1);
            Spi_Baseband_ReadWrite(spiWrite, TX_2, QAM_64QAM);
            Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData9);*/
            BB_SPI_WriteByte(PAGE2, TX_2, QAM_64QAM);
        }
        #endif
        #endif
    }
    else
    {
        printf("S RCid Fail %x %x %x %x\n", BB_SPI_ReadByte(PAGE2, FEC_7),
                                            BB_SPI_ReadByte(PAGE2, FEC_8),
                                            BB_SPI_ReadByte(PAGE2, FEC_9),
                                            BB_SPI_ReadByte(PAGE2, FEC_10));

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

     /*kData1 = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);        //page2,R[12][13]30->4C
     Spi_Baseband_ReadWrite(spiWrite, FSM_0 , kData1 & PAGE_2);
     VALUE_C5 = Spi_Baseband_ReadWrite (spiRead, AAGC_2_RD,  0x00 );
     VALUE_C6  = Spi_Baseband_ReadWrite (spiRead, AAGC_3_RD,  0x00 );
     Spi_Baseband_ReadWrite(spiWrite, FSM_0 , kData1);*/
     VALUE_C5 = BB_SPI_ReadByte (PAGE2, AAGC_2_RD);
     VALUE_C6  = BB_SPI_ReadByte (PAGE2, AAGC_3_RD);

    if((VALUE_C5 >= POWER_GATE)&&(VALUE_C6 >= POWER_GATE))
     //if(VALUE_C6 >= POWER_GATE)
     {
        /*fData1 = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
        Spi_Baseband_ReadWrite(spiWrite, FSM_0 , fData1 & PAGE_2);
        Spi_Baseband_ReadWrite(spiWrite, AGC_2 , AAGC_GAIN_FAR);
        Spi_Baseband_ReadWrite(spiWrite, AGC_3 , AAGC_GAIN_FAR);
        Spi_Baseband_ReadWrite(spiWrite, FSM_0 , fData1);*/
        BB_SPI_WriteByte(PAGE2, AGC_2 , AAGC_GAIN_FAR);
        BB_SPI_WriteByte(PAGE2, AGC_3 , AAGC_GAIN_FAR);

     }
     if(((VALUE_C5 < POWER_GATE)&&(VALUE_C6 < POWER_GATE))&&(VALUE_C5 > 0x00)&&(VALUE_C6 >0x00))
     {
       /*gData1 = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
       Spi_Baseband_ReadWrite(spiWrite, FSM_0 , gData1 & PAGE_2);
       Spi_Baseband_ReadWrite(spiWrite, AGC_2, AAGC_GAIN_NEAR);
       Spi_Baseband_ReadWrite(spiWrite, AGC_3 ,AAGC_GAIN_NEAR);
       Spi_Baseband_ReadWrite(spiWrite, FSM_0 , gData1);*/
       BB_SPI_WriteByte(PAGE2, AGC_2, AAGC_GAIN_NEAR);
       BB_SPI_WriteByte(PAGE2, AGC_3 ,AAGC_GAIN_NEAR);
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
            /*hData0 = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
            Spi_Baseband_ReadWrite(spiWrite, FSM_0 , hData0 & PAGE_2);
            BB_SPI_WriteByte(PAGE2, AGC_2, AAGC_GAIN_FAR);
            BB_SPI_WriteByte(PAGE2, AGC_3 ,AAGC_GAIN_FAR);
            Spi_Baseband_ReadWrite(spiWrite, FSM_0 , hData0);*/

            BB_SPI_WriteByte(PAGE2, AGC_2, AAGC_GAIN_FAR);
            BB_SPI_WriteByte(PAGE2, AGC_3 ,AAGC_GAIN_FAR);
        };
        break;
        
        case 1:
        {
            /*hData1 = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
            Spi_Baseband_ReadWrite(spiWrite, FSM_0 , hData1 & PAGE_2);
            Spi_Baseband_ReadWrite(spiWrite, AGC_2, AAGC_GAIN_NEAR);
            Spi_Baseband_ReadWrite(spiWrite, AGC_3 ,AAGC_GAIN_NEAR);
            Spi_Baseband_ReadWrite(spiWrite, FSM_0 , hData1);
            */
            BB_SPI_WriteByte(PAGE2, AGC_2, AAGC_GAIN_FAR);
            BB_SPI_WriteByte(PAGE2, AGC_3 ,AAGC_GAIN_FAR);
        };
        break;
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
        BB_softReset(BB_SKY_MODE );
    }
}

void Sky_Osdmsg_Ptf(void)
{
    uint8_t iData_page_temp=0;

    Txosd_Buffer[0]=0x55;
    Txosd_Buffer[1]=0xaa;
    /*iData_page_temp= Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);   //page2
    Spi_Baseband_ReadWrite(spiWrite, FSM_0 , iData_page_temp& PAGE_2);*/

    Txosd_Buffer[3]=0;
    Txosd_Buffer[7]=0;
    Txosd_Buffer[8]/*iData_C3_temp = Spi_Baseband_ReadWrite(spiRead, AAGC_0_RD, 0x00);*/=0;
    Txosd_Buffer[9]/*iData_C7_temp = Spi_Baseband_ReadWrite(spiRead, AAGC_4_RD, 0x00);*/=0;
    Txosd_Buffer[10]/*iData_C4_temp= Spi_Baseband_ReadWrite(spiRead, AAGC_1_RD, 0x00);*/=0;
    //  Txosd_Buffer[4]/*iData_C5_temp*/= Spi_Baseband_ReadWrite(spiRead, AAGC_2_RD, 0x00);
    //  Txosd_Buffer[5]/*iData_C6_temp*/= Spi_Baseband_ReadWrite(spiRead, AAGC_3_RD, 0x00);
    //  Txosd_Buffer[6]/*iData_Eb_temp*/= Spi_Baseband_ReadWrite(spiRead, FEC_4_RD , 0x00);

    Txosd_Buffer[4]/*iData_C5_temp*/= BB_SPI_ReadByte(PAGE2, AAGC_2_RD);
    Txosd_Buffer[5]/*iData_C6_temp*/= BB_SPI_ReadByte(PAGE2, AAGC_3_RD);
    Txosd_Buffer[6]/*iData_Eb_temp*/= BB_SPI_ReadByte(PAGE2, FEC_4_RD);

    Txosd_Buffer[11]/*iData_E0_temp= Spi_Baseband_ReadWrite(spiRead, FEC_0_RD , 0x00);*/=0;

    Txosd_Buffer[13]/*iData_SNRH_temp = Spi_Baseband_ReadWrite(spiRead, CE_9_RD, 0x00);*/ =0;
    Txosd_Buffer[14]/*iData_SNRL_temp= Spi_Baseband_ReadWrite(spiRead, CE_A_RD, 0x00);*/ =0;

    Txosd_Buffer[15]/*iData_E7_temp = Spi_Baseband_ReadWrite(spiRead, FEC_3_RD_1, 0x00);*/=0;
    Txosd_Buffer[16]/*iData_E8_temp= Spi_Baseband_ReadWrite(spiRead, FEC_3_RD_2, 0x00);*/=0;
    Txosd_Buffer[17]/*iData_EE_temp = Spi_Baseband_ReadWrite(spiRead, FEC_9_RD, 0x00);*/=0;
    Txosd_Buffer[18]/*iData_EF_temp = Spi_Baseband_ReadWrite(spiRead, FEC_10_RD, 0x00);*/=0;
    //  Spi_Baseband_ReadWrite(spiWrite, FSM_0 , iData_page_temp);

    Txosd_Buffer[19]=0;
    SysState.TxcyosdOnly=ENABLE;
}

//*********************TX RX initial(14ms irq)**************

void wimax_vsoc_tx_isr()
{
   //time isr?;
//   uart_print_str(VSOC_UART9_BASE,"BB_TX\n");
}

int rx_cnt = 0;
void wimax_vsoc_rx_isr()
{
    INTR_NVIC_DisableIRQ(BB_RX_ENABLE_VECTOR_NUM);   

    INTR_NVIC_EnableIRQ(TIMER_INTR00_VECTOR_NUM);
    start_timer(init_timer0_0);
    
    if(0 == rx_cnt++ % 100)
    {
        printf("\nRX! %x %x\n", BB_SPI_ReadByte(PAGE2, FEC_4_RD), BB_SPI_ReadByte(PAGE2, AGC3_3));
    }
}

void TIM2_IRQHandler(void)
{
    Reg_Read32(BASE_ADDR_TIMER0 + TMRNEOI_0);
    switch (Timer2_Delay_Cnt)
    {
        case 0:
        {
            Timer2_Delay_Cnt++ ;
        } 
        break;

        case 1:
        {
            Timer2_Delay_Cnt ++ ;
        }
        break;
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
            //Sky_Adjust_AGCGain();
            
            INTR_NVIC_DisableIRQ(TIMER_INTR00_VECTOR_NUM);
            stop_timer(init_timer0_0);
            INTR_NVIC_EnableIRQ(BB_RX_ENABLE_VECTOR_NUM);   
        }
        break;
    }
}

void TIM3_IRQHandler(void)
{
    INTR_NVIC_ClearPendingIRQ(TIMER_INTR01_VECTOR_NUM);
    if(Timer3_Delay2_Cnt < 560)
    {
        Timer3_Delay2_Cnt ++;
    }
    else
    {
        Sky_Hanlde_SpecialIrq();
        Timer3_Delay2_Cnt  = 0;
    }
}

#endif
