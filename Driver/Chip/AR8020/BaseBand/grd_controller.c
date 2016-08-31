#include "grd_controller.h"

#ifdef BASEBAND_GRD

#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "config_baseband_frqdata.h"
#include "config_rcfrq_pattern.h"
#include "config_baseband_register.h"
#include "sys_peripheral_communication.h"
#include "stm32f7xx_hal_tim.h"

/**
      * @brief  Ground Controller Parm Init.
      *
      * @illustrate
      *
         (+) Grd_FlagTypeDef.
         (+) Grd_HandleTypeDef .
         (+) Grd_QAMTypeDef.

      * @{
      *
      * @}
      */

extern struct RC_FRQ_CHANNEL Rc_frq[];
extern struct IT_FRQ_CHANNEL It_frq[];
extern Sys_FlagTypeDef SysState;
extern uint8_t Txosd_Buffer[];
extern TIM_HandleTypeDef  Tim2Handle;
extern TIM_HandleTypeDef  Tim3Handle;
Grd_FlagTypeDef   GrdState;
Grd_HandleTypeDef GrdStruct;
Grd_QAMTypeDef    GrdQam;
uint8_t  Timer2_Delay_Cnt  = 0;
uint8_t  Timer3_Delay1_Cnt = 0;
uint32_t Timer3_Delay2_Cnt = 0;

void Grd_Parm_Initial(void)
{
  GrdStruct.RCChannel=1;
  GrdStruct.ITManualChannel=0;
  GrdStruct.ITAutoChannel=0;
  GrdStruct.Sweepfrqunlock=0;
  GrdStruct.Sweepfrqlock=0;
  GrdStruct.ITTxChannel=0;
  GrdStruct.Harqcnt=0;
  GrdStruct.ReHarqcnt=0;
  GrdStruct.ITTxCnt=0;
  GrdStruct.CgITfrqspan=0;
  GrdStruct.SweepCyccnt=0;
  GrdStruct.ITunlkcnt=0;

  GrdState.ITManualmode = DISABLE;
  GrdState.ITAutomode = ENABLE;
  GrdState.GetITfrq = DISABLE;
  GrdState.RegetITfrq = DISABLE;
  GrdState.Endsweep = DISABLE;
  GrdState.GetfrqOnly= ENABLE;
  GrdState.FEClock = DISABLE;
  GrdState.Allowjglock=DISABLE;

  GrdQam.Down16qam12=DISABLE;
  GrdQam.Downqpsk12=DISABLE;
  GrdQam.Downbpsk12=DISABLE;

  GrdQam.Up64qam12=DISABLE;
  GrdQam.Up16qam12=DISABLE;
  GrdQam.Upqpsk12=DISABLE;

}

void Grd_Id_Initial(void)   //地面端初始化
 {
    uint8_t cPage=0;
    cPage = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);  //ID号写入寄存器
    Spi_Baseband_ReadWrite(spiWrite, FSM_0, cPage|PAGE_1);  // page1
    Spi_Baseband_ReadWrite(spiWrite, TX_3, RC_ID_BIT39_32);
    Spi_Baseband_ReadWrite(spiWrite, TX_4, RC_ID_BIT31_24);
    Spi_Baseband_ReadWrite(spiWrite, TX_5, RC_ID_BIT23_16);
    Spi_Baseband_ReadWrite(spiWrite, TX_6, RC_ID_BIT15_08);
    Spi_Baseband_ReadWrite(spiWrite, TX_7, RC_ID_BIT07_00);
    Spi_Baseband_ReadWrite(spiWrite, FSM_0, cPage);
 }

/**
      * @brief  Ground Remote Controller Section.
      *
      * @illustrate
      *
         (+) Grd_Write_Rcfrq().
         (+) Grd_Id_Initial().
         (+) Grd_RC_Controller().

      * @{
      *
      * @}
      */

// write remote controller frq channnel in baseband_grd.
void Grd_Write_Rcfrq(uint8_t frqchannel)
{
   uint8_t dpage=0;

   dpage = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
   Spi_Baseband_ReadWrite(spiWrite, FSM_0 , dpage & PAGE_2);  // page2
   #if defined( GRD_RF8003_2P3) || defined( GRD_RF8003_2P4)

   Spi_Baseband_ReadWrite(spiWrite, AGC3_b, Rc_frq[frqchannel].frq1);
   Spi_Baseband_ReadWrite(spiWrite, AGC3_a, Rc_frq[frqchannel].frq2);
   Spi_Baseband_ReadWrite(spiWrite, AGC3_9, Rc_frq[frqchannel].frq3);
   Spi_Baseband_ReadWrite(spiWrite, AGC3_8, Rc_frq[frqchannel].frq4);
   Spi_Baseband_ReadWrite(spiWrite, FSM_0 , dpage);

   #else

   Spi_Baseband_ReadWrite(spiWrite, AGC3_a, Rc_frq[frqchannel].frq1 );
   Spi_Baseband_ReadWrite(spiWrite, AGC3_b, Rc_frq[frqchannel].frq2 );
   Spi_Baseband_ReadWrite(spiWrite, AGC3_c, Rc_frq[frqchannel].frq3 );
   Spi_Baseband_ReadWrite(spiWrite, AGC3_d, Rc_frq[frqchannel].frq4 );
   Spi_Baseband_ReadWrite(spiWrite, AGC3_e, Rc_frq[frqchannel].frq5 );
   Spi_Baseband_ReadWrite(spiWrite, FSM_0 , dpage);

   #endif
}


void Grd_RC_Controller(void)
 {

   Grd_Write_Rcfrq(RCFH_PATTERN[GrdStruct.RCChannel]);
   Txosd_Buffer[2] = GrdStruct.RCChannel;
   if( GrdStruct.RCChannel < 9)
   {
     GrdStruct.RCChannel += 1;
   }
   else
   {
     GrdStruct.RCChannel = 0;
   }
}

/**
      * @brief  Ground Image Transmission Section.
      *
      * @illustrate
      *
         (+) Ground write image transmissions frequency in baseband.
            --Grd_Write_Itsweepfrq();
            --Grd_Write_Itworkfrq();
         (+) Grd_Id_Initial().
         (+) Grd_RC_Controller().

      * @{
      *
      * @}
      */

/*!< Specifies the struct of sweeping power with image transmissions.*/
struct SWEEP_POWER SP[IT_FRQ_NUM]={0};
/*!< Specifies the frequency sequency in method of sweeping power.*/
struct FRQ_SEQ_ORDER FSO[IT_FRQ_NUM]={0};
/*!< Specifies the data structure of working at current / alternative / others.*/
struct CURRENT_ALTER_OTHERS_FRQ CAOF[IT_FRQ_NUM]={0};
/*!< Specifies the data structure of SNR 8 times in one cycle.*/
struct SNR_PERCYC SNRPC[SNRNUM_PER_CYC]={0};
/*!< Specifies the data structure of SNR 4*14ms in 4 cycles.
   The last list is average.*/
uint32_t Snr_Frq_Block[FRQ_SNR_BLOCK_ROWS][FRQ_SNR_BLOCK_LISTS]={0};
/*!< Specifies the data structure of SNR 32*14ms in 32 cycles.
   The last list is average.*/
uint32_t Snr_Qam_Block[QAM_SNR_BLOCK_ROWS][QAM_SNR_BLOCK_LISTS]={0};
/*!< Specifies the data structure of ldpc err num in every cycles.*/
uint16_t  Ldpc_Block[LDPC_STATIC_NUM]={0};

const uint8_t ITTX_FRQ[8]={0xE0,0xE1,0xE2,0xE3,0xE4,0xE5,0xE6,0xE7};

// write image transmissions sweeping frq channnel in baseband_grd.

void Grd_Write_Itsweepfrq(uint8_t frqchannel)
{
  uint8_t epage=0;

   epage = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
   Spi_Baseband_ReadWrite(spiWrite, FSM_0 , epage & PAGE_2);  // page2

   #if defined( GRD_RF8003_2P3) || defined( GRD_RF8003_2P4)

   Spi_Baseband_ReadWrite(spiWrite, AGC3_7, It_frq[frqchannel].frq1);
   Spi_Baseband_ReadWrite(spiWrite, AGC3_6, It_frq[frqchannel].frq2);
   Spi_Baseband_ReadWrite(spiWrite, AGC3_5, It_frq[frqchannel].frq3);
   Spi_Baseband_ReadWrite(spiWrite, AGC3_4, It_frq[frqchannel].frq4);
   Spi_Baseband_ReadWrite(spiWrite, FSM_0 , epage);

   #else

   Spi_Baseband_ReadWrite(spiWrite, AGC3_5, It_frq[frqchannel].frq1);
   Spi_Baseband_ReadWrite(spiWrite, AGC3_6, It_frq[frqchannel].frq2);
   Spi_Baseband_ReadWrite(spiWrite, AGC3_7, It_frq[frqchannel].frq3);
   Spi_Baseband_ReadWrite(spiWrite, AGC3_8, It_frq[frqchannel].frq4);
   Spi_Baseband_ReadWrite(spiWrite, AGC3_9, It_frq[frqchannel].frq5);
   Spi_Baseband_ReadWrite(spiWrite, FSM_0 , epage);

   #endif


}

// write image transmissions working frq channnel in baseband_grd.

void Grd_Write_Itworkfrq(uint8_t sweepfrqcnt)
{
   uint8_t fpage=0;

   fpage = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
   Spi_Baseband_ReadWrite(spiWrite, FSM_0 , fpage & PAGE_2);  // page2

   #if defined( GRD_RF8003_2P3) || defined( GRD_RF8003_2P4)

   Spi_Baseband_ReadWrite(spiWrite, AGC3_3, It_frq[sweepfrqcnt].frq1);
   Spi_Baseband_ReadWrite(spiWrite, AGC3_2, It_frq[sweepfrqcnt].frq2);
   Spi_Baseband_ReadWrite(spiWrite, AGC3_1, It_frq[sweepfrqcnt].frq3);
   Spi_Baseband_ReadWrite(spiWrite, AGC3_0, It_frq[sweepfrqcnt].frq4);
   Spi_Baseband_ReadWrite(spiWrite, FSM_0 , fpage);

   #else

   Spi_Baseband_ReadWrite(spiWrite, AGC3_0, It_frq[sweepfrqcnt].frq1);
   Spi_Baseband_ReadWrite(spiWrite, AGC3_1, It_frq[sweepfrqcnt].frq2);
   Spi_Baseband_ReadWrite(spiWrite, AGC3_2, It_frq[sweepfrqcnt].frq3);
   Spi_Baseband_ReadWrite(spiWrite, AGC3_3, It_frq[sweepfrqcnt].frq4);
   Spi_Baseband_ReadWrite(spiWrite, AGC3_4, It_frq[sweepfrqcnt].frq5);
   Spi_Baseband_ReadWrite(spiWrite, FSM_0 , fpage);

   #endif
}

// Read Reg[EB] of baseband , lock or not.
uint8_t Grd_Baseband_Fec_Lock(void)
{
   uint8_t lk_temp;
   uint8_t ipage;
   ipage =  Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
   Spi_Baseband_ReadWrite(spiWrite, FSM_0 , ipage & PAGE_2);
   lk_temp = Spi_Baseband_ReadWrite (spiRead, FEC_6_RD, 0x00);
   Spi_Baseband_ReadWrite(spiWrite,FSM_0 ,ipage);
   if( 0x01 == (lk_temp & 0x01)) return 1;
   else return 0;
 }


void Grd_Sweeping_Energy_Statistic(uint8_t i)
{
  uint8_t   eData = 0;
  uint32_t  Energy_Hgh = 0;
  uint32_t  Energy_Mid = 0;
  uint32_t  Energy_Low = 0;

  eData = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);   //page2
  Spi_Baseband_ReadWrite(spiWrite, FSM_0 , eData & PAGE_2);
  Energy_Low = Spi_Baseband_ReadWrite(spiRead, AAGC_0_RD, 0x00);
  Energy_Mid = Spi_Baseband_ReadWrite(spiRead, AAGC_4_RD, 0x00);
  Energy_Hgh = Spi_Baseband_ReadWrite(spiRead, AAGC_1_RD, 0x00);
  Spi_Baseband_ReadWrite(spiWrite, FSM_0 , eData);

  SP[i].frqchannel= i;
  SP[i].powerall1=SP[i].powerall2;
  SP[i].powerall2=SP[i].powerall3;
  SP[i].powerall3=SP[i].powerall4;
  SP[i].powerall4=((Energy_Hgh<<16)|(Energy_Mid<<8)|Energy_Low);
  SP[i].poweravr=(SP[i].powerall1+SP[i].powerall2+SP[i].powerall3+SP[i].powerall4)>>2;

  if((SP[i].powerall1>=SP[i].powerall2)&&(SP[i].powerall1>=SP[i].powerall3)&&(SP[i].powerall1>=SP[i].powerall4))
  {
     SP[i].powerwave =SP[i].powerall1 - SP[i].poweravr;
  }
  else if((SP[i].powerall2>=SP[i].powerall1)&&(SP[i].powerall2>=SP[i].powerall3)&&(SP[i].powerall2>=SP[i].powerall4))
  {
     SP[i].powerwave =(SP[i].powerall2-SP[i].poweravr);
  }
  else if((SP[i].powerall3>=SP[i].powerall1)&&(SP[i].powerall3>=SP[i].powerall2)&&(SP[i].powerall3>=SP[i].powerall4))
  {
     SP[i].powerwave =(SP[i].powerall3-SP[i].poweravr);
  }
  else if((SP[i].powerall4>=SP[i].powerall1)&&(SP[i].powerall4>=SP[i].powerall2)&&(SP[i].powerall4>=SP[i].powerall3))
  {
     SP[i].powerwave =(SP[i].powerall4-SP[i].poweravr);
  }

   Txosd_Buffer[8]=((SP[i].poweravr&0xFF0000)>>16);

   Txosd_Buffer[9]=((SP[i].poweravr&0xFF00)>>8);

   Txosd_Buffer[10]=SP[i].poweravr;

}

/**
  * @brief  Sort the frq channel from sweeping results.
  * @param  num: the number of image transmissions frq channel.
  *
  */
void Grd_Itfrq_Sort(uint8_t num)   // sort  SP[]-->FSO[]
{
    uint8_t i=0,j=0,k=0;
    uint32_t k_temp=0;
    uint8_t num_temp=0;
    uint8_t iw_temp=0;
    uint8_t m=num;

    for(k=0;k<num;k++)
    {
        FSO[k].frqchannel = SP[k].frqchannel;
        FSO[k].poweravr = SP[k].poweravr;
        FSO[k].powerwave = SP[k].powerwave;
    }
    for( i=1; i<num; i++ )
     {
        m-=1;
        for(j=0;j<m;j++)
         {
           if(FSO[j].poweravr > FSO[j+1].poweravr)
           {
             k_temp = FSO[j].poweravr;
             num_temp = FSO[j].frqchannel;
             iw_temp = FSO[j].powerwave;
             FSO[j].poweravr = FSO[j+1].poweravr;
             FSO[j].frqchannel = FSO[j+1].frqchannel;
             FSO[j].powerwave = FSO[j+1].powerwave;
             FSO[j+1].poweravr = k_temp;
             FSO[j+1].frqchannel = num_temp;
             FSO[j+1].powerwave = iw_temp;
           }
         }
     }
}

void Grd_Alterfrq_Updute(uint8_t Itfrqchannel)
{
    uint8_t m_caf = IT_FRQ_NUM;
    uint8_t i = 0;
    uint8_t j = 0;
    uint32_t k_temp = 0;
    uint8_t num_temp = 0;
    uint8_t iw_temp = 0;

    for( i=1; i<IT_FRQ_NUM; i++ )
     {
        m_caf -= 1;
        for(j=1;j<m_caf;j++)
         {
           if(CAOF[j].poweravr > CAOF[j+1].poweravr)
           {
             k_temp = CAOF[j].poweravr;
             num_temp = CAOF[j].frqchannel;
             iw_temp = CAOF[j].powerwave;
             CAOF[j].poweravr = CAOF[j+1].poweravr;
             CAOF[j].frqchannel = CAOF[j+1].frqchannel;
             CAOF[j].powerwave = CAOF[j+1].powerwave;
             CAOF[j+1].poweravr = k_temp;
             CAOF[j+1].frqchannel = num_temp;
             CAOF[j+1].powerwave = iw_temp;
           }
         }
     }

}

/**
  * @brief  Sweeping as (1,2,3...) before FEC locked in ground.
  * @note   the sweeping sequency is different from that after locked.
  *
  */
void Grd_Sweeping_Before_Fec_Locked(void)
{
  Grd_Write_Itsweepfrq(GrdStruct.Sweepfrqunlock );
  Grd_Sweeping_Energy_Statistic(GrdStruct.Sweepfrqunlock );

  Txosd_Buffer[7]= GrdStruct.Sweepfrqunlock ;

  if(GrdStruct.Sweepfrqunlock < IT_FRQ_NUM-1)
   {
     GrdStruct.Sweepfrqunlock += 1;
   }
  else
   {
     GrdStruct.Sweepfrqunlock = 0;
     if(GrdStruct.SweepCyccnt >= 6)
      {
         GrdStruct.SweepCyccnt = 0;
         GrdState.Endsweep = ENABLE;
      }
     else
      {
         GrdStruct.SweepCyccnt ++;
      }
   }
   Grd_Itfrq_Sort(IT_FRQ_NUM);
}
void Grd_Sweeping_After_Fec_Locked(void)
{
//  uint8_t icaf = 0;
  Grd_Write_Itsweepfrq(CAOF[GrdStruct.Sweepfrqlock].frqchannel);
  Grd_Sweeping_Energy_Statistic(CAOF[GrdStruct.Sweepfrqlock].frqchannel);
  CAOF[GrdStruct.Sweepfrqlock].frqchannel = SP[CAOF[GrdStruct.Sweepfrqlock].frqchannel].frqchannel;
  CAOF[GrdStruct.Sweepfrqlock].poweravr = SP[CAOF[GrdStruct.Sweepfrqlock].frqchannel].poweravr;
  CAOF[GrdStruct.Sweepfrqlock].powerwave = SP[CAOF[GrdStruct.Sweepfrqlock].frqchannel].powerwave;
  Grd_Alterfrq_Updute(IT_FRQ_NUM);

  //CHANNEL_IMAGE_SWEEPING_ptf=GROUNDStruct.Sweepfrqunlock ;
  //Txosd_Buffer[6]= GROUNDStruct.Sweepfrqunlock ;

  if(GrdStruct.Sweepfrqlock < IT_FRQ_NUM-1)
   {
     GrdStruct.Sweepfrqlock += 1;
   }
  else
   {
     GrdStruct.Sweepfrqlock = 0;
   }
}

/**
  * @brief  Get the best working frq channel according to sweeping results.
  * @param  iflag: the diff case of getting image transmissions frq channel.
               (1) get the diff image frq channel to lock.
               (2) get the first image transmission frq channel to FEC lock.
               (3) image transmissions frq channel hopping.
  *
  */
void Grd_Get_Itfrq(uint8_t iflag)
{
  uint8_t icur=0;
  uint8_t icnt=0;
  uint8_t icur2=0;
  uint8_t  num_temp=0;
  uint32_t aver_temp=0;
  uint32_t wa_temp=0;

  switch (iflag)
   {
     case 0 :
        {

          for(icnt=0;icnt<IT_FRQ_NUM;icnt++)
          {
           if(GrdStruct.ITTxChannel != FSO[icnt].frqchannel)
            {
               GrdStruct.ITTxChannel = FSO[icnt].frqchannel;
               GrdState.Allowjglock=DISABLE;
               GrdStruct.ITTxCnt=1;
               for(icur=0;icur<IT_FRQ_NUM;icur++)
               {
                 CAOF[icur].frqchannel = FSO[icur].frqchannel;
                 CAOF[icur].poweravr = FSO[icur].poweravr;
                 CAOF[icur].powerwave = FSO[icur].powerwave;
               }
            }
          }
        } break;

     case 1 :
        {
            GrdStruct.ITTxChannel = FSO[0].frqchannel;
            GrdStruct.ITTxCnt = 1;
            GrdState.Allowjglock=DISABLE;
            for(icur=0;icur<IT_FRQ_NUM;icur++)
             {
                 CAOF[icur].frqchannel=FSO[icur].frqchannel;
                 CAOF[icur].poweravr=FSO[icur].poweravr;
                 CAOF[icur].powerwave=FSO[icur].powerwave;
             }

        } break;
     case 2 :
        {
            GrdStruct.ITTxChannel= CAOF[1].frqchannel;
            GrdStruct.ITTxCnt = 1;
            GrdState.Allowjglock =DISABLE;

              num_temp= CAOF[0].frqchannel;
              aver_temp= CAOF[0].poweravr;
              wa_temp= CAOF[0].powerwave;

              for(icur2=0;icur2<IT_FRQ_NUM-1;icur2++)
               {
                 CAOF[icur2].frqchannel = CAOF[icur2+1].frqchannel;
                 CAOF[icur2].poweravr = CAOF[icur2+1].poweravr;
                 CAOF[icur2].powerwave = CAOF[icur2+1].powerwave;
               }

               CAOF[IT_FRQ_NUM-1].frqchannel = num_temp;
               CAOF[IT_FRQ_NUM-1].poweravr = aver_temp;
               CAOF[IT_FRQ_NUM-1].powerwave = wa_temp;
        } break;
     }
}

void Grd_Fecunlock_Getfrq(void)
{
   GrdStruct.ITunlkcnt ++ ;
   if(GrdStruct.ITunlkcnt >= SPAN_ITUNLK)
    {
      if(ENABLE== GrdState.FEClock)
      {
        Grd_Get_Itfrq(2);
      }
      else
      {
        Grd_Get_Itfrq(0);
      }
   }
}

void Grd_Txmsg_Frq_Change(uint8_t i)
{
  uint8_t ipage=0;
  ipage = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);   //page1
  Spi_Baseband_ReadWrite(spiWrite, FSM_0, ipage|PAGE_1);
  Spi_Baseband_ReadWrite(spiWrite, IT_FREQ_TX, ITTX_FRQ[i]);
  Spi_Baseband_ReadWrite(spiWrite, FSM_0, ipage);
}

/**
  * @brief  Build the struct of SNR array.
  * @note   SNR array :
            (+) get the SNR value per 8 times in one cycle.
            (
+) get the SNR array of frq in 4*14ms.
            (+) get the SNR array of QAM in 32*14ms.
  *
  */

void Grd_Getsnr(uint8_t i)  //get SNR value at present
{
   uint8_t idata_hs;
   SNRPC[i].num=i;
   idata_hs = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);
   SNRPC[i].snrhgh= Spi_Baseband_ReadWrite(spiRead, CE_9_RD, 0x00);
   SNRPC[i].snrlow= Spi_Baseband_ReadWrite(spiRead, CE_A_RD, 0x00);
   Spi_Baseband_ReadWrite(spiWrite, FSM_0, idata_hs);
   SNRPC[i].snrall= (SNRPC[i].snrhgh<<8)|SNRPC[i].snrlow;
}

void Grd_Frqsnr_Array(void)
{
   uint8_t inum=0;
   uint8_t irows=0;
   uint8_t jlists=0;
   uint8_t inum_2=0;

   for(irows=0;irows<FRQ_SNR_BLOCK_ROWS-1;irows++)
   {
    for(jlists=0;jlists<FRQ_SNR_BLOCK_LISTS;jlists++)
    {
      Snr_Frq_Block[irows][jlists]=Snr_Frq_Block[irows+1][jlists];
    }
   }
   for(inum=0;inum<FRQ_SNR_BLOCK_LISTS-1;inum++)
   {
      Snr_Frq_Block[FRQ_SNR_BLOCK_ROWS-1][inum]=SNRPC[inum].snrall;
   }

   for(inum_2=0;inum_2<FRQ_SNR_BLOCK_LISTS-1;inum_2++)
   {
      Snr_Frq_Block[FRQ_SNR_BLOCK_ROWS-1][FRQ_SNR_BLOCK_LISTS-1]+=Snr_Frq_Block[FRQ_SNR_BLOCK_ROWS-1][inum_2];
   }

   Snr_Frq_Block[FRQ_SNR_BLOCK_ROWS-1][FRQ_SNR_BLOCK_LISTS-1]=Snr_Frq_Block[FRQ_SNR_BLOCK_ROWS-1][FRQ_SNR_BLOCK_LISTS-1]>>3;

    Txosd_Buffer[13]=((Snr_Frq_Block[FRQ_SNR_BLOCK_ROWS-1][FRQ_SNR_BLOCK_LISTS-1]&0xFF00)>>8);
    Txosd_Buffer[14]=Snr_Frq_Block[FRQ_SNR_BLOCK_ROWS-1][FRQ_SNR_BLOCK_LISTS-1];
}

void Grd_Qamsnr_Array(void)
{
  uint8_t  inum=0;
  uint8_t  inum2=0;
  uint8_t  irows=0;
  uint8_t  jlists=0;

   for(irows=0;irows<QAM_SNR_BLOCK_ROWS-1;irows++)
   {
    for(jlists=0;jlists<QAM_SNR_BLOCK_LISTS;jlists++)
    {
      Snr_Qam_Block[irows][jlists]=Snr_Qam_Block[irows+1][jlists];
    }
   }
   for(inum=0;inum<QAM_SNR_BLOCK_LISTS-1;inum++)     //the last data
   {
     Snr_Qam_Block[QAM_SNR_BLOCK_ROWS-1][inum]=SNRPC[inum].snrall;
   }
   for(inum2=0;inum2<QAM_SNR_BLOCK_LISTS-1;inum2++)    //Sum
   {

      Snr_Qam_Block[QAM_SNR_BLOCK_ROWS-1][QAM_SNR_BLOCK_LISTS-1] = \
        Snr_Qam_Block[QAM_SNR_BLOCK_ROWS-1][QAM_SNR_BLOCK_LISTS-1]+ \
        Snr_Qam_Block[QAM_SNR_BLOCK_ROWS-1][inum2];
   }
    Snr_Qam_Block[QAM_SNR_BLOCK_ROWS-1][QAM_SNR_BLOCK_LISTS-1]=     \
         Snr_Qam_Block[QAM_SNR_BLOCK_ROWS-1][QAM_SNR_BLOCK_LISTS-1]>>3;
}

/**
  * @brief  Determine SNR array of ground.
  * @note   Snr_Frq_Block:(4x9)
           ____________________________
           |__|__|__|__|__|__|__|__|__|
           |__|__|__|__|__|__|__|__|__|
           |__|__|__|__|__|__|__|__|__|
           |__|__|__|__|__|__|__|__|__|
           Require:  one of the block is smaller than threshold and the lists is all
                     smaller than threshold, the number >=2.
  *
  */
void Grd_Frq_Snrblock_Determine(uint16_t iMCS)
{
   uint8_t jrows=0;
   uint8_t jlist=0;
   uint8_t snr_list_cnt=0;
   uint8_t snr_rows_cnt=0;

   for(jlist=0;jlist<FRQ_SNR_BLOCK_LISTS-1;jlist++)
   {
      if(Snr_Frq_Block[0][jlist] < iMCS)
      {
        for(jrows=0;jrows<FRQ_SNR_BLOCK_ROWS;jrows++)
        {
          if(Snr_Frq_Block[jrows][jlist]<iMCS)
          snr_rows_cnt++;
        }
        if(FRQ_SNR_BLOCK_ROWS == snr_rows_cnt)
        {
          snr_list_cnt++;
        }
      }
   }
   if(snr_list_cnt>=2)
   {
     GrdState.RegetITfrq=ENABLE;
   }
}

/**
  * @brief   Calculation the fluctuate and average of sweeping power.
  * @mothod  The alter frq channel greater than or equal to the working frq 3dB and the
             fluctuate is smallest.
  *
  */
uint8_t Grd_Sweeppower_Fluctuate_Average(void)     //??????????
{
   uint8_t iwa=0;
   uint8_t wave_cnt=0;

   if(CAOF[0].poweravr >= 2*CAOF[1].poweravr)
   {
     for(iwa=0;iwa<8 ;iwa++)
     {
      if( CAOF[1].powerwave <= CAOF[iwa].powerwave)
       wave_cnt++;
     }
   }
   if(wave_cnt >= 7) return 1;
   else return 0;
}
void Grd_Ldpc_Err_Num_Statistics(void)    //2 // 2 sec
{

  uint8_t   cpage=0;
  uint8_t   ldpc_cnt=0;
  uint16_t  ldpc_err_num_rd_low=0;
  uint16_t  ldpc_err_num_rd_hgh=0;

  for(ldpc_cnt=0;ldpc_cnt<LDPC_STATIC_NUM-1;ldpc_cnt++)
  {
    Ldpc_Block[ldpc_cnt]=Ldpc_Block[ldpc_cnt+1];

  }
  cpage= Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);   //page2
  Spi_Baseband_ReadWrite(spiWrite, FSM_0 , cpage& PAGE_2);
  ldpc_err_num_rd_low=Spi_Baseband_ReadWrite(spiRead, FEC_3_RD_2, 0x00);
  ldpc_err_num_rd_hgh=Spi_Baseband_ReadWrite(spiRead, FEC_3_RD_1, 0x00);
  Spi_Baseband_ReadWrite(spiWrite, FSM_0 , cpage);
  Ldpc_Block[LDPC_STATIC_NUM-1]=(ldpc_err_num_rd_hgh<<8)|ldpc_err_num_rd_low;

  GrdStruct.Ldpcreadcnt++;
  if(GrdStruct.Ldpcreadcnt>LDPC_STATIC_NUM)
   {
      GrdState.Ldpcjgflag=ENABLE;
   }
   else
   {
      GrdState.Ldpcjgflag=DISABLE;
   }

}
uint8_t Grd_Ldpc_Block_Determine(void)
{
  uint8_t arr_cnt=0;
  uint8_t ldpc_err_num=0;

  if(ENABLE==GrdState.Ldpcjgflag)
  {
    for(arr_cnt=0;arr_cnt<LDPC_STATIC_NUM-1;arr_cnt++)
    {
      if(Ldpc_Block[arr_cnt]>0x10)  ldpc_err_num++;
    }
    if(ldpc_err_num > 0)
    {
      ldpc_err_num=0;
      return 0;
    }
    else return 1;
  }
  else
  {
   return 0;
  }
}

void Baseband_MsgOSD_Ptf(void)
{
 uint8_t iData_page_temp;
  /* uint8_t iData_C5_temp;
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
//  Txosd_Buffer[7]= 0x00;
  iData_page_temp= Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);   //page2
  Spi_Baseband_ReadWrite(spiWrite, FSM_0 , iData_page_temp& PAGE_2);

  Txosd_Buffer[8]/*iData_C3_temp */= Spi_Baseband_ReadWrite(spiRead, AAGC_0_RD, 0x00);
  Txosd_Buffer[9]/*iData_C7_temp*/ = Spi_Baseband_ReadWrite(spiRead, AAGC_4_RD, 0x00);
  Txosd_Buffer[10]/*iData_C4_temp*/= Spi_Baseband_ReadWrite(spiRead, AAGC_1_RD, 0x00);
  Txosd_Buffer[4]/*iData_C5_temp*/= Spi_Baseband_ReadWrite(spiRead, AAGC_2_RD, 0x00);
  Txosd_Buffer[5]/*iData_C6_temp*/= Spi_Baseband_ReadWrite(spiRead, AAGC_3_RD, 0x00);
  Txosd_Buffer[6]/*iData_Eb_temp*/= Spi_Baseband_ReadWrite(spiRead, FEC_6_RD , 0x00);
  Txosd_Buffer[11]/*iData_E0_temp*/= Spi_Baseband_ReadWrite(spiRead, FEC_0_RD , 0x00);
  Txosd_Buffer[13]/*iData_SNRH_temp*/ = Spi_Baseband_ReadWrite(spiRead, CE_9_RD, 0x00);
  Txosd_Buffer[14]/*iData_SNRL_temp*/ = Spi_Baseband_ReadWrite(spiRead, CE_A_RD, 0x00);
  Txosd_Buffer[12]/*iData_EA_temp*/ = Spi_Baseband_ReadWrite(spiRead, FEC_5_RD, 0x00);
  Txosd_Buffer[15]/*iData_E7_temp*/ = Spi_Baseband_ReadWrite(spiRead, FEC_3_RD_1, 0x00);
  Txosd_Buffer[16]/*iData_E8_temp*/= Spi_Baseband_ReadWrite(spiRead, FEC_3_RD_2, 0x00);
  Txosd_Buffer[17]/*iData_EE_temp*/ = Spi_Baseband_ReadWrite(spiRead, FEC_9_RD, 0x00);
  Txosd_Buffer[18]/*iData_EF_temp*/ = Spi_Baseband_ReadWrite(spiRead, FEC_10_RD, 0x00);
  Spi_Baseband_ReadWrite(spiWrite, FSM_0 , iData_page_temp);

  Txosd_Buffer[19]=0;
  //HAL_UART_Transmit(&Uart1Handle, (u8 *)&Txosd_Buffer, 20, 0xFFFF);
}


void Grd_Itfrq_Hopping(void)
{
   uint8_t iData_harq=0;
   uint8_t iData_qam=0;
   uint8_t QAM_MODE=0;

   GrdStruct.CgITfrqspan++;
   if(GrdStruct.CgITfrqspan>200)
   {
    GrdStruct.CgITfrqspan = 10;
   }

   iData_harq = Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);   //page2  R[EA]重传次数
   Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData_harq&PAGE_2);
   GrdStruct.Harqcnt = Spi_Baseband_ReadWrite(spiRead, FEC_5_RD, 0x00);
   Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData_harq);
   Txosd_Buffer[12]/*iData_EA_temp*/ = GrdStruct.Harqcnt;

   iData_qam=Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);     //page2   R[E0] QAM模式
   Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData_qam&PAGE_2);
   QAM_MODE= Spi_Baseband_ReadWrite(spiRead, FEC_0_RD, 0x00);
   Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData_qam);

   if(((GrdStruct.Harqcnt &0xf0)>>4)>=2)
   {
      GrdStruct.Harqcnt=0;

      switch((QAM_MODE&0x03))
      {
          case 0:  {                                //   BPSK_90
                     if( GrdStruct.CgITfrqspan>=SPAN_ALTFRQ )
                      {
                        Grd_Frq_Snrblock_Determine(BPSK1_2);
                        if(ENABLE == GrdState.RegetITfrq)
                        {
                           if(Grd_Sweeppower_Fluctuate_Average())
                            {
                              Grd_Get_Itfrq(2);
                              GrdStruct.CgITfrqspan=0;
                            }
                        }
                     }
                   }   break;
          case 1:  {                                //   QPSK_B0 (1/2)
                    if( GrdStruct.CgITfrqspan>=SPAN_ALTFRQ )
                       {
                         Grd_Frq_Snrblock_Determine(QPSK1_2);
                        if(ENABLE ==GrdState.RegetITfrq)
                        {
                          GrdState.RegetITfrq = DISABLE;
                           if(Grd_Sweeppower_Fluctuate_Average())
                            {
                              Grd_Get_Itfrq(2);
                              GrdStruct.CgITfrqspan=0;
                            }
                        }
                    }
                   }   break;
          case 2:  {                                //   16QAM (1/2)
                    if( GrdStruct.CgITfrqspan>=SPAN_ALTFRQ )
                       {
                        Grd_Frq_Snrblock_Determine(QAM16_1_2);
                        if(ENABLE ==GrdState.RegetITfrq)
                        {

                           if(Grd_Sweeppower_Fluctuate_Average())
                            {
                               Grd_Get_Itfrq(2);
                               GrdStruct.CgITfrqspan=0;
                            }

                        }
                    }
                   }   break;
          case 3:  {                                //   64QAM (1/2)
                    if( GrdStruct.CgITfrqspan>=SPAN_ALTFRQ )
                       {
                        Grd_Frq_Snrblock_Determine(QAM64_1_2);
                        if(ENABLE ==GrdState.RegetITfrq)
                        {

                           if(Grd_Sweeppower_Fluctuate_Average())
                            {
                               Grd_Get_Itfrq(3);
                               GrdStruct.CgITfrqspan=0;
                            }
                        }
                    }
                   }   break;
     }

   }
}

void Grd_Working_Qam_Change(void)
{
   uint8_t  icnt = 0;
   uint8_t  snr_cnt=0;
   uint32_t snr_aver = 0;
   uint8_t iQAM=0;
	 uint8_t  impage=0;
   impage= Spi_Baseband_ReadWrite(spiRead, FSM_0, 0x00);   //page2
   Spi_Baseband_ReadWrite(spiWrite, FSM_0 , impage& PAGE_2);
   iQAM=Spi_Baseband_ReadWrite(spiRead, FEC_0_RD, 0x00);
   Spi_Baseband_ReadWrite(spiWrite, FSM_0 , impage);
   switch (iQAM & 0x03)
   {
   case 0:
       {

         for(icnt=0;icnt<SNRNUM_PER_CYC ;icnt++)
         {
           if(SNRPC[icnt].snrall> 2*MCS3_0_THRE)       //BPSK  1/2  +3dB
           {
              SNRPC[icnt].snrall = 2* MCS3_0_THRE;
           }
         }
          Grd_Qamsnr_Array();
         for(snr_cnt=0;snr_cnt<QAM_SNR_BLOCK_ROWS;snr_cnt++)
          {
             snr_aver = snr_aver + Snr_Qam_Block[snr_cnt][QAM_SNR_BLOCK_LISTS-1];
          }
         snr_aver = snr_aver>>5;
         if(snr_aver > MCS3_1_THRE)
         {
             Grd_Ldpc_Err_Num_Statistics();
         }
         else
         {
           GrdStruct.Ldpcreadcnt=0;
         }
         if(Grd_Ldpc_Block_Determine())
         {
            GrdQam.Upqpsk12= ENABLE;
            GrdStruct.Ldpcreadcnt=0;
         }
       } break;
    case 1:                       // 4QAM 1/2 +3dB
       {
         for(icnt=0;icnt<SNRNUM_PER_CYC ;icnt++)
         {
           if(SNRPC[icnt].snrall >2* MCS3_1_THRE)
           {
             SNRPC[icnt].snrall = 2* MCS3_1_THRE;
           }
         }
          Grd_Qamsnr_Array();
         for(snr_cnt=0;snr_cnt<QAM_SNR_BLOCK_ROWS;snr_cnt++)
          {
             snr_aver = snr_aver + Snr_Qam_Block[snr_cnt][QAM_SNR_BLOCK_LISTS-1];
          }
         snr_aver=snr_aver>>5;
         if(snr_aver > MCS3_3_THRE)
         {
          Grd_Ldpc_Err_Num_Statistics();
         }
         if (snr_aver < MCS3_1_THRE)
         {
          // GrdQam.Downbpsk12= ENABLE;
           GrdStruct.Ldpcreadcnt=0;
         }
         if(Grd_Ldpc_Block_Determine())
         {
            GrdQam.Up16qam12= ENABLE;
            GrdStruct.Ldpcreadcnt=0;
         }
       } break;
    case 2:                           //16QAM 1/2  +3dB
       {
         for(icnt=0;icnt<SNRNUM_PER_CYC ;icnt++)
         {
           if(SNRPC[icnt].snrall >2* MCS3_3_THRE)
           {
             SNRPC[icnt].snrall =2* MCS3_3_THRE;
           }
         }
          Grd_Qamsnr_Array();
         for(snr_cnt=0;snr_cnt<QAM_SNR_BLOCK_ROWS;snr_cnt++)
          {
             snr_aver = snr_aver + Snr_Qam_Block[snr_cnt][QAM_SNR_BLOCK_LISTS-1];
          }
         snr_aver=snr_aver>>5;
         if(snr_aver > MCS3_4_THRE)
         {
          Grd_Ldpc_Err_Num_Statistics();
         }
         else if(snr_aver < MCS3_3_THRE)
         {
          GrdStruct.Ldpcreadcnt=0;
          if(snr_aver <= MCS3_1_THRE) {}// GrdQam.Downbpsk12= ENABLE;
          else GrdQam.Downqpsk12= ENABLE;

         }
         if(Grd_Ldpc_Block_Determine())
         {
            GrdQam.Up64qam12= ENABLE;
            GrdStruct.Ldpcreadcnt=0;
         }

       } break;
    case 3:                         //64 QAM 1/2  +3dB
       {
         for(icnt=0;icnt<SNRNUM_PER_CYC ;icnt++)
         {
           if(SNRPC[icnt].snrall >2* MCS3_4_THRE)
           {
             SNRPC[icnt].snrall = 2*MCS3_4_THRE;
           }
         }
          Grd_Qamsnr_Array();
         for(snr_cnt=0;snr_cnt<QAM_SNR_BLOCK_ROWS;snr_cnt++)
          {
             snr_aver = snr_aver + Snr_Qam_Block[snr_cnt][8];
          }
         snr_aver = snr_aver >> 5;
         if(snr_aver <= MCS3_4_THRE)
          {
            if(snr_aver <= MCS3_3_THRE)
             {
               if(snr_aver <= MCS3_1_THRE)
               {
            //      GrdQam.Downbpsk12=ENABLE;
               }
               else
               {
                  GrdQam.Downqpsk12=ENABLE;
               }
             }
             else
             {
               GrdQam.Down16qam12=ENABLE;
             }
          }
       } break;
   }
  }

void Grd_Txmsg_Qam_Change(void)
{
   uint8_t iData9=0;

   if( ENABLE==GrdQam.Downbpsk12)  // BPSK_1/2  0x90
    {
       iData9 = Spi_Baseband_ReadWrite(spiRead,FSM_0,0x00);
       Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData9|PAGE_1);
       Spi_Baseband_ReadWrite(spiWrite, QAM_CHANGE, 0xF1);
       Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData9);
    }
   if(ENABLE==GrdQam.Downqpsk12)   //QPSK_1/2   0xB0
    {
       iData9 = Spi_Baseband_ReadWrite(spiRead,FSM_0,0x00);
       Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData9|PAGE_1);
       Spi_Baseband_ReadWrite(spiWrite, QAM_CHANGE, 0xF3);
       Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData9);
    }

   if(ENABLE==GrdQam.Down16qam12)  //16QAM_1/2    0xD0
    {
       iData9 = Spi_Baseband_ReadWrite(spiRead,FSM_0,0x00);
       Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData9|PAGE_1);
       Spi_Baseband_ReadWrite(spiWrite, QAM_CHANGE, 0xF7);
       Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData9);

    }

    //MCS_UP
   if(ENABLE==GrdQam.Upqpsk12) //QPSK_1/2   0xB0
    {
       iData9 = Spi_Baseband_ReadWrite(spiRead,FSM_0,0x00);
       Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData9|PAGE_1);
       Spi_Baseband_ReadWrite(spiWrite, QAM_CHANGE, 0xF3);
       Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData9);

   }

   if(ENABLE==GrdQam.Up16qam12)  //16QAM_1/2    0xD0
    {
       iData9 = Spi_Baseband_ReadWrite(spiRead,FSM_0,0x00);
       Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData9|PAGE_1);
       Spi_Baseband_ReadWrite(spiWrite, QAM_CHANGE, 0xF7);
       Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData9);

    }
   if(ENABLE==GrdQam.Up64qam12)  //64QAM_1/2    0xF0
    {
       iData9 = Spi_Baseband_ReadWrite(spiRead,FSM_0,0x00);
       Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData9|PAGE_1);
       Spi_Baseband_ReadWrite(spiWrite, QAM_CHANGE, 0xF9);
       Spi_Baseband_ReadWrite(spiWrite, FSM_0, iData9);
    }

}
void Grd_Qamflag_Clear(void)
{
  GrdQam.Up16qam12=DISABLE;
  GrdQam.Up64qam12=DISABLE;
  GrdQam.Upqpsk12=DISABLE;
  GrdQam.Down16qam12=DISABLE;
  GrdQam.Downbpsk12=DISABLE;
  GrdQam.Downqpsk12=DISABLE;
}

void Grd_IT_Controller(void)
{

   if(DISABLE==GrdState.FEClock)
    {
      Grd_Sweeping_Before_Fec_Locked();
    }
   else
    {
      Grd_Sweeping_After_Fec_Locked();
    }

   if(ENABLE == GrdState.Endsweep)
    {
      if(ENABLE == GrdState.GetfrqOnly)
      {
          Grd_Get_Itfrq(1);
          GrdState.GetfrqOnly = DISABLE;
      }
     switch(GrdStruct.ITTxCnt)
      {
       case 1:  GrdStruct.ITTxCnt=2;break;
       case 2:  {
                  GrdStruct.ITTxCnt = 0;
                  GrdState.Allowjglock = ENABLE;
                  Grd_Write_Itworkfrq(GrdStruct.ITTxChannel);
                  Txosd_Buffer[3]= GrdStruct.ITTxChannel;
                 };break;
       default: GrdStruct.ITTxCnt = 0;
      }
     Grd_Txmsg_Frq_Change(GrdStruct.ITTxChannel);       //Trans to sky
   }
   if(ENABLE == GrdState.Allowjglock)
   {
     if(Grd_Baseband_Fec_Lock())
     {
       GrdState.FEClock=ENABLE;
       GrdStruct.ITunlkcnt=0;

       if( ENABLE == GrdState.ITManualmode)
       {
          GrdState.ITAutomode = DISABLE;
       }
       else if( ENABLE == GrdState.ITAutomode)
       {
           GrdState.ITManualmode = DISABLE;
           Grd_Itfrq_Hopping();
           Grd_Working_Qam_Change();
           Grd_Txmsg_Qam_Change();
           Grd_Qamflag_Clear();
       }
     }
     else
     {
          Grd_Fecunlock_Getfrq();
     }
   }
}
void Grd_Osdmsg_Ptf(void)
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
  Txosd_Buffer[6]/*iData_EB_temp*/= Spi_Baseband_ReadWrite(spiRead, FEC_6_RD, 0x00);
  Txosd_Buffer[11]/*iData_E0_temp*/= Spi_Baseband_ReadWrite(spiRead, FEC_0_RD, 0x00);
  Txosd_Buffer[4]/*iData_C5_temp*/= Spi_Baseband_ReadWrite(spiRead, AAGC_2_RD, 0x00);
  Txosd_Buffer[5]/*iData_C6_temp*/= Spi_Baseband_ReadWrite(spiRead, AAGC_3_RD, 0x00);
  Txosd_Buffer[15]/*iData_E7_temp*/ = Spi_Baseband_ReadWrite(spiRead, FEC_3_RD_1, 0x00);
  Txosd_Buffer[16]/*iData_E8_temp*/= Spi_Baseband_ReadWrite(spiRead, FEC_3_RD_2, 0x00);
  Txosd_Buffer[17]/*iData_EE_temp */= Spi_Baseband_ReadWrite(spiRead, FEC_9_RD, 0x00);
  Txosd_Buffer[18]/*iData_EF_temp */= Spi_Baseband_ReadWrite(spiRead, FEC_10_RD, 0x00);
  Spi_Baseband_ReadWrite(spiWrite, FSM_0 , iData_page_temp);

  Txosd_Buffer[19]=0;
  SysState.TxcyosdOnly=ENABLE;


}

//*********************TX RX initial(14ms irq)**************

void TIM2_IRQHandler(void)
{
    __HAL_TIM_CLEAR_IT(&Tim2Handle, TIM_IT_UPDATE);
    switch (Timer2_Delay_Cnt)
    {
       case 0:
        {
           Timer2_Delay_Cnt ++ ;

        } break;

       case 1:
        {
          Timer2_Delay_Cnt=0;
           HAL_TIM_Base_Start_IT(&Tim3Handle);//?1.25ms??
           HAL_NVIC_EnableIRQ(TIM3_IRQn);
           HAL_TIM_Base_Stop_IT(&Tim2Handle);//?1ms??
           HAL_NVIC_DisableIRQ(TIM2_IRQn);

        } break;
    }
}


void TIM3_IRQHandler(void)
{
   __HAL_TIM_CLEAR_IT(&Tim3Handle, TIM_IT_UPDATE);
   switch (Timer3_Delay1_Cnt)
   {
     case 0:
      {
        Timer3_Delay1_Cnt++;

      }  break;
     case 1:
      {
         Timer3_Delay1_Cnt++;
         Grd_Getsnr(0);
         Grd_RC_Controller();
         Grd_IT_Controller();

      } break;
     case 2:
      {
         Timer3_Delay1_Cnt++;
         Grd_Getsnr(1);
         Grd_Osdmsg_Ptf();
      } break;
     case 3:
      {
         Timer3_Delay1_Cnt++;
         Grd_Getsnr(2);
         Hanlde_Cy7c_Msg();
      } break;
     case 4:
      {
         Timer3_Delay1_Cnt++;
         Grd_Getsnr(3);

      } break;
     case 5:
      {
         Timer3_Delay1_Cnt++;
         Grd_Getsnr(4);
      } break;
     case 6:
      {
         Timer3_Delay1_Cnt++;
         Grd_Getsnr(5);
      } break;
     case 7:
      {
         Timer3_Delay1_Cnt++;
         Grd_Getsnr(6);

      }  break;
     case 8:
      {
         Timer3_Delay1_Cnt = 0;
         Grd_Getsnr(7);
         Grd_Frqsnr_Array();
         HAL_TIM_Base_Stop_IT(&Tim3Handle);//?1.25ms??
         HAL_NVIC_DisableIRQ(TIM3_IRQn);
      }
    }
}

#endif



